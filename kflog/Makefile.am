####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kflog
kflog_SOURCES = kflogconfig.cpp printdialog.cpp kflogstartlogo.cpp flighttask.cpp flight.cpp mapmatrix.cpp singlepoint.cpp radiopoint.cpp lineelement.cpp isohypse.cpp glidersite.cpp elevpoint.cpp airspace.cpp airport.cpp mapcontents.cpp basemapelement.cpp map.cpp dataview.cpp mapcalc.cpp mapcontrolview.cpp kflog.cpp main.cpp 
kflog_LDADD   = -lm -lkfile -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = main.cpp kflog.cpp kflog.h kflogui.rc kflog.desktop lo32-app-kflog.png lo16-app-kflog.png mapcontrolview.h mapcontrolview.cpp mapcalc.cpp mapcalc.h wp.h dataview.cpp dataview.h map.cpp map.h basemapelement.cpp basemapelement.h mapcontents.cpp mapcontents.h resource.h airport.cpp airport.h airspace.cpp airspace.h elevpoint.cpp elevpoint.h glidersite.cpp glidersite.h isohypse.cpp isohypse.h lineelement.cpp lineelement.h radiopoint.cpp radiopoint.h singlepoint.cpp singlepoint.h mapmatrix.cpp mapmatrix.h flight.cpp flight.h flighttask.cpp flighttask.h kflogstartlogo.cpp kflogstartlogo.h printdialog.h printdialog.cpp kflogconfig.cpp kflogconfig.h mapdefaults.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kflog.desktop $(kde_appsdir)/Applications/kflog.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-kflog.png $(kde_icondir)/locolor/32x32/apps/kflog.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-kflog.png $(kde_icondir)/locolor/16x16/apps/kflog.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kflog.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/kflog.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/kflog.png

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kflog_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kflog
rc_DATA = kflogui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kflog.pot; \
	fi

