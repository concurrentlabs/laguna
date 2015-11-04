#!/bin/bash
SAMPLE_FREQ=${1}
POLLING_COUNT=${2}
INTERFACE=${3}
COUNTER=0
 
echo Time \ \ \  Gbps
SAMPLE1=`ifconfig ${INTERFACE}|grep "TX bytes"|cut -d':' -f 3|cut -d ' ' -f1 `
while [ $COUNTER -lt $POLLING_COUNT ]
do
  sleep $SAMPLE_FREQ
  SAMPLE2=`ifconfig ${INTERFACE}|grep "TX bytes"|cut -d':' -f 3|cut -d ' ' -f1 `
  echo `date +"%H:%M:%S"` $(echo "scale=5; ($SAMPLE2-$SAMPLE1)/${SAMPLE_FREQ}/125000000"| bc -l)
  SAMPLE1=$SAMPLE2
  let COUNTER=COUNTER+1
done

