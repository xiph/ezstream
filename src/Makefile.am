AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

noinst_LTLIBRARIES = libcommon.la libezstream.la
noinst_HEADERS	 = \
	attributes.h \
	cfg.h \
	cfg_decoder.h \
	cfg_encoder.h \
	cfg_intake.h \
	cfg_private.h \
	cfg_server.h \
	cfg_stream.h \
	cfgfile_xml.h \
	cmdline.h \
	ezconfig0.h \
	ezstream.h \
	log.h \
	mdata.h \
	playlist.h \
	stream.h \
	util.h \
	xalloc.h

libcommon_la_SOURCES = \
	cfg.c \
	cfg_decoder.c \
	cfg_encoder.c \
	cfg_intake.c \
	cfg_server.c \
	cfg_stream.c \
	cfgfile_xml.c \
	log.c \
	util.c \
	xalloc.c

libezstream_la_SOURCES = \
	cmdline.c \
	mdata.c \
	playlist.c \
	stream.c
libezstream_la_DEPENDENCIES = \
	$(builddir)/libcommon.la \
	$(top_builddir)/compat/libcompat.la
libezstream_la_LIBADD = @EZ_LIBS@ \
	$(libezstream_la_DEPENDENCIES)

bin_SCRIPTS	 = ezstream-file.sh
bin_PROGRAMS	 = ezstream ezstream-cfgmigrate

ezstream_SOURCES = ezstream.c
ezstream_DEPENDENCIES = libezstream.la
ezstream_LDADD	 = $(ezstream_DEPENDENCIES)

ezstream_cfgmigrate_SOURCES = ezstream-cfgmigrate.c ezconfig0.c
ezstream_cfgmigrate_DEPENDENCIES = \
	$(builddir)/libcommon.la \
	$(top_builddir)/compat/libcompat.la
ezstream_cfgmigrate_LDADD = @EZ_LIBS@ \
	$(ezstream_cfgmigrate_DEPENDENCIES)

AM_CPPFLAGS	 = @EZ_CPPFLAGS@ -I$(top_srcdir)/compat
AM_CFLAGS	 = @EZ_CFLAGS@
AM_LDFLAGS	 = @EZ_LDFLAGS@ -avoid-version

CLEANFILES	 = core *.core *~ .*~ *.gcno *.gcda
