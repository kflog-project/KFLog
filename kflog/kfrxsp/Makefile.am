####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkfrxsp.la


INCLUDES = $(all_includes)


LDFLAGS =              


libkfrxsp_la_LDFLAGS = -version-info 0:0:0

libkfrxsp_la_METASOURCES = AUTO

libkfrxsp_la_SOURCES = soaringpilot.cpp flightrecorder.cpp 


EXTRA_DIST = flightrecorder.cpp xsp.desktop soaringpilot.cpp soaringpilot.h 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kflog/logger/
	$(INSTALL_DATA) $(srcdir)/xsp.desktop $(kde_datadir)/kflog/logger/xsp.desktop

uninstall-local:
	-rm -f $(kde_datadir)/kflog/logger/xsp.desktop

####### kdevelop will overwrite this part!!! (end)############
