#!/bin/bash

systemctl enable iptables

iptables -A INPUT -p tcp -m tcp --dport 80   -j ACCEPT
iptables -A INPUT -p tcp -m tcp --dport 161  -j ACCEPT
iptables -A INPUT -p tcp -m tcp --dport 162  -j ACCEPT
iptables -A INPUT -p tcp -m tcp --dport 705  -j ACCEPT
iptables -A INPUT -p tcp -m tcp --dport 8080 -j ACCEPT
if [ -f /etc/sysconfig/iptables ]
then
    d=`date +%m%d%y%H%m%s`
    mv /etc/sysconfig/iptables /etc/sysconfig/iptables.$d
fi
/sbin/service iptables save

ip6tables -A INPUT -p tcp -m tcp --dport 80   -j ACCEPT
ip6tables -A INPUT -p tcp -m tcp --dport 161  -j ACCEPT
ip6tables -A INPUT -p tcp -m tcp --dport 162  -j ACCEPT
ip6tables -A INPUT -p tcp -m tcp --dport 705  -j ACCEPT
ip6tables -A INPUT -p tcp -m tcp --dport 8080 -j ACCEPT
if [ -f /etc/sysconfig/ip6tables ]
then
    d=`date +%m%d%y%H%m%s`
    mv /etc/sysconfig/ip6tables /etc/sysconfig/ip6tables.$d
fi
/sbin/service ip6tables save

exit 0
