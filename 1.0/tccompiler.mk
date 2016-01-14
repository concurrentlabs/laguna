#-------------------------------------------------------------------------
# Target Architecture
#-------------------------------------------------------------------------
HOST_OS         := linux
TARGET_OS       := linux
#CPU_ARCH	    := x86_64

ifdef TC_HOME
export $TC_HOME
endif

ifndef CPU_ARCH
ifeq ($(findstring 86_64,$(shell uname -m)),86_64)
CPU_ARCH = x86_64
endif
ifeq ($(findstring i686,$(shell uname -m)),i686)
CPU_ARCH = i686
endif
endif

#-------------------------------------------------------------------------
# Make Options
#-------------------------------------------------------------------------
ifdef BUILDINFO
MAKEOPTS        = BUILDINFO=1
CMDVERBOSE      =
else
ifdef VERBOSE
MAKEOPTS        = VERBOSE=1
CMDVERBOSE      =
else
MAKEOPTS        = -s
CMDVERBOSE      = @
endif
endif
MAKEOPTS       += HOST_OS=$(HOST_OS) TARGET_OS=$(TARGET_OS)

#-------------------------------------------------------------------------
# Compiler/Linker Options
#-------------------------------------------------------------------------
CFLAGS_DEFS    += -D_GNU_SOURCE
CFLAGS_INC     += -I.
CFLAGS_OPTS    += -Wall -Werror -Wextra -Wno-unused-parameter -Wno-deprecated-declarations
ifndef CC
CC             := gcc
endif
ifndef CXX
CXX            := g++
endif
ifndef AR
AR             := ar
endif
ARFLAGS        := rcs
SHAREDLIBEXT   := so
#LDFLAGS_DEFS
#LDFLAGS_INC
LD				:= g++
#CFLAGS_INC	+= -isystem /usr/include/glib-2.0 -isystem /usr/lib/glib-2.0/include

#-------------------------------------------------------------------------
# Directory Definitions
#-------------------------------------------------------------------------
ifndef OBJDIR
OBJDIR          = ./obj
endif

ifndef TARGETDIR
ifeq ($(TARGETTYPE),APPLICATION)
TARGETDIR       = $(TC_HOME)/bin
else
TARGETDIR       = $(TC_HOME)/lib
endif
endif
INSTALLDIR       = $(TC_HOME)/install

BLDOUTDIR_DBG   = debug
BLDOUTDIR_REL   = release

ifndef VPATH
VPATH           = $(OBJDIR):$(OBJDIR)/$(BUILDTYPE)
endif

#-------------------------------------------------------------------------
# Multithreading
#-------------------------------------------------------------------------
CFLAGS_OPTS    += -pthread
LDLIBS         += -lpthread -ldl -lrt `pkg-config --libs glib-2.0` `pkg-config --libs gthread-2.0`
MAKEFILE       = Makefile

#-------------------------------------------------------------------------
# C++
#-------------------------------------------------------------------------
# CXXFLAGS       = -std=c++0x
CXXFLAGS       = -std=gnu++11 -Wno-literal-suffix
LDLIBS         += -lstdc++ -lm -lboost_regex

#-------------------------------------------------------------------------
# zeromq/czmq
#-------------------------------------------------------------------------
LDLIBS          += -lzmq -lczmq

#-------------------------------------------------------------------------
# Pkt capture
#-------------------------------------------------------------------------
LDLIBS         += -lpcap -lpcre -lpfring -lnuma

#-------------------------------------------------------------------------
# curl, jasson
#-------------------------------------------------------------------------
LDLIBS         += -ljansson -lcurl

#-------------------------------------------------------------------------
# Compiler/Linker Options
#-------------------------------------------------------------------------
ifdef RELEASE
	CFLAGS_OPTS    += -O3 -fdata-sections -ffunction-sections `pkg-config --cflags glib-2.0`
	LDFLAGS_OPTS   += -Wl,--gc-sections
else
	CFLAGS_OPTS    += -g -Wall -Wformat-security -fstack-protector-all `pkg-config --cflags glib-2.0`
	CFLAGS_DEFS    += -DTRANSC_DEBUG
endif
#Passing Version to transc
CFLAGS_OPTS        += -DTRANSC_VER='"$(TRANSCDIR_VER)"' -DTRANSC_REL='"$(TRANSCDIR_REL)"'
#-------------------------------------------------------------------------
# define compiler and linker flags
#-------------------------------------------------------------------------

CFLAGS        += $(CFLAGS_OPTS) $(CFLAGS_DEFS) $(CFLAGS_INC)
LDFLAGS       += $(LDFLAGS_OPTS) $(LDFLAGS_DEFS) $(LDFLAGS_INC)
