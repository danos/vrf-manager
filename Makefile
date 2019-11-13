CC = gcc
AR = ar rsc
RM = rm -f

libdir = /usr/lib/$(DEB_HOST_MULTIARCH)
incdir = /usr/include

pkgversion = $(shell dpkg-parsechangelog | grep -E '^Version:' | awk '{ print $$2 }')

LIB = libvrfmanager.a
LIBPATH = lib/Vyatta

SRC = vrfmanager/vrf_manager.c
HDR = $(SRC:.c=.h)
OBJ = $(SRC:.c=.o)

all: library libvrfmanager-vyatta.pc
	@echo "\nCompilation complete.\n"

library:
	mkdir -p $(LIBPATH)
	$(CC) -c -fPIC $(SRC) -o $(OBJ)
	$(AR) $(LIBPATH)/$(LIB) -o $(OBJ)

libvrfmanager-vyatta.pc: libvrfmanager-vyatta.pc.in
	sed -e 's,@libdir@,$(libdir),g' -e 's,@incdir@,$(incdir),g' -e 's,@version@,$(pkgversion),g' $< > $@

install:
	mkdir -p $(DESTDIR)$(libdir)
	mkdir -p $(DESTDIR)$(incdir)
	cp $(HDR) $(DESTDIR)$(incdir)
	cp $(LIBPATH)/$(LIB) $(DESTDIR)$(libdir)
	mkdir -p $(DESTDIR)$(libdir)/pkgconfig
	install -m 644 libvrfmanager-vyatta.pc $(DESTDIR)$(libdir)/pkgconfig/

clean: test_clean
	$(RM) $(LIBPATH)/$(LIB)
	$(RM) $(OBJ)
	$(RM) libvrfmanager-vyatta.pc

include test/C-lib/Makefile
