# Transparent Caching Server Control Plane V1.0 #

These notes provide instructions on howto download and install the Laguna Transparent Caching Server Control Plane on 64-bit Linux CentOS-7 systems. Ensure at least the *Compatibility libraries* and *Development Tools* group of packages are available on the build server, in addition others may be required--use *yum* to resolve these package dependencies.

See *centos7-scripts/SCRIPTS* directory for scripts to help setup build and deployment systems.


**About TCS Control Plane**
- Supports an Out-of-bound architecture
- Analyzes URL signature pattern and performs interceptions by injecting RST/FIN to origin and 302 redirects to the cache/edge server
- Transmits 302 redirect information to reverse proxy, edge server or Transparent Caching Server(TCS) URL format:
`http://<edge proxy hostname address>/ccur/<site type>/<site target>/tcshost/<video server host address>/tcskey/<cache key id >/tcsopt/<options>/tcsosig/<original URL signature>`

<center>
**TOPOLOGY**
</center>

<center>
<img src="https://github.com/avenishp/test-repo/raw/master/tcs/tcs_control_plane.png" alt="1">
</center>

**Step-by-step guide**

Firstly, create a working sandbox into which download and build the libraries and/or packages specified in steps 1-8 before attempting to build the TCS Control Plane application outlined in step 9 below.
 
**1) [C YAML File Parser Library](http://pyyaml.org/wiki/LibYAML)**

    Download http://pyyaml.org/download/libyaml/yaml-0.1.5.tar.gz
    tar -zxvf yaml-0.1.5.tar.gz
    ./configure
    make
    sudo make install && ldconfig
  
**2) [C Logging Library](https://github.com/HardySimpson/zlog)**

    Download zlog https://github.com/HardySimpson/zlog/archive/latest-stable.tar.gz
    tar -zxvf zlog-latest-stable.tar.gz
    cd zlog-latest-stable/
    make
    sudo make install && ldconfig
  
**3) [C JSON library](http://www.digip.org/jansson)**

    Download jansson http://www.digip.org/jansson/releases/jansson-2.7.tar.gz
    tar –zxvf jansson-2.7.tar.gz
    ./configure
    make
    make check
    sudo make install && ldconfig
  
**4) [ZeroMQ Distrubuted Messaging Library](http://zeromq.org)**

    Dowload zeromq http://download.zeromq.org/zeromq-4.1.3.tar.gz
    Make sure that libsodium, libtool, pkg-config, build-essential, autoconf, and automake are installed.
    Check whether uuid-dev package, uuid/e2fsprogs RPM or equivalent on your system is installed
    ./configure
    make
    make check
    sudo make install && ldconfig
  
**5) [C Binding for ZeroMQ Library](https://github.com/zeromq/czmq/releases)**

    Download czmq https://github.com/zeromq/czmq/archive/v3.0.2.tar.gz
    ./autogen.sh
    ./configure 
    make
    make check 
    sudo make install && ldconfig
  
**6) [PF_RING™ Linux kernel module and user-space packet processing framework](https://github.com/ntop)**

    git clone https://github.com/ntop/PF_RING.git
    cd PF_RING/kernel
    make
    sudo make install
    cd ../userland/lib
    sudo make install
    sudo insmod ./pf_ring.ko
 
**7) [C Lock-free Data Structure Library](http://www.liblfds.org)**

    git clone https://github.com/liblfds/liblfds6.1.1
    cd liblfds6.1.1/liblfds611
    mkdir bin; mkdir obj
    make –f makefile.linux ardbg
    sudo cp inc/liblfds611.h usr/local/include
    sudo cp bin/liblfds611.a /usr/local/lib
  
**8) [C Prototyping Tools Library provides cp_mempool](http://cprops.sourceforge.net)**

    Download http://sourceforge.net/projects/cprops/files/cprops/cprops-0.1.12/libcprops-0.1.12.tar.gz
    tar –zxvf libcprops-0.1.12.tar.gz
    cd libcprops-0.1.12
    ./configure
    make
    sudo make install && ldconfig

**9) [C Transparent Caching Server Control Plane](https://github.com/concurrentlabs/laguna)**

    git clone https://github.com/concurrentlabs/laguna
    Ensure soft link /usr/lib64/libpcap.so -> libpcap.so.1.5.3 exists
    cd 1.0
    make release
    make package
  
**10) [Install TCS Package]()**

    rpm -ivh install/rpm/x86_64/release/RPMS/x86_64/transparent_caching-1.4.1-1.x86_64.rpm


**Notes**

**1) Building RPMs**
- *make PFRING=1 package* or *make package* will an create RPM with pfring the as packet capture option
- RPM location: <home dir>/install/rpm/x86_64/release/RPMS/x86_64/transparent_caching-x.x.x-x.x86_64.rpm

**2) Configuration**
- Modify */etc/sysconfig/transparent_caching/config.yaml* to specify redirect location, monitoring interface and outgoing interface
- (optional) modify */etc/sysconfig/transparent_caching/trlog.conf* to direct "INFO" output results to console or file.

**3) Running**
- service [re]start transc
- Play any youtube, netflix, etc site clip and check output log (/var/log/trservice.log and /var/log/trcomp.log).


**libpcap support**
- Add *PFRING=0* option to *make*, in order to build a libpcap version of control plane. EX: "make PFRING=0 package"

**Misc info**
- Valgrind vs liblfds:
valgrind does not understand some of the instructions set pprovided by liblfds - lockless operations, this issue will be addressed on version liblfds v.7.0.0.
http://www.liblfds.org/phpBB3/viewtopic.php?f=2&t=482
