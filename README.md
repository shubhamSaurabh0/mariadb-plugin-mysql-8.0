# Audit Plugin for MySQL Server

*Both the primary AWS repo and other repos maintained are not up anymore. I used this code to create a new plugin for MySQL to support it on AWS Amazaon Linux Operating system. If you have a question, you can post it here however i am won't be that active on this. This is posted here as part of my responsibility to give back to the community*

The Audit Plugin for MySQL Server is used by [Amazon RDS for
MySQL](https://aws.amazon.com/rds/mysql/) to enable logging of server activity,
typically for security and compliance purposes.

The plugin source code is published openly on Github as a service to the
community of MySQL and MariaDB users. Amazon Web Services does not offer any
warranties or support for the plugin. The intended audience is software
developers with C++ skills who are able to independently compile and use the
plugin.

This plugin is not official and not associated with
[MySQL](https://www.mysql.com/). MySQL is a registered trademark of
[Oracle](https://www.oracle.com/legal/trademarks.html) and/or its affiliates.

## Based on the MariaDB Audit Plugin

This plugin is based on the [MariaDB Audit
Plugin](https://github.com/MariaDB/server/tree/10.8/plugin/server_audit).

In MySQL 5.6, the plugin API was compatible with the MariaDB Audit Plugin.
In MySQL 5.7 and 8.0, the plugin APIs have however diverged significantly.
So, Amazon RDS for MariaDB created a [modified version that is compatible
with MySQL 5.7 and 8.0](https://aws.amazon.com/about-aws/whats-new/2021/06/amazon-rds-supports-mariadb-audit-plugin-for-mysql-version-8-0/).

This version maintains the functionality of the original audit plugin where
possible, but adapts the plugin to use the MySQL 8.0 plugin API.

To view the MySQL 5.7 compatible version, see the branch `mysql-5.7` in this
repository.

## Compilation

Copy the contents of this repository on top of the sources or MySQL so it
compiles as part of the MySQL build.

## Installation

1. Place the file `server_audit.so` in the plugin directory. You can run `SHOW VARIABLES LIKE 'plugin_dir';` to find the correct path.
2. Activate the plugin by adding `plugin_load_add = server_audit` to `my.cnf` or by running `INSTALL PLUGIN server_audit SONAME ‘server_audit.so’;` once.

## Usage

For details on usage, refer to the MariaDB Server documentation:

* [system variables](https://mariadb.com/kb/en/mariadb-audit-plugin-options-and-system-variables/)
* [status variables](https://mariadb.com/kb/en/mariadb-audit-plugin-status-variables/)
* [logging settings](https://mariadb.com/kb/en/mariadb-audit-plugin-log-settings/)

## Contributing

There are no plans to add any major features to this plugin. If you have modifications to improve the auditing in MySQL/MariaDB, [we recommend contributing to the MariaDB Server project](https://mariadb.org/contribute/), where the whole server and the original audit plugin is developed in a large and active open source community.

This Audit Plugin for MySQL does accept contributions, but the goal of the project is mainly to be stable and bug free. Notes for developers can be found in [DEVELOP.md](DEVELOP.md).
