## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = ezstream

ezstream_SOURCES = ezstream.c configfile.c
ezstream_LDADD = @XIPH_LIBS@
    
AM_CFLAGS = @XIPH_CFLAGS@
AM_CPPFLAGS = @XIPH_CPPFLAGS@

EXTRA_DIST = getopt.c getopt.h configfile.h


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"

