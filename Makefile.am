AUTOMAKE_OPTIONS = 1.9 foreign
ACLOCAL_AMFLAGS  = -I m4

SUBDIRS 	 = build-aux compat doc examples m4 src win32

dist_doc_DATA	 = COPYING NEWS README

EXTRA_DIST	 = \
	autogen.sh \
	m4/ccflags.m4 m4/libshout.m4 m4/libvorbis.m4 m4/libxml2.m4 \
	m4/progname.m4 m4/taglib.m4 m4/tree.m4 m4/vacopy.m4 m4/vars.m4

CLEANFILES	 = core *.core *~ .*~

.PHONY: regen-win32 snapshot

regen-win32:
	cd win32; ${MAKE} regen-win32

snapshot:
	${MAKE} distcheck distdir=${PACKAGE}-snapshot-`date +'%Y%m%d'`
