## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ezstream

ezstream_SOURCES = ezstream.c configfile.c playlist.c util.c
ezstream_LDADD = @LIBOBJS@ @XIPH_LIBS@
    
AM_CFLAGS = @XIPH_CFLAGS@
AM_CPPFLAGS = @XIPH_CPPFLAGS@

EXTRA_DIST = ezsignals.h getopt.h strlfctns.h configfile.h playlist.h util.h

CLEANFILES = core *.core *~ .*~

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

