#!/bin/bash

rpm -Uvh ../RPMS/x86_64/3rd_party/rpmforge-release-0.5.3-1.el7.rf.x86_64.rpm
rpm -Uvh ../RPMS/x86_64/3rd_party/epel-release-7-5.noarch.rpm

yum install --assumeyes update

yum install --assumeyes wget.x86_64

yum install --assumeyes net-snmp.x86_64
yum install --assumeyes net-snmp-libs.x86_64
yum install --assumeyes net-snmp-utils.x86_64

yum install --assumeyes bzip2
yum install --assumeyes numactl.x86_64
yum install --assumeyes pkgconfig autoconf

yum install --assumeyes libpcap.x86_64

yum install --assumeyes libyaml.x86_64

yum install --assumeyes apr-util

yum install --assumeyes gtk2.x86_64

# the following are required for python-2.7
yum install --assumeyes python-setuptools
yum install --assumeyes python-pip
yum install --assumeyes supervisor
yum install --assumeyes python-tornado.noarch
yum install --assumeyes python-netaddr.noarch
yum install --assumeyes pysnmp.noarch
yum install --assumeyes python-zmq.x86_64
yum install --assumeyes python-yaml

yum install --assumeyes iptables-services.x86_64

yum install --assumeyes net-tools

yum install --assumeyes boost-regex.x86_64

yum install --assumeyes ntp ntpdate ntp-doc
ntpdate pool.ntp.org
systemctl enable ntpd
systemctl start ntpd

exit 0
