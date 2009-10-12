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
#include <qsettings.h>
#include <qtextstream.h>
#include <qgl.h>

// include files for KDE
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

// application specific includes
#include "kflog.h"
#include "centertodialog.h"
#include "dataview.h"
#include "evaluationdialog.h"
#include "evaluationview.h"
#include "flight.h"
#include "flightdataprint.h"
#include "flightloader.h"
#include "helpwindow.h"
#include "igcpreview.h"
#include "kflogconfig.h"
#include "kflogstartlogo.h"
#include "map.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapcontrolview.h"
#include "mapprint.h"
#include "recorderdialog.h"
#include "taskdataprint.h"
#include "waypoints.h"
#include "igc3ddialog.h"
#include "basemapelement.h"
#include "airport.h"
#include "topolegend.h"
#include "objecttree.h"
#include "elevationfinder.h"

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
extern QSettings _settings;

KFLogApp::KFLogApp()
  : KDockMainWindow(0, "KFLogMainWindow"), showStartLogo(false)
{
  extern MapConfig _globalMapConfig;
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  config = kapp->config();

  config->setGroup("General Options");
  showStartLogo=false;

  if (_settings.readBoolEntry("/GeneralOptions/Logo", true) && (!kapp->isRestored() ) )
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

  new KAction(tr("&Open Flight"), "fileopen", CTRL+Key_O,
      this, SLOT(slotFileOpen()), actionCollection(), "file_open");
  new KAction(tr("&Open Task"), "fileopen",  CTRL+Key_T,
      this, SLOT(slotTaskOpen()), actionCollection(), "file_open_task");

  fileOpenRecent = KStdAction::openRecent(this,
      SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileClose = new KAction(tr("&Close Flight"), "fileclose",
      CTRL+Key_W, &_globalMapContents, SLOT(closeFlight()),
      actionCollection(), "file_close");

  fileRecorder = new KAction(tr("Open Recorder"), "connect_no", 0, this,
      SLOT(slotOpenRecorderDialog()), actionCollection(),
      "recorderdialog");

  KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());

  savePixmap = new KAction(tr("Export to PNG..."), "image", 0, map,
      SLOT(slotSavePixmap()), actionCollection(), "file_save_as");

  flightPrint = new KAction(tr("Print Flightdata"), "fileprint", 0, this,
      SLOT(slotFlightPrint()), actionCollection(), "file_print_preview");

  KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

  KActionMenu* mapMoveMenu = new KActionMenu(tr("Move map"), "move",
      actionCollection(), "move_map");
  mapMoveMenu->setDelayed(false);

  KAction* viewRedraw = KStdAction::redisplay(map, SLOT(slotRedrawMap()),
      actionCollection());
  viewRedraw->setAccel(Key_F5);

  viewCenterTask = new KAction(tr("Center to &Task"), "centertask",
      Key_F6, map,
      SLOT(slotCenterToTask()), actionCollection(), "view_actual_size");

  viewCenterFlight = new KAction(tr("Center to &Flight"), "centerflight",
      Key_F7, map,
      SLOT(slotCenterToFlight()), actionCollection(), "view_fit_to_page");

  new KAction(tr("Center to &Homesite"), "gohome",
      CTRL+Key_Home, &_globalMapMatrix,
      SLOT(slotCenterToHome()), actionCollection(), "view_fit_to_width");

  viewCenterTo = new KAction(tr("Center to..."), "centerto", Key_F8, this,
      SLOT(slotCenterTo()), actionCollection(), "view_fit_to_height");

//  flightEvaluation = new KAction(tr("Evaluation"), "flightevaluation",
//      CTRL+Key_E, this, SLOT(slotEvaluateFlight()), actionCollection(),
//      "evaluate_flight");


  mapMoveMenu->insert(new KAction(tr("move map north-west"), "movemap_nw",
      KShortcut("7"),
      &_globalMapMatrix, SLOT(slotMoveMapNW()), actionCollection(), "move_map_nw"));

  mapMoveMenu->insert(new KAction(tr("move map north"), "movemap_n",
      KShortcut("Up;8"),
      &_globalMapMatrix, SLOT(slotMoveMapN()), actionCollection(), "move_map_n"));

  mapMoveMenu->insert(new KAction(tr("move map northeast"), "movemap_ne",
      KShortcut("9"),
      &_globalMapMatrix, SLOT(slotMoveMapNE()), actionCollection(), "move_map_ne"));

  mapMoveMenu->insert(new KAction(tr("move map west"), "movemap_w",
      KShortcut("Left;4"),
      &_globalMapMatrix, SLOT(slotMoveMapW()), actionCollection(), "move_map_w"));

  mapMoveMenu->insert(new KAction(tr("move map east"), "movemap_e",
      KShortcut("Right;6"),
      &_globalMapMatrix, SLOT(slotMoveMapE()), actionCollection(), "move_map_e"));

  mapMoveMenu->insert(new KAction(tr("move map south-west"), "movemap_sw",
      KShortcut("1"),
      &_globalMapMatrix, SLOT(slotMoveMapSW()), actionCollection(), "move_map_sw"));

  mapMoveMenu->insert(new KAction(tr("move map south"), "movemap_s",
      KShortcut("Down;2"),
      &_globalMapMatrix, SLOT(slotMoveMapS()), actionCollection(), "move_map_s"));

  mapMoveMenu->insert(new KAction(tr("move map south-east"), "movemap_se",
      KShortcut("3"),
      &_globalMapMatrix, SLOT(slotMoveMapSE()), actionCollection(), "move_map_se"));

  KStdAction::zoomIn(&_globalMapMatrix, SLOT(slotZoomIn()), actionCollection());
  KStdAction::zoomOut(&_globalMapMatrix, SLOT(slotZoomOut()), actionCollection());
  KStdAction::zoom(map, SLOT(slotZoomRect()), actionCollection());


  /*
   * we urgently need icons for this actions in order to
   * place them in the toolbar!!!
   */
  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());

  viewData = new KToggleAction(tr("Show Flight&data"), "view_detailed", 0, this,
      SLOT(slotToggleDataView()), actionCollection(), "toggle_data_view");

  viewHelpWindow = new KToggleAction(tr("Show &HelpWindow"), "info",
      CTRL+Key_H, this, SLOT(slotToggleHelpWindow()), actionCollection(),
      "toggle_help_window");

  viewEvaluationWindow = new KToggleAction(tr("Show &EvaluationWindow"), "history",
      CTRL+Key_E, this, SLOT(slotToggleEvaluationWindow()), actionCollection(),
      "toggle_evaluation_window");

  viewMapControl = new KToggleAction(tr("Show Map&control"), 0, this,
      SLOT(slotToggleMapControl()), actionCollection(), "toggle_map_control");

  viewMap = new KToggleAction(tr("Show &Map"), 0, this,
      SLOT(slotToggleMap()), actionCollection(), "toggle_map");

  // We can't use CTRL-W, because this shortcut is reserved for closing a file ...
  viewWaypoints = new KToggleAction(tr("Show &Waypoints"), "waypoint",
      CTRL+Key_R, this, SLOT(slotToggleWaypointsDock()), actionCollection(),
      "waypoints");

  viewLegend = new KToggleAction(tr("Show &Legend"), "blend",
      CTRL+Key_L, this, SLOT(slotToggleLegendDock()), actionCollection(),
      "toggle_legend");

  viewObjectTree = new KToggleAction(tr("Show KFLog&Browser"), "view_tree",
      CTRL+Key_B, this, SLOT(slotToggleObjectTreeDock()), actionCollection(),
      "toggle_objectTree");

  flightOptimization = new KAction(tr("Optimize"), "wizard", 0,
      this, SLOT(slotOptimizeFlight()), actionCollection(), "optimize_flight");


  flightOptimizationOLC = new KAction(tr("Optimize (OLC)"), "wizard", 0,
      this, SLOT(slotOptimizeFlightOLC()), actionCollection(), "optimize_flight_olc");


  //Animation actions
  animateFlightStart = new KAction(tr("&Start Flight Animation"), "1rightarrow",
                        Key_F12, map, SLOT(slotAnimateFlightStart()), actionCollection(),
                        "start_animate");
  animateFlightStop = new KAction(tr("Stop Flight &Animation"), "player_stop",
                        Key_F11, map, SLOT(slotAnimateFlightStop()), actionCollection(),
                        "stop_animate");
        //Stepping actions
        stepFlightNext = new KAction(tr("Next Flight Point"), "forward",
                        CTRL+Key_Up, map, SLOT(slotFlightNext()), actionCollection(),
                  "next_flight_point");
        stepFlightPrev = new KAction(tr("Prev Flight Point"), "back",
                        CTRL+Key_Down, map, SLOT(slotFlightPrev()), actionCollection(),
                        "prev_flight_point");
        stepFlightHome = new KAction(tr("First Flight Point"), "start",
                        Key_Home, map, SLOT(slotFlightHome()), actionCollection(),
                        "first_flight_point");
        stepFlightEnd = new KAction(tr("Last Flight Point"), "finish",
                        Key_End, map, SLOT(slotFlightEnd()), actionCollection(),
                        "last_flight_point");
        stepFlightStepNext = new KAction(tr("Step +10 Flight Points"), "stepforward",
                        Key_PageUp, map, SLOT(slotFlightStepNext()), actionCollection(),
                        "next_step_flight_point");
        stepFlightStepPrev = new KAction(tr("Step -10 Flight Points"), "stepback",
                        Key_PageDown, map, SLOT(slotFlightStepPrev()), actionCollection(),
                        "prev_step_flight_point");

        /**
         * Igc3d action
         */
        viewIgc3D = new KAction(tr("View flight in 3D"), "vectorgfx",
                        CTRL+Key_R, this, SLOT(slotFlightViewIgc3D()), actionCollection(),
                        "view_flight_3D");

        /**
         * OpenGL action
         */
        viewIgcOpenGL = new KAction(tr("View flight in 3D (OpenGL)"), "openglgfx",
                        0, this, SLOT(slotFlightViewIgcOpenGL()), actionCollection(),
                        "view_flight_opengl");

  viewFlightDataType = new KSelectAction(
      tr("Show Flightdata"), "idea", 0,
      actionCollection(), "view_flight_data");

  connect(viewFlightDataType, SIGNAL(activated(int)), this,
      SLOT(slotSelectFlightData(int)));

  QStringList dataList;
  dataList.append(tr("Altitude"));
  dataList.append(tr("Cycling"));
  dataList.append(tr("Speed"));
  dataList.append(tr("Vario"));
  dataList.append(tr("Solid"));

  viewFlightDataType->setItems(dataList);
  viewFlightDataType->setCurrentItem(_settings.readNumEntry("/Flight/DrawType", MapConfig::Altitude));

  KActionMenu* flightMenu = new KActionMenu(tr("F&light"),
      actionCollection(), "flight");
  flightMenu->insert(viewEvaluationWindow);
  flightMenu->insert(flightOptimization);
  flightMenu->insert(flightOptimizationOLC);
  //  flightMenu->insert(viewWaypoints);
  flightMenu->insert(viewFlightDataType);
  flightMenu->insert(viewIgc3D);
  flightMenu->insert(viewIgcOpenGL);
