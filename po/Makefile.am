####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = kflog.pot 

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
