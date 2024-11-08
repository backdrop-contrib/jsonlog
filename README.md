JSONLog
======================

Logs watchdog events JSON-formatted

to log files.
Provides a Logstash/ElasticSearch-ready log source.

Installation
------------

- Install this module using the official Backdrop CMS instructions at

Settings overridable by server environment vars to secure simple and safe centralized configuration of multiple sites/hosts.
Just SetEnv drupal_[Drupal conf var] '[value]'.

    jsonlog_severity_threshold: defaults to warning
    jsonlog_truncate: defaults to 64 (Kb), logger uses file locking
    jsonlog_site_id: defaults to server's hostname + database name + database prefix (if any))
    jsonlog_canonical: name; for site identification across multiple instances
    jsonlog_dir: defaults to PHP:ini error_log (unless that is 'syslog', then checks the usual suspects /var/log/...) + /drupal-jsonlog
    jsonlog_file_time: none | Ymd (default) | YW | Ym
    jsonlog_newline_prepend: bool|null (version >=8.x-1.3 only)
    jsonlog_tags: comma-separated list; server env var + Drupal conf var


Issues
------

Bugs and feature requests should be reported in [the Issue Queue](https://github.com/backdrop-contrib/jsonlog/issues).

Current Maintainers
-------------------

- [Eli Lisseck](https://github.com/elisseck).
- [Anthony Nemirovsky](https://github.com/anemirovsky).

Credits
-------

- Backdrop development supported by [Giant Rabbit](https://giantrabbit.com).
- Ported to Backdrop by [Alejandro Madrigal](https://github.com/AleMadLei/)
- Originally written for Drupal and co-maintained by [jacobfriis](https://www.drupal.org/u/jacobfriis), [lorenzs](https://www.drupal.org/u/lorenzs)

License
-------

This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
