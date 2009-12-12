/***********************************************************************
**
**   kflog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <unistd.h>
#include <pwd.h>
#include <dlfcn.h>

// include files for QT
#include <qdir.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qgl.h>
#include <qwhatsthis.h>

// include files for KDE
#include <kaboutapplication.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <knotifyclient.h>
#include <kstdaction.h>
#include <ktip.h>
#include <ktoolbar.h>

// application specific includes
#include "kflog.h"
#include "airport.h"
#include "basemapelement.h"
#include "centertodialog.h"
#include "dataview.h"
#include "elevationfinder.h"
#include "evaluationdialog.h"
#include "evaluationview.h"
#include "flight.h"
#include "flightdataprint.h"
#include "flightloader.h"
#include "helpwindow.h"
#include "igc3ddialog.h"
#include "igcpreview.h"
#include "kflogconfig.h"
#include "kflogstartlogo.h"
#include "map.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapcontrolview.h"
#include "mapprint.h"
#include "objecttree.h"
#include "recorderdialog.h"
#include "taskdataprint.h"
#include "topolegend.h"
#include "waypoints.h"

#define STATUS_LABEL(a,b,c) \
  a = new KStatusBarLabel( "", 0, statusBar() ); \
  a->setFixedWidth( b ); \
  a->setFixedHeight( statusLabel->sizeHint().height() ); \
  a->setFrameStyle( QFrame::NoFrame | QFrame::Plain ); \
  a->setMargin(0); \
  a->setLineWidth(0); \
  a->setAlignment( c | AlignVCenter );

TranslationList surfaces;
TranslationList waypointTypes;
TranslationList taskTypes;


KFLogApp::KFLogApp()
  : KDockMainWindow(0, "KFLogMainWindow"), showStartLogo(false)
{
  extern MapConfig _globalMapConfig;
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  config = kapp->config();

  config->setGroup("General Options");
  showStartLogo=false;

  if (config->readBoolEntry("Logo", true) && (!kapp->isRestored() ) )
    {
      showStartLogo = true;
      startLogo = new KFLogStartLogo();
      startLogo->show();
    }

  // initialize internal translation lists
  initSurfaces();
  initTypes();
  initTaskTypes();
  
  ElevationFinder::instance();
  
  connect(&_globalMapMatrix, SIGNAL(displayMatrixValues(int, bool)),
      &_globalMapConfig, SLOT(slotSetMatrixValues(int, bool)));
  connect(&_globalMapMatrix, SIGNAL(printMatrixValues(int)),
      &_globalMapConfig, SLOT(slotSetPrintMatrixValues(int)));
  connect(&_globalMapConfig, SIGNAL(configChanged()), &_globalMapMatrix,
      SLOT(slotInitMatrix()));
  connect(&_globalMapContents, SIGNAL(errorOnMapLoading()),
      SLOT(slotStartComplete()));
  connect(this, SIGNAL(flightDataTypeChanged(int)), &_globalMapConfig,
      SLOT(slotSetFlightDataType(int)));
  connect(&_globalMapMatrix, SIGNAL(projectionChanged()),
      &_globalMapContents, SLOT(slotReloadMapData()));

  _globalMapConfig.slotReadConfig();

  initStatusBar();
  initView();
  if(showStartLogo && startLogo!=0)
      startLogo->raise();

  initActions();

  if(showStartLogo && startLogo!=0)
      startLogo->raise();

  readOptions();

  activateDock();

  connect(&_globalMapMatrix, SIGNAL(matrixChanged()), map,
      SLOT(slotRedrawMap()));
  connect(map, SIGNAL(showFlightPoint(const QPoint&, const flightPoint&)),
      this, SLOT(slotShowPointInfo(const QPoint&, const flightPoint&)));
  // Plannung
  connect(&_globalMapContents, SIGNAL(activatePlanning()),
     map,SLOT(slotActivatePlanning()));

  connect(&_globalMapContents, SIGNAL(taskHelp(QString)),
      helpWindow, SLOT(slotShowHelpText(QString)) );
  connect(map, SIGNAL(taskPlanningEnd()), helpWindow, SLOT(slotClearView()) );

  connect(map, SIGNAL(showTaskText(FlightTask*)),
      dataView, SLOT(slotShowTaskText(FlightTask*)));
  connect(map, SIGNAL(taskPlanningEnd()), dataView, SLOT(setFlightData()));
  connect(map, SIGNAL(showPoint(const QPoint&)),
      this, SLOT(slotShowPointInfo(const QPoint&)));
  connect(&_globalMapContents, SIGNAL(contentsChanged()),map, SLOT(slotRedrawMap()));
  slotModifyMenu();
  connect(map, SIGNAL(showPoint(const QPoint&)), evaluationWindow, SLOT(slotRemoveFlightPoint()));
  connect(map, SIGNAL(showFlightPoint(const QPoint&, const flightPoint&)),
      evaluationWindow, SLOT(slotShowFlightPoint(const QPoint&, const flightPoint&)));

  slotCheckDockWidgetStatus();
  KTipDialog::showTip(this, "kflog/tips");
}

KFLogApp::~KFLogApp()
{
  extern MapMatrix _globalMapMatrix;

  _globalMapMatrix.writeMatrixOptions();
}

void KFLogApp::initActions()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;

  // File menu

  fileNewWaypoint = new QAction(SmallIcon("waypoint"), tr("New &Waypoint"), 0, this, "file_new_waypoint");
  connect(fileNewWaypoint, SIGNAL(activated()), waypoints, SLOT(slotNewWaypoint()));
  fileNewTask = new QAction(SmallIcon("task"), tr("New &Task"), CTRL+Key_N, this, "file_new_task");
  connect(fileNewTask, SIGNAL(activated()), &_globalMapContents, SLOT(slotNewTask()));
  fileNewFlightGroup = new QAction(tr("New &Flight group"), 0, this, "file_new_flight_group");
  connect(fileNewFlightGroup, SIGNAL(activated()), &_globalMapContents, SLOT(slotNewFlightGroup()));
  QPopupMenu * fileNew = new QPopupMenu( this );
  fileNewWaypoint->addTo( fileNew );
  fileNewTask->addTo( fileNew );
  fileNewFlightGroup->addTo( fileNew );

  fileOpenFlight = new QAction(SmallIcon("fileopen"), tr("&Open Flight"), CTRL+Key_O, this, "file_open_flight");
  connect(fileOpenFlight, SIGNAL(activated()), this, SLOT(slotOpenFile()));
  fileOpenTask = new QAction(SmallIcon("fileopen"), tr("Open &Task"), CTRL+Key_T, this, "file_open_task");
  connect(fileOpenTask, SIGNAL(activated()), this, SLOT(slotOpenTask()));

  fileOpenRecent = new QPopupMenu( this );
  QStringList datalist;
  config->setGroup("General Options");
  datalist = config->readListEntry("RecentFiles");
  int size = std::min((int)datalist.size(), 5);
  QAction *recentFileActs[size];
  for (int i = 0; i < size; ++i) {
    recentFileActs[i] = new QAction(this);
    recentFileActs[i]->setToolTip(datalist[i]); // FIXME: in Qt4 setData must be used
    recentFileActs[i]->setText(tr("%1 %2").arg(i + 1).arg(QFileInfo(datalist[i]).fileName()));
    connect(recentFileActs[i], SIGNAL(activated()), this, SLOT(slotOpenRecentFile()));
    recentFileActs[i]->addTo(fileOpenRecent);
  }

  fileClose = new QAction(SmallIcon("cancel"), tr("&Close Flight/Task"), CTRL+Key_W, this, "file_close");
  connect(fileClose, SIGNAL(activated()), &_globalMapContents, SLOT(closeFlight()));

  fileSavePixmap = new QAction(SmallIcon("image"), tr("Export to PNG..."), 0, this, "file_export_pixmap");
  connect(fileSavePixmap, SIGNAL(activated()), map, SLOT(slotSavePixmap()));
  filePrint = new QAction(SmallIcon("fileprint"), tr("Print..."), CTRL+Key_P, this, "file_print");
  connect(filePrint, SIGNAL(activated()), this, SLOT(slotFilePrint()));
  filePrintFlight = new QAction(SmallIcon("fileprint"), tr("Print Flightdata"), 0, this, "file_print_flightdata");
  connect(filePrintFlight, SIGNAL(activated()), this, SLOT(slotFlightPrint()));

  fileOpenRecorder = new QAction(SmallIcon("connect_no"), tr("Open Recorder"), 0, this, "file_open_recorder");
  connect(fileOpenRecorder, SIGNAL(activated()), this, SLOT(slotOpenRecorderDialog()));

  fileQuit = new QAction(SmallIcon("exit"), "&Quit", CTRL+Key_Q, this, "file_quit");
  connect(fileQuit, SIGNAL(activated()), qApp, SLOT(closeAllWindows()));
  connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );

  QPopupMenu * file = new QPopupMenu( this );
  menuBar()->insertItem( "&File", file );
  file->insertItem(SmallIcon("filenew"), "&New", fileNew);
  fileOpenFlight->addTo( file );
  fileOpenTask->addTo( file );
  file->insertItem(SmallIcon("fileopen"), "&Open recent files", fileOpenRecent);
  fileClose->addTo( file );
  file->insertSeparator();
  fileSavePixmap->addTo( file );
  file->insertSeparator();
  filePrint->addTo( file );
  filePrintFlight->addTo( file );
  file->insertSeparator();
  fileOpenRecorder->addTo( file );
  file->insertSeparator();
  fileQuit->addTo( file );

  // view menu

  viewCenterTask = new QAction(SmallIcon("centertask"), tr("Center to &Task"), Key_F6, this, "view_center_task");
  connect(viewCenterTask, SIGNAL(activated()), map, SLOT(slotCenterToTask()));
  viewCenterFlight = new QAction(SmallIcon("centerflight"), tr("Center to &Flight"), Key_F7, this, "view_center_flight");
  connect(viewCenterFlight, SIGNAL(activated()), map, SLOT(slotCenterToFlight()));
  viewCenterHomesite = new QAction(SmallIcon("gohome"), tr("Center to &Homesite"), CTRL+Key_Home, this, "view_center_homeside");
  connect(viewCenterHomesite, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotCenterToHome()));
  viewCenterTo = new QAction(SmallIcon("centerto"), tr("&Center to..."), Key_F8, this, "view_center_to");
  connect(viewCenterTo, SIGNAL(activated()), this, SLOT(slotCenterTo()));

  viewZoomIn = new QAction(SmallIcon("viewmag+"), tr("Zoom &In"), CTRL+Key_Plus, this, "view_zoom_in");
  connect(viewZoomIn, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotZoomIn()));
  viewZoomOut = new QAction(SmallIcon("viewmag-"), tr("Zoom &Out"), CTRL+Key_Minus, this, "view_zoom_out");
  connect(viewZoomOut, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotZoomOut()));
  viewZoom = new QAction(SmallIcon("viewmagfit"), tr("&Zoom..."), 0, this, "view_zoom");
  connect(viewZoom, SIGNAL(activated()), map, SLOT(slotZoomRect()));
  viewRedraw = new QAction(SmallIcon("reload"), tr("&Redraw"), Key_F5, this, "view_redraw");
  connect(viewRedraw, SIGNAL(activated()), map, SLOT(slotRedrawMap()));

  viewMoveNW = new QAction(SmallIcon("movemap_nw"), tr("move map north-west"), Key_7, this, "view_move_nw");
  connect(viewMoveNW, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapNW()));
  viewMoveN = new QAction(SmallIcon("movemap_n"), tr("move map north"), Key_8, this, "view_move_n");
  connect(viewMoveN, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapN()));
  viewMoveNE = new QAction(SmallIcon("movemap_ne"), tr("move map north-east"), Key_9, this, "view_move_ne");
  connect(viewMoveNE, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapNE()));
  viewMoveW = new QAction(SmallIcon("movemap_w"), tr("move map west"), Key_4, this, "view_move_w");
  connect(viewMoveW, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapW()));
  viewMoveE = new QAction(SmallIcon("movemap_e"), tr("move map east"), Key_6, this, "view_move_e");
  connect(viewMoveE, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapE()));
  viewMoveSW = new QAction(SmallIcon("movemap_sw"), tr("move map south-west"), Key_1, this, "view_move_sw");
  connect(viewMoveSW, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapSW()));
  viewMoveS = new QAction(SmallIcon("movemap_s"), tr("move map south"), Key_2, this, "view_move_s");
  connect(viewMoveS, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapS()));
  viewMoveSE = new QAction(SmallIcon("movemap_se"), tr("move map south-east"), Key_3, this, "view_move_se");
  connect(viewMoveSE, SIGNAL(activated()), &_globalMapMatrix, SLOT(slotMoveMapSE()));

  QPopupMenu * viewMove = new QPopupMenu( this );
  viewMoveNW->addTo( viewMove );
  viewMoveN->addTo( viewMove );
  viewMoveNE->addTo( viewMove );
  viewMoveW->addTo( viewMove );
  viewMoveE->addTo( viewMove );
  viewMoveSW->addTo( viewMove );
  viewMoveS->addTo( viewMove );
  viewMoveSE->addTo( viewMove );

  QPopupMenu * view = new QPopupMenu( this );
  menuBar()->insertItem( "&View", view );
  viewCenterTask->addTo( view );
  viewCenterFlight->addTo( view );
  viewCenterHomesite->addTo( view );
  viewCenterTo->addTo( view );
  view->insertSeparator();
  viewZoomIn->addTo( view );
  viewZoomOut->addTo( view );
  viewZoom->addTo( view );
  viewRedraw->addTo( view );
  view->insertSeparator();
  view->insertItem(SmallIcon("move"), "Move map", viewMove);

  // flight menu

  flightEvaluationWindow = new QAction(SmallIcon("history"), tr("Show &EvaluationWindow"), CTRL+Key_E, this, "toggle_evaluation_window");
  flightEvaluationWindow->setToggleAction(true);
  connect(flightEvaluationWindow, SIGNAL(activated()), this, SLOT(slotToggleEvaluationWindow()));
  flightOptimization = new QAction(SmallIcon("wizard"), tr("Optimize"), 0, this, "optimize_flight");
  connect(flightOptimization, SIGNAL(activated()), this, SLOT(slotOptimizeFlight()));
  flightOptimizationOLC = new QAction(SmallIcon("wizard"), tr("Optimize (OLC)"), 0, this, "optimize_flight_olc");
  connect(flightOptimizationOLC, SIGNAL(activated()), this, SLOT(slotOptimizeFlightOLC()));

  flightDataType = new QPopupMenu( this );
  QStringList dataList;
  dataList.append(tr("Altitude"));
  dataList.append(tr("Cycling"));
  dataList.append(tr("Speed"));
  dataList.append(tr("Vario"));
  dataList.append(tr("Solid"));
  for (int i = 0; i < 5; ++i)
    flightDataType->insertItem(dataList[i], i);
  connect(flightDataType, SIGNAL(activated(int)), this, SLOT(slotSelectFlightData(int)));
  config-> setGroup("Flight");
  flightDataType->setItemChecked(config->readNumEntry("Draw Type", MapConfig::Altitude), true);

  flightIgc3D = new QAction(SmallIcon("vectorgfx"), tr("View flight in 3D"), CTRL+Key_R, this, "view_flight_3D");
  connect(flightIgc3D, SIGNAL(activated()), this, SLOT(slotFlightViewIgc3D()));
  flightIgcOpenGL = new QAction(SmallIcon("openglgfx"), tr("View flight in 3D (OpenGL)"), 0, this, "view_flight_opengl");
  connect(flightIgcOpenGL, SIGNAL(activated()), this, SLOT(slotFlightViewIgcOpenGL()));

  flightAnimateStart = new QAction(SmallIcon("1rightarrow"), tr("&Start Flight Animation"), Key_F12, this, "start_animate");
  connect(flightAnimateStart, SIGNAL(activated()), map, SLOT(slotAnimateFlightStart()));
  flightAnimateStop = new QAction(SmallIcon("player_stop"), tr("Stop Flight &Animation"), Key_F11, this, "stop_animate");
  connect(flightAnimateStop, SIGNAL(activated()), map, SLOT(slotAnimateFlightStop()));
  flightAnimateNext = new QAction(SmallIcon("forward"), tr("Next Flight Point"), CTRL+Key_Up, this, "next_flight_point");
  connect(flightAnimateNext, SIGNAL(activated()), map, SLOT(slotFlightNext()));
  flightAnimatePrev = new QAction(SmallIcon("back"), tr("Prev Flight Point"), CTRL+Key_Down, this, "prev_flight_point");
  connect(flightAnimatePrev, SIGNAL(activated()), map, SLOT(slotFlightPrev()));
  flightAnimate10Next = new QAction(SmallIcon("stepforward"), tr("Step +10 Flight Points"), Key_PageUp, this, "next_step_flight_point");
  connect(flightAnimate10Next, SIGNAL(activated()), map, SLOT(slotFlightStepNext()));
  flightAnimate10Prev = new QAction(SmallIcon("stepback"), tr("Step -10 Flight Points"), Key_PageDown, this, "prev_step_flight_point");
  connect(flightAnimate10Prev, SIGNAL(activated()), map, SLOT(slotFlightStepPrev()));
  flightAnimateHome = new QAction(SmallIcon("start"), tr("First Flight Point"), Key_Home, this, "first_flight_point");
  connect(flightAnimateHome, SIGNAL(activated()), map, SLOT(slotFlightHome()));
  flightAnimateEnd = new QAction(SmallIcon("finish"), tr("Last Flight Point"), Key_End, this, "last_flight_point");
  connect(flightAnimateEnd, SIGNAL(activated()), map, SLOT(slotFlightEnd()));

  QPopupMenu * flight = new QPopupMenu( this );
  menuBar()->insertItem( "F&light", flight );
  flightEvaluationWindow->addTo(flight);
  flightOptimization->addTo(flight);
  flightOptimizationOLC->addTo(flight);
  flight->insertItem(SmallIcon("idea"), tr("Show Flightdata"), flightDataType);
  flightIgc3D->addTo(flight);
  flightIgcOpenGL->addTo(flight);
  flight->insertSeparator();
  flightAnimateStart->addTo(flight);
  flightAnimateStop->addTo(flight);
  flightAnimateNext->addTo(flight);
  flightAnimatePrev->addTo(flight);
  flightAnimate10Next->addTo(flight);
  flightAnimate10Prev->addTo(flight);
  flightAnimateHome->addTo(flight);
  flightAnimateEnd->addTo(flight);

  // window menu

  windowMenu = new QPopupMenu( this );
  menuBar()->insertItem( "&Window", windowMenu );
  windowMenu->setCheckable(true);
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotWindowsMenuAboutToShow()));

  // settings menu

  settingsEvaluationWindow = new QAction(SmallIcon("history"), tr("Show &EvaluationWindow"), CTRL+Key_E, this, "toggle_evaluation_window");
  settingsEvaluationWindow->setToggleAction(true);
  connect(settingsEvaluationWindow, SIGNAL(activated()), this, SLOT(slotToggleEvaluationWindow()));
  settingsFlightData = new QAction(SmallIcon("view_detailed"), tr("Show Flight&data"), CTRL+Key_E, this, "toggle_data_view");
  settingsFlightData->setToggleAction(true);
  connect(settingsFlightData, SIGNAL(activated()), this, SLOT(slotToggleDataView()));
  settingsHelpWindow = new QAction(SmallIcon("info"), tr("Show HelpWindow"), CTRL+Key_H, this, "toggle_help_window");
  settingsHelpWindow->setToggleAction(true);
  connect(settingsHelpWindow, SIGNAL(activated()), this, SLOT(slotToggleHelpWindow()));
  settingsObjectTree = new QAction(SmallIcon("view_tree"), tr("Show KFLog&Browser"), CTRL+Key_B, this, "view_tree");
  settingsObjectTree->setToggleAction(true);
  connect(settingsObjectTree, SIGNAL(activated()), this, SLOT(slotToggleObjectTreeDock()));
  settingsLegend = new QAction(SmallIcon("blend"), tr("Show Legend"), CTRL+Key_L, this, "toggle_legend");
  settingsLegend->setToggleAction(true);
  connect(settingsLegend, SIGNAL(activated()), this, SLOT(slotToggleLegendDock()));
  settingsMap = new QAction(tr("Show &Map"), 0, this, "toggle_map");
  settingsMap->setToggleAction(true);
  connect(settingsMap, SIGNAL(activated()), this, SLOT(slotToggleMap()));
  settingsMapControl = new QAction(tr("Show Map&control"), 0, this, "toggle_map_control");
  settingsMapControl->setToggleAction(true);
  connect(settingsMapControl, SIGNAL(activated()), this, SLOT(slotToggleMapControl()));
  settingsToolBar = new QAction(tr("Show Toolbar"), 0, this, "toggle_toolbar");
  settingsToolBar->setToggleAction(true);
  connect(settingsToolBar, SIGNAL(activated()), this, SLOT(slotToggleToolBar()));
  settingsStatusBar = new QAction(tr("Show Statusbar"), 0, this, "toggle_statusbar");
  settingsStatusBar->setToggleAction(true);
  connect(settingsStatusBar, SIGNAL(activated()), this, SLOT(slotToggleStatusBar()));
  settingsWaypoints = new QAction(SmallIcon("waypoint"), tr("Show &Waypoints"), CTRL+Key_R, this, "toggle_waypoints");
  settingsWaypoints->setToggleAction(true);
  connect(settingsWaypoints, SIGNAL(activated()), this, SLOT(slotToggleWaypointsDock()));

  settings = new QPopupMenu( this );
  menuBar()->insertItem( "&Settings", settings );
  settingsEvaluationWindow->addTo(settings);
  settingsFlightData->addTo(settings);
  settingsHelpWindow->addTo(settings);
  settingsObjectTree->addTo(settings);
  settingsLegend->addTo(settings);
  settingsMap->addTo(settings);
  settingsMapControl->addTo(settings);
  settingsToolBar->addTo(settings);
  settingsStatusBar->addTo(settings);
  // We can't use CTRL-W, because this shortcut is reserved for closing a file ...
  settingsWaypoints->addTo(settings);
  settings->insertSeparator();
  settings->insertItem(SmallIcon("configure_shortcuts"), tr("Configure Shortcuts..."), this, SLOT(slotConfigureKeyBindings()));
  settings->insertItem(SmallIcon("configure_toolbars"), tr("Configure Toolbars..."), this, SLOT(slotConfigureToolbars()));
  settings->insertItem(SmallIcon("configure"), tr("KFLog &Setup..."), this, SLOT(slotConfigureKFLog()));

  // help menu

  QPopupMenu * help = new QPopupMenu( this );
  menuBar()->insertItem( "&Help", help );
  //FIXME: link to manual must be added
  help->insertItem(SmallIcon("contexthelp"), tr("What's This?"), this, SLOT(slotWhatsThis()), CTRL+Key_F1);
  //FIXME: dialog to swith application language must be added
  help->insertItem(SmallIcon("idea"), tr("Tip of the day"), this, SLOT(slotTipOfDay()));
  help->insertItem(SmallIcon("kflog"), tr("About KFLog"), this, SLOT(slotShowAbout()));

  // toolbar

  KToolBar * tb = toolBar();
  fileOpenFlight->addTo(tb);
  tb->addSeparator();
  viewZoom->addTo(tb);
  viewZoomIn->addTo(tb);
  viewZoomOut->addTo(tb);
  viewRedraw->addTo(tb);
  tb->addSeparator();
  viewCenterTask->addTo(tb);
  viewCenterFlight->addTo(tb);
  viewCenterHomesite->addTo(tb);
  tb->addSeparator();
  flightEvaluationWindow->addTo(tb);
}

void KFLogApp::initStatusBar()
{
  /* Alternativ könnte der Balken auch nur während des Zeichnens erscheinen */
  statusProgress = new KProgress(statusBar());
  statusProgress->setFixedWidth(120);
  statusProgress->setFixedHeight( statusProgress->sizeHint().height() - 4 );
  statusProgress->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusProgress->setMargin( 0 );
  statusProgress->setLineWidth(0);
  statusProgress->setBackgroundMode( QWidget::PaletteBackground );

  statusLabel = new KStatusBarLabel( "", 0, statusBar() );
  statusLabel->setFixedHeight( statusLabel->sizeHint().height() );
  statusLabel->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusLabel->setMargin(0);
  statusLabel->setLineWidth(0);

  STATUS_LABEL(statusTimeL, 80, AlignHCenter);
  STATUS_LABEL(statusAltitudeL, 80, AlignRight);
  STATUS_LABEL(statusVarioL, 80, AlignRight);
  STATUS_LABEL(statusSpeedL, 100, AlignRight);
  STATUS_LABEL(statusLatL, 110, AlignHCenter);
  STATUS_LABEL(statusLonL, 110, AlignHCenter);

  statusBar()->addWidget( statusLabel, 1, false );
  statusBar()->addWidget( statusTimeL, 0, false );
  statusBar()->addWidget( statusAltitudeL, 0, false );
  statusBar()->addWidget( statusSpeedL, 0, false );
  statusBar()->addWidget( statusVarioL, 0, false );
  statusBar()->addWidget( statusProgress, 0,  false );
  statusBar()->addWidget( statusLatL, 0, false );
  statusBar()->addWidget( statusLonL, 0, false );
}

