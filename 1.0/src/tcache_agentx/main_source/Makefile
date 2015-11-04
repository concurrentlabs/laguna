#CXX=/opt/centos/devtoolset-1.1/root/usr/bin/c++
CXX=g++

NETSNMPCONFIG=net-snmp-config

NETSNMPCFLAGS := $(shell $(NETSNMPCONFIG) --base-cflags)
NETSNMPLIBS := $(shell $(NETSNMPCONFIG) --agent-libs)
LDFLAGS=-lzmq -lczmq -lpthread

STRICT_FLAGS = -Wall
CFLAGS=-I. $(NETSNMPCFLAGS) $(STRICT_FLAGS) -std=gnu++11

all: ../bin/tcache_agentx

.PHONEY: all clean

OBJS = tcache_agentx.o clientTable.o clientRedirectTable.o \
    clientTopDeviceTable.o videoTable.o videoRedirectTable.o \
    domainTable.o domainRedirectTable.o redirectedServiceTable.o \
	tcPlaneStatus.o trafficCount.o redirectCount.o maxTableXmit.o \
	edgeProbeDuration.o tablePurgeDuration.o tcPlaneStartTime.o \
	tcPlaneVersion.o modeTable.o

tcPlaneStatus.o: tcache_agentx.h ../util_source/io_rtns.o

trafficCount.o: tcache_agentx.h ../util_source/io_rtns.o

redirectCount.o: tcache_agentx.h ../util_source/io_rtns.o

clientTable.o: clientTable.h tcache_agentx.h ../util_source/io_rtns.o

clientRedirectTable.o: clientRedirectTable.h tcache_agentx.h ../util_source/io_rtns.o

clientTopDeviceTable.o: clientTopDeviceTable.h tcache_agentx.h ../util_source/io_rtns.o

videoTable.o: videoTable.h tcache_agentx.h

videoRedirectTable.o: videoRedirectTable.h tcache_agentx.h

domainTable.o: domainTable.h tcache_agentx.h

domainRedirectTable.o: domainRedirectTable.h tcache_agentx.h

redirectedServiceTable.o: redirectedServiceTable.h tcache_agentx.h

maxTableXmit.o: maxTableXmit.h tcache_agentx.h

edgeProbeDuration.o: edgeProbeDuration.h tcache_agentx.h

tablePurgeDuration.o: tablePurgeDuration.h tcache_agentx.h

tcPlaneStartTime.o: tcPlaneStartTime.h tcache_agentx.h

tcPlaneVersion.o: tcPlaneVersion.h tcache_agentx.h

modeTable.o: modeTable.h tcache_agentx.h

tcache_agentx.o: clientTable.h clientRedirectTable.h clientTopDeviceTable.h \
    videoTable.h videoRedirectTable.h domainTable.h domainRedirectTable.h redirectedServiceTable.h\
    tcPlaneStatus.h trafficCount.h redirectCount.h tcache_agentx.h ../util_source/tcp_rtns.h \
	maxTableXmit.h edgeProbeDuration.h tablePurgeDuration.h tcPlaneStartTime.h tcPlaneVersion.h modeTable.h

%.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

../bin/tcache_agentx: $(OBJS)
	$(CXX) $(NETSNMPLIBS) $(LDFLAGS) $(OBJS) ../util_source/io_rtns.o ../util_source//tcp_rtns.o -o $@

clean:
	rm -f *.o
