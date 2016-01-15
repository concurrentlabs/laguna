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


class WS_ServersHandler(tornado.web.RequestHandler):
    """
    Web Service API - GET to retrieve json data. POST to update config via json.
    """

    def get(self, urlQuery):
        self.set_header("Server", "API Server v" + version.version)

        json_data = "{}"

        # Force the api ip_restrict to check.
        passed = ip_restrict(self.request.remote_ip, api=True)

        if not passed:
            #if config.logging:
            #    log("%s\t%s%s%s://%s%s\tForbidden\tIP Restricted: %s" % (config.http_response_codes["forbidden"], config.ip["log_prefix"], config.log_message_formats["requested_prefix"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip), "WARNING")

            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The exception will catch this so force it again

        self.set_header("Content-Type", "application/json")

        try:
            if urlQuery == "servers":
                json_data = list_to_json_array(config.servers_with_weights, "servers")
            elif urlQuery == "server_defaults":
                json_data = list_to_json_array(config.server_defaults, "server_defaults")
            elif urlQuery == "server_ports":
                json_data = list_to_json_array(config.server_ports, "server_ports")
            elif urlQuery == "servers_interface_ip":
                json_data = list_to_json_array(config.servers_interface_ip, "servers_interface_ip")
            elif urlQuery == "server_config":
                json_data = list_to_json_array(config.servers_server_config, "server_config")
            elif urlQuery == "purge":
                json_data = list_to_json_array(config.servers_purge, "purge")
            elif len(urlQuery) == 0:  # All
                json_data = list_to_json_array(config.servers_with_weights, "servers", start=False, stop=False)
                json_data += list_to_json_array(config.server_defaults, "server_defaults", start=False, stop=False)
                json_data += list_to_json_array(config.server_ports, "server_ports", start=False, stop=False)
                json_data += list_to_json_array(config.servers_interface_ip, "servers_interface_ip", start=False)
                json_data += list_to_json_array(config.servers_server_config, "server_config", start=False, stop=False)
                json_data += list_to_json_array(config.servers_purge, "purge", start=False)

            if config.logging:
                log("GET %s\t%s://%s%s\t%s\t%s" % (
                    config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"]))
        except BaseException, e:
            log("GET %s\t%s://%s%s\t%s\t%s\t%s" % (
                config.http_response_codes["general_error"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], e), "CRITICAL")

        self.write(json_data)

    @synchronized(config.update_lock)
    def post(self, *args, **kwargs):
        """
        Add XSRF back in after testing...
        Check the header for the 'application/json' Content-Type
        """
        self.set_header("Server", "API Server v" + version.version)

        json_data = "{}"
        update_queue = False

        header = str(self.request.headers["Content-Type"]).lower()  # If Content-Type is not there or malformed then the server catches before this point.

        if args is None or args[0] == "" or header != "application/json":
            #if config.logging:
            #    log("%s\t%s%s%s://%s%s\tForbidden\tHeader not set correctly" % (config.http_response_codes["forbidden"], config.ip["log_prefix"], config.log_message_formats[
            #        "requested_prefix"], self.request.protocol, self.request.host, self.request.uri), "WARNING")

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
        dict_data = "{}"
        try:
            dict_data = convert(json.loads(self.request.body))
            if command == "servers_interface_ip":
                if len(dict_data[command]) > 0:
                    update_queue = True
                    config.servers_interface_ip[:] = []  # Clears a list
                    for data in dict_data[command]:
                        config.servers_interface_ip.append(data)
            elif command == "server_defaults":
                if len(dict_data[command]) > 0:
                    update_queue = True
                    config.server_defaults.clear()
                    config.server_defaults = dict_data[command]
                    config.server_default_port = ":" + str(config.server_defaults["port"])  # Must be present in the servers.yaml file.
                    if config.server_default_port == ":80" or config.server_default_port == ":":
                        config.server_default_port = ""  # Set it to empty since it's not needed for browsers/players.
            elif command == "server_ports":
                if len(dict_data[command]) > 0:
                    update_queue = True
                    config.server_ports.clear()
                    config.server_ports = dict_data[command]
            elif command == "server_config":
                if len(dict_data[command]) > 0:
                    update_queue = True
                    config.servers_server_config.clear()
                    config.servers_server_config = dict_data[command]
            elif command == "purge":
                if len(dict_data[command]) > 0:
                    update_queue = True
                    config.servers_purge.clear()
                    config.servers_purge = dict_data[command]
            elif command == "servers" or command == "servers/add" or command == "servers/delete":
                if len(dict_data["servers"]) > 0:
                    update_queue = True
                    add_servers = False
                    if command == "servers/add":
                        add_servers = True

                    config.server_count = len(config.servers)
                    config.servers_weighted_count = len(config.servers_weighted)
            elif command == "write":  # Writes out the yaml file
                yaml_write("servers", "servers.yaml")

            if update_queue:
                config.server_queue.put_nowait("update")

            if config.logging:
                log("POST %s\t%s://%s%s\t%s\t%s\t%s" % (
                    config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], dict_data))

            self.write("{\"%s\": %s}" % ("success", True))
        except BaseException, e:
            log("POST %s\t%s://%s%s\t%s\t%s\t%s\t%s" % (
                config.http_response_codes["general_error"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], e, dict_data), "CRITICAL")
            self.write("{\"%s\": %s}" % ("success", False))
