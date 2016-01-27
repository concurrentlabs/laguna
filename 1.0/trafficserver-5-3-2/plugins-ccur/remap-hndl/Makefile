.PHONY: all clean

all: remap-hndl.so

CXX=g++
# CXX=/opt/centos/devtoolset-1.1/root/usr/bin/c++

CCFLAGS=-I../../lib -I../../proxy/api/ts -I../include \
	-D__STDC_FORMAT_MACROS -std=gnu++11

remap-hndl.o: remap-hndl.cc ../include/ccur_remap.h
	$(CXX) $(CCFLAGS) -o$@ -Wall -Werror -fPIC -c $<
	# $(CXX) $(CCFLAGS) -o$@ -Werror -fPIC -c $<

remap-hndl.so: remap-hndl.o
	$(CXX) -lboost_regex -shared -o $@ $<

install: remap-hndl.so
	cp remap-hndl.so /usr/local/libexec/trafficserver

clean:
	rm -rf *.o *.so *.lo
