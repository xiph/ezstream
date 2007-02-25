AUTOMAKE_OPTIONS = 1.9 foreign

bin_PROGRAMS =	ezstream

ezstream_SOURCES = ezstream.c configfile.c playlist.c util.c
ezstream_LDADD = @LIBOBJS@ @XIPH_LIBS@
    
AM_CFLAGS =	@XIPH_CFLAGS@
AM_CPPFLAGS =	@XIPH_CPPFLAGS@

EXTRA_DIST =	configfile.h ezsignals.h getopt.h playlist.h strlfctns.h \
		util.h

CLEANFILES =	core *.core *~ .*~
