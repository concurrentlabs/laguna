#!/usr/bin/env python
#

import config
from util_functions import synchronized


@synchronized(config.update_lock)
def update_servers(server_index, add=True):
    try:
        try:
            config.servers_bad.remove(server_index)  # Removes any previous server to keep from collecting the same IP.
        except BaseException, e:
            pass

        if add:
            config.servers_bad.append(server_index)
    except BaseException, e:
        pass  # Simply want a clean exception - no logging required at this time.


@synchronized(config.update_lock)
def check_bad_servers(index):  # Put the checking for bad servers in a small function so that locking and unlocking is fast
    passed = False

    if len(config.servers_bad) > 0:
        bad_pass = True
        for server in config.servers_bad:
            if server == index:
                bad_pass = False
                break  # Jump out of for loop

        if bad_pass:
            passed = True
    else:
        passed = True

    return passed