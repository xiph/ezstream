AUTOMAKE_OPTIONS = 1.9 foreign

EXTRA_DIST =	README.win32 config.h ezstream.sln ezstream.vcproj

CLEANFILES =	core *.core *~ .*~ ezstream.1.pdf

do_subst =	sed -e 's,!!EXAMPLES_DIR!!,${PACKAGE_TARNAME}-${PACKAGE_VERSION}-win32/examples,g'

.PHONY:		regen-win32

regen-win32: Makefile
	$(do_subst) < $(top_srcdir)/doc/ezstream.1.in | nroff -Tps -mandoc - | ps2pdf - ezstream.1.pdf
