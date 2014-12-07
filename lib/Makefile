# Generated automatically from Makefile.in by configure.
SHELL	= /bin/sh

# stuff from configure:
prefix = /usr/local
INSTALL	= /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644

# where the standard modules live
hopelib	= ${prefix}/share/hope/lib

# experimental version of some libraries
newlib	= $(hopelib).new

all:

install:
	$(INSTALL) -d $(hopelib)
	for f in *.hop; do $(INSTALL_DATA) $$f $(hopelib); done
	$(INSTALL) -d $(newlib)
	sed -f list.sed Standard.hop >Standard-new.hop
	$(INSTALL_DATA) Standard-new.hop $(newlib)/Standard.hop
	rm -f Standard-new.hop

distclean clean clobber:
	rm -f Standard-new.hop
