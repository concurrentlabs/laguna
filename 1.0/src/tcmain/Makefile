#-------------------------------------------------------------------------
# Compiler settings
#-------------------------------------------------------------------------
-include $(TC_HOME)/tccompiler.mk

#-------------------------------------------------------------------------
# Target Macros
#-------------------------------------------------------------------------
TARGETNAME      = transc
TARGETDESC      = "Laguna Transparent caching Application"
TARGETSRC       = transc.c
TARGETLOGCONF   = trlog.conf
TARGETMAINCONF  = config.yaml
TARGETDIR       = $(TC_HOME)/bin
TARGETTYPE      = APPLICATION

#-------------------------------------------------------------------------
# Directories
#-------------------------------------------------------------------------
TRANSC_CONFDIR      = etc/sysconfig/transparent_caching
TRANSC_MAINDIR      = $(TC_HOME)/src/tcmain
TRANSC_SRCDIR     	= $(TC_HOME)/src/transc
ZLOG_SRCDIR     	= $(TC_HOME)/src/zlog/src
YAML_SRCDIR     	= $(TC_HOME)/src/yaml
MISC_SRCDIR     	= $(TC_HOME)/src/misc
LIBLFDS_SRCDIR  	= $(TC_HOME)/src/liblfds/liblfds611

#-------------------------------------------------------------------------
# Library Dependencies
#-------------------------------------------------------------------------
LDLIBLINK  += $(TC_HOME)/lib/tcplane_snmp.o $(TC_HOME)/lib/blacklist.o
LDLIBLINK  += $(TC_HOME)/lib/libtransc.a
LDLIBLINK  += $(TC_HOME)/lib/libmisc.a 
LDLIBLINK  += /usr/local/lib/libzlog.a
LDLIBLINK  += /usr/local/lib/libyaml.a
LDLIBLINK  += /usr/local/lib/liblfds611.a
LDLIBLINK  += /usr/local/lib/libcprops.a

#-------------------------------------------------------------------------
# Compiler Options
#-------------------------------------------------------------------------
CFLAGS_INC     += -I$(TRANSC_SRCDIR) -I$(MISC_SRCDIR) \
                  -I$(ZLOG_SRCDIR) -I$(LIBLFDS_SRCDIR)/inc \
			      -I$(YAML_SRCDIR)/include

			      
ifeq ($(TRANSC_BUILDCFG_LIBPFRING),0)
CFLAGS_DEFS    += -DTRANSC_BUILDCFG_LIBPCAP
endif

#-------------------------------------------------------------------------
# Standard Makefile Inclusion
#-------------------------------------------------------------------------
-include $(TC_HOME)/tcgmake.mk
