CONFIG += qt \
    warn_on \
    release

TEMPLATE = app

SOURCES = airport.cpp \
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
    glidersite.cpp \
    helpwindow.cpp \
    igc3ddialog.cpp \
    igc3dflightdata.cpp \
    igc3dpolyhedron.cpp \
    igc3dview.cpp \
    igc3dviewstate.cpp \
    igcpreview.cpp \
    isohypse.cpp \
    kflog.cpp \
    kflogconfig.cpp \
    kflogstartlogo.cpp \
    lineelement.cpp \
    main.cpp \
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
    waypointdict.cpp \
    waypointimpfilterdialog.cpp \
    waypointlist.cpp \
    waypoints.cpp \
    welt2000.cpp \
    whatsthat.cpp \
    guicontrols/coordedit.cpp \
    guicontrols/kfloglistview.cpp

HEADERS = airport.h \
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
    filetools.h \
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
    kflog.h \
    kflogconfig.h \
    kflogstartlogo.h \
    lineelement.h \
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
    waypointdict.h \
    waypointimpfilterdialog.h \
    waypointlist.h \
    waypoints.h \
    welt2000.h \
    wgspoint.h \
    whatsthat.h \
    wp.h \
    guicontrols/coordedit.h \
    guicontrols/kfloglistview.h

pics.extra = mkdir -p  ~/.kflog/pics/; \
    cp -r ./pics/* ~/.kflog/pics; \
    mkdir -p ~/.kflog/map-icons/; \
    cp -r ./map-icons/* ~/.kflog/map-icons; \
    mkdir -p ~/.kflog/mapdata/landscape; \
    cp ../README-AIRSPACE ~/.kflog/mapdata/landscape/README-MAP; \
    mkdir -p ~/.kflog/mapdata/airfields; \
    cp ../README-AIRSPACE ~/.kflog/mapdata/airfields/README-AIRFIELDS; \
    mkdir -p ~/.kflog/mapdata/airspaces; \
    cp ../README-AIRSPACE ~/.kflog/mapdata/airspaces/README-AIRSPACE
pics.path = ./pics
pics.files = 
INSTALLS += pics

DESTDIR = ../release

