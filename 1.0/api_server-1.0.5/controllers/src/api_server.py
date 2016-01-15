#!/usr/bin/env python

import os
import sys

import tornado.ioloop
import tornado.web
from tornado.options import define, options

import config
import log
from alerts import AlertMonitoring
from ws_config import WS_ConfigHandler
from ws_servers import WS_ServersHandler
from tcs_handler import TCSHandler
from command_handler import CommandHandler

# Called here so that the initial portion of the import is ran ONCE - MUST be called AFTER config
from snmp_servers import snmp_servers

#(r'^/(.*)$', RoutingHandler)

define('port', default=config.port)

app_static_path = os.path.join(os.path.dirname(__file__), '../static')

application = tornado.web.Application([
    (r"/v1/config/(.*)$", WS_ConfigHandler),
    (r"/v1/servers/(.*)$", WS_ServersHandler),
    (r"/v1/components/configurations/transparentcache/config/(.*)$", TCSHandler),
    (r"/v1/components/commands/transparentcache/(.*)$", CommandHandler),
    (r"/(crossdomain\.xml)", tornado.web.StaticFileHandler, {'path': app_static_path}),
    (r"/(clientaccesspolicy\.xml)", tornado.web.StaticFileHandler, {'path': app_static_path}),
    ], debug=config.debug, static_path=app_static_path)  #,  xsrf_cookies=True)

if __name__ == '__main__':
    # Launch threads here...

    try:
        if len(config.alert_email["from_email"]) > 0 and len(config.alert_email["to_email"]) > 0 and len(config.alert_email["smtp_server"]) > 0:
            alert_thread = AlertMonitoring(config.alert_email["interval"])  # Interval is in seconds
            alert_thread.setDaemon(True)
            alert_thread.start()
    except BaseException, e:
        log.log("Alert thread not running. Some value maybe missing. %s" % e, level="WARNING", log_type="ERROR")

    # Look for --port on the command line and if not there then use config.port
    #port = config.port

    #if len(sys.argv) > 1:
    #    for arg in sys.argv:
    #        if str(arg).find("--port") >= 0:
    #            port = int(str(arg).replace("--port=", ""))

    #application.listen(port)
    ##
    log.log("API Server started")
    log.log("API Server started", log_type="ERROR")
    ##
    try:
        tornado.options.parse_command_line()
        application.listen(options.port)
        tornado.ioloop.IOLoop.instance().start()
    except KeyboardInterrupt, e:
        log.log("API Server Exception (stopped) - {0}".format(e.message), level="CRITICAL", log_type="ERROR")

    # snmp thread does not need to join back to the main thread since the app is dieing

    ##
    log.log("API Server stopped")
    log.log("API Server stopped", log_type="ERROR")
    ##
