####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkfrcai.la


INCLUDES = $(all_includes)


AM_LDFLAGS =                


libkfrcai_la_LDFLAGS = -version-info 0:0:0

libkfrcai_la_METASOURCES = AUTO

libkfrcai_la_SOURCES = cambridge.cpp flightrecorder.cpp 


EXTRA_DIST = cai.desktop flightrecorder.cpp cambridge.cpp cambridge.h

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kflog/logger/
	$(INSTALL_DATA) $(srcdir)/cai.desktop $(kde_datadir)/kflog/logger/cai.desktop

uninstall-local:
	-rm -f $(kde_datadir)/kflog/logger/cai.desktop

####### kdevelop will overwrite this part!!! (end)############
