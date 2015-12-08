#-------------------------------------------------------------------------
# Compiler settings
#-------------------------------------------------------------------------
-include $(TC_HOME)/tccompiler.mk

#-------------------------------------------------------------------------
# Target Macros
#-------------------------------------------------------------------------
TARGETNAME      = libtransc
TARGETDESC      = "Laguna Transparent Caching library"
TARGETSRC       = tcinit.c tcshared.c tcload.c tcloadsys.c\
                  tcutils.c tcintf.c
TARGETDIR       = $(TC_HOME)/lib
TARGETTYPE      = STATICLIB

#-------------------------------------------------------------------------
# Directories
#-------------------------------------------------------------------------
TRANSC_SRCDIR     	= $(TC_HOME)/src/transc
ZLOG_SRCDIR     	= $(TC_HOME)/src/zlog/src
YAML_SRCDIR     	= $(TC_HOME)/src/yaml
MISC_SRCDIR     	= $(TC_HOME)/src/misc
LIBLFDS_SRCDIR  	= $(TC_HOME)/src/liblfds/liblfds611

#-------------------------------------------------------------------------
# Http Processing
#-------------------------------------------------------------------------
%.o : httpprc/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : httpprc/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += tchttpparse.c tctcpstrm.c tchttpproc.c tchttpinit.c
                  
#-------------------------------------------------------------------------
# Pkt Injection Thread
#-------------------------------------------------------------------------
%.o : pktgen/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : pktgen/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += tcpktgen.c tcpktiotx.c tcpfrpsend.c tcplpsend.c \
				tcpktgeninit.c tcoutintf.c tcpktinj.c

#-------------------------------------------------------------------------
# Packet Processing Thread
#-------------------------------------------------------------------------
%.o : pktprc/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : pktprc/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += tcplpcap.c tcpfrpcap.c tcpktprc.c tcpktiorx.c tcpktprcinit.c tcmonintf.c
                  
#-------------------------------------------------------------------------
# Back ground Thread
#-------------------------------------------------------------------------
%.o : bkgrnd/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : bkgrnd/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += tcbkgrnd.c

#-------------------------------------------------------------------------
# Mib Thread
#-------------------------------------------------------------------------
%.o : mib/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : mib/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += tcplanesnmp.c

#-------------------------------------------------------------------------
# sim prc Thread
#-------------------------------------------------------------------------
%.o : simprc/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : simprc/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += tcsim.c tcsimsend.c tcsimutil.c

#-------------------------------------------------------------------------
# health Thread
#-------------------------------------------------------------------------
%.o : health/%.c
ifndef VERBOSE
ifneq ($(QUIETHEADER),TRUE)
	@echo "$<"
endif
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) $(TARGET_ARCH_FLAGS) \
	    -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : health/%.c
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $(INCPATHS) $< > $(OBJDIR)/$(@F)

TARGETSRC	   += cdn_status.c tchealth.c

#-------------------------------------------------------------------------
# Library Dependencies
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# Compiler Options
#-------------------------------------------------------------------------
CFLAGS_INC     += -I$(TRANSC_SRCDIR) -I$(MISC_SRCDIR) \
          -I$(ZLOG_SRCDIR) -I$(LIBLFDS_SRCDIR)/inc \
	      -I$(YAML_SRCDIR)/include -I/usr/local/include
	      
ifeq ($(TRANSC_BUILDCFG_LIBPFRING),0)
CFLAGS_DEFS    += -DTRANSC_BUILDCFG_LIBPCAP
endif

#-------------------------------------------------------------------------
# Standard Makefile Inclusion
#-------------------------------------------------------------------------
-include $(TC_HOME)/tcgmake.mk
