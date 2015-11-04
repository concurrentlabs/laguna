#!/bin/bash

snmpset -Os -c private -v2c localhost TCACHE-MIB::tablePurgeDuration.0 u 900
exit 0
