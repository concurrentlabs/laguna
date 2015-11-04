#!/bin/bash

rpm -Uvh ../RPMS/x86_64/3rd_party/rpmforge-release-0.5.3-1.el7.rf.x86_64.rpm
rpm -Uvh ../RPMS/x86_64/3rd_party/epel-release-7-5.noarch.rpm

yum install --assumeyes update

yum install --assumeyes wget.x86_64

yum install "kernel-devel-uname-r == $(uname -r)"
yum install --assumeyes kernel-headers.x86_64
yum install --assumeyes kernel-tools.x86_64
yum install --assumeyes kernel-tools-libs.x86_64
yum install --assumeyes dkms.noarch

yum install --assumeyes rpm-build
yum install --assumeyes redhat-rpm-config

yum install --assumeyes gcc.x86_64
yum install --assumeyes gcc-c++.x86_64
yum install --assumeyes gcc-gfortran.x86_64
yum install --assumeyes libgcc.x86_64

yum install --assumeyes net-snmp.x86_64
yum install --assumeyes net-snmp-libs.x86_64
yum install --assumeyes net-snmp-utils.x86_64
yum install --assumeyes net-snmp-devel.x86_64

yum install --assumeyes bzip2
yum install --assumeyes numactl.x86_64
yum install --assumeyes pkgconfig autoconf

yum install --assumeyes ncurses-devel
yum install --assumeyes sqlite-devel
yum install --assumeyes readline-devel
yum install --assumeyes tcl-devel tk-devel
yum install --assumeyes openssl-devel
yum install --assumeyes gmp-devel
yum install --assumeyes gdbm-devel
yum install --assumeyes bzip2-devel
yum install --assumeyes expat-devel
yum install --assumeyes tix-devel
yum install --assumeyes libffi-devel
yum install --assumeyes libGL-devel
yum install --assumeyes libX11-devel
yum install --assumeyes numactl-devel.x86_64

yum install --assumeyes libpcap.x86_64
yum install --assumeyes libpcap-devel.x86_64
yum install --assumeyes pcre-devel.x86_64

yum install --assumeyes libyaml.x86_64
yum install --assumeyes libyaml-devel.x86_64

yum install --assumeyes apr-devel
yum install --assumeyes apr-util
yum install --assumeyes apr-util-devel

yum install --assumeyes gtk2.x86_64
yum install --assumeyes gtk2-devel.x86_64

yum install --assumeyes wxGTK-devel.x86_64

yum install --assumeyes subversion-devel

yum install --assumeyes boost-devel.x86_64
yum install --assumeyes boost-regex.x86_64

# the following are required for python-2.7
yum install --assumeyes python-setuptools
yum install --assumeyes python-pip
yum install --assumeyes supervisor
yum install --assumeyes python-tornado.noarch
yum install --assumeyes python-netaddr.noarch
yum install --assumeyes pysnmp.noarch
yum install --assumeyes python-zmq.x86_64
yum install --assumeyes python-yaml

yum install --assumeyes libreport-plugin-mailx.x86_64
yum install --assumeyes mailcap.noarch

yum install --assumeyes iptables-services.x86_64

yum install --assumeyes net-tools

yum install --assumeyes ntp ntpdate ntp-doc
ntpdate pool.ntp.org
systemctl enable ntpd
systemctl start ntpd

exit 0
