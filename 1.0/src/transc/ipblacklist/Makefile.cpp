TC_HOME=../../..
TARGETDIR=$(TC_HOME)/lib
#-------------------------------------------------------------------------
# Compiler settings
#-------------------------------------------------------------------------
-include $(TC_HOME)/tccompiler.mk

TRANSC_SRCDIR       = $(TC_HOME)/src/transc
ZLOG_SRCDIR         = $(TC_HOME)/src/zlog/src
YAML_SRCDIR         = $(TC_HOME)/src/yaml
MISC_SRCDIR         = $(TC_HOME)/src/misc
LIBLFDS_SRCDIR      = $(TC_HOME)/src/liblfds/liblfds611

CFLAGS_INC     += -I$(TRANSC_SRCDIR) -I$(MISC_SRCDIR) \
          -I$(ZLOG_SRCDIR) -I$(LIBLFDS_SRCDIR)/inc \
          -I$(YAML_SRCDIR)/include -I/usr/local/include

.PHONEY: all clean

all: $(TARGETDIR)/blacklist.o

$(TARGETDIR)/blacklist.o: ipblacklist.cpp ipblacklist.h
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o

#-------------------------------------------------------------------------
# Standard Makefile Inclusion
#-------------------------------------------------------------------------
-include $(TC_HOME)/tcgmake.mk