void KFLogApp::initView()
{
  // we need icons for these parts ...
  mapViewDock = createDockWidget("Map", 0, 0, tr("Map"));
  dataViewDock = createDockWidget("Flight-Data", 0, 0, tr("Flight-Data"));
  helpWindowDock = createDockWidget("Help", 0, 0, tr("Help"));
  evaluationWindowDock = createDockWidget("Evaluation", 0, 0, tr("Evaluation"));
  mapControlDock = createDockWidget("Map-Control", 0, 0, tr("Map-Control"));
  waypointsDock = createDockWidget("Waypoints", 0, 0, tr("Waypoints"));
  legendDock = createDockWidget("Legend", 0, 0, tr("Legend"));
  objectTreeDock = createDockWidget("LoadedObjects", 0, 0, tr("KFLog Browser"));
  extern MapContents _globalMapContents;


  connect(mapControlDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideMapControlDock()));
  connect(mapControlDock, SIGNAL(hasUndocked()),
      SLOT(slotHideMapControlDock()));
  connect(mapViewDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideMapViewDock()));
  connect(mapViewDock, SIGNAL(hasUndocked()),
      SLOT(slotHideMapViewDock()));
  connect(dataViewDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideDataViewDock()));
  connect(dataViewDock, SIGNAL(hasUndocked()),
      SLOT(slotHideDataViewDock()));
  connect(helpWindowDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideHelpWindowDock()));
  connect(helpWindowDock, SIGNAL(hasUndocked()),
      SLOT(slotHideHelpWindowDock()));
  connect(evaluationWindowDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideEvaluationWindowDock()));
  connect(evaluationWindowDock, SIGNAL(hasUndocked()),
      SLOT(slotHideEvaluationWindowDock()));
  connect(waypointsDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideWaypointsDock()));
  connect(waypointsDock, SIGNAL(hasUndocked()),
      SLOT(slotHideWaypointsDock()));
   connect(legendDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideLegendDock()));
  connect(legendDock, SIGNAL(hasUndocked()),
      SLOT(slotHideLegendDock()));
  connect(objectTreeDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideObjectTreeDock()));
  connect(objectTreeDock, SIGNAL(hasUndocked()),
      SLOT(slotHideObjectTreeDock()));

  setView(mapViewDock);
  setMainDockWidget(mapViewDock);

  QFrame* mapViewFrame = new QFrame(mapViewDock);
  map = new Map(this, mapViewFrame, "KFLog-Map");

  QHBoxLayout* mapLayout = new QHBoxLayout(mapViewFrame,2,1);
  mapLayout->addWidget(map);
  mapLayout->activate();

  QFrame* mapControlFrame = new QFrame(mapControlDock);
  mapControl = new MapControlView(mapControlFrame);
  mapControlDock->setWidget(mapControlFrame);

  mapViewDock->setWidget(mapViewFrame);

  dataView = new DataView(dataViewDock);
  dataViewDock->setWidget(dataView);

  helpWindow = new HelpWindow(helpWindowDock);
  helpWindowDock->setWidget(helpWindow);

  evaluationWindow = new EvaluationDialog(evaluationWindowDock);
  evaluationWindowDock->setWidget(evaluationWindow);

  waypoints = new Waypoints(waypointsDock);
  waypointsDock->setWidget(waypoints);

  legend = new TopoLegend(legendDock);
  legendDock->setWidget(legend);

  objectTree = new ObjectTree(objectTreeDock);
  objectTreeDock->setWidget(objectTree);

  /* Standard positions for the docking windows
   * Arguments for manualDock():
   * dock target, dock side, remaining space in target (in percent)
   *
   * (hl, 2004-03-12): per default, we want to have every dockwindow within the mainwindow.
   */
  objectTreeDock->manualDock( mapViewDock, KDockWidget::DockRight, 67 );
  dataViewDock->manualDock( objectTreeDock, KDockWidget::DockBottom, 67 );
  mapControlDock->manualDock( dataViewDock, KDockWidget::DockBottom, 62 );
  helpWindowDock->manualDock( mapControlDock, KDockWidget::DockCenter);
  waypointsDock->manualDock(mapViewDock, KDockWidget::DockBottom, 70);
  legendDock->manualDock(waypointsDock, KDockWidget::DockRight, 90);
  evaluationWindowDock->manualDock( mapViewDock, KDockWidget::DockCenter);

  connect(map, SIGNAL(changed(QSize)), mapControl, SLOT(slotShowMapData(QSize)));
  connect(map, SIGNAL(waypointSelected(Waypoint *)), waypoints, SLOT(slotAddWaypoint(Waypoint *)));
  connect(map, SIGNAL(waypointDeleted(Waypoint *)), waypoints, SLOT(slotDeleteWaypoint(Waypoint *)));
  connect(map, SIGNAL(waypointEdited(Waypoint *)), waypoints, SLOT(slotEditWaypoint(Waypoint *)));
  connect(map, SIGNAL(elevation(int)), legend, SLOT(highlightLevel(int)));
  connect(map, SIGNAL(regWaypointDialog(QWidget *)), this, SLOT(slotRegisterWaypointDialog(QWidget *)));

  extern MapMatrix _globalMapMatrix;
  connect(mapControl, SIGNAL(scaleChanged(double)), &_globalMapMatrix,
      SLOT(slotSetScale(double)));

  connect(dataView, SIGNAL(wpSelected(const unsigned int)), map,
      SLOT(slotCenterToWaypoint(const unsigned int)));
  connect(dataView, SIGNAL(flightSelected(BaseFlightElement *)), &_globalMapContents,
      SLOT(slotSetFlight(BaseFlightElement *)));
  connect(dataView, SIGNAL(editFlightGroup()), &_globalMapContents,
      SLOT(slotEditFlightGroup()));

  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), this,
      SLOT(slotModifyMenu()));
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), map,
      SLOT(slotShowCurrentFlight()));
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), dataView,
      SLOT(setFlightData()));
  connect(&_globalMapContents, SIGNAL(newFlightAdded(Flight*)), objectTree,
      SLOT(slotNewFlightAdded(Flight*)));
  connect(&_globalMapContents, SIGNAL(newFlightGroupAdded(FlightGroup*)), objectTree,
      SLOT(slotNewFlightGroupAdded(FlightGroup*)));
  connect(&_globalMapContents, SIGNAL(newTaskAdded(FlightTask*)), objectTree,
      SLOT(slotNewTaskAdded(FlightTask*)));
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), objectTree,
      SLOT(slotSelectedFlightChanged()));

  connect(waypoints, SIGNAL(copyWaypoint2Task(Waypoint *)),
      map, SLOT(slotAppendWaypoint2Task(Waypoint *)));

  connect(waypoints, SIGNAL(waypointCatalogChanged( WaypointCatalog * )),
      map, SLOT(slotWaypointCatalogChanged( WaypointCatalog * )));

  connect(waypoints, SIGNAL(centerMap(int, int)),
      &_globalMapMatrix, SLOT(slotCenterTo(int, int)));

  connect(objectTree, SIGNAL(selectedFlight(BaseFlightElement *)), &_globalMapContents, SLOT(slotSetFlight(BaseFlightElement *)));
  connect(objectTree, SIGNAL(newTask()), &_globalMapContents, SLOT(slotNewTask()));
  connect(objectTree, SIGNAL(openTask()), this, SLOT(slotOpenTask()));
  connect(objectTree, SIGNAL(closeTask()), &_globalMapContents, SLOT(closeFlight()));
  connect(objectTree, SIGNAL(newFlightGroup()), &_globalMapContents, SLOT(slotNewFlightGroup()));
  connect(objectTree, SIGNAL(editFlightGroup()), &_globalMapContents, SLOT(slotEditFlightGroup()));
  connect(objectTree, SIGNAL(openFlight()), this, SLOT(slotOpenFile()));
  connect(objectTree, SIGNAL(openFile(const char*)), this, SLOT(slotOpenFile(const char*)));
  connect(objectTree, SIGNAL(optimizeFlight()), this, SLOT(slotOptimizeFlight()));
  connect(objectTree, SIGNAL(optimizeFlightOLC()), this, SLOT(slotOptimizeFlightOLC()));

  connect(&_globalMapContents, SIGNAL(closingFlight(BaseFlightElement*)),
      objectTree, SLOT(slotCloseFlight(BaseFlightElement*)));

  connect(&_globalMapContents, SIGNAL(currentFlightChanged()),
      evaluationWindow, SLOT(slotShowFlightData()));

  connect(evaluationWindow, SIGNAL(showCursor(const QPoint&, const QPoint&)),
      map, SLOT(slotDrawCursor(const QPoint&, const QPoint&)));

}

