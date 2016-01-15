#!/usr/bin/env python
#

import random
from itertools import cycle, islice
import tornado.web
import config
from update_servers import check_bad_servers

"""
    Each algorithm cycles the processing loop based on the number weighted edges - 1 before throwing an error.
"""

def rand(max_count):
    if max_count < 1:
        raise tornado.web.HTTPError(404)
    else:
        retries = max_count - 1
        passed = False
        index = 0

        while retries >= 0:
            if max_count == 1:
                index = 0
            else:
                index = random.randrange(0, max_count)

            # Check to see if index is in the edge_bad list
            passed = check_bad_servers(index)

            retries -= 1

        if passed:
            return index
        else:
            raise tornado.web.HTTPError(404)


def round_robin():
    retries = config.servers_weighted_count - 1
    passed = False
    index = 0

    while retries >= 0:
        try:
            index = int(config.server_list.next())
        except StopIteration:
            config.server_list = rnd_robin(config.server_index)
            index = int(config.server_list.next())

        # Check to see if index is in the edge_bad list
        # NOTE: Commented out below for performance test...
        passed = check_bad_servers(index)

        if passed:
            return index

        retries -= 1

    if passed:  # passed would have already returned before but left here from earlier version - final sanity check.
        return index
    else:
        raise tornado.web.HTTPError(404)


def rnd_robin(*iterables):
    pending = len(iterables)
    nexts = cycle(iter(iterable).next for iterable in iterables)
    while pending:
        try:
            for next in nexts:
                yield next()
        except StopIteration:
            pending -= 1
            nexts = cycle(islice(nexts, pending))


"""
from itertools import chain, izip_longest

def roundrobin(*iterables):
    sentinel = object()
    return (x for x in chain(*izip_longest(fillvalue=sentinel, *iterables)) if x is not sentinel)

"""