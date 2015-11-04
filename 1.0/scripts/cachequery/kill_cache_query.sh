#!/bin/bash

pid=`cat /var/run/cache_query.pid`
kill $pid
exit 0