void KFLogApp::slotShowAbout()
{
  qDebug("aboutDialog");
  KAboutApplication aboutDialog(this, "about", true);
  aboutDialog.exec();
}

void KFLogApp::slotShowPointInfo(const QPoint& pos, const flightPoint& point)
{
  statusBar()->clear();
  statusTimeL->setText(printTime(point.time, true));
  QString text;
  text.sprintf("%4d m  ", point.height);
  statusAltitudeL->setText(text);
  text.sprintf("%3.1f km/h  ", getSpeed(point));
  statusSpeedL->setText(text);
  text.sprintf("%2.1f m/s  ", getVario(point));
  statusVarioL->setText(text);

  statusLatL->setText(printPos(pos.y()));
  statusLonL->setText(printPos(pos.x(), false));
}

void KFLogApp::slotShowPointInfo(const QPoint& pos)
{
  statusBar()->clear();
  statusTimeL->setText("");
  statusAltitudeL->setText("");
  statusSpeedL->setText("");
  statusVarioL->setText("");

  statusLatL->setText(printPos(pos.y()));
  statusLonL->setText(printPos(pos.x(), false));
}

void KFLogApp::saveOptions()
{
  config->setGroup("General Options");
  config->writeEntry("Geometry", size());
  config->writeEntry("Show Toolbar", toolBar("mainToolBar")->isShown());
  config->writeEntry("Show Statusbar",statusBar()->isShown());
  config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

  config->setGroup("Waypoints");
  qDebug("saving options...");
  if (config->readNumEntry("DefaultWaypointCatalog", KFLogConfig::LastUsed) ==
      KFLogConfig::LastUsed && waypoints->getCurrentCatalog() != NULL)
    {
      // Only write the path, if a waypoint-catalog is opened.
      // Otherwise KFLog crashes on a clean installation.
      qDebug("saving catalog name");
      config->writeEntry("DefaultCatalogName", waypoints->getCurrentCatalog()->path);
    }

  config->setGroup(0);

  writeDockConfig(config, "Window Layout");
}

