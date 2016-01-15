#!/usr/bin/env python
#
  
# Web Service - Config call

import datetime
import fcntl
import json
import yaml
import shutil
import tornado.web
import config
from util_functions import list_to_json_array
from util_functions import value_to_json
from util_functions import synchronized
from util_functions import json_to_yaml
import version
from log import log
from ip import ip_restrict
import zmq
import collections

class TCSHandler(tornado.web.RequestHandler):
    """
    Web Service API - GET to retrieve json data. POST to update config via json.
    """

    #  curl -H "Content-Type: application/json" http://localhost:8088/v1/components/configurations/transparentcache/config/ > config.json

    def get(self, urlQuery):
        self.set_header("Server", "API Server v" + version.version)

        json_data = "{}"

        # Force the api ip_restrict to check.
        passed = ip_restrict(self.request.remote_ip, api=True)

        if not passed:
            if config.logging:
                log("%s\t%s://%s%s\tForbidden\tIP Restricted: %s" % (config.http_response_codes["forbidden"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip), "WARNING")

            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The exception will catch this so force it again

        self.set_header("Content-Type", "application/json")
        tcs_config_stream = None

        try:
            # temp for now
            tcs_config_stream = file('%s%s' % (config.servers_server_config.get('path'), config.servers_server_config.get('file')), 'r')
            tcs_config = yaml.load(tcs_config_stream)

            if urlQuery == 'services':
                json_data = list_to_json_array(tcs_config['services'], "services")
            elif len(urlQuery) == 0:  # All
                json_data = json.dumps(tcs_config)

                #json_data = value_to_json('version', tcs_config['version'], stop=False)
                #json_data += value_to_json('modeofoperation', tcs_config['modeofoperation'], start=False, stop=False)
                #json_data += value_to_json('monitoringinterface', tcs_config['monitoringinterface'], start=False,stop=False)
                #json_data += value_to_json('outgoinginterface', tcs_config['outgoinginterface'], start=False,stop=False)
                #json_data += value_to_json('redirectaddress', tcs_config['redirectaddress'], start=False,stop=False)
                #json_data += value_to_json('pcap-filter', tcs_config['pcap-filter'], start=False,stop=False)
                #json_data += list_to_json_array(tcs_config['services'], "services", start=False, stop=True)

            #if config.logging:
            #    log("GET %s\t%s://%s%s\t%s\t%s" % (
            #        config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"]))
        except BaseException, e:
            log("GET %s\t%s://%s%s\t%s\t%s\t%s" % (
                config.http_response_codes["general_error"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip, self.request.headers["User-Agent"], e), "CRITICAL")
        finally:
            tcs_config_stream.close()

        self.write(json_data)

    @synchronized(config.update_lock)
    def post(self, *args, **kwargs):
        """
        Add XSRF back in after testing...
        Check the header for the 'application/json' Content-Type
        """
        # curl -X POST -H "Content-Type: application/json" -d @config.json http://localhost:8088/v1/components/configurations/transparentcache/config/
        self.set_header("Server", "API Server v" + version.version)

        header = str(self.request.headers["Content-Type"]).lower()  # If Content-Type is not there or malformed then the server catches before this point.

        # NOTE: Not checking for arguments now since the whole json file is being sent
        #if args is None or args[0] == "" or header != "application/json":
        #if header != 'application/json':
            #if config.logging:
            #    log("%s\t%s%s://%s%s\tForbidden\tHeader not set correctly" % (config.http_response_codes["forbidden"], config.ip["log_prefix"], self.request.protocol, self.request.host, self.request.uri), "WARNING")

        #    raise tornado.web.HTTPError(config.http_response_codes["forbidden"])

        # Force the api ip_restrict to check.
        passed = ip_restrict(self.request.remote_ip, api=True)

        if not passed:
            if config.logging:
                log("%s\t%s://%s%s\tForbidden\tIP Restricted: %s" % (config.http_response_codes["forbidden"], self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip), "WARNING")

            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The exception will catch this so force it again

        self.set_header("Content-Type", "application/json")

        # NOTE: Not currently used since the whole json file is being sent.
        #command = str(args[0]).lower()

        tcs_config_stream = None
        socket = None
        try:
            #yaml_datax = yaml.safe_dump(json.loads(self.request.body))
            #yaml_data = str(yaml_data).replace('!!python/unicode ', '')
            yaml_data = json_to_yaml(json.loads(self.request.body))

            file_name = '%s%s' % (config.servers_server_config.get('path'), config.servers_server_config.get('file'))
            dest_file = config.servers_server_config.get('file') + "_" + str(datetime.datetime.now()).replace("-", "").replace(" ", "").replace(":", "").replace(".", "")

            shutil.copyfile(file_name, '%s%s' % (config.servers_server_config.get('path'), 'yamls/') + dest_file)

            tcs_config_stream = file(file_name, 'w')
            fcntl.flock(tcs_config_stream.fileno(), fcntl.LOCK_EX) #unlocked by close
            # Read in the current yaml and write it out to a new file with the current time appended
            # Take the config settings and build a json data string
            # Write the json data to the yaml.dump method using the write enabled stream for the yaml file

            tcs_config_stream.write(yaml_data)
            tcs_config_stream.close() #unlocks the file
            tcs_config_stream = None
            #log("closing stream","INFO")
            #log("yaml data:%s" % (yaml_datax),"INFO")
            #if config.logging:
            #    log("POST %s\t%s://%s%s\t%s\t%s" % (
            #        config.http_response_codes["found"], self.request.protocol, self.request.host, self.request.uri,
            #        self.request.remote_ip, self.request.headers["User-Agent"]))
            # Fire the zeromq message to let the system know to read the config.yaml again
            context = zmq.Context()
            socket = context.socket(zmq.REQ)
            #socket.setsockopt(zmq.LINGER, 0)
            socket.connect("tcp://localhost:5555")  # The PORT is fixed for this release and on the same server as the TCS.
            socket.send('10')  # Value is fixed
            msg = socket.recv()  # Have to recv since it blocks...
            if msg == "END":
                self.write("{\"success\": true}")
            else:
                self.write("{\"success\": false}")
                #log("socket.recv() err")
        except BaseException, e:
            log("POST %s\t%s://%s%s\t%s\t%s\t%s" % (
                config.http_response_codes["general_error"], self.request.protocol, self.request.host, self.request.uri,
                self.request.remote_ip, self.request.headers["User-Agent"], e), "CRITICAL")
            self.write("{\"success\": false}")
        finally:
            if tcs_config_stream:
                tcs_config_stream.close() #unlocks the file
                #log("closing stream","INFO")
            if socket:
                socket.close()
                #log("close socket","INFO")
