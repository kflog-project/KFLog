####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkfrgcs.la


INCLUDES = $(all_includes)


LDFLAGS =   


libkfrgcs_la_LDFLAGS = -version-info 0:0:0

libkfrgcs_la_METASOURCES = AUTO

libkfrgcs_la_SOURCES = dbbconv.cpp flightrecorder.cpp grecord.cpp utils.cpp vlconv.cpp vlapisys_linux.cpp vlapihlp.cpp vlapi2.cpp 


EXTRA_DIST = vlapi2.cpp vlapi2.h vlapihlp.cpp vlapisys_linux.cpp vlconv.cpp utils.cpp grecord.cpp flightrecorder.cpp dbbconv.cpp dbbconv.h grecord.h utils.h vla_support.h vlapierr.h vlapihlp.h vlapityp.h vlconv.h README gcs.desktop 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kflog/logger/
	$(INSTALL_DATA) $(srcdir)/gcs.desktop $(kde_datadir)/kflog/logger/gcs.desktop

uninstall-local:
	-rm -f $(kde_datadir)/kflog/logger/gcs.desktop

####### kdevelop will overwrite this part!!! (end)############
