####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkfrfil.la


INCLUDES = $(all_includes)


LDFLAGS =            


libkfrfil_la_LDFLAGS = -version-info 0:0:0

libkfrfil_la_METASOURCES = AUTO

libkfrfil_la_SOURCES = filser.cpp flightrecorder.cpp 


EXTRA_DIST = fil.desktop flightrecorder.cpp filser.cpp filser.h 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kflog/logger/
	$(INSTALL_DATA) $(srcdir)/fil.desktop $(kde_datadir)/kflog/logger/fil.desktop

uninstall-local:
	-rm -f $(kde_datadir)/kflog/logger/fil.desktop

####### kdevelop will overwrite this part!!! (end)############
