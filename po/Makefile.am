####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = de.gmo kflog.pot 

install-data-local:
	$(mkinstalldirs) $$(kde_locale)/de/
	$(INSTALL_DATA) $(srcdir)/de.gmo $$(kde_locale)/de/kflog.mo

uninstall-local:
	-rm -f $$(kde_locale)/de/kflog.mo

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
