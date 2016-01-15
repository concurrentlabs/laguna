#!/usr/bin/env python
#

"""
SNMP_EDGES - Polls the edges listed in the servers.yaml file and pulls back the snmp information
"""


import config
import log

from pysnmp.entity.rfc3413.oneliner import cmdgen

# Call the below lines ONCE
cmdGen = cmdgen.CommandGenerator()
community = cmdgen.CommunityData('public', mpModel=0)

target = []

for server in config.servers:
    target.append(cmdgen.UdpTransportTarget((server, 161)))


def snmp_servers():  # Pass in cmdGen from the thread creation instead of instantiating each time!
    index = 0

    # Determine the correct snmp oids to look for and the better one gets the prize!

    errorIndication, errorStatus, errorIndex, varBinds = cmdGen.getCmd(
        community,
        target[0],
        cmdgen.MibVariable(config.snmps[0])
    )

    # Check for errors and print out results
    if errorIndication:
        log.log(errorIndication, "ERROR")
    else:
        if errorStatus:
            log.log('%s at %s' % (
                errorStatus.prettyPrint(),
                errorIndex and varBinds[int(errorIndex) - 1] or '?'), "ERROR")
        else:
            for name, val in varBinds:
                print('%s = %s' % (name.prettyPrint(), val.prettyPrint()))

    return index  #Hardcode for now


"""
Network Interface Statistics

List NIC names: .1.3.6.1.2.1.2.2.1.2
Get Bytes IN: .1.3.6.1.2.1.2.2.1.10
Get Bytes IN for NIC 4: .1.3.6.1.2.1.2.2.1.10.4
Get Bytes OUT: .1.3.6.1.2.1.2.2.1.16
Get Bytes OUT for NIC 4: .1.3.6.1.2.1.2.2.1.16.4

CPU Statistics

Load
1 minute Load: .1.3.6.1.4.1.2021.10.1.3.1
5 minute Load: .1.3.6.1.4.1.2021.10.1.3.2
15 minute Load: .1.3.6.1.4.1.2021.10.1.3.3

CPU
percentage of user CPU time: .1.3.6.1.4.1.2021.11.9.0
raw user cpu time: .1.3.6.1.4.1.2021.11.50.0
percentages of system CPU time: .1.3.6.1.4.1.2021.11.10.0
raw system cpu time: .1.3.6.1.4.1.2021.11.52.0
percentages of idle CPU time: .1.3.6.1.4.1.2021.11.11.0
raw idle cpu time: .1.3.6.1.4.1.2021.11.53.0
raw nice cpu time: .1.3.6.1.4.1.2021.11.51.0

Memory Statistics

Total Swap Size: .1.3.6.1.4.1.2021.4.3.0
Available Swap Space: .1.3.6.1.4.1.2021.4.4.0
Total RAM in machine: .1.3.6.1.4.1.2021.4.5.0
Total RAM used: .1.3.6.1.4.1.2021.4.6.0
Total RAM Free: .1.3.6.1.4.1.2021.4.11.0
Total RAM Shared: .1.3.6.1.4.1.2021.4.13.0
Total RAM Buffered: .1.3.6.1.4.1.2021.4.14.0
Total Cached Memory: .1.3.6.1.4.1.2021.4.15.0

Disk Statistics

disk / 100000 (or)

includeAllDisks 10% for all partitions and disks

The OIDs are as follows

Path where the disk is mounted: .1.3.6.1.4.1.2021.9.1.2.1
Path of the device for the partition: .1.3.6.1.4.1.2021.9.1.3.1
Total size of the disk/partion (kBytes): .1.3.6.1.4.1.2021.9.1.6.1
Available space on the disk: .1.3.6.1.4.1.2021.9.1.7.1
Used space on the disk: .1.3.6.1.4.1.2021.9.1.8.1
Percentage of space used on disk: .1.3.6.1.4.1.2021.9.1.9.1
Percentage of inodes used on disk: .1.3.6.1.4.1.2021.9.1.10.1

System Uptime: .1.3.6.1.2.1.1.3.0
"""