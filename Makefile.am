AUTOMAKE_OPTIONS = 1.9 foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS =	conf doc src win32

dist_doc_DATA = COPYING README

CLEANFILES =	core *.core *~ .*~

.PHONY:		snapshot

snapshot:
	${MAKE} dist distdir=${PACKAGE}-snapshot-`date +'%Y%m%d'`
