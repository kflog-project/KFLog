####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kflog

## INCLUDES were found outside kdevelop specific part

kflog_SOURCES = downloadlist.cpp topolegend.cpp optimizationwizard.cpp waypoint.cpp flightrecorderpluginbase.cpp optimization.cpp waypointdict.cpp taskdialog.cpp centertodialog.cpp tasks.cpp waypoints.cpp waypointlist.cpp waypointimpfilterdialog.cpp waypointdialog.cpp waypointcatalog.cpp translationlist.cpp translationelement.cpp taskdataprint.cpp singlepoint.cpp recorderdialog.cpp radiopoint.cpp projectionlambert.cpp projectioncylindric.cpp projectionbase.cpp olcdialog.cpp mapprint.cpp mapmatrix.cpp mapcontrolview.cpp mapcontents.cpp mapconfig.cpp mapcalc.cpp map.cpp lineelement.cpp kflogstartlogo.cpp kflogconfig.cpp isohypse.cpp igcpreview.cpp igc3dviewstate.cpp igc3dview.cpp igc3dpolyhedron.cpp igc3dflightdata.cpp igc3ddialog.cpp glidersite.cpp flighttask.cpp flightselectiondialog.cpp flightgroup.cpp flightdataprint.cpp flight.cpp evaluationview.cpp evaluationframe.cpp evaluationdialog.cpp dataview.cpp configprintelement.cpp configdrawelement.cpp basemapelement.cpp baseflightelement.cpp airspace.cpp airport.cpp kflog.cpp main.cpp 
kflog_LDADD   = ./guicontrols/libguicontrols.a ./kfrgcs/libkfrgcs.la ./kfrgmn/libkfrgmn.la ./kfrfil/libkfrfil.la ./kfrxsp/libkfrxsp.la ./kfrxcu/libkfrxcu.la -lkdeprint -lm $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = guicontrols kfrgcs kfrgmn map-icons pics kfrfil kfrxsp kfrxcu 

