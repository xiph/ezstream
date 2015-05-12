AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

TESTS		 = \
	check_ezstream \
	check_cfg
check_PROGRAMS	 = $(TESTS)

check_ezstream_SOURCES = \
	check_ezstream.c
check_ezstream_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_ezstream_LDADD = $(check_ezstream_DEPENDENCIES) @CHECK_LIBS@

check_cfg_SOURCES = \
	check_cfg.c
check_cfg_DEPENDENCIES = $(top_builddir)/src/libezstream.la
check_cfg_LDADD  = $(check_cfg_DEPENDENCIES) @CHECK_LIBS@

AM_CPPFLAGS	 = @EZ_CPPFLAGS@ \
	-I$(top_srcdir)/compat \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src
AM_CFLAGS	 = @EZ_CFLAGS@ @CHECK_CFLAGS@
AM_LDFLAGS	 = @EZ_LDFLAGS@

CLEANFILES	 = *~ *.core core *.gcno *.gcda
