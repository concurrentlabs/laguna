.PHONY: all clean

all: cache-hndl-v1.so cache-hndl-v2.so cache-hndl-v3.so

CXX=g++
# CXX=/opt/centos/devtoolset-1.1/root/usr/bin/c++
# CCFLAGS=-I../../lib -I../../proxy/api/ts -I../include -D__STDC_FORMAT_MACROS -std=gnu++11
CCFLAGS=-I/usr/local/include/ts -I../include -D__STDC_FORMAT_MACROS -std=gnu++11

cache-hndl-v1.o: cache-hndl-v1.cc ../include/ccur_remap.h
	$(CXX) $(CCFLAGS) -o$@ -Wall -Werror -fPIC -c $<
	# $(CXX) $(CCFLAGS) -o$@ -Werror -fPIC -c $<

cache-hndl-v2.o: cache-hndl-v2.cc ../include/ccur_remap.h
	$(CXX) $(CCFLAGS) -o$@ -Wall -Werror -fPIC -c $<
	# $(CXX) $(CCFLAGS) -o$@ -Werror -fPIC -c $<

cache-hndl-v3.o: cache-hndl-v3.cc ../include/ccur_remap.h
	$(CXX) $(CCFLAGS) -o$@ -Wall -Werror -fPIC -c $<
	# $(CXX) $(CCFLAGS) -o$@ -Werror -fPIC -c $<

cache-hndl-v1.so: cache-hndl-v1.o
	$(CXX) -lboost_regex -shared -o $@ $<

cache-hndl-v2.so: cache-hndl-v2.o
	$(CXX) -lboost_regex -shared -o $@ $<

cache-hndl-v3.so: cache-hndl-v3.o
	$(CXX) -lboost_regex -shared -o $@ $<

install-v1: cache-hndl-v1.so
	cp cache-hndl-v1.so /usr/local/libexec/trafficserver

install-v2: cache-hndl-v2.so
	cp cache-hndl-v2.so /usr/local/libexec/trafficserver

install-v3: cache-hndl-v3.so
	cp cache-hndl-v3.so /usr/local/libexec/trafficserver

install: cache-hndl-v1.so cache-hndl-v2.so cache-hndl-v3.so
	cp cache-hndl-v1.so /usr/local/libexec/trafficserver
	cp cache-hndl-v2.so /usr/local/libexec/trafficserver
	cp cache-hndl-v3.so /usr/local/libexec/trafficserver

clean:
	rm -rf *.o *.so *.lo
