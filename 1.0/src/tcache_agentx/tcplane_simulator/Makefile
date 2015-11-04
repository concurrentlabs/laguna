CC=/opt/centos/devtoolset-1.1/root/usr/bin/gcc
CC=gcc
# CXX=/opt/centos/devtoolset-1.1/root/usr/bin/g++
CXX=g++

STRICT_FLAGS = -Wall
CXXFLAGS=$(STRICT_FLAGS) -std=gnu++11
# LDFLAGS=-lpthread -ljansson -lzmq -lczmq -lcurl -lstdc++ -lm
LDFLAGS=-lpthread -ljansson -lzmq -lczmq -lcurl

.PHONEY: all clean

all: tcplane_simulator

# dummy_code.o: dummy_code.c dummy_code.h
#	$(CC) -c $(CFLAGS) -o $@ $<

edge_status.o: edge_status.c edge_status.h
	$(CXX) -c $(CFLAGS) -o $@ $<

tcplane_snmp.o: tcplane_snmp.cpp tcplane_snmp.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<

tcplane_simulator.o: tcplane_simulator.c
	$(CXX) -c $(CXXFLAGS) -o $@ $<

tcplane_simulator: tcplane_snmp.o edge_status.o dummy_code.o tcplane_simulator.o
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o tcplane_simulator
