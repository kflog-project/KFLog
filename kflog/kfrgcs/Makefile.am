####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkfrgcs.la


INCLUDES = $(all_includes)


LDFLAGS =    


libkfrgcs_la_LDFLAGS = -version-info 0:0:0

libkfrgcs_la_METASOURCES = AUTO

libkfrgcs_la_SOURCES = vlconv.cpp vlapisys_linux.cpp vlapihlp.cpp vlapi2.cpp utils.cpp grecord.cpp flightrecorder.cpp dbbconv.cpp 


EXTRA_DIST = dbbconv.cpp dbbconv.h flightrecorder.cpp gcs.desktop grecord.cpp grecord.h README utils.cpp utils.h vla_support.h vlapi2.cpp vlapi2.h vlapierr.h vlapihlp.cpp vlapihlp.h vlapisys_linux.cpp vlapityp.h vlconv.cpp vlconv.h 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kflog/logger/
	$(INSTALL_DATA) $(srcdir)/gcs.desktop $(kde_datadir)/kflog/logger/gcs.desktop

uninstall-local:
	-rm -f $(kde_datadir)/kflog/logger/gcs.desktop

####### kdevelop will overwrite this part!!! (end)############
