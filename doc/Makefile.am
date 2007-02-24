AUTOMAKE_OPTIONS = 1.9 foreign

man_MANS =	ezstream.1

EXTRA_DIST =	ezstream.1.in

do_subst =	sed -e 's,!!EXAMPLES_DIR!!,@EXAMPLES_DIR@,g'

ezstream.1:	ezstream.1.in Makefile
		$(do_subst) < $(srcdir)/ezstream.1.in > ezstream.1

CLEANFILES =	*~ *.core core ${man_MANS}
