#-------------------------------------------------------------------------
# Compiler settings
#-------------------------------------------------------------------------
-include $(TC_HOME)/tccompiler.mk

#-------------------------------------------------------------------------
# Target Macros
#-------------------------------------------------------------------------
#TARGETDESC      = "Laguna Transparent caching Builder"
#TARGETDIR       = $(TC_HOME)/bin

#-------------------------------------------------------------------------
# Source Directories
#-------------------------------------------------------------------------
TRANSC_SRCDIR    = $(TC_HOME)/src/transc
MISC_SRCDIR      = $(TC_HOME)/src/misc
TCMAIN_SRCDIR    = $(TC_HOME)/src/tcmain

#-------------------------------------------------------------------------
# Library Dependencies
#-------------------------------------------------------------------------
#
# g Library
# using stock that comes with OS distribution
#

#
# libpcap Library
# using stock that comes with OS distribution
#

#
# pcre Library
# using stock that comes with OS distribution
#

#
# libpfring Library
# using packaged rpm
#

#
# yaml Library
# User responsible for compiling from open source.
#

#
# zlog Library
# User responsible for compiling from open source.
#

#
# liblfds Library
# User responsible for compiling from open source.
#

#
# Misc Library
#
misc_dbg:
	@if [ -e "$(MISC_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(MISC_SRCDIR) $(MAKEDEFS) debug; fi

misc_rel:
	@if [ -e "$(MISC_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(MISC_SRCDIR) $(MAKEDEFS) release; fi

misc_clean:
	@if [ -e "$(MISC_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(MISC_SRCDIR) $(MAKEDEFS) clean; fi
	    
#
# Transc Library
#
transc_dbg:
	@if [ -e "$(TRANSC_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TRANSC_SRCDIR) $(MAKEDEFS) debug; fi
	@cd $(TRANSC_SRCDIR)/mib; make -f Makefile.cpp
	@cd $(TRANSC_SRCDIR)/ipblacklist; make -f Makefile.cpp

transc_rel:
	@if [ -e "$(TRANSC_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TRANSC_SRCDIR) $(MAKEDEFS) release; fi
	@cd $(TRANSC_SRCDIR)/mib; make -f Makefile.cpp
	@cd $(TRANSC_SRCDIR)/ipblacklist; make -f Makefile.cpp

transc_clean:
	@if [ -e "$(TRANSC_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TRANSC_SRCDIR) $(MAKEDEFS) clean; fi

#
# Service Main Application
#
transcmain_dbg:
	@if [ -e "$(TCMAIN_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TCMAIN_SRCDIR) $(MAKEDEFS) debug; fi

transcmain_rel:
	@if [ -e "$(TCMAIN_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TCMAIN_SRCDIR) $(MAKEDEFS) release; fi

transcmain_clean:
	@if [ -e "$(TCMAIN_SRCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TCMAIN_SRCDIR) $(MAKEDEFS) clean; fi

#
# General build targets -- application
#
bld_debug: misc_dbg transc_dbg transcmain_dbg

bld_rel: misc_rel transc_rel transcmain_rel

bld_clean: misc_clean transc_clean transcmain_clean

debug: bld_debug
release: bld_rel
clean: bld_clean

default: usage

#-------------------------------------------------------------------------
# Standard Makefile Inclusion
#-------------------------------------------------------------------------
-include $(TC_HOME)/tcgmake.mk

