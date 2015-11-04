#!/bin/bash

snmpset -Os -c private -v2c localhost TCACHE-MIB::edgeProbeDuration.0 u 20
exit 0
