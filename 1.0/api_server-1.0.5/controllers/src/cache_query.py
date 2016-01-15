#!/usr/bin/python

import io
import os
import signal
import sys
from subprocess import call

hit = miss = bypass = expired = 0

#########################################################################################
#########################################################################################
def main():
    try:
        s = '/var/log/nginx/cache.log'
        istream = io.open(s, 'r')
    except IOError:
        print 'ERROR: failed to open %s' % s
        exit(-1)

    try:
        open("/var/run/cache_query.pid","wb").write('%d' % os.getpid())
    except IOError:
        print 'ERROR: failed to open %s' % s
        exit(-1)

    signal.signal(signal.SIGINT, reset_stats)

    global hit, miss, bypass, expired

    istream.seek(0, 2)
    banner()
    print_stat_line()
    while 1:
        for s in istream.readlines():
            l = s.split(' ')
            if l[2] == 'HIT':
                hit += 1
                print_stat_line()
            elif l[2] == 'MISS':
                miss += 1
                print_stat_line()
            elif l[2] == 'BYPASS':
                bypass += 1
                print_stat_line()
            elif l[2] == 'EXPIRED':
                expired += 1
                print_stat_line()
            ### ??? ###
            # expired = 0

            stream = file('/usr/local/bin/api_server/static/data/cache.json', 'w')
            stream.write('{"hit": %d, "miss": %d}' % (hit, miss))
            stream.close()

    exit(0)

#########################################################################################
# reset stats
#########################################################################################
def reset_stats(sig, stack):
    # print "reset_stats fired."
    global hit, miss, bypass, expired
    hit = miss = bypass = expired = 0
    banner()
    print_stat_line()
    return

#########################################################################################
#########################################################################################
def banner():
    call(["clear"])
    print '\n    Cache Statistics\n    ================\n'
    return

#########################################################################################
#########################################################################################
def print_stat_line():
    global hit, miss, bypass, expired
    sys.stdout.write('    HIT: %5d    MISS: %5d\n' % (hit, miss))  #    BYPASS: %5d    EXPIRED: %5d\n'
    #    % (hit, miss, bypass, expired))
    return

#########################################################################################
#########################################################################################
if __name__ == "__main__":
    main()
