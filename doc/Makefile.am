AUTOMAKE_OPTIONS = 1.10 foreign subdir-objects

man_MANS	 = ezstream.1 ezstream-cfgmigrate.1 ezstream-file.sh.1

do_subst	 = sed \
	-e 's|!!EXAMPLES_DIR!!|@EXAMPLES_DIR@|g'

.PHONY: $(man_MANS)

ezstream.1: ezstream.1.in Makefile
	$(do_subst) < $(builddir)/ezstream.1.in > ezstream.1

ezstream-cfgmigrate.1: ezstream-cfgmigrate.1.in Makefile
	$(do_subst) < $(builddir)/ezstream-cfgmigrate.1.in > ezstream-cfgmigrate.1

ezstream-file.sh.1: ezstream-file.sh.1.in Makefile
	$(do_subst) < $(builddir)/ezstream-file.sh.1.in > ezstream-file.sh.1

CLEANFILES	 = *~ *.core core $(man_MANS)
