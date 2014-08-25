###############################################################################
# KFLog4 qmake project file
#
# $Id$
###############################################################################

!win32 {
    QMAKE_LFLAGS += -rdynamic
}

QT += network \
      xml

CONFIG += qt \
    warn_on \
    debug
    
# additional define for Qt4.6
contains(QT_VERSION, ^4\\.[6]\\..*) {
    DEFINES += QT_4_6
}
    
TEMPLATE = app

# Put all generated objects into an extra directory
OBJECTS_DIR = .obj
MOC_DIR     = .obj

!win32 {
    LIBS += -ldl
}

# Map and GUI icons are handled by the Qt resource system
RESOURCES = kflog.qrc

SOURCES = \
    aboutwidget.cpp \
    airfield.cpp \
    airspace.cpp \
	  airspacelistviewitem.cpp \
    altitude.cpp \
    authdialog.cpp \
    baseflightelement.cpp \
    basemapelement.cpp \
    centertodialog.cpp \
    configmapelement.cpp \
    coordedit.cpp \
    da4record.cpp \
    dataview.cpp \
    distance.cpp \
    downloadmanager.cpp \
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
    helpwindow.cpp \
    httpclient.cpp \
    igc3ddialog.cpp \
    igc3dflightdata.cpp \
    igc3dpolyhedron.cpp \
    igc3dview.cpp \
    igc3dviewstate.cpp \
    isohypse.cpp \
    isolist.cpp \
    kflogconfig.cpp \
    kflogtreewidget.cpp \
    lineelement.cpp \
    main.cpp \
    mainwindow.cpp \
    map.cpp \
    mapcalc.cpp \
    mapconfig.cpp \
    mapcontents.cpp \
    mapcontrolview.cpp \
    mapmatrix.cpp \
    objecttree.cpp \
    openairparser.cpp \
    optimization.cpp \
    optimizationwizard.cpp \
    projectionbase.cpp \
    projectioncylindric.cpp \
    projectionlambert.cpp \
    radiopoint.cpp \
    recorderdialog.cpp \
    rowdelegate.cpp \
    runway.cpp \
    singlepoint.cpp \
    taskdataprint.cpp \
    taskdialog.cpp \
    tasklistviewitem.cpp \
    topolegend.cpp \
    waypoint.cpp \
    waypointcatalog.cpp \
    waypointdialog.cpp \
    waypointimpfilterdialog.cpp \
    waypointtreeview.cpp \
    welt2000.cpp \
    wgspoint.cpp \
    whatsthat.cpp
    
HEADERS = \
    aboutwidget.h \
    airfield.h \
    airspace.h \
	  airspacelistviewitem.h \
	  airspacewarningdistance.h \
    altitude.h \
    authdialog.h \
    baseflightelement.h \
    basemapelement.h \
    centertodialog.h \
    configmapelement.h \
    coordedit.h \
    da4record.h \
    dataview.h \
    distance.h \
    downloadmanager.h \
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
    flightpoint.h \
    flightrecorderpluginbase.h \
    flightselectiondialog.h \
    flighttask.h \
    frstructs.h \
    gliders.h \
    helpwindow.h \
    httpclient.h \
    igc3ddialog.h \
    igc3dflightdata.h \
    igc3dpolyhedron.h \
    igc3dview.h \
    igc3dviewstate.h \
    isolist.h \
    isohypse.h \
    kflogconfig.h \
    kflogtreewidget.h \
    lineelement.h \
    mainwindow.h \
    map.h \
    mapcalc.h \
    mapconfig.h \
    mapcontents.h \
    mapcontrolview.h \
    mapdefaults.h \
    mapmatrix.h \
    objecttree.h \
    openairparser.h \
    optimization.h \
    optimizationwizard.h \
    projectionbase.h \
    projectioncylindric.h \
    projectionlambert.h \
    radiopoint.h \
    recorderdialog.h \
    rowdelegate.h \
    singlepoint.h \
    resource.h \
    runway.h \
    taskdataprint.h \
    taskdialog.h \
    tasklistviewitem.h \
    topolegend.h \
    waypoint.h \
    waypointcatalog.h \
    waypointdialog.h \
    waypointimpfilterdialog.h \
    waypointtreeview.h \
    welt2000.h \
    wgspoint.h \
    whatsthat.h
    
# Note! qmake do prefix the .path variable with $(INSTALL_ROOT)
# in the generated makefile. If the .path variable starts not with
# a slash, $(INSTALL_ROOT) followed by the current path is added as
# prefix to it.

landscape.path  = /mapdata/landscape
landscape.files = ../README-MAP

airfields.path  = /mapdata/airfields
airfields.files = ../README-AIRFIELDS

airspaces.path  = /mapdata/airspaces
airspaces.files = ../README-AIRSPACE

INSTALLS += landscape airfields airspaces

DESTDIR = ../release/bin
