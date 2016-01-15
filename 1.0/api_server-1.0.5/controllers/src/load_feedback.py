#!/usr/bin/env python
#

import threading
import datetime
import config
import log
import snmp_servers


class LoadFeedback(threading.Thread):
    def run(self):

        while True:  # May need to change to a variable later so that it can be controlled a little easier...
            now = datetime.datetime.now()

            if config.logging and config.debug:
                log.log("Thread - LoadFeedback\t" )


