AUTOMAKE_OPTIONS = 1.9 foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS =	compat doc examples m4 src win32

dist_doc_DATA = COPYING NEWS README

CLEANFILES =	core *.core *~ .*~

.PHONY:		regen-win32 snapshot

regen-win32:
	cd win32; ${MAKE} regen-win32

snapshot:
	${MAKE} distcheck distdir=${PACKAGE}-snapshot-`date +'%Y%m%d'`
