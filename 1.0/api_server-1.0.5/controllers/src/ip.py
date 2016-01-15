#!/usr/bin/env python
#

from netaddr import IPNetwork, IPAddress
import config


def ip_restrict(ip, api=False):
    passed = False

    # TODO: Put into IPSet and do range search and denys

    if IPAddress(ip).is_loopback():
        return True  # Loopback means it's running locally so all is good.

    if api:
        for net in config.api_ip_allow_range:
            if str(net).find("-") >= 0:
                continue  # TODO: Use another method of check but for now just continue...

            if IPAddress(ip) in IPNetwork(net):
                passed = True
                break

    return passed