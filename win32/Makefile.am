AUTOMAKE_OPTIONS = 1.9 foreign

SUBDIRS 	 = shout

EXTRA_DIST	 = \
	README.win32 config.h compat.h \
	ezstream.sln ezstream.vcproj \
	libflac.vcproj libogg.vcproj libshout.vcproj libspeex.vcproj \
	libtheora.vcproj libvorbis.vcproj libvorbisfile.vcproj libz.vcproj \
	taglib.vcproj taglib_c.vcproj \
	tfile.cpp

CLEANFILES	 = core *.core *~ .*~ ezstream.1.pdf

do_subst	 = sed -e 's,!!EXAMPLES_DIR!!,${PACKAGE_TARNAME}-${PACKAGE_VERSION}-win32/examples,g'

.PHONY: regen-win32

regen-win32: Makefile
	$(do_subst) < $(top_srcdir)/doc/ezstream.1.in | nroff -Tps -mandoc - | ps2pdf - ezstream.1.pdf
