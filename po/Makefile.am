####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = de.gmo kflog.pot no.gmo sv.gmo fr.gmo 

install-data-local:
	$(mkinstalldirs) $(kde_locale)/de/
	$(INSTALL_DATA) $(srcdir)/de.gmo $(kde_locale)/de/kflog.mo
	$(mkinstalldirs) $(kde_locale)/no/
	$(INSTALL_DATA) $(srcdir)/no.gmo $(kde_locale)/no/kflog.mo
	$(mkinstalldirs) $(kde_locale)/sv/
	$(INSTALL_DATA) $(srcdir)/sv.gmo $(kde_locale)/sv/kflog.mo
	$(mkinstalldirs) $(kde_locale)/fr/
	$(INSTALL_DATA) $(srcdir)/fr.gmo $(kde_locale)/fr/kflog.mo

uninstall-local:
	-rm -f $(kde_locale)/de/kflog.mo
	-rm -f $(kde_locale)/no/kflog.mo
	-rm -f $(kde_locale)/sv/kflog.mo
	-rm -f $(kde_locale)/fr/kflog.mo

####### kdevelop will overwrite this part!!! (end)############
POFILES = AUTO

merge:
	@catalogs=`find . -name \*.po`; \
	for cat in $$catalogs; do \
	name=$(PACKAGE).pot ; \
	echo $$cat $$name; \
	msgmerge $$cat $$name > $$cat.new ; \
	if diff $$cat $$cat.new; then \
	  rm $$cat.new;  \
	else  \
	  mv $$cat.new $$cat ; \
	fi; \
	done
