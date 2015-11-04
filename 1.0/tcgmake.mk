
.PHONY: debug release

#TODO: below code need to be written better than keying based on TARGETDESC
#Few lines below might need to be revisited later...
#-------------------------------------------------------------------------
# Target Definitions
#-------------------------------------------------------------------------
TARGETOBJ       = $(notdir $(patsubst %.c,%.o,$(TARGETSRC)))
TARGETBLDDEPS   = $(notdir $(patsubst %.c,%.d,$(TARGETSRC)))
ifdef TARGETDESC
TARGETBLDDEP   += buildmsg
endif
TARGETBLDDEP   += $(TARGETBLDDEPS)

ifdef RELEASE
TARGETDEPS      = cleanop $(TARGETBLDDEP) $(TARGETOBJ)
else
TARGETDEPS      = $(TARGETBLDDEP) $(TARGETOBJ)
endif

#
# Now, construct the complete build target list
#
ifeq ($(TARGETTYPE),APPLICATION)
	BUILDTARGET     = $(TARGETDIR)/$(BUILDTYPE)/$(TARGETNAME)
else
	ifeq ($(TARGETTYPE),STATICLIB)
		BUILDTARGET     = $(TARGETDIR)/$(BUILDTYPE)/$(TARGETNAME).a
		NOSHAREDLIBS    = 1
		export NOSHAREDLIBS
	else
		ifeq ($(TARGETTYPE),SHAREDLIB)
		BUILDTARGET     = $(TARGETDIR)/$(BUILDTYPE)/$(TARGETNAME).$(SHAREDLIBEXT)
		endif
	endif
endif
ifndef NOSHAREDLIBS
CFLAGS_OPTS    += -fPIC 
CFLAGS_DEFS    += -DSHARED_LIBRARY 
endif

#-------------------------------------------------------------------------
# Build Type-Specific Items/Messages
#-------------------------------------------------------------------------
ifdef RELEASE
BUILDTYPE       = $(BLDOUTDIR_REL)
BUILDTYPEMSG    = "- RELEASE Build"
else
BUILDTYPE       = $(BLDOUTDIR_DBG)
BUILDTYPEMSG    = "- DEBUG Build"
endif

#-------------------------------------------------------------------------
# Build Rules
#-------------------------------------------------------------------------
%.o : %.c 
ifndef VERBOSE
	@echo "$<"
endif
	$(CMDVERBOSE)$(CC) $(CFLAGS) -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.o : %.cpp
# ifndef VERBOSE
# 	@echo "$<"
# endif
	$(CMDVERBOSE)$(CXX) $(CFLAGS) $(CXXFLAGS) -o $(OBJDIR)/$(BUILDTYPE)/$(@F) -c $<

%.d : %.c 
	$(CMDVERBOSE)$(CC) -M $(CFLAGS) $< > $(OBJDIR)/$(@F)

%.d : %.cpp 
	$(CMDVERBOSE)$(CXX) -M $(CXXFLAGS) $< > $(OBJDIR)/$(@F)

#-------------------------------------------------------------------------
# Common Target Build Definitions
#-------------------------------------------------------------------------

#
# Actual build targets
#
bldtarget: preproc $(BUILDTARGET) postproc

#Application linker 
$(TARGETDIR)/$(BUILDTYPE)/$(TARGETNAME): $(TARGETDEPS)
ifndef VERBOSE
	@echo "$@"
endif
	$(CMDVERBOSE)$(APPWRAPPER) $(CC) $(LDFLAGS) \
	    -o $@ $(foreach file,$(notdir $(TARGETOBJ)), \
		$(OBJDIR)/$(BUILDTYPE)/$(file))  \
		$(LDLIBLINK) $(LDLIBS)
	$(CMDVERBOSE)cd $(TARGETDIR) ; \
	    if [ -e $(TARGETNAME) ]; then rm -Rf $(TARGETNAME); fi ; \
	    ln -fs $(BUILDTYPE)/$(TARGETNAME) .;
	$(CMDVERBOSE)cd $(TARGETDIR)/$(BUILDTYPE) ; \
	    if [ ! -e $(TARGETLOGCONF) ]; then cp $(TRANSC_MAINDIR)/$(TARGETLOGCONF) .; fi ; \
	    if [ ! -e $(TARGETMAINCONF) ]; then cp $(TRANSCRPM_SCRIPT_DIR)/$(TRANSC_CONFDIR)/$(TARGETMAINCONF) .; fi ;

#library linker 
$(TARGETDIR)/$(BUILDTYPE)/$(TARGETNAME).a: $(TARGETDEPS)
ifndef VERBOSE
	@echo "$@"
