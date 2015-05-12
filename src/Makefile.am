AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

noinst_LTLIBRARIES = libezstream.la
noinst_HEADERS	 = \
	attributes.h \
	cfg.h \
	cfg_decoder.h \
	cfg_encoder.h \
	cfg_private.h \
	cfg_xmlfile.h \
	cmdline.h \
	ezstream.h \
	log.h \
	metadata.h \
	playlist.h \
	util.h \
	xalloc.h
libezstream_la_SOURCES = \
	cfg.c \
	cfg_decoder.c \
	cfg_encoder.c \
	cfg_xmlfile.c \
	cmdline.c \
	log.c \
	metadata.c \
	playlist.c \
	util.c \
	xalloc.c
libezstream_la_DEPENDENCIES = \
	$(top_builddir)/compat/libcompat.la
libezstream_la_LIBADD = @EZ_LIBS@ \
	$(libezstream_la_DEPENDENCIES)

bin_SCRIPTS	 = ezstream-file.sh
bin_PROGRAMS	 = ezstream

ezstream_SOURCES = ezstream.c
ezstream_DEPENDENCIES = libezstream.la
ezstream_LDADD	 = $(ezstream_DEPENDENCIES)

AM_CPPFLAGS	 = @EZ_CPPFLAGS@ -I$(top_srcdir)/compat
AM_CFLAGS	 = @EZ_CFLAGS@
AM_LDFLAGS	 = @EZ_LDFLAGS@ -avoid-version

CLEANFILES	 = core *.core *~ .*~
