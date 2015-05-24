AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

TESTS		 = \
	check_cfg \
	check_cfg_xmlfile \
	check_log
check_PROGRAMS	 = $(TESTS)

check_cfg_SOURCES = \
	check_cfg.c
check_cfg_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_LDADD  = $(check_cfg_DEPENDENCIES) @CHECK_LIBS@

check_cfg_xmlfile_SOURCES = \
	check_cfg_xmlfile.c
check_cfg_xmlfile_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_xmlfile_LDADD = $(check_cfg_xmlfile_DEPENDENCIES) @CHECK_LIBS@

check_log_SOURCES = \
	check_log.c
check_log_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_log_LDADD	 = $(check_cfg_xmlfile_DEPENDENCIES) @CHECK_LIBS@

AM_CPPFLAGS	 = @EZ_CPPFLAGS@ \
	-I$(top_srcdir)/compat \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src
AM_CFLAGS	 = @EZ_CFLAGS@ @CHECK_CFLAGS@ \
	-DSRCDIR="\"$(srcdir)\"" \
	-DEXAMPLESDIR="\"$(top_srcdir)/examples\""
AM_LDFLAGS	 = @EZ_LDFLAGS@

EXTRA_DIST	 = \
	config-bad.xml \
	config-bad2.xml \
	config-bad3.xml \
	config-bad4.xml

CLEANFILES	 = *~ *.core core *.gcno *.gcda
