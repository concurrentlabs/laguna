#!/bin/bash

useradd tcsbuild

useradd -d /var/www         -s /sbin/nologin apache
useradd -d /var/lib/jenkins -s /sbin/nologin jenkins

usermod -a -G wheel tcsbuild
usermod -a -G wheel apache
usermod -a -G wheel jenkins

exit 0
