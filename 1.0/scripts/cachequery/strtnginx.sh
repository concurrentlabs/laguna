#!/bin/sh

CACHE_LOC=/var/cache/tomnginx

/etc/init.d/./nginx stop
echo "cleaning up nginx cache: $CACHE_LOC"
rm -rf /var/cache/tomnginx/*
/etc/init.d/./nginx start
du -sh $CACHE_LOC