//  flightMenu->insert(mapPlanning);
  flightMenu->popupMenu()->insertSeparator();
  flightMenu->insert(animateFlightStart);
  flightMenu->insert(animateFlightStop);
  flightMenu->insert(stepFlightNext);
  flightMenu->insert(stepFlightPrev);
  flightMenu->insert(stepFlightStepNext);
  flightMenu->insert(stepFlightStepPrev);
  flightMenu->insert(stepFlightHome);
  flightMenu->insert(stepFlightEnd);

  KStdAction::configureToolbars(this,
      SLOT(slotConfigureToolbars()), actionCollection());
  KStdAction::keyBindings(this,
      SLOT(slotConfigureKeyBindings()), actionCollection());
  KStdAction::tipOfDay(this,
      SLOT(slotTipOfDay()), actionCollection());

  KStdAction::preferences(this, SLOT(slotConfigureKFLog()), actionCollection());

  KActionMenu *w = new KActionMenu(tr("&Window"), "igc",
      actionCollection(), "window");
  windowMenu = w->popupMenu();
  windowMenu->setCheckable(true);
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotWindowsMenuAboutToShow()));

  KActionMenu *m = new KActionMenu(tr("&New"), "filenew", actionCollection(), "file_new");
  m->popupMenu()->insertItem(SmallIcon("waypoint"), tr("&Waypoint"), waypoints, SLOT(slotNewWaypoint()));
  m->popupMenu()->insertItem(SmallIcon("task"), tr("&Task"), &_globalMapContents, SLOT(slotNewTask()), CTRL+Key_N);
  m->popupMenu()->insertItem(tr("&Flight group"), &_globalMapContents, SLOT(slotNewFlightGroup()));

  createGUI();
}