void KFLogApp::readOptions()
{
  config->setGroup("General Options");

  // bar status settings
  bool bViewToolbar = config->readBoolEntry("Show Toolbar", true);
  if(toolBar("mainToolBar")->isShown()!=bViewToolbar)
    slotToggleToolBar();

  bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
  if(statusBar()->isShown()!=bViewStatusbar)
    slotToggleStatusBar();

  // bar position settings
  KToolBar::BarPosition toolBarPos;
  toolBarPos=(KToolBar::BarPosition) config->readNumEntry("ToolBarPos",
      KToolBar::Top);
  toolBar("mainToolBar")->setBarPos(toolBarPos);
  QSize size=config->readSizeEntry("Geometry", new QSize(950,700));

  config->setGroup("Path");
  flightDir = config->readEntry("DefaultFlightDirectory",
      getpwuid(getuid())->pw_dir);
  taskDir = config->readEntry("DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);

  config->setGroup("Scale");
  mapControl->slotSetMinMaxValue(config->readNumEntry("Lower Limit", 10),
      config->readNumEntry("Upper Limit", 1500));

  if(!size.isEmpty())  resize(size);

  readDockConfig(config, "Window Layout");
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KFLogApp::slotSetProgress(int value)  { statusProgress->setValue(value); }

void KFLogApp::slotOpenFile()
{
  slotStatusMsg(tr("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.igc *.IGC", this,
      tr("Select IGC-File"), true);
  IGCPreview* preview = new IGCPreview(dlg);
  dlg->setPreviewWidget(preview);
  QString filter;
  filter.append("*.igc *.flightgear *.trk *.gdn |"+tr("All flight type files"));
  filter.append("\n*.igc|"+tr("IGC (*.igc)"));
  filter.append("\n*.trk *.gdn|"+tr("Garmin (*.trk, *.gdn)"));
  dlg->setFilter(filter);
  dlg->setCaption(tr("Open flight"));
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName, this))
    {
      KNotifyClient::event(this->winId(), tr("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  FlightLoader flightLoader;
  QFile file (fName);
  if(flightLoader.openFlight(file))
  {
    slotSetCurrentFile(fName);
  }

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotOpenFile(const char* surl)
{
  slotStatusMsg(tr("Opening file..."));

  extern MapContents _globalMapContents;
  QUrl url = QUrl(surl);
  if(url.isLocalFile())
  {
    QFile file (url.path());
    if (url.fileName().right(9).lower()==".kflogtsk")
    {
      //this is probably a taskfile. Try to open it as a task
      if (_globalMapContents.loadTask(file))
        slotSetCurrentFile(url.path());
    }
    else
    {
      //try to open as flight
      FlightLoader flightLoader;
      if(flightLoader.openFlight(file))
        slotSetCurrentFile(url.path());
    } // .kflogtsk
  } //isLocalFile

  slotStatusMsg(tr("Ready."));
}

/**
 * Opens a task-file-open-dialog.
 */
void KFLogApp::slotOpenTask()
{
  slotStatusMsg(tr("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.kflogtsk *.KFLOGTSK", this,
      tr("Select Task-File"), true);
  dlg->setCaption(tr("Open task"));
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName, this))
    {
      KNotifyClient::event(this->winId(), tr("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  extern MapContents _globalMapContents;
  QFile file(fName);
  if (_globalMapContents.loadTask(file))
      slotSetCurrentFile(fName);

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotOpenRecentFile()
{
  QString fileName;
  QAction *action = (QAction*)(sender());
  if (action)
    fileName = action->toolTip();
  else
    return;

  slotStatusMsg(tr("Opening file..."));

  extern MapContents _globalMapContents;
  FlightLoader flightLoader;
  QUrl url (fileName);
  if(url.isLocalFile())
  {
    QFile file (url.path());
    if (url.fileName().right(9).lower()==".kflogtsk")
    {
      //this is probably a taskfile. Try to open it as a task
      if (_globalMapContents.loadTask(file))
        slotSetCurrentFile(url.path());
    }
    else
    {
      //try to open as flight
      if(flightLoader.openFlight(file))
        slotSetCurrentFile(url.path());
    } // .kflogtsk
  } //isLocalFile

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotSetCurrentFile(const QString &fileName)
{
  config->setGroup("General Options");
  QStringList files = config->readListEntry("RecentFiles"), newFiles;
  int recentFilesMax = config->readNumEntry("RecentFilesMax", 5);
  int index = 0;

  newFiles.append(fileName);
  for(QStringList::iterator it = files.begin(); (it != files.end() && index<=recentFilesMax); ++it)
  {
    if(*it!=fileName)
    {
      newFiles.append(*it);
      index++;
    }
  }

  config->writeEntry("RecentFiles", newFiles);

  int size = std::min((int)newFiles.size(), 5);
  QAction *recentFileActs[size];
  fileOpenRecent->clear();
  for (int i = 0; i < size; ++i) {
    recentFileActs[i] = new QAction(this);
    recentFileActs[i]->setToolTip(newFiles[i]); // FIXME: in Qt4 setData must be used
    recentFileActs[i]->setText(tr("%1 %2").arg(i + 1).arg(QFileInfo(newFiles[i]).fileName()));
    connect(recentFileActs[i], SIGNAL(activated()), this, SLOT(slotOpenRecentFile()));
    recentFileActs[i]->addTo(fileOpenRecent);
  }
}

void KFLogApp::slotFilePrint()
{
  slotStatusMsg(tr("Printing..."));

  // viewCenterFlight is enabled, when a flight is loaded ...
  MapPrint::MapPrint(viewCenterFlight->isEnabled());

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotFlightPrint()
{
  slotStatusMsg(tr("Printing..."));

  extern MapContents _globalMapContents;
  BaseFlightElement *f = _globalMapContents.getFlight();
  if(f)
    {
      switch (f->getTypeID())
        {
          case BaseMapElement::Flight:
            FlightDataPrint::FlightDataPrint((Flight *)f);
            break;
          case BaseMapElement::Task:
            TaskDataPrint::TaskDataPrint((FlightTask*)f);
            break;
          default:
            QString tmp;
            tmp.sprintf(tr("Not yet available for type: %d"), f->getTypeID());
            QMessageBox::warning(0, tr("Type not available"), tmp, QMessageBox::Ok, 0);
        }
    }
  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotFileQuit()
{
  slotStatusMsg(tr("Exiting..."));
  //saveOptions();   //Now done in queryClose

  close();
}

void KFLogApp::slotStatusMsg(const QString &text)
{
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KFLogApp::slotHideEvaluationWindowDock()  { flightEvaluationWindow->setOn(false); settingsEvaluationWindow->setOn(false); }

void KFLogApp::slotHideDataViewDock()  { settingsFlightData->setOn(false); }

void KFLogApp::slotHideHelpWindowDock()  { settingsHelpWindow->setOn(false); }

void KFLogApp::slotHideObjectTreeDock() { settingsObjectTree->setOn(false); }

void KFLogApp::slotHideLegendDock() { settingsLegend->setOn(false); }

void KFLogApp::slotHideMapViewDock()  { settingsMap->setOn(false); }

void KFLogApp::slotHideMapControlDock()  { settingsMapControl->setOn(false); }

void KFLogApp::slotHideWaypointsDock() { settingsWaypoints->setOn(false); }

void KFLogApp::slotCheckDockWidgetStatus()
{
  // Here is still a bug. The toggle status is invalid, when the widget is a non active
  // TabWidget.
  //          Florian
  flightEvaluationWindow->setOn(evaluationWindowDock->isShown());
  settingsEvaluationWindow->setOn(evaluationWindowDock->isShown());
  settingsFlightData->setOn(dataViewDock->isShown());
  settingsHelpWindow->setOn(helpWindowDock->isShown());
  settingsObjectTree->setOn(objectTreeDock->isShown());
  settingsLegend->setOn(legendDock->isShown());
  settingsMap->setOn(mapViewDock->isShown());
  settingsMapControl->setOn(mapControlDock->isShown());
  settingsStatusBar->setOn(statusBar()->isShown());
  settingsToolBar->setOn(toolBar("mainToolBar")->isShown());
  settingsWaypoints->setOn(waypointsDock->isShown());
}

void KFLogApp::slotToggleDataView()  { dataViewDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleHelpWindow()  { helpWindowDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleEvaluationWindow()  { evaluationWindowDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleLegendDock() { legendDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleMapControl() { mapControlDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleMap() { mapViewDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleObjectTreeDock() { objectTreeDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotToggleStatusBar()
{
  slotStatusMsg(tr("Toggle the statusbar..."));

  if(statusBar()->isShown())
    statusBar()->hide();
  else
    statusBar()->show();

  slotCheckDockWidgetStatus();
  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotToggleToolBar()
{
  slotStatusMsg(tr("Toggling toolbar..."));

  if(toolBar("mainToolBar")->isShown())
      toolBar("mainToolBar")->hide();
  else
      toolBar("mainToolBar")->show();

  slotCheckDockWidgetStatus();
  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotToggleWaypointsDock() { waypointsDock->changeHideShowState(); slotCheckDockWidgetStatus(); }

void KFLogApp::slotSelectFlightData(int id)
{
  switch(id)
    {
      case MapConfig::Altitude:    // Altitude
        emit flightDataTypeChanged(MapConfig::Altitude);
        break;
      case MapConfig::Cycling:     // Cycling
        emit flightDataTypeChanged(MapConfig::Cycling);
        break;
      case MapConfig::Speed:       // Speed
        emit flightDataTypeChanged(MapConfig::Speed);
        break;
      case MapConfig::Vario:       // Vario
        emit flightDataTypeChanged(MapConfig::Vario);
        break;
      case MapConfig::Solid:       // Solid color
        emit flightDataTypeChanged(MapConfig::Solid);
        break;
    }
  map->slotRedrawFlight();
  for(int i=0; i<5; i++)
    flightDataType->setItemChecked(i, false);
  flightDataType->setItemChecked(id, true);
}

/*
 * Now as a Dockwidget

 void KFLogApp::slotEvaluateFlight()
{
  KDialog* dialog = new KDialog(this,"Evaluation Dialog");
  EvaluationDialog* evaluation =  new EvaluationDialog(dialog);
  QBoxLayout * l = new QVBoxLayout( dialog );
  l->addWidget(evaluation);

//  EvaluationDialog* evaluation =  new EvaluationDialog(this);
  extern MapContents _globalMapContents;
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), evaluation,
      SLOT(slotShowFlightData()));
  connect(evaluation, SIGNAL(showCursor(QPoint, QPoint)), map,
      SLOT(slotDrawCursor(QPoint, QPoint)));

//  evaluation->show();
  dialog->show();
  dialog->resize(640,480);
}
*/

void KFLogApp::slotCenterTo()
{
  CenterToDialog* center = new CenterToDialog(this, tr("center-to-dialog"));

  extern MapMatrix _globalMapMatrix;

  connect(center, SIGNAL(centerTo(int,int)), &_globalMapMatrix,
    SLOT(slotCenterTo(int, int)));

  center->show();
}

void KFLogApp::slotOptimizeFlight()
{
  extern MapContents _globalMapContents;
  Flight *f = (Flight *)_globalMapContents.getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight)
    {
      if(f->optimizeTask())
        {
          // Okay, update flightdata and redraw map
          dataView->setFlightData();
          map->slotRedrawFlight();
          objectTree->slotFlightChanged();
        }
    }
}

void KFLogApp::slotOptimizeFlightOLC()
{
  extern MapContents _globalMapContents;
  Flight *f = (Flight *)_globalMapContents.getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight){
      if(f->optimizeTaskOLC(map))
        {
          // Okay, update flightdata and redraw map
          dataView->setFlightData();
          map->slotRedrawFlight();
          objectTree->slotFlightChanged();
        }
  }
}

void KFLogApp::slotConfigureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );
  KEditToolbar dlg(actionCollection(), xmlFile());
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));

  if (dlg.exec())  createGUI();
}

void KFLogApp::slotConfigureKeyBindings()
{
  KKeyDialog::configure(actionCollection(), this);
}

void KFLogApp::slotConfigureKFLog()
{
  KFLogConfig* confDlg = new KFLogConfig(this, config, "kflogconfig");

  connect(confDlg, SIGNAL(scaleChanged(int, int)), mapControl,
      SLOT(slotSetMinMaxValue(int, int)));

  extern MapConfig _globalMapConfig;
  connect(confDlg, SIGNAL(configOk()), &_globalMapConfig,
      SLOT(slotReadConfig()));

  extern MapContents _globalMapContents;
  connect(confDlg, SIGNAL(configOk()), &_globalMapContents,
      SLOT(reProject()));

  connect(confDlg, SIGNAL(configOk()), map, SLOT(slotRedrawMap()));

  connect(confDlg, SIGNAL(newDrawType(int)), this, SLOT(slotSelectFlightData(int)));

  confDlg->exec();

  delete confDlg;
}

void KFLogApp::slotNewToolbarConfig()
{
   applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KFLogApp::slotStartComplete()
{
  if(showStartLogo && (startLogo != 0L))
    {
      delete startLogo;
      startLogo = 0L;
      showStartLogo=false;
    }
}

void KFLogApp::slotFlightViewIgc3D()
{
  Igc3DDialog * igc3d = new Igc3DDialog(this);
  extern MapContents _globalMapContents;
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), igc3d,
      SLOT(slotShowFlightData()));
  extern MapContents _globalMapContents;
}

void KFLogApp::slotFlightViewIgcOpenGL()
{
  #define CHECK_ERROR_EXIT  error = (char *)dlerror(); \
    if(error != NULL) \
      { \
        warning("%s", error); \
        return; \
      }

  char *error;
  qWarning("KFLogApp::slotFlightViewIgcOpenGL()");

  void* libHandle = dlopen("libopengl_igc.so", RTLD_NOW);
  CHECK_ERROR_EXIT
  char* (*getCaption)();
  getCaption = (char* (*) ()) dlsym(libHandle, "getCaption");
  CHECK_ERROR_EXIT
  qWarning("%s", (*getCaption)());

  QWidget* (*run)();
  run = (QWidget* (*) ()) dlsym(libHandle, "getMainWidget");
  CHECK_ERROR_EXIT
  //QWidget* glWidget = (QWidget*)(*run)();

  void (*addFlight)(Flight*);
  addFlight = (void (*) (Flight*)) dlsym(libHandle, "addFlight");
  CHECK_ERROR_EXIT
  extern MapContents _globalMapContents;
  (void)(*addFlight)((Flight*)_globalMapContents.getFlight());
}



bool KFLogApp::queryClose()
{
  saveOptions();
  if (waypoints->saveChanges()) {
    return true;
  }
  return false;
}

/** insert available flights into menu */
void KFLogApp::slotWindowsMenuAboutToShow()
{
  extern MapContents _globalMapContents;
  QPtrList<BaseFlightElement> flights = *(_globalMapContents.getFlightList());
  QPtrListIterator<BaseFlightElement> it(flights);
  BaseFlightElement *flight;

  windowMenu->clear();

  for (int i = 0 ; it.current(); ++it , i++)
    {
      flight = it.current();
      int id = windowMenu->insertItem(flight->getFileName(), &_globalMapContents,
          SLOT(slotSetFlight(int)));

      windowMenu->setItemParameter(id, i);
      windowMenu->setItemChecked(id, _globalMapContents.getFlightIndex() == i);
    }
}

/** set menu items enabled/disabled */
void KFLogApp::slotModifyMenu()
{
  extern MapContents _globalMapContents;
  if (_globalMapContents.getFlightList()->count() > 0)
    {
      switch(_globalMapContents.getFlight()->getTypeID())
        {
          case BaseMapElement::Flight:
            fileClose->setEnabled(true);
            filePrintFlight->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimization->setEnabled(true);
            flightOptimizationOLC->setEnabled(true);
            flightIgc3D->setEnabled(true);
            flightIgcOpenGL->setEnabled(true);
            flightAnimateStart->setEnabled(true);
            flightAnimateStop->setEnabled(true);
            flightAnimateNext->setEnabled(true);
            flightAnimatePrev->setEnabled(true);
            flightAnimate10Next->setEnabled(true);
            flightAnimate10Prev->setEnabled(true);
            flightAnimateHome->setEnabled(true);
            flightAnimateEnd->setEnabled(true);
            windowMenu->setEnabled(true);
            break;
          case BaseMapElement::Task:
            fileClose->setEnabled(true);
            filePrintFlight->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(false);
//            flightEvaluation->setEnabled(false);
            flightOptimization->setEnabled(false);
            flightOptimizationOLC->setEnabled(false);
            flightIgc3D->setEnabled(false);
            flightIgcOpenGL->setEnabled(false);
            flightAnimateStart->setEnabled(false);
            flightAnimateStop->setEnabled(false);
            flightAnimateNext->setEnabled(false);
            flightAnimatePrev->setEnabled(false);
            flightAnimate10Next->setEnabled(false);
            flightAnimate10Prev->setEnabled(false);
            flightAnimateHome->setEnabled(false);
            flightAnimateEnd->setEnabled(false);
            windowMenu->setEnabled(true);
            break;
          case BaseMapElement::FlightGroup:
            fileClose->setEnabled(true);
            filePrintFlight->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimization->setEnabled(true);
            flightOptimizationOLC->setEnabled(true);
            flightIgc3D->setEnabled(true);
            flightIgcOpenGL->setEnabled(true);
            flightAnimateStart->setEnabled(true);
            flightAnimateStop->setEnabled(true);
            flightAnimateNext->setEnabled(true);
            flightAnimatePrev->setEnabled(true);
            flightAnimate10Next->setEnabled(true);
            flightAnimate10Prev->setEnabled(true);
            flightAnimateHome->setEnabled(true);
            flightAnimateEnd->setEnabled(true);
            windowMenu->setEnabled(true);
            break;
        }
    }
  else
    {
      fileClose->setEnabled(false);
      filePrintFlight->setEnabled(false);
      viewCenterTask->setEnabled(false);
      viewCenterFlight->setEnabled(false);
//      flightEvaluation->setEnabled(false);
      flightOptimization->setEnabled(false);
      flightOptimizationOLC->setEnabled(false);
      flightIgc3D->setEnabled(false);
      flightIgcOpenGL->setEnabled(false);
      flightAnimateStart->setEnabled(false);
      flightAnimateStop->setEnabled(false);
      flightAnimateNext->setEnabled(false);
      flightAnimatePrev->setEnabled(false);
      flightAnimate10Next->setEnabled(false);
      flightAnimate10Prev->setEnabled(false);
      flightAnimateHome->setEnabled(false);
      flightAnimateEnd->setEnabled(false);
      windowMenu->setEnabled(false);
    }
}

void KFLogApp::slotOpenRecorderDialog()
{
  extern MapContents _globalMapContents;

  RecorderDialog* dlg = new RecorderDialog(this, config, "recorderDialog");
  connect(dlg, SIGNAL(addCatalog(WaypointCatalog *)), waypoints,
    SLOT(slotAddCatalog(WaypointCatalog *)));
  connect(dlg, SIGNAL(addTask(FlightTask *)), &_globalMapContents,
    SLOT(slotAppendTask(FlightTask *)));
  dlg->exec();
  delete dlg;
}

void KFLogApp::initSurfaces()
{
  surfaces.setAutoDelete(true);

  surfaces.append(new TranslationElement(Airport::Unknown, tr("Unknown")));
  surfaces.append(new TranslationElement(Airport::Grass, tr("Grass")));
  surfaces.append(new TranslationElement(Airport::Asphalt, tr("Asphalt")));
  surfaces.append(new TranslationElement(Airport::Concrete, tr("Concrete")));

  surfaces.sort();
}

void KFLogApp::initTypes()
{
  waypointTypes.setAutoDelete(true);

  // don't know if we really need all of them
  waypointTypes.append(new TranslationElement(BaseMapElement::NotSelected, tr("(not selected)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::AerialRailway, tr("Aerial railway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Airfield, tr("Airfield")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Airport, tr("Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::AmbHeliport, tr("Ambul. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Balloon, tr("Balloon")));
  waypointTypes.append(new TranslationElement(BaseMapElement::City, tr("City")));
  waypointTypes.append(new TranslationElement(BaseMapElement::CivHeliport, tr("Civil Heliport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::IntAirport, tr("Int. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::MilAirport, tr("Mil. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::CivMilAirport, tr("Civil/Mil. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::ClosedAirfield, tr("Closed Airfield")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Glidersite, tr("Glider site")));
  waypointTypes.append(new TranslationElement(BaseMapElement::HangGlider, tr("Hang glider")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Highway, tr("Highway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Landmark, tr("Landmark")));
  waypointTypes.append(new TranslationElement(BaseMapElement::MilHeliport, tr("Mil. Heliport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::UltraLight, tr("Ultralight")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Parachute, tr("Parachute")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Outlanding, tr("Outlanding")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Obstacle, tr("Obstacle")));
  waypointTypes.append(new TranslationElement(BaseMapElement::ObstacleGroup, tr("Obstacle group")));
  waypointTypes.append(new TranslationElement(BaseMapElement::LightObstacleGroup, tr("Obstacle group (lighted)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::LightObstacle, tr("Obstacle (lighted)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Railway, tr("Railway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Road, tr("Road")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Village, tr("Village,City")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Turnpoint, tr("Turnpoint")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Thermal, tr("Thermal")));

  waypointTypes.sort();
}

/** No descriptions */
void KFLogApp::slotSavePixmap( QUrl url, int width, int height ){
  map->slotSavePixmap(url,width,height);
}

/* Slot to set filename for WaypointCatalog */
void KFLogApp::slotSetWaypointCatalog(QString catalog)
{
  waypoints->slotSetWaypointCatalogName( catalog );
}

/** No descriptions */
void KFLogApp::initTaskTypes()
{
  taskTypes.setAutoDelete(true);

  taskTypes.append(new TranslationElement(FlightTask::Route, tr("Traditional Route")));
  taskTypes.append(new TranslationElement(FlightTask::FAIArea, tr("FAI Area")));
  //taskTypes.append(new TranslationElement(FlightTask::AAT, tr("Area Assigned")));

  taskTypes.sort();
}

/** Connects the dialogs addWaypoint signal to the waypoint object. */
void KFLogApp::slotRegisterWaypointDialog(QWidget * dialog){
  connect(dialog, SIGNAL(addWaypoint(Waypoint *)), waypoints, SLOT(slotAddWaypoint(Waypoint *)));
}

/** Called to the What's This? mode. */
void KFLogApp::slotWhatsThis(){
  QWhatsThis::enterWhatsThisMode();
}

/** Called to force display of the "Tip of the Day" dialog. */
void KFLogApp::slotTipOfDay(){
  KTipDialog::showTip(this,"kflog/tips",true);
}
