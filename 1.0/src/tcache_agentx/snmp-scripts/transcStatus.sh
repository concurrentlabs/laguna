#!/bin/bash

snmpget -Os -c private -v2c localhost 1.3.6.1.4.1.1457.4.1.1.2.0
exit 0
