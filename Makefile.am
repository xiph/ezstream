AUTOMAKE_OPTIONS = 1.9 foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS =	conf doc m4 src win32

dist_doc_DATA = COPYING NEWS README

CLEANFILES =	core *.core *~ .*~

.PHONY:		snapshot

snapshot:
	${MAKE} distcheck distdir=${PACKAGE}-snapshot-`date +'%Y%m%d'`
