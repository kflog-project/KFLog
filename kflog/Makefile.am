####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kflog
kflog_SOURCES = evaluationview.cpp evaluationdialog.cpp kflogconfig.cpp printdialog.cpp kflogstartlogo.cpp flighttask.cpp flight.cpp mapmatrix.cpp singlepoint.cpp radiopoint.cpp lineelement.cpp isohypse.cpp glidersite.cpp elevpoint.cpp airspace.cpp airport.cpp mapcontents.cpp basemapelement.cpp map.cpp dataview.cpp mapcalc.cpp mapcontrolview.cpp kflog.cpp main.cpp 
kflog_LDADD   =  -lm $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = main.cpp kflog.cpp kflog.h kflogui.rc kflog.desktop mapcontrolview.h mapcontrolview.cpp mapcalc.cpp mapcalc.h wp.h dataview.cpp dataview.h map.cpp map.h basemapelement.cpp basemapelement.h mapcontents.cpp mapcontents.h resource.h airport.cpp airport.h airspace.cpp airspace.h elevpoint.cpp elevpoint.h glidersite.cpp glidersite.h isohypse.cpp isohypse.h lineelement.cpp lineelement.h radiopoint.cpp radiopoint.h singlepoint.cpp singlepoint.h mapmatrix.cpp mapmatrix.h flight.cpp flight.h flighttask.cpp flighttask.h kflogstartlogo.cpp kflogstartlogo.h printdialog.h printdialog.cpp kflogconfig.cpp kflogconfig.h mapdefaults.h evaluationdialog.cpp evaluationdialog.h evaluationview.cpp evaluationview.h hi16-app-kflog.png hi32-app-kflog.png hi48-app-kflog.png hi16-mimetype-x-igc.png hi32-mimetype-x-igc.png hi48-mimetype-x-igc.png x-igc.desktop hi22-action-movemap_e.png hi22-action-movemap_n.png hi22-action-movemap_ne.png hi22-action-movemap_nw.png hi22-action-movemap_s.png hi22-action-movemap_se.png hi22-action-movemap_sw.png hi22-action-movemap_w.png hi16-action-flightevaluation.png hi22-action-flightevaluation.png hi32-action-flightevaluation.png hi16-action-centerflight.png hi16-action-centertask.png hi22-action-centerflight.png hi22-action-centertask.png 

install-data-local:
	$(mkinstalldirs) $$(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kflog.desktop $$(kde_appsdir)/Applications/kflog.desktop
	$(mkinstalldirs) $$(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-kflog.png $$(kde_icondir)/hicolor/16x16/apps/kflog.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-kflog.png $$(kde_icondir)/hicolor/32x32/apps/kflog.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/hi48-app-kflog.png $$(kde_icondir)/hicolor/48x48/apps/kflog.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/16x16/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi16-mimetype-x-igc.png $$(kde_icondir)/hicolor/16x16/mimetypes/x-igc.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/32x32/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi32-mimetype-x-igc.png $$(kde_icondir)/hicolor/32x32/mimetypes/x-igc.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi48-mimetype-x-igc.png $$(kde_icondir)/hicolor/48x48/mimetypes/x-igc.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_e.png $$(kde_icondir)/hicolor/22x22/action/movemap_e.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_n.png $$(kde_icondir)/hicolor/22x22/action/movemap_n.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_ne.png $$(kde_icondir)/hicolor/22x22/action/movemap_ne.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_nw.png $$(kde_icondir)/hicolor/22x22/action/movemap_nw.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_s.png $$(kde_icondir)/hicolor/22x22/action/movemap_s.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_se.png $$(kde_icondir)/hicolor/22x22/action/movemap_se.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_sw.png $$(kde_icondir)/hicolor/22x22/action/movemap_sw.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_w.png $$(kde_icondir)/hicolor/22x22/action/movemap_w.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/16x16/action/
	$(INSTALL_DATA) $(srcdir)/hi16-action-flightevaluation.png $$(kde_icondir)/hicolor/16x16/action/flightevaluation.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-flightevaluation.png $$(kde_icondir)/hicolor/22x22/action/flightevaluation.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/32x32/action/
	$(INSTALL_DATA) $(srcdir)/hi32-action-flightevaluation.png $$(kde_icondir)/hicolor/32x32/action/flightevaluation.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/16x16/action/
	$(INSTALL_DATA) $(srcdir)/hi16-action-centerflight.png $$(kde_icondir)/hicolor/16x16/action/centerflight.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/16x16/action/
	$(INSTALL_DATA) $(srcdir)/hi16-action-centertask.png $$(kde_icondir)/hicolor/16x16/action/centertask.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-centerflight.png $$(kde_icondir)/hicolor/22x22/action/centerflight.png
	$(mkinstalldirs) $$(kde_icondir)/hicolor/22x22/action/
	$(INSTALL_DATA) $(srcdir)/hi22-action-centertask.png $$(kde_icondir)/hicolor/22x22/action/centertask.png

uninstall-local:
	-rm -f $$(kde_appsdir)/Applications/kflog.desktop
	-rm -f $$(kde_icondir)/hicolor/16x16/apps/kflog.png
	-rm -f $$(kde_icondir)/hicolor/32x32/apps/kflog.png
	-rm -f $$(kde_icondir)/hicolor/48x48/apps/kflog.png
	-rm -f $$(kde_icondir)/hicolor/16x16/mimetypes/x-igc.png
	-rm -f $$(kde_icondir)/hicolor/32x32/mimetypes/x-igc.png
	-rm -f $$(kde_icondir)/hicolor/48x48/mimetypes/x-igc.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_e.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_n.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_ne.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_nw.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_s.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_se.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_sw.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/movemap_w.png
	-rm -f $$(kde_icondir)/hicolor/16x16/action/flightevaluation.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/flightevaluation.png
	-rm -f $$(kde_icondir)/hicolor/32x32/action/flightevaluation.png
	-rm -f $$(kde_icondir)/hicolor/16x16/action/centerflight.png
	-rm -f $$(kde_icondir)/hicolor/16x16/action/centertask.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/centerflight.png
	-rm -f $$(kde_icondir)/hicolor/22x22/action/centertask.png

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