void KFLogApp::initStatusBar()
{
  /* Alternativ k�nnte der Balken auch nur w�hrend des Zeichnens erscheinen */
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
  connect(objectTree, SIGNAL(openTask()), this, SLOT(slotTaskOpen()));
  connect(objectTree, SIGNAL(closeTask()), &_globalMapContents, SLOT(closeFlight()));
  connect(objectTree, SIGNAL(newFlightGroup()), &_globalMapContents, SLOT(slotNewFlightGroup()));
  connect(objectTree, SIGNAL(editFlightGroup()), &_globalMapContents, SLOT(slotEditFlightGroup()));
  connect(objectTree, SIGNAL(openFlight()), this, SLOT(slotFileOpen()));
  connect(objectTree, SIGNAL(openFile(const QUrl&)), this, SLOT(slotFileOpenRecent(const KURL&)));
  connect(objectTree, SIGNAL(optimizeFlight()), this, SLOT(slotOptimizeFlight()));
  connect(objectTree, SIGNAL(optimizeFlightOLC()), this, SLOT(slotOptimizeFlightOLC()));

  connect(&_globalMapContents, SIGNAL(closingFlight(BaseFlightElement*)),
      objectTree, SLOT(slotCloseFlight(BaseFlightElement*)));

  connect(&_globalMapContents, SIGNAL(currentFlightChanged()),
      evaluationWindow, SLOT(slotShowFlightData()));

  connect(evaluationWindow, SIGNAL(showCursor(const QPoint&, const QPoint&)),
      map, SLOT(slotDrawCursor(const QPoint&, const QPoint&)));

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
  _settings.writeEntry("/GeneralOptions/GeometryWidth", size().width());
  _settings.writeEntry("/GeneralOptions/GeometryHeight", size().height());
  _settings.writeEntry("/GeneralOptions/ShowToolbar", viewToolBar->isChecked());
  _settings.writeEntry("/GeneralOptions/ShowStatusbar",viewStatusBar->isChecked());
  _settings.writeEntry("/GeneralOptions/ToolBarPos", (int) toolBar("mainToolBar")->barPos());

  //qDebug("saving options...");
  if (_settings.readNumEntry("/Waypoints/DefaultWaypointCatalog", KFLogConfig::LastUsed) ==
      KFLogConfig::LastUsed && waypoints->getCurrentCatalog() != NULL)
    {
      // Only write the path, if a waypoint-catalog is opened.
      // Otherwise KFLog crashes on a clean installation.
      //qDebug("saving catalog name");
      _settings.writeEntry("/Waypoints/DefaultCatalogName", waypoints->getCurrentCatalog()->path);
    }

  writeDockConfig(config, "Window Layout");

  fileOpenRecent->saveEntries(config,"Recent Files");
}

