#!/usr/bin/env python
#  Multithreaded HTTP Load Generator


import time
import sys
import os
import httplib
import Queue
from threading import Thread



# URL parameters
USE_SSL = False  # HTTPS/SSL support
HOST = 'localhost:8080'
PATH = '/'

# load parameters
THREADS = 1
INTERVAL = 0  # secs
RAMPUP = 0  # secs


def main():
    manager = LoadManager()
    manager.msg = (HOST, PATH)
    manager.start(THREADS, INTERVAL, RAMPUP)


class LoadManager:
    def __init__(self):
        self.msg = ('localhost', '/')
        self.start_time = time.time()
        self.q = Queue.Queue()


    def start(self, threads=1, interval=0, rampup=0):
        try:
            os.remove('results.csv')
        except:
            pass

        # start the thread for reading and writing queued results
        rw = ResultWriter(self.q, self.start_time)
        rw.setDaemon(True)
        rw.start()

        # start the agent threads
        for i in range(threads):
            spacing = (float(rampup) / float(threads))
            if i > 0:
                time.sleep(spacing)
            agent = LoadAgent(self.q, interval, self.msg, self.start_time)
            agent.setDaemon(True)
            print 'starting thread # ' + str(i)
            agent.start()
        while True:
            time.sleep(.25)


class LoadAgent(Thread):
    def __init__(self, q, interval, msg, start_time):
        Thread.__init__(self)
        self.interval = interval
        self.msg = msg
        self.start_time = start_time
        self.q = q

        # choose timer to use
        if sys.platform.startswith('win'):
            self.default_timer = time.clock
        else:
            self.default_timer = time.time


    def run(self):
        while True:
            start = self.default_timer()
            try:
                self.send(self.msg)
            except Exception, e:
                print e
            finish = self.default_timer()
            latency = (finish - start)
            self.q.put((time.time(), latency))
            expire_time = (self.interval - latency)
            if expire_time > 0:
                time.sleep(expire_time)


    def send(self, msg):
        if USE_SSL:
            conn = httplib.HTTPSConnection(msg[0])
        else:
            conn = httplib.HTTPConnection(msg[0])
        try:
            #conn.set_debuglevel(1)
            conn.request('GET', msg[1])
            conn.getresponse().read()
        except Exception, e:
            raise Exception('Connection Error: %s' % e)
        finally:
            conn.close()


class ResultWriter(Thread):
    def __init__(self, q, start_time):
        Thread.__init__(self)
        self.q = q
        self.start_time = start_time


    def run(self):
        f = open('results.csv', 'a')
        while True:
            try:
                q_tuple = self.q.get(False)
                trans_end_time, latency = q_tuple
                elapsed = (trans_end_time - self.start_time)
                f.write('%.3f,%.3f\n' % (elapsed, latency))
                f.flush()
                print '%.3f' % latency
            except Queue.Empty:
                # re-check queue for messages every x sec
                time.sleep(.25)


if __name__ == '__main__':
    main()

