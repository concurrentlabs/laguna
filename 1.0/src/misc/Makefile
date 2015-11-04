#-------------------------------------------------------------------------
# Compiler settings
#-------------------------------------------------------------------------
-include $(TC_HOME)/tccompiler.mk

#-------------------------------------------------------------------------
# Target Macros
#-------------------------------------------------------------------------
TARGETNAME      = libmisc
TARGETDESC      = "Laguna misc opensource helper library"
TARGETSRC       = http_parser.c msem.c mthread.c evlog.c\
				tcpktparse.c lkfq.c tcutil.c tcregex.c
TARGETDIR       = $(TC_HOME)/lib
TARGETTYPE      = STATICLIB

#-------------------------------------------------------------------------
# Directories
#-------------------------------------------------------------------------
MISC_SRCDIR     	= $(TC_HOME)/src/misc
ZLOG_SRCDIR     	= $(TC_HOME)/src/zlog/src
LIBLFDS_SRCDIR     	= $(TC_HOME)/src/liblfds/liblfds611

#-------------------------------------------------------------------------
# Library Dependencies
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# Compiler Options
#-------------------------------------------------------------------------
CFLAGS_INC     += -I$(MISC_SRCDIR) -I$(LIBLFDS_SRCDIR)/inc -I$(LIBLFDS_SRCDIR)/src -I$(ZLOG_SRCDIR)

#-------------------------------------------------------------------------
# Standard Makefile Inclusion
#-------------------------------------------------------------------------
-include $(TC_HOME)/tcgmake.mk


