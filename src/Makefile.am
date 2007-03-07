AUTOMAKE_OPTIONS = 1.9 foreign

bin_PROGRAMS =	ezstream

ezstream_SOURCES = ezstream.c compat.c configfile.c playlist.c util.c
ezstream_LDADD = @LIBOBJS@ @XIPH_LIBS@
    
AM_CFLAGS =	@XIPH_CFLAGS@
AM_CPPFLAGS =	@XIPH_CPPFLAGS@

EXTRA_DIST =	compat.h configfile.h getopt.h playlist.h strfctns.h util.h

CLEANFILES =	core *.core *~ .*~
