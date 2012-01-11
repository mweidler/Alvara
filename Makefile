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

# For infos regarding large file support, see the excellent page 
# http://people.redhat.com/berrange/notes/largefile.html

COPT = -W -Wall -Werror -O2 $(shell getconf LFS_CFLAGS)
LOPT = -s $(shell getconf LFS_LDFLAGS) $(shell getconf LFS_LIBS)
TAGVERSION = $(shell git describe HEAD | sed 's/-/./;s/\([^-]*\).*/\1/')

#getconf LFS_CFLAGS: RHEL 5.7 64Bit:      empty
#                    Ubuntu 10.04 32Bit: -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
#                    Linux Mint 12 64Bit: empty

alvara: alvara_main.cpp sha1.o ContentList.o StreamPersistence.o Alvara.o
	@echo "#define ALVARA_VERSION \"$(TAGVERSION)\"" >version.h
	g++ $(COPT) $(LOPT) alvara_main.cpp sha1.o ContentList.o StreamPersistence.o Alvara.o -o alvara 

sha1.o: sha1.c sha1.h
	g++ $(COPT) -c sha1.c -o sha1.o

ContentList.o: ContentList.cpp ContentList.hpp
	g++ $(COPT) -c ContentList.cpp -o ContentList.o

Alvara.o: Alvara.cpp Alvara.hpp
	g++ $(COPT) -c Alvara.cpp -o Alvara.o

StreamPersistence.o: StreamPersistence.cpp StreamPersistence.hpp
	g++ $(COPT) -c StreamPersistence.cpp -o StreamPersistence.o

clobber: clean

clean:
	rm alvara sha1.o ContentList.o StreamPersistence.o Alvara.o
	
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
