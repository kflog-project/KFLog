###############################################################################
#
# KFLog qmake project file for Qt5
#
# Copyright (c): 2008-2023 Axel Pauli
#
###############################################################################

# version check for Qt 4.8
contains(QT_VERSION, ^4\\.[8]\\..*) {
  MY_QT_REL=4_8
}

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets \
        printsupport
  
  DEFINES += QT_5
}

# Version check for both Qt 4.x releases. One must be installed, if Qt 5.x is not
# installed.
lessThan(QT_MAJOR_VERSION, 5) {
  ! contains(MY_QT_REL, 4_8) {
  message("Cannot build KFLog with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.8 or higher!")
  }
}

TEMPLATE = app

QT += network \
      xml

!win32 {
  QMAKE_LFLAGS += -rdynamic
  QMAKE_CXXFLAGS += -Wall -Wextra
  
  CONFIG += qt \
      warn_on \
      debug
      
  # Put all generated objects into an extra directory
  OBJECTS_DIR = .obj
  MOC_DIR     = .obj
  
  LIBS += -ldl
}

win32 {
  CONFIG += qt \
      warn_on
}

TRANSLATIONS += kflog_de.ts

# Map and GUI icons are handled by the Qt resource system
RESOURCES = kflog.qrc

win32:RC_FILE = kflog.rc

SOURCES = \
    aboutwidget.cpp \
    airfield.cpp \
    AirfieldSelectionList.cpp \
    airspace.cpp \
    AirspaceHelper.cpp \
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
    filetools.cpp \
    flight.cpp \
    flightdataprint.cpp \
    flightgroup.cpp \
    flightgrouplistviewitem.cpp \
    flightlistviewitem.cpp \
    flightloader.cpp \
    flightrecorderpluginbase.cpp \
    flightselectiondialog.cpp \
    flighttask.cpp \
    Frequency.cpp \
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
    MessageHelpBox.cpp \
    objecttree.cpp \
    OpenAip.cpp \
    OpenAipPoiLoader.cpp \
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
    Speed.cpp \
    taskdataprint.cpp \
    TaskEditor.cpp \
    tasklistviewitem.cpp \
    topolegend.cpp \
    ThermalPoint.cpp \
    waypoint.cpp \
    waypointcatalog.cpp \
    waypointdialog.cpp \
    waypointimpfilterdialog.cpp \
    waypointtreeview.cpp \
    wgspoint.cpp \
    whatsthat.cpp
    
HEADERS = \
    aboutwidget.h \
    airfield.h \
    AirfieldSelectionList.h \
    airspace.h \
    AirspaceHelper.h \
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
    filetools.h \
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
    Frequency.h \
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
    MessageHelpBox.h \
    MetaTypes.h \
    objecttree.h \
    OpenAip.h \
    OpenAipPoiLoader.h \
    openairparser.h \
    optimization.h \
    optimizationwizard.h \
    projectionbase.h \
    projectioncylindric.h \
    projectionlambert.h \
    radiopoint.h \
    recorderdialog.h \
    resource.h \
    rowdelegate.h \
    runway.h \
    singlepoint.h \
    Speed.h \
    ThermalPoint.h \
    taskdataprint.h \
    TaskEditor.h \
    tasklistviewitem.h \
    topolegend.h \
    waypoint.h \
    waypointcatalog.h \
    waypointdialog.h \
    waypointimpfilterdialog.h \
    waypointtreeview.h \
    wgspoint.h \
    whatsthat.h
    
# Note! qmake do prefix the .path variable with $(INSTALL_ROOT)
# in the generated makefile. If the .path variable starts not with
# a slash, $(INSTALL_ROOT) followed by the current path is added as
# prefix to it.

landscape.path  = /mapdata/landscape
landscape.files = ../README-LANDSCAPE

points.path  = /mapdata/points
points.files = ../README-POINTS

airspaces.path  = /mapdata/airspaces
airspaces.files = ../README-AIRSPACES

translations.path  = /translations
translations.files = ../README-TRANSLATIONS

INSTALLS += landscape points airspaces translations

!win32 {
DESTDIR = ../release/bin
}

OTHER_FILES += \
    kflog.rc
