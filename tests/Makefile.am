AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

TESTS		 = \
	check_cfg \
	check_cfg_decoder \
	check_cfg_encoder \
	check_cfg_server \
	check_cfg_stream \
	check_cfg_xmlfile \
	check_cmdline \
	check_log \
	check_mdata \
	check_playlist \
	check_stream \
	check_util \
	check_xalloc
check_PROGRAMS	 = $(TESTS)

noinst_HEADERS = check_cfg.h

check_cfg_SOURCES = check_cfg.c
check_cfg_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_LDADD  = $(check_cfg_DEPENDENCIES) @CHECK_LIBS@

check_cfg_decoder_SOURCES = check_cfg_decoder.c
check_cfg_decoder_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_decoder_LDADD = $(check_cfg_decoder_DEPENDENCIES) @CHECK_LIBS@

check_cfg_encoder_SOURCES = check_cfg_encoder.c
check_cfg_encoder_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_encoder_LDADD = $(check_cfg_encoder_DEPENDENCIES) @CHECK_LIBS@

check_cfg_server_SOURCES = check_cfg_server.c
check_cfg_server_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_server_LDADD = $(check_cfg_server_DEPENDENCIES) @CHECK_LIBS@

check_cfg_stream_SOURCES = check_cfg_stream.c
check_cfg_stream_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_stream_LDADD = $(check_cfg_stream_DEPENDENCIES) @CHECK_LIBS@

check_cfg_xmlfile_SOURCES = check_cfg_xmlfile.c
check_cfg_xmlfile_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_xmlfile_LDADD = $(check_cfg_xmlfile_DEPENDENCIES) @CHECK_LIBS@

check_cmdline_SOURCES = check_cmdline.c
check_cmdline_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cmdline_LDADD = $(check_cmdline_DEPENDENCIES) @CHECK_LIBS@

check_log_SOURCES = check_log.c
check_log_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_log_LDADD	 = $(check_log_DEPENDENCIES) @CHECK_LIBS@

check_mdata_SOURCES = check_mdata.c
check_mdata_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_mdata_LDADD = $(check_mdata_DEPENDENCIES) @CHECK_LIBS@

check_playlist_SOURCES = check_playlist.c
check_playlist_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_playlist_LDADD = $(check_playlist_DEPENDENCIES) @CHECK_LIBS@

check_stream_SOURCES = check_stream.c
check_stream_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_stream_LDADD = $(check_stream_DEPENDENCIES) @CHECK_LIBS@

check_util_SOURCES = check_util.c
check_util_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_util_LDADD = $(check_util_DEPENDENCIES) @CHECK_LIBS@

check_xalloc_SOURCES = check_xalloc.c
check_xalloc_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_xalloc_LDADD = $(check_xalloc_DEPENDENCIES) @CHECK_LIBS@

AM_CPPFLAGS	 = @EZ_CPPFLAGS@ \
	-I$(top_srcdir)/compat \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src
AM_CFLAGS	 = @EZ_CFLAGS@ @CHECK_CFLAGS@ \
	-DSRCDIR="\"$(srcdir)\"" \
	-DBUILDDIR="\"$(builddir)\"" \
	-DEXAMPLESDIR="\"$(top_srcdir)/examples\""
AM_LDFLAGS	 = @EZ_LDFLAGS@

EXTRA_DIST	 = \
	bad-executable.sh \
	config-bad.xml \
	config-bad2.xml \
	config-bad3.xml \
	config-bad4.xml \
	config-bad5.xml \
	play-bad.sh \
	play-bad2.sh \
	play-bad3.sh \
	playlist-bad.txt \
	playlist-bad2.txt \
	playlist.txt \
	null.raw \
	test01-artist+album+title.ogg \
	test02-whitespace.ogg \
	test03-apostrophe.ogg \
	test04-backticks.ogg \
	test05-spawnshell.ogg \
	test06-shellvar.ogg \
	test07-japanese.ogg \
	test08-arabic.ogg \
	test09-apostrophe+spawnshell.ogg \
	test10-apostrophe+backticks.ogg \
	test11-artist+album.ogg \
	test12-artist.ogg \
	test13-album+title.ogg \
	test14-album.ogg \
	test15-title.ogg \
	test16-nometa.ogg \
	.test17ogg \
	test18-emptymeta.ogg \
	test19-onlywhitespace.ogg \
	test-meta01.sh \
	test-meta02-error.sh \
	test-meta03-huge.sh \
	test-meta04-kill.sh \
	test-meta05-empty.sh

CLEANFILES	 = *~ *.core core *.gcno *.gcda
