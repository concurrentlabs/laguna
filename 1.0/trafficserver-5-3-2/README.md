Using Traffic Control [patch](https://github.com/Comcast/traffic_control/blob/master/traffic_server/patches/trafficserver-5.3.2-089d585.diff) for ATS version 5.3.2 build and install Traffic Server.

1. wget https://github.com/Comcast/traffic_control/raw/master/traffic_server/patches/trafficserver-5.3.2-089d585.diff
2. wget http://archive.apache.org/dist/trafficserver/trafficserver-5.3.2.tar.bz2
3. tar -jxvf ./trafficserver-5.3.2.tar.bz2
4. cd trafficserver-5.3.2
5. patch -p1 < ../trafficserver-5.3.2-089d585.diff
6. autoreconf -vfi
7. ./configure --prefix=/opt/trafficserver --with-user=ats --with-group=ats --with-build-number=1.10112015 --enable-experimental-plugins --with-max-api-stats=4096
8. make
9. make install

