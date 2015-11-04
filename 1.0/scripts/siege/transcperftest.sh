#!/bin/bash
siege -f urls.txt -c50 -r500000 -b -i &
sleep 3
siege -f urls.txt -c50 -r500000 -b -i &
sleep 5
siege -f urls.txt -c50 -r500000 -b -i &
sleep 3
siege -f urls.txt -c50 -r500000 -b -i &
sleep 5
siege -f urls.txt -c50 -r500000 -b -i &
sleep 3
siege -f urls.txt -c50 -r500000 -b -i &
