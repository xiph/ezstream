## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.9 foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS =	conf doc src win32

dist_doc_DATA = COPYING README

CLEANFILES =	core *.core *~ .*~

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"