void KFLogApp::readOptions()
{
  // bar status settings
  bool bViewToolbar = _settings.readBoolEntry("/GeneralOptions/ShowToolbar", true);
  viewToolBar->setChecked(bViewToolbar);
  slotViewToolBar();

  bool bViewStatusbar = _settings.readBoolEntry("/GeneralOptions/ShowStatusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();

  // bar position settings
  KToolBar::BarPosition toolBarPos;
  toolBarPos=(KToolBar::BarPosition) _settings.readNumEntry("/GeneralOptions/ToolBarPos",
      KToolBar::Top);
  toolBar("mainToolBar")->setBarPos(toolBarPos);
  QSize size(_settings.readNumEntry("/GeneralOptions/GeometryWidth", 950),
             _settings.readNumEntry("/GeneralOptions/GeometryHeight", 700));

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");
  flightDir = _settings.readEntry("/Path/DefaultFlightDirectory",
      getpwuid(getuid())->pw_dir);
  taskDir = _settings.readEntry("/Path/DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);

  mapControl->slotSetMinMaxValue(_settings.readNumEntry("/Scale/Lower Limit", 10),
                                 _settings.readNumEntry("/Scale/Upper Limit", 1500));

  if(!size.isEmpty())  resize(size);

  readDockConfig(config, "Window Layout");
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KFLogApp::slotSetProgress(int value)  { statusProgress->setValue(value); }

void KFLogApp::slotFileOpen()
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

//  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName))
    {
      KNotifyClient::event(tr("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  FlightLoader flightLoader;
  QFile file (fName);
  if(flightLoader.openFlight(file))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(tr("Opening file..."));

  extern MapContents _globalMapContents;
  FlightLoader flightLoader;
  if(url.isLocalFile())
    {
      QFile file (url.path());
      if (url.filename().right(9).lower()==".kflogtsk") {
        //this is probably a taskfile. Try to open it as a task
        if (_globalMapContents.loadTask(file))
          fileOpenRecent->setCurrentItem(-1);

      } else {
        //try to open as flight
        if(flightLoader.openFlight(file))
          {
            // Just a workaround. It's the only way to not have the item
            // checked after loading the flight. Otherwise we had to take
            // care that the item is unchecked, when the flight is closed ...
            fileOpenRecent->setCurrentItem(-1);
          } //loadFile
       } // .kflogtsk
    } //isLocalFile

  slotStatusMsg(tr("Ready."));
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

void KFLogApp::slotViewToolBar()
{
  slotStatusMsg(tr("Toggling toolbar..."));

  if(!viewToolBar->isChecked())
      toolBar("mainToolBar")->hide();
  else
      toolBar("mainToolBar")->show();

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotViewStatusBar()
{
  slotStatusMsg(tr("Toggle the statusbar..."));

  if(!viewStatusBar->isChecked())
      statusBar()->hide();
  else
      statusBar()->show();

  slotStatusMsg(tr("Ready."));
}

void KFLogApp::slotStatusMsg(const QString &text)
{
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KFLogApp::slotHideMapControlDock()  { viewMapControl->setChecked(false); }

void KFLogApp::slotHideMapViewDock()  { viewMap->setChecked(false); }

void KFLogApp::slotHideDataViewDock()  { viewData->setChecked(false); }

void KFLogApp::slotHideHelpWindowDock()  { viewHelpWindow->setChecked(false); }

void KFLogApp::slotHideEvaluationWindowDock()  { viewEvaluationWindow->setChecked(false); }

void KFLogApp::slotHideWaypointsDock() { viewWaypoints->setChecked(false); }

void KFLogApp::slotHideLegendDock() { viewLegend->setChecked(false); }

void KFLogApp::slotHideObjectTreeDock() { viewObjectTree->setChecked(false); }

void KFLogApp::slotCheckDockWidgetStatus()
{
  // Here is still a bug. The toggle status is invalid, when the widget is a non active
  // TabWidget.
  //          Florian
  viewMapControl->setChecked(mapControlDock->isShown());
  viewMap->setChecked(mapViewDock->isShown());
  viewData->setChecked(dataViewDock->isShown());
  viewHelpWindow->setChecked(helpWindowDock->isShown());
  viewEvaluationWindow->setChecked(evaluationWindowDock->isShown());
  viewWaypoints->setChecked(waypointsDock->isShown());
  viewLegend->setChecked(legendDock->isShown());
  viewObjectTree->setChecked(objectTreeDock->isShown());
}

void KFLogApp::slotToggleDataView()  { dataViewDock->changeHideShowState(); }

void KFLogApp::slotToggleHelpWindow()  { helpWindowDock->changeHideShowState(); }

void KFLogApp::slotToggleEvaluationWindow()  { evaluationWindowDock->changeHideShowState(); }

void KFLogApp::slotToggleMapControl() { mapControlDock->changeHideShowState(); }

void KFLogApp::slotToggleMap() { mapViewDock->changeHideShowState(); }

void KFLogApp::slotToggleWaypointsDock() { waypointsDock->changeHideShowState(); }

void KFLogApp::slotToggleLegendDock() { legendDock->changeHideShowState(); }

void KFLogApp::slotToggleObjectTreeDock() { objectTreeDock->changeHideShowState(); }

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
  viewFlightDataType->setCurrentItem(id);
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
  KKeyDialog::configureKeys(actionCollection(), xmlFile());
}

void KFLogApp::slotConfigureKFLog()
{
  KFLogConfig* confDlg = new KFLogConfig(this, "kflogconfig");

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
  QWidget* glWidget = (QWidget*)(*run)();

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
            flightPrint->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimization->setEnabled(true);
            flightOptimizationOLC->setEnabled(true);
            animateFlightStart->setEnabled(true);
            animateFlightStop->setEnabled(true);
            stepFlightNext->setEnabled(true);
            stepFlightPrev->setEnabled(true);
            stepFlightStepNext->setEnabled(true);
            stepFlightStepPrev->setEnabled(true);
            stepFlightHome->setEnabled(true);
            stepFlightEnd->setEnabled(true);
            viewIgc3D->setEnabled(true);
            viewIgcOpenGL->setEnabled(true);
//            mapPlanning->setEnabled(false);
            windowMenu->setEnabled(true);
            break;
          case BaseMapElement::Task:
            fileClose->setEnabled(true);
            flightPrint->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(false);
//            flightEvaluation->setEnabled(false);
            flightOptimization->setEnabled(false);
            flightOptimizationOLC->setEnabled(false);
            animateFlightStart->setEnabled(false);
            animateFlightStop->setEnabled(false);
            stepFlightNext->setEnabled(false);
            stepFlightPrev->setEnabled(false);
            stepFlightStepNext->setEnabled(false);
            stepFlightStepPrev->setEnabled(false);
            stepFlightHome->setEnabled(false);
            stepFlightEnd->setEnabled(false);
            viewIgc3D->setEnabled(false);
            viewIgcOpenGL->setEnabled(false);
//            mapPlanning->setEnabled(true);
            windowMenu->setEnabled(true);
            break;
          case BaseMapElement::FlightGroup:
            fileClose->setEnabled(true);
            flightPrint->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimization->setEnabled(true);
            flightOptimizationOLC->setEnabled(true);
            animateFlightStart->setEnabled(true);
            animateFlightStop->setEnabled(true);
            stepFlightNext->setEnabled(true);
            stepFlightPrev->setEnabled(true);
            stepFlightStepNext->setEnabled(true);
            stepFlightStepPrev->setEnabled(true);
            stepFlightHome->setEnabled(true);
            stepFlightEnd->setEnabled(true);
            viewIgc3D->setEnabled(true);
            viewIgcOpenGL->setEnabled(true);
//            mapPlanning->setEnabled(false);
            windowMenu->setEnabled(true);
            break;
        }
    }
  else
    {
      fileClose->setEnabled(false);
      flightPrint->setEnabled(false);
      viewCenterTask->setEnabled(false);
      viewCenterFlight->setEnabled(false);
//      flightEvaluation->setEnabled(false);
      flightOptimization->setEnabled(false);
      flightOptimizationOLC->setEnabled(false);
      animateFlightStart->setEnabled(false);
      animateFlightStop->setEnabled(false);
      stepFlightNext->setEnabled(false);
      stepFlightPrev->setEnabled(false);
      stepFlightStepNext->setEnabled(false);
      stepFlightStepPrev->setEnabled(false);
      stepFlightHome->setEnabled(false);
      stepFlightEnd->setEnabled(false);
      viewIgc3D->setEnabled(false);
      viewIgcOpenGL->setEnabled(false);
//     mapPlanning->setEnabled(false);
      windowMenu->setEnabled(false);
    }
}

void KFLogApp::slotOpenRecorderDialog()
{
  extern MapContents _globalMapContents;

  RecorderDialog* dlg = new RecorderDialog(this, "recorderDialog");
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

/** No descriptions */
void KFLogApp::slotTaskOpen()
{
  slotStatusMsg(tr("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.kflogtsk *.KFLOGTSK", this,
      tr("Select Task-File"), true);
  dlg->setCaption(tr("Open task"));
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

//  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName))
    {
      KNotifyClient::event(tr("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  extern MapContents _globalMapContents;
  QFile file(fName);
  if (_globalMapContents.loadTask(file))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(tr("Ready."));
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

/** Called to force display of the "Tip of the Day" dialog. */
void KFLogApp::slotTipOfDay(){
  KTipDialog::showTip(this,"kflog/tips",true);
}
