<?php
/**
 * @file
 * JSONlog module helper functions.
 */


/**
 * Establish default site ID.
 *
 * Server's hostname + __ + (default) database name.
 * If default database also has a prefix: + __ + database prefix.
 *
 * @return string
 */
function jsonlog_default_site_id() {
  if (($site_id = getenv('drupal_jsonlog_siteid'))) {
    return $site_id;
  }

  // Server's hostname + database name + database prefix (if any).
  $db =& $GLOBALS['databases']['default']['default'];
  $site_id = drupal_strtolower(preg_replace('/[^\w\d\.\-_]/', '-', gethostname()))
    . '__' . $db['database']
    . (!$db['prefix'] ? '' : ('__' . $db['prefix']));
  unset($db); // Clear ref.

  return $site_id;
}

/**
 * Establish default logging file path.
 *
 * Directory default:
 * PHP:ini error_log - unless that is 'syslog' (then checks the usual suspects /var/log/...) - plus /drupal-jsonlog.
 *
 * Filename default:
 * jsonlog site ID .log.json.
 *
 * @return string
 *   Empty upon failure.
 */
function jsonlog_default_file() {
  if (($file = getenv('drupal_jsonlog_file'))) {
    return $file;
  }

  // Default web server log dir for common *nix distributions.
  $default_log_dirs = array(
    'debian' => '/var/log/apache2',
    'redhat' => '/var/log/httpd',
  );

  $dir = '';
  if (!($server_log = ini_get('error_log')) || $server_log === 'syslog') {
    // Try default web server log dirs for common *nix distributions.
    foreach ($default_log_dirs as $val) {
      if (file_exists($val)) {
        $dir = $val;
        break;
      }
    }
  }
  else {
    $dir = str_replace('\\', '/', dirname($server_log));
  }

  if ($dir) {
    if (!($site_id = variable_get('jsonlog_siteid'))) {
      $site_id = jsonlog_default_site_id();
    }

    return $dir . '/drupal-jsonlog/' . $site_id . '.log.json';
  }
  return '';
}

/**
 * Checks if log dir+file exists and is writable, and logs to watchdog.
 *
 * Usable by drush script; prints messages via drush_log() instead drupal_set_message() if in CLI mode.
 *
 * @see jsonlog_default_file()
 *
 * @param string $file
 *   Default: empty (~ use default file location algo)
 *
 * @return boolean
 */
function jsonlog_test_filing($file = '') {
  $is_drush = drupal_is_cli();

  if ($file) {
    $restore_file_setting = TRUE;
  }
  else {
    $restore_file_setting = FALSE;
    if (!($file = getenv('drupal_jsonlog_file'))) {
      if (!($file = variable_get('jsonlog_file'))) {
        if (!($_file = jsonlog_default_file())) {
          if (!$is_drush) {
            drupal_set_message(t('jsonlog: Failed to establish the server\'s default logging directory.'), 'error');
          }
          else {
            drush_log(dt('jsonlog: Failed to establish the server\'s default logging directory.'), 'error');
          }
          return FALSE;
        }
      }
    }
  }

  $dir = dirname($file);
  if ($dir === '') {
    if (!$is_drush) {
      drupal_set_message(t('jsonlog: Log directory path name is empty.'), 'error');
    }
    else {
      drush_log(dt('jsonlog: Log directory path name is empty.'), 'error');
    }
    return FALSE;
  }
  if (!file_exists($dir)) {
    if (!$is_drush) {
      drupal_set_message(
        t(
          'jsonlog: The log directory doesn\'t exist.!breakPlease create the directory \'!dir\'!breakor change the \'Log file\' setting.',
          array('!dir' => $dir, '!break' => '<br/>')
        ),
        'error'
      );
    }
    else {
      drush_log(
        dt(
          'jsonlog: The log directory doesn\'t exist.!breakPlease create the directory \'!dir\'!breakor change the \'Log file\' setting.',
          array('!dir' => $dir, '!break' => ', ')
        ),
        'error'
      );
    }
    return FALSE;
  }
  if (!file_exists($file)) {
    if (!touch($file)) {
      if (!$is_drush) {
        drupal_set_message(
          t(
            'jsonlog: The log file doesn\'t exist.!breakPlease create the file \'!file\'!breakor change the \'Log file\' setting.',
            array('!file' => $file, '!break' => '<br/>')
          ),
          'error'
        );
      }
      else {
        drush_log(
          dt(
            'jsonlog: The log file doesn\'t exist.!breakPlease create the file \'!file\'!breakor change the \'Log file\' setting.',
            array('!file' => $file, '!break' => ', ')
          ),
          'error'
        );
      }
      return FALSE;
    }
  }
  elseif (!is_writable($file)) {
    if (!$is_drush) {
      drupal_set_message(
        t(
          'jsonlog: The log file is not writable for the webserver user.!breakPlease check permissions of the file \'!file\'!breakand make sure that it\'s parent directory is executable for the webserver user.',
          array('!file' => $file, '!break' => '<br/>')
        ),
        'error'
      );
    }
    else {
      drush_log(
        dt(
          'jsonlog: The log file is not writable for the webserver user.!breakPlease check permissions of the file \'!file\'!breakand make sure that it\'s parent directory is executable for the webserver user.',
          array('!file' => $file, '!break' => ', ')
        ),
        'error'
      );
    }
    return FALSE;
  }

  // Temporarily set the conf var to test value
  // - otherwise jsonlog_watchdog() would still use the old value for this write.
  $original_file = NULL;
  if ($restore_file_setting) {
    $original_file = variable_get('jsonlog_file', NULL);
    variable_set('jsonlog_file', $file);
  }

  try {
    watchdog(
      'jsonlog',
      'Testing watchdog logging - please check if this entry was written to file \'!file\'.',
      array('!file' => $file),
      ($threshold = getenv('drupal_jsonlog_severity_threshold')) ? $threshold : variable_get('jsonlog_severity_threshold', WATCHDOG_WARNING)
    );
  }
  catch (Exception $xc) {
    // Ignore; some watchdog implementation failed, and (probably) threw a (database) PDOException.
  }

  // Restore original value.
  if ($restore_file_setting) {
    if ($original_file === NULL) {
      variable_del('jsonlog_file');
    }
    else {
      variable_set('jsonlog_file', $original_file);
    }
  }

  if (!$is_drush) {
    drupal_set_message(
      t(
        'jsonlog: Logging to \'!file\' seems to work,!breakbut please check if that file now contains an entry whose message starts with \'Testing watchdog logging\'.',
        array('!file' => $file, '!break' => '<br/>')
      ),
      'warning'
    );
  }
  else {
    drush_log(
      dt(
        'jsonlog: Logging to \'!file\' seems to work,!breakbut please check if that file now contains an entry whose message starts with \'Testing watchdog logging\'.',
        array('!file' => $file, '!break' => ' ')
      ),
      'warning'
    );
  }

  return TRUE;
}
