####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = kflog po doc 

EXTRA_DIST = kflog.kdevprj AUTHORS COPYING ChangeLog INSTALL README TODO kflog.lsm kflog.spec README-MAP 

install-data-local:
	$(mkinstalldirs) $$(kde_datadir)/kflog/mapdata/
	$(INSTALL_DATA) $(srcdir)/README-MAP $$(kde_datadir)/kflog/mapdata/README

uninstall-local:
	-rm -f $$(kde_datadir)/kflog/mapdata/README

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

EXTRA_DIST = admin 

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
	
KDE_ICON = AUTO
