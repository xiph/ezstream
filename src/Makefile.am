AUTOMAKE_OPTIONS = 1.9 foreign

bin_PROGRAMS =	ezstream

ezstream_SOURCES = compat.c configfile.c ezstream.c metadata.c playlist.c \
		util.c xalloc.c
ezstream_LDADD = @LIBOBJS@ @XIPH_LIBS@ @TAGLIB_LIBS@

INCLUDES =	@COMPAT_INCLUDES@

AM_CFLAGS =	@XIPH_CFLAGS@ @TAGLIB_CFLAGS@
AM_CPPFLAGS =	@XIPH_CPPFLAGS@ @TAGLIB_CPPFLAGS@

EXTRA_DIST =	compat.h configfile.h getopt.h metadata.h playlist.h \
		strfctns.h util.h xalloc.h

CLEANFILES =	core *.core *~ .*~
