AUTOMAKE_OPTIONS = 1.9 foreign

bin_PROGRAMS	 = ezstream
bin_SCRIPTS	 = ezstream-file.sh

ezstream_SOURCES = \
	compat.c \
	configfile.c \
	ezstream.c \
	getopt.c \
	metadata.c \
	playlist.c \
	util.c \
	xalloc.c
ezstream_LDADD	 = @LIBOBJS@ @EZ_LIBS@

AM_CFLAGS	 = @EZ_CFLAGS@
AM_CPPFLAGS	 = @EZ_CPPFLAGS@ @COMPAT_INCLUDES@
AM_LDFLAGS	 = @EZ_LDFLAGS@

EXTRA_DIST	 = \
	configfile.h \
	ezstream.h \
	metadata.h \
	playlist.h \
	strfctns.h \
	util.h \
	xalloc.h

CLEANFILES	 = core *.core *~ .*~
