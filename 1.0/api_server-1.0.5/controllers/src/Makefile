###################################################################################
###################################################################################
ECHO=echo

.PHONEY: all clean

all:
	$(ECHO) "building pycs"
	python -m compileall .
	mv *.pyc ..
	chmod 755 ../api_server.pyc

clean:
	$(ECHO) "cleaning pycs"
	rm -f ../*.pyc

run:
	$(ECHO) "running api_server"
	cd .. && python ./api_server.pyc
