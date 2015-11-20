.PHONY: all clean

all: remap-hndl.so

CCFLAGS=-I../../lib -I../../proxy/api/ts -I../include \
	-D__STDC_FORMAT_MACROS -std=gnu++11

remap-hndl.o: remap-hndl.cc ../include/ccur_remap.h
	g++ $(CCFLAGS) -o$@ -Wall -Werror -fPIC -c $<
	# g++ $(CCFLAGS) -o$@ -Werror -fPIC -c $<

remap-hndl.so: remap-hndl.o
	g++ -lboost_regex -shared -o $@ $<

install: remap-hndl.so
	cp remap-hndl.so /usr/local/libexec/trafficserver

clean:
	rm -rf *.o *.so *.lo