endif
	$(CMDVERBOSE)$(AR) $(ARFLAGS) $@ \
	    $(foreach file,$(notdir $(TARGETOBJ)), \
		$(OBJDIR)/$(BUILDTYPE)/$(file)) $(AROBJS)
	$(CMDVERBOSE)cd $(TARGETDIR) ; \
	    if [ -e ./$(TARGETNAME).a ]; then rm -Rf $(TARGETNAME).a ; fi ; \
	    ln -fs $(BUILDTYPE)/$(TARGETNAME).a .

$(TARGETDIR)/$(BUILDTYPE)/$(TARGETNAME).$(SHAREDLIBEXT): $(TARGETDEPS)
ifndef VERBOSE
	@echo "$@"
endif

#-------------------------------------------------------------------------
# Pre/Post/Clean Targets
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# pre-processing
#-------------------------------------------------------------------------
preproc: basedirchk dircheck

basedirchk:
	$(CMDVERBOSE)if [ -z "$(TC_HOME)" ]; then \
	    echo "The environment variable TC_HOME must be defined."; \
	    fi

dircheck:
	$(CMDVERBOSE)if [ ! -d "$(TARGETDIR)" ]; then mkdir -p $(TARGETDIR); fi
	$(CMDVERBOSE)if [ ! -d "$(TARGETDIR)/$(BLDOUTDIR_DBG)" ]; then \
	    mkdir $(TARGETDIR)/$(BLDOUTDIR_DBG); fi
	$(CMDVERBOSE)if [ ! -d "$(TARGETDIR)/$(BLDOUTDIR_REL)" ]; then \
	    mkdir $(TARGETDIR)/release; fi
	$(CMDVERBOSE)if [ ! -d "$(OBJDIR)" ]; then mkdir $(OBJDIR); fi
	$(CMDVERBOSE)if [ ! -d "$(OBJDIR)/$(BLDOUTDIR_DBG)" ]; then \
	    mkdir $(OBJDIR)/$(BLDOUTDIR_DBG); fi
	$(CMDVERBOSE)if [ ! -d "$(OBJDIR)/$(BLDOUTDIR_REL)" ]; then \
	    mkdir $(OBJDIR)/$(BLDOUTDIR_REL); fi

buildmsg:
	@echo ""
	@echo "Building $(TARGETDESC) ($(TARGET_OS)-$(CPU_ARCH)) $(BUILDTYPEMSG)"
	@echo "---------------"

#-------------------------------------------------------------------------
# post-processing
#-------------------------------------------------------------------------
postproc: uptodatemsg

uptodatemsg:
	@echo "$(TARGETDESC) up-to-date."
	@echo ""


#-------------------------------------------------------------------------
# debug, release and clean
#-------------------------------------------------------------------------

#-------------------------------------------------------------------------
# Phony target for debug builds
#-------------------------------------------------------------------------
ifdef TARGETTYPE
debug:
	$(CMDVERBOSE)$(MAKE) $(MAKEOPTS) -C . 
endif

#-------------------------------------------------------------------------
# Phony target for release builds
#-------------------------------------------------------------------------
ifdef TARGETTYPE
release:
	$(CMDVERBOSE)$(MAKE) $(MAKEOPTS) RELEASE=y -C . 
endif

#-------------------------------------------------------------------------
# Phony target for clean builds
#-------------------------------------------------------------------------
ifdef TARGETDESC
CLEANDEPS       = cleanmsg cleanop cleanpost 
else
CLEANDEPS       = cleanop
endif
clean: $(CLEANDEPS)

cleanmsg:
	@echo ""
	@echo "Cleaning the $(TARGETDESC) ($(TARGET_OS)-$(CPU_ARCH))"
	@echo "-------------------------------------------------------------------------"

cleanop:
	$(CMDVERBOSE)rm -Rf $(OBJDIR)/*.d $(OBJDIR)/*.o \
	    $(TARGETDIR)/$(TARGETNAME) \
	    $(OBJDIR)/$(BLDOUTDIR_DBG)/*.o $(OBJDIR)/$(BLDOUTDIR_REL)/*.o
ifneq ($(TARGETTYPE),APPLICATION)
	$(CMDVERBOSE)rm -Rf $(TARGETDIR)/$(TARGETNAME)*
	$(CMDVERBOSE)rm -Rf $(TARGETDIR)/$(BLDOUTDIR_DBG)/$(TARGETNAME)* 
	$(CMDVERBOSE)rm -Rf $(TARGETDIR)/$(BLDOUTDIR_REL)/$(TARGETNAME)* 
endif

cleanpost:
	@echo "Clean build complete."
	@echo ""

#
# Include the dependency targets here
#
ifneq ($(TARGETBLDDEP),)
-include $(foreach file,$(TARGETBLDDEP),$(OBJDIR)/$(file))
endif
