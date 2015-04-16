AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

bin_PROGRAMS	 = ezstream
bin_SCRIPTS	 = ezstream-file.sh

noinst_HEADERS	 = \
	cfg.h \
	configfile.h \
	ezstream.h \
	metadata.h \
	playlist.h \
	util.h \
	xalloc.h
ezstream_SOURCES = \
	cfg.c \
	configfile.c \
	ezstream.c \
	metadata.c \
	playlist.c \
	util.c \
	xalloc.c
ezstream_LDADD	 = @LIBOBJS@ @EZ_LIBS@

AM_CFLAGS	 = @EZ_CFLAGS@
AM_CPPFLAGS	 = @EZ_CPPFLAGS@ -I$(top_srcdir)/compat
AM_LDFLAGS	 = @EZ_LDFLAGS@

CLEANFILES	 = core *.core *~ .*~
