#!/usr/bin/env python
#

# Web Service - Config call

import json
import tornado.web
import config
from util_functions import list_to_json_array
from util_functions import value_to_json
from util_functions import synchronized
from util_functions import convert
from util_functions import yaml_write
import version
from log import log
from ip import ip_restrict
from config_functions import read
from version import version

class WS_ConfigHandler(tornado.web.RequestHandler):
    """
    Web Service API - GET to retrieve json data. POST to update config via json.
    """

    def get(self, urlQuery):
        self.set_header("Server", "API Server v" + version)

        json_data = "{}"

        # Force the api ip_restrict to check.
        passed = ip_restrict(self.request.remote_ip, api=True)

        if not passed:
            #if config.logging:
            #    log("%s\t%s%s%s://%s%s\tForbidden\tIP Restricted: %s" % (config.http_response_codes["forbidden"], config.ip["log_prefix"], config.log_message_formats["requested_prefix"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip), "WARNING")

            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The exception will catch this so force it again

        self.set_header("Content-Type", "application/json")

        try:
            if urlQuery == "port":
                json_data = value_to_json("port", config.port)
            elif urlQuery == "log_when":
                json_data = value_to_json("log_when", config.log_when)
            elif urlQuery == "log_utc":
                json_data = value_to_json("log_utc", config.log_utc)
            elif urlQuery == "log_rotation":
                json_data = value_to_json("log_rotation", config.log_rotation)
            elif urlQuery == "log_backups":
                json_data = value_to_json("log_backups", config.log_backups)
            elif urlQuery == "http_response_codes":
                json_data = list_to_json_array(config.http_response_codes, "http_response_codes")
            elif urlQuery == "alert_email":
                json_data = list_to_json_array(config.alert_email, "alert_email")
            elif urlQuery == "keys":
                json_data = list_to_json_array(config.keys, "keys")
            elif urlQuery == "version":
                json_data = value_to_json("version", version)
            elif len(urlQuery) == 0:  # All
                json_data = value_to_json("port", config.port, stop=False)
                json_data += value_to_json("log_when", config.log_when, start=False, stop=False)
                json_data += value_to_json("log_utc", config.log_utc, start=False, stop=False)
                json_data += value_to_json("log_rotation", config.log_rotation, start=False, stop=False)
                json_data += value_to_json("log_backups", config.log_backups, start=False, stop=False)
                json_data += list_to_json_array(config.http_response_codes, "http_response_codes", start=False, stop=False)
                json_data += list_to_json_array(config.alert_email, "alert_email", start=False, stop=False)
                json_data += list_to_json_array(config.api_ip_allow_range, "api_ip_allow_range", start=False, stop=False)
                json_data += list_to_json_array(config.keys, "keys", start=False, stop=False)
                json_data += value_to_json("version", version, start=False)

            if config.logging:
                log("GET %s\t%s://%s%s\t%s\t%s" % (
                    config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"]))
        except BaseException, e:
            log("GET %s\t%s://%s%s\t%s\t%s\t%s" % (
                config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], e), "CRITICAL")

        self.write(json_data)

    @synchronized(config.update_lock)
    def post(self, *args, **kwargs):
        """
        Add XSRF back in after testing...
        Check the header for the 'application/json' Content-Type
        """
        self.set_header("Server", "API Server v" + version)

        dict_data = "{}"

        header = str(self.request.headers["Content-Type"]).lower()  # If Content-Type is not there or malformed then the server catches before this point.

        if args is None or args[0] == "" or header != "application/json":
            #if config.logging:
            #    log("%s\t%s://%s%s\tForbidden\tHeader not set correctly" % (config.http_response_codes["forbidden"], self.request.protocol, self.request.host, self.request.uri), "WARNING")

            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])

        # Force the api ip_restrict to check.
        passed = ip_restrict(self.request.remote_ip, api=True)

        if not passed:
            #if config.logging:
            #    log("%s\t%s%s%s://%s%s\tForbidden\tIP Restricted: %s" % (config.http_response_codes["forbidden"], config.ip["log_prefix"], config.log_message_formats["requested_prefix"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip), "WARNING")

            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The exception will catch this so force it again

        self.set_header("Content-Type", "application/json")

        command = str(args[0]).lower()

        # Break down the json into a dictionary. Some will be a simple name/value pair and others an array and others a list.
        try:
            dict_data = convert(json.loads(self.request.body))
            if command == "log_rotation":
                if len(dict_data[command]) > 0:
                    config.log_rotation = dict_data[command]
            elif command == "log_backups":
                if len(dict_data[command]) > 0:
                    config.log_backups = dict_data[command]
            elif command == "log_when":
                if len(dict_data[command]) > 0:
                    config.log_when = dict_data[command]
            elif command == "alert_email":
                config.alert_email.clear()
                config.alert_email = dict_data[command]
            elif command == "keys":
                if len(dict_data[command]) > 0:
                    config.keys.clear()
                    config.keys = dict_data[command]
            elif command == "read":
                read()
            elif command == "write":  # Writes out the yaml file
                yaml_write("config", "config.yaml")

            if config.logging:
                log("POST %s\t%s://%s%s\t%s\t%s\t%s" % (
                    config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], dict_data))

            self.write("{\"%s\": %s}" % ("success", True))
        except BaseException, e:
            log("POST %s\t%s://%s%s\t%s\t%s\t%s\t%s" % (
                config.http_response_codes["general_error"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], e, dict_data),
                "CRITICAL")
            self.write("{\"%s\": %s}" % ("success", False))