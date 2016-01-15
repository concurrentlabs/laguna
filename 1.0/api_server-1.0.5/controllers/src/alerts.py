#!/usr/bin/env python
#

import threading
import time
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import config
from log import log
from update_servers import update_servers


class AlertMonitoring(threading.Thread):
    """
    Threaded Server Check. This thread checks availability (up or down) of all of the unique servers in the
    edge list.
    """

    def __init__(self, interval):
        threading.Thread.__init__(self)
        self.interval = interval

    def run(self):
        while True:
            try:
                # NOTE: Add the servers to the list
                # Add other types of logic (i.e., for single server that is bad, etc)
                if len(config.servers_bad) >= config.servers_weighted_count:
                    msg = MIMEText("All of the Request Router processing servers are not responding!")
                    msg['From'] = config.alert_email["from_email"]
                    msg['To'] = config.alert_email["to_email"]
                    msg['Subject'] = config.alert_email["subject"]

                    self.server = smtplib.SMTP(config.alert_email["smtp_server"])
                    # self.server.set_debuglevel(1)  # Only set debuglevel to see full trace stack!
                    self.server.sendmail(config.alert_email["from_email"], config.alert_email["to_email"], msg.as_string())
                    self.server.quit()
                    log("Alert email sent to " + config.alert_email["to_email"] + ". ")
            except BaseException, e:  # try/catch here for clean processing only
                log("Unable to email " + str(e))

            time.sleep(self.interval)