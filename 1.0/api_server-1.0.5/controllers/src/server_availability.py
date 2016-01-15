#!/usr/bin/env python
#

import threading
import Queue
import time
from pysnmp.entity.rfc3413.oneliner import cmdgen
from log import log
from update_servers import update_servers
import config


class ServerAvailability(threading.Thread):
    """
    Threaded Server Check. This thread checks availability (up or down) of all of the unique servers in the
    edge list.
    """

    def __init__(self, timeout, retries, interval):
        threading.Thread.__init__(self)
        self.timeout = timeout
        self.retries = retries
        self.interval = interval
        self.queue = config.server_queue
        # Passing in the servers is passing in a reference which means that any changes are automatically reflected here. To solve this
        # we are creating a local copy of the servers that can be changed by receiving an update from the queue.
        self.servers = []
        for server in config.servers_interface_ip:
            self.servers.append(server)

        # Call the below lines ONCE
        self.cmdGen = cmdgen.CommandGenerator()
        self.community = cmdgen.CommunityData('public', mpModel=0)

        self.server_count = len(self.servers)  # Count of the weighted servers. Used to keep complexity down between weighted vs non-weighted.
        self.target = []

        for server in self.servers:
            self.target.append(cmdgen.UdpTransportTarget((server, 161), timeout=self.timeout, retries=self.retries))

    def run(self):
        while True:
            for index in range(self.server_count):
                errorIndication, errorStatus, errorIndex, varBinds = self.cmdGen.getCmd(
                    self.community,
                    self.target[index],
                    cmdgen.MibVariable(".1.3.6.1.2.1.25.1.1.0")  # System Uptime
                )

                if errorIndication is not None:  # Most likely the Request Timed Out (system down or unreachable)
                    log("Server: {0} Error: {1}".format(self.servers[index], str(errorIndication)), level="CRITICAL", log_type="ERROR")
                    update_servers(index)
                else:
                    update_servers(index, False)  # This will remove the server from the bad list

                # If errorStatus is valid then there is an issue with the PDU on the server AND errorIndex-1 will represent the varBinds index of the error.

            time.sleep(self.interval)

            try:
                queue_data = self.queue.get_nowait()
                if queue_data is not None and queue_data == "update":  # This usually comes from the API section.
                    self.servers[:] = []
                    self.target[:] = []
                    for server in config.servers_interface_ip:
                        self.servers.append(server)
                        self.target.append(cmdgen.UdpTransportTarget((server, 161), timeout=self.timeout, retries=self.retries))
            except Queue.Empty, e:
                pass
