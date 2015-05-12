AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

noinst_LTLIBRARIES = libcompat.la
noinst_HEADERS	 = compat.h
libcompat_la_SOURCES =
libcompat_la_DEPENDENCIES = @LTLIBOBJS@
libcompat_la_LIBADD = $(libcompat_la_DEPENDENCIES)

AM_CPPFLAGS	 = @EZ_CPPFLAGS@
AM_CFLAGS	 = @EZ_CFLAGS@
AM_LDFLAGS	 = @EZ_LDFLAGS@ -avoid-version

CLEANFILES	 = *~ *.core core *.gcno *.gcda
