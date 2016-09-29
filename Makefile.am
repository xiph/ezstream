AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects
ACLOCAL_AMFLAGS  = -I m4

SUBDIRS 	 = build-aux compat doc examples m4 src tests

dist_doc_DATA	 = COPYING NEWS README

EXTRA_DIST	 = \
	autogen.sh \
	m4/attribute.m4 m4/ccflags.m4 m4/libshout.m4 m4/libxml2.m4 \
	m4/progname.m4 m4/taglib.m4 m4/tree.m4 m4/vars.m4

CLEANFILES	 = core *.core *~ .*~

.PHONY: snapshot

snapshot:
	${MAKE} distcheck distdir=${PACKAGE}-snapshot-`date +'%Y%m%d'`
