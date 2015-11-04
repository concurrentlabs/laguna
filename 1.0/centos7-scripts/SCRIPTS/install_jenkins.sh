#!/bin/bash

wget -O /etc/yum.repos.d/jenkins.repo http://pkg.jenkins-ci.org/redhat/jenkins.repo
rpm --import https://jenkins-ci.org/redhat/jenkins-ci.org.key
yum install --assumeyes jenkins

yum install --assumeyes java-1.7.0-openjdk

yum install --assumeyes httpd

firewall-cmd --zone=public --add-port=8080/tcp --permanent
firewall-cmd --zone=public --add-service=http --permanent
firewall-cmd --reload

chkconfig --level 2345 jenkins on
systemctl restart jenkins.service

exit 0
