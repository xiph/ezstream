AUTOMAKE_OPTIONS = 1.9 foreign

bin_PROGRAMS	 = ezstream

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

INCLUDES	 = @COMPAT_INCLUDES@
AM_CFLAGS	 = @EZ_CFLAGS@
AM_CPPFLAGS	 = @EZ_CPPFLAGS@
AM_LDFLAGS	 = @EZ_LDFLAGS@

EXTRA_DIST	 = \
	compat.h \
	configfile.h \
	metadata.h \
	playlist.h \
	strfctns.h \
	util.h \
	xalloc.h

CLEANFILES	 = core *.core *~ .*~
