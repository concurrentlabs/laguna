#!/bin/bash

D1=`date`
DATE=`date -d "$D1 - 2 days" +%Y%m%d`
#DATE=`date +%Y%m%d`

#group simulation log files together before taring
FILEBASE=\/var\/log\/trsim-$DATE.*.log

echo "Removing old simulation log files: "$FILEBASE

#all the trsim log files for today
`rm -rf $FILEBASE`

exit 0