#!/usr/bin/env python
#

# Web Service - Purge call

import config
import json
import version

from ip import ip_restrict
from log import log
from util_functions import synchronized

import tornado.web
import tornado.httpclient

class CommandHandler(tornado.web.RequestHandler):
    'Web Service API - GET to retrieve json data. POST to purge Media Hawk Edges via json.'


    def initialize(self):
        json_data = {}
        self.SUPPORTED_METHODS = ("POST")


    def post(self, *args, **kwargs):
        self.set_header("Server", "API Server v" + version.version)
        log("Enter CommandHandler.put()")
        header = str(self.request.headers["Content-Type"]).lower()
        log("request header: %s" % header)

        # Force the api ip_restrict to check.
        passed = ip_restrict(self.request.remote_ip, api=True)
        if not passed:
            log('%s\t%s://%s%s\tForbidden\tIP Restricted: %s' % (config.http_response_codes["forbidden"], \
                    self.request.protocol, self.request.host, self.request.uri, self.request.remote_ip), "WARNING")
            log('ip_restrict failed.')
            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])

        try:
            log("processing body: %s" % self.request.body)
            self.json_data = json.loads(self.request.body)
            if not self.json_data.has_key('command'):
                raise CmdError

            if(self.json_data.get('command') == 'purge'):
                self.process_purge_requests()
        except CmdError:
            log('CommandHandler post handler, must provide command value in json data.')
            self.set_status(500)
            self.write('{”success”: ”False", "message" : "no command" }')
            raise tornado.web.HTTPError(config.http_response_codes["general_error"])


    def process_purge_requests(self):
        log('Enter process_purge_requests')

        try:
            if not self.json_data.has_key('hosts'):
                raise HostsError
            if not self.json_data.has_key('targets'):
                raise TargetsError
            hosts = self.json_data['hosts']
            targets = self.json_data['targets']
	    self.write('{ "purgeResults": [');
	    firstTime = True;
            for iter1 in hosts:
                for iter2 in targets:
                    ret = self.xmit_purge(iter1, iter2)
#		    we now continue iterating the hosts/targets and issue the purge requests
#		    to each edge and track the http status for each purge and return in the 
#		    response body
		    if firstTime :
		      self.write('{"edge":"%s", "target":"%s", "code":"%s"}' %(iter1,iter2,ret))
		    else:
		      self.write(', {"edge":"%s", "target":"%s", "code":"%s"}' %(iter1,iter2,ret))
		    firstTime = False;
	    self.write('] }');
            self.set_status(200)
#            self.write('{”success”: ”True", "message" : "processing complete." }')
        except HostsError:
            log("CommandHandler post handler, must provide command value in json data.")
            self.set_status(500)
            self.write('{”success”: ”False", "message" : "no host table in json data" }')
        except TargetsError:
            log("CommandHandler post handler, must provide targets table in json data.")
            self.set_status(500)
            self.write('{”success”: ”False", "message" : "no target table in json data." }')
        except XmitError:
            log("CommandHandler post handler, http response code: %s." % ret)
            self.set_status(ret)
            self.write('{”success”: ”False", "message" : "edge purge failed." }')


    def xmit_purge(self, host, target):
        log('Enter xmit_purge')

        url = 'http://' + host
        body = '/tcspurge/ccur/' + target
        url = url + body
        log('url to edge %s' % url)
        # request = tornado.httpclient.HTTPRequest(url=url, method='PURGE', body=body,
        request = tornado.httpclient.HTTPRequest(url, 'PURGE',
            allow_nonstandard_methods=True)
        client = tornado.httpclient.HTTPClient()
        try:
            response = client.fetch(request)
        except tornado.httpclient.HTTPError as e:
            return e.code
        client.close()
        log('Bottom of ximit_purge')
        return 200


class Error(Exception):
    pass


class CmdError(Error):
    pass


class HostsError(Error):
    pass


class TargetsError(Error):
    pass


class XmitError(Error):
    pass
