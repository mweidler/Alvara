#
# Makefile
#
# Installs, links or uninstalls alvara in the $HOME/bin directory or
# sets symbolic links from $HOME/bin to alvara in the current directory.
#
#
# Copyright (C) 2011 Marc Weidler (marc.weidler@web.de)
#
# THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.
# THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT
# WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
# THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.
# SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY
# SERVICING, REPAIR OR CORRECTION.

COPT = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -Wall
#COPT = 

alvara: alvara.o sha1.o ContentList.o
	g++ $(COPT) alvara.o sha1.o ContentList.o -o alvara 

sha1.o: sha1.c sha1.h
	gcc $(COPT) -c sha1.c -o sha1.o

ContentList.o: ContentList.cpp ContentList.hpp
	g++ $(COPT) -c ContentList.cpp -o ContentList.o

alvara.o: alvara.cpp
	g++ $(COPT) -c alvara.cpp -o alvara.o

clobber: clean

clean:
	rm alvara alvara.o sha1.o ContentList.o
	
install:
	@mkdir -p $(HOME)/bin
	@echo "... installing alvara to $(HOME)/bin"
	@cp -f alvara $(HOME)/bin

link:
	@mkdir -p $(HOME)/bin
	@echo "... linking alvara to $(HOME)/bin"
	@rm -f $(HOME)/bin/alvara
	@ln -s `pwd`/alvara $(HOME)/bin/alvara

uninstall:
	echo "... uninstalling $(HOME)/bin/alvara
	rm -f $(HOME)/bin/alvara

.PHONY: install link uninstall
