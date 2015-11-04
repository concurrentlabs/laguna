#CXX=/opt/centos/devtoolset-1.1/root/usr/bin/c++
CXX	= g++
LIB     = ..//lib
OBJS    = io_rtns.o tcp_rtns.o

CFLAGS=-O3 -m64 -Wall -Werror -D_REENTRANT -std=gnu++11

# all: $(LIB)/libtcutils.a
all: $(OBJS)

.PHONEY: all clean

$(LIB)/libtcutils.a: $(IO_OBJS)
	ar -r $@ $(IO_OBJS)
	ranlib $@

%.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o
