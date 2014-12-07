SHELL	= /bin/sh

configure_args =
dirs	= doc lib src

all: config.status
	for dir in $(dirs); do (cd $$dir; make all); done

install: config.status
	for dir in $(dirs); do (cd $$dir; make install); done

clean: config.status
	for dir in $(dirs); do (cd $$dir; make clean); done

distclean: config.status
	for dir in $(dirs); do (cd $$dir; make distclean); done
	rm -f config.cache config.log config.status

clobber: config.status
	for dir in $(dirs); do (cd $$dir; make clobber); done
	for dir in $(dirs); do rm -f $$dir/Makefile; done
	rm -f src/config.h src/config.h.in src/stamp-h.in
	rm -f config.cache config.log config.status
	rm -f configure

config.status: configure
	./configure $(configure_args)

configure: configure.in
	autoheader
	autoconf
