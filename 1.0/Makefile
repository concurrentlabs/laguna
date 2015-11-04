#-------------------------------------------------------------------------
# Compiler settings
#-------------------------------------------------------------------------
export TC_HOME	:= $(shell pwd)
-include $(TC_HOME)/tccompiler.mk

#-------------------------------------------------------------------------
# Install config
#-------------------------------------------------------------------------
#software major version
export TRANSCDIR_VER	= 1.4.1
#software minor version
export TRANSCDIR_REL	= 1
#script version
TRANSCDIR_SCRIPT_VER    = 1.0.0
export TRANSCRPM_SCRIPT_DIR	= $(TC_HOME)/scripts/rpm/transparent_caching_$(TRANSCDIR_SCRIPT_VER)

#-------------------------------------------------------------------------
# Application Dependencies
#-------------------------------------------------------------------------
TRANSCDIR 				= $(TC_HOME)/src

ifndef PFRING
TRANSC_BUILDCFG_LIBPFRING := 1
else
ifeq ($(PFRING),1)
TRANSC_BUILDCFG_LIBPFRING := 1
else
TRANSC_BUILDCFG_LIBPFRING := 0
endif
endif
export TRANSC_BUILDCFG_LIBPFRING

#-------------------------------------------------------------------------
# help
#-------------------------------------------------------------------------
usage:
	@echo ""
	@echo "make <option>"
	@echo ""
	@echo ""
	@echo "Supported <options>:"
	@echo "  debug               Debug version of 'transc'"
	@echo "  release             Release version of 'transc'"
	@echo "  package             Release package version of 'transc'"
	@echo "  clean               Remove *.o, *.a, *.so, and libraries"
	@echo ""
	
#-------------------------------------------------------------------------
# main build
#-------------------------------------------------------------------------
#
# transc source
#
transc_dbg: 
	@if [ -e "$(TRANSCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TRANSCDIR) $(MAKEDEFS) debug; fi

transc_rel:
	@if [ -e "$(TRANSCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TRANSCDIR) $(MAKEDEFS) release; fi

transc_clean:
	@if [ -e "$(TRANSCDIR)/Makefile" ]; then \
	    $(MAKE) $(MAKEOPTS) -C $(TRANSCDIR) $(MAKEDEFS) clean; fi
	    
#TODO: ugly packaging, Need to move below logic to tcmain/Makefile and build the package post build.
transc_package: transc_rel
	@if [ -e "$(TC_HOME)install/rpm/$(CPU_ARCH)/release" ]; then \
	    rm -rf $(TC_HOME)install/rpm/$(CPU_ARCH)/release; \
	fi
	@if [ -e "$(TC_HOME)/bin/release/transc" ]; then \
	    echo "copying executable..."; \
	    cp -f $(TC_HOME)/bin/release/transc \
	    $(TRANSCRPM_SCRIPT_DIR)/usr/local/bin/transparent_caching/; \
		mkdir -p $(TC_HOME)/install/rpm $(TC_HOME)/install/rpm/$(CPU_ARCH)/release/BUILD;\
		mkdir -p $(TC_HOME)/install/rpm $(TC_HOME)/install/rpm/$(CPU_ARCH)/release/RPMS;\
	fi
# Build the binary package.
# Remove the --quiet option for verbose output of the build steps.
	rpmbuild -bb \
        --define="_scversion $(TRANSCDIR_SCRIPT_VER)" \
		--define="_version $(TRANSCDIR_VER)" \
		--define="_release $(TRANSCDIR_REL)" \
		--define="_topdir $(TC_HOME)/install/rpm/$(CPU_ARCH)/release" \
		--define="__PKGS_DIR $(TC_HOME)/scripts/rpm" \
		$(TRANSCRPM_SCRIPT_DIR)/transparent_caching.spec
		
#TODO: ugly packaging, Needs to move below logic to tcmain/Makefile and build the package post build.
transc_package-dbg: transc_dbg
	@if [ -e "$(TC_HOME)install/rpm/$(CPU_ARCH)/debug" ]; then \
	    rm -rf $(TC_HOME)install/rpm/$(CPU_ARCH)/debug; \
	fi
	@if [ -e "$(TC_HOME)/bin/debug/transc" ]; then \
	    echo "copying executable..."; \
	    cp -f $(TC_HOME)/bin/debug/transc \
	    $(TRANSCRPM_SCRIPT_DIR)/usr/local/bin/transparent_caching/; \
		mkdir -p $(TC_HOME)/install/rpm $(TC_HOME)/install/rpm/$(CPU_ARCH)/debug/BUILD;\
		mkdir -p $(TC_HOME)/install/rpm $(TC_HOME)/install/rpm/$(CPU_ARCH)/debug/RPMS;\
	fi
# Build the binary package.
# Remove the --quiet option for verbose output of the build steps.
	rpmbuild -bb \
	    --define="_scversion $(TRANSCDIR_SCRIPT_VER)" \
		--define="_version $(TRANSCDIR_VER)" \
		--define="_release $(TRANSCDIR_REL)" \
		--define="_topdir $(TC_HOME)/install/rpm/$(CPU_ARCH)/debug" \
		--define="__PKGS_DIR $(TC_HOME)/scripts/rpm" \
		$(TRANSCRPM_SCRIPT_DIR)/transparent_caching.spec

#
# Generic build targets
#
debug: transc_dbg

release: transc_rel

package: transc_package

package-dbg: transc_package-dbg

clean: transc_clean

