#!/usr/bin/env python
#

import os
import yaml

# This import statment MUST come after the lock

BASE_DIR = os.path.join(os.path.dirname(__file__), '..')


def read():
    import config

    config_stream = file('%s/config.yaml' % BASE_DIR, 'r')
    server_conf_stream = file('%s/servers.yaml' % BASE_DIR, 'r')
    try:
        # Config section...
        # IMPORTANT - config.yaml should be ../config.yaml and servers.yaml should be ../servers.yaml on Red Hat!
        configs = yaml.load(config_stream)
        config.debug = configs["debug"]
        config.port = configs["port"]
        config.http_response_codes = {}
        config.http_response_codes = configs["http_response_codes"]
        config.logging = configs["logging"]
        config.log_file = configs["log_file"]
        config.errors_log_file = configs["errors_log_file"]
        config.log_format = configs["log_format"]
        config.log_date_format = configs["log_date_format"]
        config.log_when = configs["log_when"]
        config.log_utc = configs["log_utc"]
        config.log_rotation = configs["log_rotation"]
        config.log_backups = configs["log_backups"]
        config.cookie_expires = {}
        config.cookie_expires = configs["cookie_expires"]
        config.alert_email = {}
        config.alert_email = configs["alert_email"]

        config.api_ip_allow_range = []
        config.api_ip_allow_range = configs["api_ip_allow_range"]

        config.keys = {}
        config.keys = configs["keys"]

        config.msg_code = {}
        config.msg_code = configs["msg_code"]

        #Server section...
        server_conf = yaml.load(server_conf_stream)

        config.server_defaults = {}
        config.server_defaults = server_conf["server_defaults"]
        config.server_default_port = ":" + str(config.server_defaults["port"])  # Must be present in the servers.yaml file.
        if config.server_default_port == ":80" or config.server_default_port == ":":
            config.server_default_port = ""  # Set it to empty since it's not needed for browsers/players.

        config.server_ports = {}
        config.server_ports = server_conf["server_ports"]  # 0MQ port - default to 5555

        config.servers_ip = []
        config.servers_ip = server_conf["servers"]

        config.servers_interface_ip = []
        config.servers_interface_ip = server_conf["servers_interface_ip"]

        config.servers_server_config = {}
        config.servers_server_config = server_conf["server_config"]

        config.servers_purge = {}
        config.servers_purge = server_conf["purge"]

    except yaml.YAMLError, e:
        from log import log
        log("Config Error - %s" % e, "CRITICAL")
    finally:
        config_stream.close()
        server_conf_stream.close()

