# KFLog4 qmake project file
# $Id$

QT += qt3support \
      network \
      xml

CONFIG += qt \
    warn_on \
    debug
    
TEMPLATE = app

SOURCES = \
    airport.cpp \
    airspace.cpp \
    baseflightelement.cpp \
    basemapelement.cpp \
    centertodialog.cpp \
    configdrawelement.cpp \
    configprintelement.cpp \
    da4record.cpp \
    dataview.cpp \
    distance.cpp \
    downloadlist.cpp \
    elevationfinder.cpp \
    evaluationdialog.cpp \
    evaluationframe.cpp \
    evaluationview.cpp \
    flight.cpp \
    flightdataprint.cpp \
    flightgroup.cpp \
    flightgrouplistviewitem.cpp \
    flightlistviewitem.cpp \
    flightloader.cpp \
    flightrecorderpluginbase.cpp \
    flightselectiondialog.cpp \
    flighttask.cpp \
    glidersite.cpp \
    helpwindow.cpp \
    igc3ddialog.cpp \
    igc3dflightdata.cpp \
    igc3dpolyhedron.cpp \
    igc3dview.cpp \
    igc3dviewstate.cpp \
    igcpreview.cpp \
    isohypse.cpp \
    kflogconfig.cpp \
    kflogstartlogo.cpp \
    lineelement.cpp \
    main.cpp \
    mainwindow.cpp \
    map.cpp \
    mapcalc.cpp \
    mapconfig.cpp \
    mapcontents.cpp \
    mapcontrolview.cpp \
    mapmatrix.cpp \
    mapprint.cpp \
    objecttree.cpp \
    openairparser.cpp \
    optimization.cpp \
    optimizationwizard.cpp \
    projectionbase.cpp \
    projectioncylindric.cpp \
    projectionlambert.cpp \
    radiopoint.cpp \
    recorderdialog.cpp \
    singlepoint.cpp \
    taskdataprint.cpp \
    taskdialog.cpp \
    tasklistviewitem.cpp \
    topolegend.cpp \
    translationelement.cpp \
    translationlist.cpp \
    waypoint.cpp \
    waypointcatalog.cpp \
    waypointdialog.cpp \
    waypointimpfilterdialog.cpp \
    waypoints.cpp \
    welt2000.cpp \
    whatsthat.cpp \
    guicontrols/coordedit.cpp \
    guicontrols/kfloglistview.cpp
    
HEADERS = \
    airport.h \
    airspace.h \
    baseflightelement.h \
    basemapelement.h \
    centertodialog.h \
    configdrawelement.h \
    configprintelement.h \
    da4record.h \
    dataview.h \
    distance.h \
    downloadlist.h \
    elevationfinder.h \
    evaluationdialog.h \
    evaluationframe.h \
    evaluationview.h \
    flight.h \
    flightdataprint.h \
    flightgroup.h \
    flightgrouplistviewitem.h \
    flightlistviewitem.h \
    flightloader.h \
    flightrecorderpluginbase.h \
    flightselectiondialog.h \
    flighttask.h \
    frstructs.h \
    gliders.h \
    glidersite.h \
    helpwindow.h \
    igc3ddialog.h \
    igc3dflightdata.h \
    igc3dpolyhedron.h \
    igc3dview.h \
    igc3dviewstate.h \
    igcpreview.h \
    isohypse.h \
    kflogconfig.h \
    kflogstartlogo.h \
    lineelement.h \
    mainwindow.h \
    map.h \
    mapcalc.h \
    mapconfig.h \
    mapcontents.h \
    mapcontrolview.h \
    mapdefaults.h \
    mapmatrix.h \
    mapprint.h \
    objecttree.h \
    openairparser.h \
    optimization.h \
    optimizationwizard.h \
    projectionbase.h \
    projectioncylindric.h \
    projectionlambert.h \
    radiopoint.h \
    recorderdialog.h \
    singlepoint.h \
    resource.h \
    runway.h \
    taskdataprint.h \
    taskdialog.h \
    tasklistviewitem.h \
    topolegend.h \
    translationelement.h \
    translationlist.h \
    waypoint.h \
    waypointcatalog.h \
    waypointdialog.h \
    waypointimpfilterdialog.h \
    waypoints.h \
    welt2000.h \
    wgspoint.h \
    whatsthat.h \
    wp.h \
    guicontrols/coordedit.h \
    guicontrols/kfloglistview.h
    
FORMS = optimizationwizard.ui

# Note! qmake do prefix the .path variable with $(INSTALL_ROOT)
# in the generated makefile. If the .path variable starts not with
# a slash, $(INSTALL_ROOT) followed by the current path is added as
# prefix to it.

pics.path  = /pics
pics.files = pics/*

map-icons.extra = install -d $(INSTALL_ROOT)/map-icons/small
map-icons.path  = /map-icons
map-icons.files = map-icons

landscape.path  = /mapdata/landscape
landscape.files = ../README-MAP

airfields.path  = /mapdata/airfields
airfields.files = ../README-AIRFIELDS

airspaces.path  = /mapdata/airspaces
airspaces.files = ../README-AIRSPACE

INSTALLS += pics map-icons landscape airfields airspaces

DESTDIR = ../release
