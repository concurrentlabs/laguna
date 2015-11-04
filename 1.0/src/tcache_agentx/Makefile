###################################################################################
###################################################################################
ECHO=echo
SNMP_CONF_PATH=/etc/snmp
INIT_PATH=/etc/init.d
MIB_PATH=/usr/share/snmp/mibs
SBIN_PATH=/usr/local/sbin

.PHONEY: all clean

all:
	$(ECHO) "building util_source"
	cd util_source; make
	$(ECHO) "building main_source"
	cd main_source; make

clean:
	$(ECHO) "cleaning util_source"
	cd util_source; make clean
	$(ECHO) "building main_source"
	cd main_source; make clean
	cd bin; rm -f tcache_agentx

install:
	cp -b conf/snmpd.conf	$(SNMP_CONF_PATH)
	cp -b rc.d/tcache_agentx $(INIT_PATH)
	cp -b mib/TCACHE-MIB.txt $(MIB_PATH)
	cp bin/tcache_agentx	 $(SBIN_PATH)
