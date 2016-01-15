#!/usr/bin/env python
#

import tornado.web
import config
import version
from log import log
from routing_algorithms import rand
from routing_algorithms import round_robin
from snmp_servers import snmp_servers
from geo import geo_restrict
from ip import ip_restrict
from token_auth import token_authenticated


class RoutingHandler(tornado.web.RequestHandler):

    """
    POST is not supported since videos never post but only issue GET...
    """

    @classmethod
    def get_content(cls, abspath, start=None, end=None):
        """Retrieve the content of the requested resource which is located
        at the given absolute path.

        This class method may be overridden by subclasses.  Note that its
        signature is different from other overridable class methods
        (no ``settings`` argument); this is deliberate to ensure that
        ``abspath`` is able to stand on its own as a cache key.

        This method should either return a byte string or an iterator
        of byte strings.  The latter is preferred for large files
        as it helps reduce memory fragmentation.

        .. versionadded:: 3.1
        """
        with open(abspath, "rb") as file:
            if start is not None:
                file.seek(start)
            if end is not None:
                remaining = end - (start or 0)
            else:
                remaining = None
            while True:
                chunk_size = 64 * 1024
                if remaining is not None and remaining < chunk_size:
                    chunk_size = remaining
                chunk = file.read(chunk_size)
                if chunk:
                    if remaining is not None:
                        remaining -= len(chunk)
                    yield chunk
                else:
                    if remaining is not None:
                        assert remaining == 0
                    return

    def get(self, urlQuery):
        """
        The urlQuery is NOT the query string portion but the path AFTER the domain.
        If URI query parameters are needed then use a get_argument command.
        """

        # Add redirected edge host to cookie so that we know the original request once subsequent requests from the
        # client sends back keep-alive options, etc...
        # NOTE: Cookie may not set because of cross site and since it may only apply to page and not player.

        # Cookie setting routine was removed since Safari did not set it for the video object and it's the only one
        # that send an Accept-Encoding: Identity which means there are multiple redirects for a single request.

        # IMPORTANT - Update version.py when a new version rolls out...
        self.set_header("Server", "Request Router v" + version.version)

        #file_name = None
        #if str(urlQuery).find('crossdomain.xml'):
        #    file_name = 'crossdomain.xml'
        #elif str(urlQuery).find('clientaccesspolicy.xml'):
        #    file_name = 'clientaccesspolicy.xml'

        #if file_name is not None:
        #    content = self.get_content(os.path.join(os.path.dirname(__file__), '../static/' + file_name))
            #content_length = 0
            #for chunk in content:
            #        content_length += len(chunk)
            #    self.set_header("Content-Length", content_length)

        index = 0  # Set a default value if the items below fail for some reason
        passed = True  # Used for geo restricting if enabled in the config.yaml file
        http_error = False  # Used by the try/catch to force a forbidden error

        if config.geo["restrict"] or config.ip["restrict"]:
            try:
                # Geo Restrictions - if any
                if config.geo["restrict"]:
                    passed = geo_restrict(self.request.remote_ip)

                if not passed:
                    if len(config.geo["redirect_url"]) > 0:
                        #if config.logging:
                        #    log(config.geo["log_prefix"] + config.log_message_formats["requested_prefix"] + self.request.protocol + "://" + self.request.host + self.request.uri + "\t" + config.log_message_formats["routed_prefix"] + config.geo["redirect_url"] + "\tIP: " + self.request.remote_ip, "WARNING")
                        self.redirect(config.geo["redirect_url"])
                    else:
                        if config.logging:
                            redirect = "%s://%s%s" % (self.request.protocol, self.request.host, self.request.uri)
                            log("%s - - \"GET %s\" %s %s \"%s\" \"%s\" \"-\"" % (
                                self.request.remote_ip, self.request.uri, config.http_response_codes["forbidden"], str(len(redirect)), redirect, self.request.headers["User-Agent"]), "WARNING")

                        raise tornado.web.HTTPError(config.http_response_codes["forbidden"])

                # IP restrictions - if any
                if config.ip["restrict"]:
                    passed = ip_restrict(self.request.remote_ip)

                if not passed:
                    if len(config.ip["redirect_url"]) > 0:
                        #if config.logging:
                        #    log(config.ip["log_prefix"] + config.log_message_formats["requested_prefix"] + self.request.protocol + "://" + self.request.host + self.request.uri + "\t" + config.log_message_formats["routed_prefix"] + config.ip["redirect_url"] + "\tIP: " + self.request.remote_ip, "WARNING")
                        self.redirect(config.ip["redirect_url"])
                    else:
                        if config.logging:
                            redirect = "%s://%s%s" % (self.request.protocol, self.request.host, self.request.uri)
                            log("%s - - \"GET %s\" %s %s \"%s\" \"%s\" \"-\"" % (
                                self.request.remote_ip, self.request.uri, config.http_response_codes["forbidden"], str(len(redirect)), redirect, self.request.headers["User-Agent"]), "WARNING")

                        raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The exception will catch this so force it again

            except BaseException, e:
                http_error = True

            if http_error:
                if config.logging:
                    redirect = "%s://%s%s" % (self.request.protocol, self.request.host, self.request.uri)
                    log("%s - - \"GET %s\" %s %s \"%s\" \"%s\" \"-\"" % (
                        self.request.remote_ip, self.request.uri, config.http_response_codes["forbidden"], str(len(redirect)), redirect, self.request.headers["User-Agent"]), "WARNING")
                raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The previous block was caught but this forces the error to be thrown.

        # Check for token authentication first...
        if config.token_auth:
            passed = token_authenticated(self.request.path, self.request.arguments, self.request.remote_ip)

        if passed:
            try:
                #cookie = self.get_cookie("ccur_req")

                # NOTE: Check expires on server to see how it plays out...

                #if cookie is not None:
                #    index = int(cookie)
                #else:

                if config.routing == 0:
                    index = rand(config.servers_weighted_count)
                elif config.routing == 2:
                    index = snmp_servers()  # TODO: Hard coded value of 0 for now. We need to verify more snmp oids before fully implementing
                else:
                    index = round_robin()

                # Cookies are set because flash (.f4m) uses the original host name from the initial request and NOT
                # the redirected host for each (.ism) request. We add cookies so that we can expire the request for
                # future calls by the client's browser/player.

                #if urlQuery.endswith(".f4m"):
                #    seconds = config.cookie_expires["hds"]
                #elif urlQuery.endswith("/Manifest"):
                #    seconds = config.cookie_expires["mss"]
                #else:
                #    seconds = config.cookie_expires["hls"]

                    #expires = datetime.datetime.utcnow() + datetime.timedelta(seconds=seconds)
                    #self.set_cookie("ccur_req", str(index), expires=expires)

                # HLS and HDS make multiple calls with the same original host. Currently, each request/route is logged but
                # we could only log one request: HLS - if Accept-Encoding == 'identity' then don't log
                # HDS - if the cookie is set then don't log.

                #/index.ism/manifest
                #.ism/Manifest

            except BaseException, e:
                if config.debug:
                    log(e, "CRITICAL")

            finally:
                #if config.logging:
                #    log("%s\t%s%s://%s%s\t%s%s://%s/%s\t%s\t%s" % (config.http_response_codes["redirect"], config.log_message_formats["requested_prefix"], self.request.protocol, self.request.host, self.request.uri, config.log_message_formats[
                #        "routed_prefix"], self.request.protocol, config.servers_weighted[index], urlQuery, self.request.remote_ip, self.request.headers["User-Agent"]))
                mss = str(self.request.uri).replace('/index.ism/manifest', '.ism/Manifest')
                mss = str(mss).replace('/index.f4m', '/index.f4m/index.f4m')

                redirect = "%s://%s%s%s" % (self.request.protocol, config.servers_weighted[index], config.server_default_port, mss)
                log("%s - - \"GET %s\" %s %s \"%s\" \"%s\" \"-\"" % (self.request.remote_ip, self.request.uri, config.http_response_codes["redirect"], str(len(redirect)), redirect, self.request.headers["User-Agent"]))
                self.redirect(redirect)
        else:
            if config.logging:
                redirect = "%s://%s%s" % (self.request.protocol, self.request.host, self.request.uri)
                log("%s - - \"GET %s\" %s %s \"%s\" \"%s\" \"-\"" % (self.request.remote_ip, self.request.uri, config.http_response_codes["forbidden"], str(len(redirect)), redirect, self.request.headers["User-Agent"]), "WARNING")
            raise tornado.web.HTTPError(config.http_response_codes["forbidden"])  # The previous block was caught but this forces the error to be thrown.
