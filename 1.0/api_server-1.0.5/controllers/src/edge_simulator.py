import tornado.ioloop
import tornado.web


class MainHandler(tornado.web.RequestHandler):

    def initialize(self):
        self.SUPPORTED_METHODS = ("GET", "POST", "PURGE")


    def get(self):
        self.write("Hello, world")


    def post(self):
        print ("processing body: %s" % self.request.body)
        self.write("Hello, universe")


    def purge(self):
        print ("processing purge hdr:  %s" % self.request.headers)
        print ("processing purge body: %s" % self.request.body)
        self.set_status(200)
        self.write("Hello, milky way")
        self.finish()


application = tornado.web.Application([
    (r"/", MainHandler),
    (r"/tcspurge/ccur/(.*)$", MainHandler),
    (r"/v1/components/commands/transparentcache/(.*)$", MainHandler)
])


if __name__ == "__main__":
    application.listen(80)
    tornado.ioloop.IOLoop.instance().start()