EXTRA_DIST = main.cpp kflog.cpp kflog.h kflog.desktop airport.cpp airport.h airspace.cpp airspace.h baseflightelement.cpp baseflightelement.h basemapelement.cpp basemapelement.h configdrawelement.cpp configdrawelement.h configprintelement.cpp configprintelement.h dataview.cpp dataview.h evaluationdialog.cpp evaluationdialog.h evaluationframe.cpp evaluationframe.h evaluationview.cpp evaluationview.h flight.cpp flight.h flightdataprint.cpp flightdataprint.h flightgroup.cpp flightgroup.h flightselectiondialog.cpp flightselectiondialog.h flighttask.cpp flighttask.h glidersite.cpp glidersite.h frstructs.h hi16-action-centerflight.png hi16-action-centertask.png hi16-action-flightevaluation.png hi16-action-stepback.png hi16-action-stepforward.png hi16-action-waypoint.png hi16-app-kflog.png hi16-mimetype-igc.png hi22-action-centerflight.png hi22-action-centertask.png hi22-action-flightevaluation.png hi22-action-movemap_e.png hi22-action-movemap_n.png hi22-action-movemap_ne.png hi22-action-movemap_nw.png hi22-action-movemap_s.png hi22-action-movemap_se.png hi22-action-movemap_sw.png hi22-action-movemap_w.png hi22-action-stepback.png hi22-action-waypoint.png hi22-action-stepforward.png hi32-action-flightevaluation.png hi32-action-projection.png hi32-app-kflog.png hi32-mimetype-igc.png hi48-app-kflog.png hi48-mimetype-igc.png igc3ddialog.cpp igc3ddialog.h igc3dflightdata.cpp igc3dflightdata.h igc3dpolyhedron.cpp igc3dpolyhedron.h igc3dview.cpp igc3dview.h igc3dviewstate.cpp igc3dviewstate.h igcpreview.h igcpreview.cpp isohypse.cpp isohypse.h kflogconfig.cpp kflogconfig.h kflogrc kflogstartlogo.cpp kflogstartlogo.h kflogui.rc lineelement.cpp lineelement.h map.cpp map.h mapcalc.cpp mapcalc.h mapconfig.cpp mapconfig.h mapcontents.cpp mapcontents.h mapcontrolview.cpp mapcontrolview.h mapdefaults.h mapmatrix.h mapmatrix.cpp mapprint.cpp mapprint.h olcdialog.cpp olcdialog.h projectionbase.cpp projectioncylindric.cpp projectionbase.h projectioncylindric.h projectionlambert.cpp projectionlambert.h radiopoint.cpp radiopoint.h recorderdialog.cpp recorderdialog.h resource.h runway.h singlepoint.cpp singlepoint.h taskdataprint.cpp taskdataprint.h translationelement.cpp translationelement.h translationlist.cpp translationlist.h waypointcatalog.cpp waypointcatalog.h waypointdialog.cpp waypointdialog.h waypointimpfilterdialog.cpp waypointimpfilterdialog.h waypointlist.cpp waypointlist.h waypoints.cpp waypoints.h wp.h tasks.cpp tasks.h hi16-action-save_all.png hi22-action-save_all.png centertodialog.cpp centertodialog.h hi22-action-centerwaypoint.png hi16-action-centerwaypoint.png hi22-action-centerto.png taskdialog.cpp taskdialog.h hi22-action-task.png hi16-action-task.png waypointdict.cpp waypointdict.h hi48-mimetype-declaration.png hi48-mimetype-task.png hi48-mimetype-waypoint.png hi48-mimetype-pilot.png optimization.cpp optimization.h flightrecorderpluginbase.cpp flightrecorderpluginbase.h waypoint.cpp waypoint.h optimizationwizard.cpp optimizationwizard.h topolegend.cpp topolegend.h wgspoint.h gliders.h downloadlist.cpp downloadlist.h contestlist.h contests.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kflog.desktop $(kde_appsdir)/Applications/kflog.desktop
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-centerflight.png $(kde_icondir)/hicolor/16x16/actions/centerflight.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-centertask.png $(kde_icondir)/hicolor/16x16/actions/centertask.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-flightevaluation.png $(kde_icondir)/hicolor/16x16/actions/flightevaluation.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-stepback.png $(kde_icondir)/hicolor/16x16/actions/stepback.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-stepforward.png $(kde_icondir)/hicolor/16x16/actions/stepforward.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-waypoint.png $(kde_icondir)/hicolor/16x16/actions/waypoint.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-kflog.png $(kde_icondir)/hicolor/16x16/apps/kflog.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi16-mimetype-igc.png $(kde_icondir)/hicolor/16x16/mimetypes/igc.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-centerflight.png $(kde_icondir)/hicolor/22x22/actions/centerflight.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-centertask.png $(kde_icondir)/hicolor/22x22/actions/centertask.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-flightevaluation.png $(kde_icondir)/hicolor/22x22/actions/flightevaluation.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_e.png $(kde_icondir)/hicolor/22x22/actions/movemap_e.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_n.png $(kde_icondir)/hicolor/22x22/actions/movemap_n.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_ne.png $(kde_icondir)/hicolor/22x22/actions/movemap_ne.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_nw.png $(kde_icondir)/hicolor/22x22/actions/movemap_nw.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_s.png $(kde_icondir)/hicolor/22x22/actions/movemap_s.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_se.png $(kde_icondir)/hicolor/22x22/actions/movemap_se.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_sw.png $(kde_icondir)/hicolor/22x22/actions/movemap_sw.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-movemap_w.png $(kde_icondir)/hicolor/22x22/actions/movemap_w.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-stepback.png $(kde_icondir)/hicolor/22x22/actions/stepback.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-waypoint.png $(kde_icondir)/hicolor/22x22/actions/waypoint.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-stepforward.png $(kde_icondir)/hicolor/22x22/actions/stepforward.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/actions/
	$(INSTALL_DATA) $(srcdir)/hi32-action-flightevaluation.png $(kde_icondir)/hicolor/32x32/actions/flightevaluation.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/actions/
	$(INSTALL_DATA) $(srcdir)/hi32-action-projection.png $(kde_icondir)/hicolor/32x32/actions/projection.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-kflog.png $(kde_icondir)/hicolor/32x32/apps/kflog.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi32-mimetype-igc.png $(kde_icondir)/hicolor/32x32/mimetypes/igc.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/hi48-app-kflog.png $(kde_icondir)/hicolor/48x48/apps/kflog.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi48-mimetype-igc.png $(kde_icondir)/hicolor/48x48/mimetypes/igc.png
	$(mkinstalldirs) $(kde_datadir)/kflog/
	$(INSTALL_DATA) $(srcdir)/kflogrc $(kde_datadir)/kflog/kflogrc
	$(mkinstalldirs) $(kde_datadir)/kflog/
	$(INSTALL_DATA) $(srcdir)/kflogui.rc $(kde_datadir)/kflog/kflogui.rc
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-save_all.png $(kde_icondir)/hicolor/16x16/actions/save_all.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-save_all.png $(kde_icondir)/hicolor/22x22/actions/save_all.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-centerwaypoint.png $(kde_icondir)/hicolor/22x22/actions/centerwaypoint.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-centerwaypoint.png $(kde_icondir)/hicolor/16x16/actions/centerwaypoint.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-centerto.png $(kde_icondir)/hicolor/22x22/actions/centerto.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/actions/
	$(INSTALL_DATA) $(srcdir)/hi22-action-task.png $(kde_icondir)/hicolor/22x22/actions/task.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/actions/
	$(INSTALL_DATA) $(srcdir)/hi16-action-task.png $(kde_icondir)/hicolor/16x16/actions/task.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi48-mimetype-declaration.png $(kde_icondir)/hicolor/48x48/mimetypes/declaration.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi48-mimetype-task.png $(kde_icondir)/hicolor/48x48/mimetypes/task.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi48-mimetype-waypoint.png $(kde_icondir)/hicolor/48x48/mimetypes/waypoint.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/mimetypes/
	$(INSTALL_DATA) $(srcdir)/hi48-mimetype-pilot.png $(kde_icondir)/hicolor/48x48/mimetypes/pilot.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kflog.desktop
	-rm -f $(kde_icondir)/hicolor/16x16/actions/centerflight.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/centertask.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/flightevaluation.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/stepback.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/stepforward.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/waypoint.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kflog.png
	-rm -f $(kde_icondir)/hicolor/16x16/mimetypes/igc.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/centerflight.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/centertask.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/flightevaluation.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_e.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_n.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_ne.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_nw.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_s.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_se.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_sw.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/movemap_w.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/stepback.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/waypoint.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/stepforward.png
	-rm -f $(kde_icondir)/hicolor/32x32/actions/flightevaluation.png
	-rm -f $(kde_icondir)/hicolor/32x32/actions/projection.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kflog.png
	-rm -f $(kde_icondir)/hicolor/32x32/mimetypes/igc.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kflog.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/igc.png
	-rm -f $(kde_datadir)/kflog/kflogrc
	-rm -f $(kde_datadir)/kflog/kflogui.rc
	-rm -f $(kde_icondir)/hicolor/16x16/actions/save_all.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/save_all.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/centerwaypoint.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/centerwaypoint.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/centerto.png
	-rm -f $(kde_icondir)/hicolor/22x22/actions/task.png
	-rm -f $(kde_icondir)/hicolor/16x16/actions/task.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/declaration.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/task.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/waypoint.png
	-rm -f $(kde_icondir)/hicolor/48x48/mimetypes/pilot.png

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kflog_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE
#rcdir = $(kde_datadir)/kflog
#rc_DATA = kflogui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kflog.pot; \
	fi

