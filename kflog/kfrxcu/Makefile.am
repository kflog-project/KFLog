####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkfrxcu.la


INCLUDES = $(all_includes)


LDFLAGS =


libkfrxcu_la_LDFLAGS = -version-info 0:0:0

libkfrxcu_la_METASOURCES = AUTO

libkfrxcu_la_SOURCES = cumulus.cpp flightrecorder.cpp cumuluswaypointfile.cpp


EXTRA_DIST = cumulus.cpp flightrecorder.cpp cumuluswaypointfile.cpp xcu.desktop

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kflog/logger/
	$(INSTALL_DATA) $(srcdir)/xcu.desktop $(kde_datadir)/kflog/logger/xcu.desktop

uninstall-local:
	-rm -f $(kde_datadir)/kflog/logger/xcu.desktop

####### kdevelop will overwrite this part!!! (end)############
