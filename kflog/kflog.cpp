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

// include files for QT
#include <qdir.h>
#include <qlayout.h>
#include <qprinter.h>
#include <qregexp.h>
#include <qtextstream.h>

// include files for KDE
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kstdaction.h>

// application specific includes
#include "kflog.h"
#include <centertodialog.h>
#include <dataview.h>
#include <evaluationdialog.h>
#include <flight.h>
#include <flightdataprint.h>
#include <helpwindow.h>
#include <igcpreview.h>
#include <kflogconfig.h>
#include <kflogstartlogo.h>
#include <map.h>
#include <mapcalc.h>
#include <mapconfig.h>
#include <mapcontents.h>
#include <mapcontrolview.h>
#include <mapprint.h>
#include <olcdialog.h>
#include <recorderdialog.h>
#include <taskdataprint.h>
#include <waypoints.h>
#include <tasks.h>
#include <igc3ddialog.h>
#include "basemapelement.h"
#include "airport.h"
#include "topolegend.h"

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

  slotCheckDockWidgetStatus();
  // Heavy workaround! MapConfig should tell KFLogApp, which type is selected!
  slotSelectFlightData(0);

  connect(&_globalMapMatrix, SIGNAL(matrixChanged()), map,
      SLOT(slotRedrawMap()));
  connect(map, SIGNAL(showFlightPoint(const QPoint, const struct flightPoint&)),
      this, SLOT(slotShowPointInfo(const QPoint, const struct flightPoint&)));
  // Plannung
  connect(&_globalMapContents, SIGNAL(activatePlanning()),
     map,SLOT(slotActivatePlanning()));

  connect(&_globalMapContents, SIGNAL(taskHelp(QString)),
      helpWindow, SLOT(slotShowHelpText(QString)) );
  connect(map, SIGNAL(taskPlanningEnd()), helpWindow, SLOT(slotClearView()) );

          
  connect(map, SIGNAL(showTaskText(FlightTask*)),
      dataView, SLOT(slotShowTaskText(FlightTask*)));
  connect(map, SIGNAL(taskPlanningEnd()), dataView, SLOT(setFlightData()));
  connect(map, SIGNAL(showPoint(const QPoint)),
      this, SLOT(slotShowPointInfo(const QPoint)));
  connect(&_globalMapContents, SIGNAL(contentsChanged()),map, SLOT(slotRedrawMap()));
  slotModifyMenu();
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

  new KAction(i18n("&Open Flight"), "fileopen",
      KStdAccel::key(KStdAccel::Open), this, SLOT(slotFileOpen()),
      actionCollection(), "file_open");

  fileOpenRecent = KStdAction::openRecent(this,
      SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileClose = new KAction(i18n("Close Flight"), "fileclose",
      KStdAccel::key(KStdAccel::Close), &_globalMapContents, SLOT(closeFlight()),
      actionCollection(), "file_close");

  fileRecorder = new KAction(i18n("Open Recorder"), "connect_no", 0, this,
      SLOT(slotOpenRecorderDialog()), actionCollection(),
      "recorderdialog");

  fileImportFlightGearFile = new KAction(i18n("Import FlightGear File"), "fileopen",
      0, this, SLOT(slotImportFlightGearFile()), actionCollection(),
      "file_import_flightgear");

  fileImportGardownFile = new KAction(i18n("Import Gardown File"), "fileopen",
      0, this, SLOT(slotImportGardownFile()), actionCollection(),
      "file_import_gardown");

  KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());

  savePixmap = new KAction(i18n("Export to PNG..."), "image", 0, map,
      SLOT(slotSavePixmap()), actionCollection(), "file_save_as");

  flightPrint = new KAction(i18n("Print Flightdata"), "fileprint", 0, this,
      SLOT(slotFlightPrint()), actionCollection(), "file_print_preview");

  KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

  KActionMenu* mapMoveMenu = new KActionMenu(i18n("Move map"), "move",
      actionCollection(), "move_map");
  mapMoveMenu->setDelayed(false);

  KAction* viewRedraw = KStdAction::redisplay(map, SLOT(slotRedrawMap()),
      actionCollection());
  viewRedraw->setAccel(Key_F5);

  viewCenterTask = new KAction(i18n("Center to &Task"), "centertask",
      Key_F6, map,
      SLOT(slotCenterToTask()), actionCollection(), "view_actual_size");

  viewCenterFlight = new KAction(i18n("Center to &Flight"), "centerflight",
      Key_F7, map,
      SLOT(slotCenterToFlight()), actionCollection(), "view_fit_to_page");

  new KAction(i18n("Center to &Homesite"), "gohome",
      KStdAccel::key(KStdAccel::Home), &_globalMapMatrix,
      SLOT(slotCenterToHome()), actionCollection(), "view_fit_to_width");

  viewCenterTo = new KAction(i18n("Center to..."), "centerto", Key_F8, this,
      SLOT(slotCenterTo()), actionCollection(), "view_fit_to_height");

  flightEvaluation = new KAction(i18n("Evaluation"), "flightevaluation",
      CTRL+Key_E, this, SLOT(slotEvaluateFlight()), actionCollection(),
      "evaluate_flight");


  mapMoveMenu->insert(new KAction(i18n("move map north-west"), "movemap_nw",
      KShortcut("7"),
      &_globalMapMatrix, SLOT(slotMoveMapNW()), actionCollection(), "move_map_nw"));

  mapMoveMenu->insert(new KAction(i18n("move map north"), "movemap_n",
      KShortcut("Up;8"),
      &_globalMapMatrix, SLOT(slotMoveMapN()), actionCollection(), "move_map_n"));

  mapMoveMenu->insert(new KAction(i18n("move map northeast"), "movemap_ne",
      KShortcut("9"),
      &_globalMapMatrix, SLOT(slotMoveMapNE()), actionCollection(), "move_map_ne"));

  mapMoveMenu->insert(new KAction(i18n("move map west"), "movemap_w",
      KShortcut("Left;4"),
      &_globalMapMatrix, SLOT(slotMoveMapW()), actionCollection(), "move_map_w"));

  mapMoveMenu->insert(new KAction(i18n("move map east"), "movemap_e",
      KShortcut("Right;6"),
      &_globalMapMatrix, SLOT(slotMoveMapE()), actionCollection(), "move_map_e"));

  mapMoveMenu->insert(new KAction(i18n("move map south-west"), "movemap_sw",
      KShortcut("1"),
      &_globalMapMatrix, SLOT(slotMoveMapSW()), actionCollection(), "move_map_sw"));

  mapMoveMenu->insert(new KAction(i18n("move map south"), "movemap_s",
      KShortcut("Down;2"),
      &_globalMapMatrix, SLOT(slotMoveMapS()), actionCollection(), "move_map_s"));

  mapMoveMenu->insert(new KAction(i18n("move map south-east"), "movemap_se",
      KShortcut("3"),
      &_globalMapMatrix, SLOT(slotMoveMapSE()), actionCollection(), "move_map_se"));

  KStdAction::zoomIn(&_globalMapMatrix, SLOT(slotZoomIn()), actionCollection());
  KStdAction::zoomOut(&_globalMapMatrix, SLOT(slotZoomOut()), actionCollection());
  KStdAction::zoom(map, SLOT(slotZoomRect()), actionCollection());


  /*
   * we urgently need icons for this actions in order to
   * place them in the toolbar!!!
   */
  viewData = new KToggleAction(i18n("Show Flightdata"), 0, this,
      SLOT(slotToggleDataView()), actionCollection(), "toggle_data_view");
  viewHelpWindow = new KToggleAction(i18n("Show HelpWindow"), 0,
      CTRL+Key_H, this, SLOT(slotToggleHelpWindow()), actionCollection(),
      "toggle_help_window");
  viewMapControl = new KToggleAction(i18n("Show Mapcontrol"), 0, this,
      SLOT(slotToggleMapControl()), actionCollection(), "toggle_map_control");
  viewMap = new KToggleAction(i18n("Show Map"), 0, this,
      SLOT(slotToggleMap()), actionCollection(), "toggle_map");
  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());

  // We can't use CTRL-W, because this shortcut is reserved for closing a file ...
  viewWaypoints = new KToggleAction(i18n("Show waypoints"), "waypoint",
      CTRL+Key_R, this, SLOT(slotToggleWaypointsDock()), actionCollection(),
      "waypoints");

  viewTasks = new KToggleAction(i18n("Show tasks"), "task",
      CTRL+Key_T, this, SLOT(slotToggleTasksDock()), actionCollection(),
      "tasks");

  viewLegend = new KToggleAction(i18n("Show legend"), 0,
      CTRL+Key_L, this, SLOT(slotToggleLegendDock()), actionCollection(),
      "toggle_legend");

  flightOptimization = new KAction(i18n("Optimize"), "wizard", 0,
      this, SLOT(slotOptimizeFlight()), actionCollection(), "optimize_flight");

      
  flightOptimizationOLC = new KAction(i18n("Optimize(OLC)"), "wizard", 0,
      this, SLOT(slotOptimizeFlightOLC()), actionCollection(), "optimize_flight_olc");


  // Disabled for the next release, because we only have the window ...
  olcDeclaration = new KAction(i18n("send OLC-Declaration"), 0,
      this, SLOT(slotOlcDeclaration()), actionCollection(), "olc_declaration");

  // Reset confirmations
  resetConfirmations = new KAction(i18n("Reset confirmations"), "configure", 0,
      this, SLOT(slotEnableMessages()), actionCollection(), "reset_confirmations");
      
  
  //Animation actions
  animateFlightStart = new KAction(i18n("&Start Flight Animation"), "1rightarrow",
			Key_F12, map, SLOT(slotAnimateFlightStart()), actionCollection(),
			"start_animate");
  animateFlightStop = new KAction(i18n("Stop Flight &Animation"), "player_stop",
			Key_F11, map, SLOT(slotAnimateFlightStop()), actionCollection(),
			"stop_animate");
	//Stepping actions
	stepFlightNext = new KAction(i18n("Next Flight Point"), "forward",
			CTRL+Key_Up, map, SLOT(slotFlightNext()), actionCollection(),
		  "next_flight_point");
	stepFlightPrev = new KAction(i18n("Prev Flight Point"), "back",
			CTRL+Key_Down, map, SLOT(slotFlightPrev()), actionCollection(),
			"prev_flight_point");
	stepFlightHome = new KAction(i18n("First Flight Point"), "start",
			Key_Home, map, SLOT(slotFlightHome()), actionCollection(),
			"first_flight_point");
	stepFlightEnd = new KAction(i18n("Last Flight Point"), "finish",
			Key_End, map, SLOT(slotFlightEnd()), actionCollection(),
			"last_flight_point");
	stepFlightStepNext = new KAction(i18n("Step +10 Flight Points"), "stepforward",
			Key_PageUp, map, SLOT(slotFlightStepNext()), actionCollection(),
			"next_step_flight_point");
	stepFlightStepPrev = new KAction(i18n("Step -10 Flight Points"), "stepback",
			Key_PageDown, map, SLOT(slotFlightStepPrev()), actionCollection(),
			"prev_step_flight_point");

	/**
	 * Igc3d action
	 */            	
	viewIgc3D = new KAction(i18n("View flight in 3D"), "vectorgfx",
			CTRL+Key_R, this, SLOT(slotFlightViewIgc3D()), actionCollection(),
			"view_flight_3D");
			
  KSelectAction* viewFlightDataType = new KSelectAction(
      i18n("Show Flightdata"), "idea", 0,
      actionCollection(), "view_flight_data");

  connect(viewFlightDataType, SIGNAL(activated(int)), this,
      SLOT(slotSelectFlightData(int)));

  QStringList dataList;
  dataList.append(i18n("Altitude"));
  dataList.append(i18n("Cycling"));
  dataList.append(i18n("Speed"));
  dataList.append(i18n("Vario"));
  dataList.append(i18n("Solid"));

  viewFlightDataType->setItems(dataList);
  // Heavy workaround! MapConfig should tell KFLogApp, which type is selected!
  viewFlightDataType->setCurrentItem(0);

  KActionMenu* flightMenu = new KActionMenu(i18n("F&light"),
      actionCollection(), "flight");
  flightMenu->insert(flightEvaluation);
  flightMenu->insert(flightOptimization);
  flightMenu->insert(flightOptimizationOLC);
  //  flightMenu->insert(viewWaypoints);
  flightMenu->insert(viewFlightDataType);
  flightMenu->insert(viewIgc3D);
  flightMenu->insert(olcDeclaration);
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

  KStdAction::preferences(this, SLOT(slotConfigureKFLog()), actionCollection());

  KActionMenu *w = new KActionMenu(i18n("&Window"), "igc",
      actionCollection(), "window");
  windowMenu = w->popupMenu();
  windowMenu->setCheckable(true);
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotWindowsMenuAboutToShow()));

  KActionMenu *m = new KActionMenu(i18n("&New"), "filenew", actionCollection(), "file_new");
  m->popupMenu()->insertItem(SmallIcon("waypoint"), i18n("&Waypoint"), waypoints, SLOT(slotNewWaypoint()));
  m->popupMenu()->insertItem(SmallIcon("task"), i18n("&Task"), &_globalMapContents, SLOT(slotNewTask()));
  m->popupMenu()->insertItem(i18n("&Flight group"), &_globalMapContents, SLOT(slotNewFlightGroup()));

  createGUI();
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
  // wir könnten mal Icons für die einzelnen Bereiche gebrauchen ...
  mapViewDock = createDockWidget("Map", 0, 0, i18n("Map"));
  dataViewDock = createDockWidget("Flight-Data", 0, 0, i18n("Flight-Data"));
  helpWindowDock = createDockWidget("Help", 0, 0, i18n("Help"));  
  mapControlDock = createDockWidget("Map-Control", 0, 0, i18n("Map-Control"));
  waypointsDock = createDockWidget("Waypoints", 0, 0, i18n("Waypoints"));
  tasksDock = createDockWidget("Tasks", 0, 0, i18n("Tasks"));
  legendDock = createDockWidget("Legend", 0, 0, i18n("Legend"));
  
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
  connect(waypointsDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideWaypointsDock()));
  connect(waypointsDock, SIGNAL(hasUndocked()),
      SLOT(slotHideWaypointsDock()));
  connect(tasksDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideTasksDock()));
  connect(tasksDock, SIGNAL(hasUndocked()),
      SLOT(slotHideTasksDock()));
  connect(legendDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideLegendDock()));
  connect(legendDock, SIGNAL(hasUndocked()),
      SLOT(slotHideLegendDock()));

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

  waypoints = new Waypoints(waypointsDock);
  waypointsDock->setWidget(waypoints);

  tasks = new Tasks(tasksDock);
  tasksDock->setWidget(tasks);

  legend = new TopoLegend(legendDock);
  legendDock->setWidget(legend);


  /* Standard positions for the docking windows
   * Arguments for manualDock():
   * dock target, dock side, remaining space in target (in percent)
   */
  dataViewDock->manualDock( mapViewDock, KDockWidget::DockRight, 67 );
  mapControlDock->manualDock( dataViewDock, KDockWidget::DockBottom, 62 );
  helpWindowDock->manualDock( mapControlDock, KDockWidget::DockCenter);    
  waypointsDock->manualDock(mapViewDock, KDockWidget::DockBottom, 70);
  legendDock->manualDock(waypointsDock, KDockWidget::DockRight, 90);
  tasksDock->manualDock(waypointsDock, KDockWidget::DockCenter, 50 );  
  
  connect(map, SIGNAL(changed(QSize)), mapControl,
      SLOT(slotShowMapData(QSize)));
  connect(map, SIGNAL(waypointSelected(Waypoint *)), waypoints,
    SLOT(slotAddWaypoint(Waypoint *)));
  connect(map, SIGNAL(taskPlanningEnd()), tasks,
    SLOT(slotUpdateTask()));
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
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), tasks,
      SLOT(setCurrentTask()));

  connect(waypoints, SIGNAL(copyWaypoint2Task(Waypoint *)), map,
      SLOT(slotAppendWaypoint2Task(Waypoint *)));

  connect(waypoints, SIGNAL(waypointCatalogChanged( WaypointCatalog * )), map,
      SLOT(slotWaypointCatalogChanged( WaypointCatalog * )));

  connect(tasks, SIGNAL(newTask()), &_globalMapContents,
      SLOT(slotNewTask()));
  connect(tasks, SIGNAL(openTask()), this, SLOT(slotTaskOpen()));
  connect(tasks, SIGNAL(closeTask()), &_globalMapContents, SLOT(closeFlight()));
  connect(tasks, SIGNAL(flightSelected(BaseFlightElement *)), &_globalMapContents,
      SLOT(slotSetFlight(BaseFlightElement *)));
  connect(&_globalMapContents, SIGNAL(newTaskAdded(FlightTask *)), tasks,
      SLOT(slotAppendTask(FlightTask *)));

  connect(waypoints, SIGNAL(centerMap(int, int)), &_globalMapMatrix,
      SLOT(slotCenterTo(int, int)));
}

void KFLogApp::slotShowPointInfo(const QPoint pos,
    const struct flightPoint& point)
{
  statusBar()->clear();
  QString text;
  text.sprintf("%s", (const char*)printTime(point.time, true));
  statusTimeL->setText(text);
  text.sprintf("%4d m  ", point.height);
  statusAltitudeL->setText(text);
  text.sprintf("%3.1f km/h  ", getSpeed(point));
  statusSpeedL->setText(text);
  text.sprintf("%2.1f m/s  ", getVario(point));
  statusVarioL->setText(text);

  statusLatL->setText(printPos(pos.y()));
  statusLonL->setText(printPos(pos.x(), false));
}

void KFLogApp::slotShowPointInfo(const QPoint pos)
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
  config->writeEntry("Show Toolbar", viewToolBar->isChecked());
  config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
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

  fileOpenRecent->saveEntries(config,"Recent Files");
}

void KFLogApp::readOptions()
{
  config->setGroup("General Options");

  // bar status settings
  bool bViewToolbar = config->readBoolEntry("Show Toolbar", true);
  viewToolBar->setChecked(bViewToolbar);
  slotViewToolBar();

  bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();

  // bar position settings
  KToolBar::BarPosition toolBarPos;
  toolBarPos=(KToolBar::BarPosition) config->readNumEntry("ToolBarPos",
      KToolBar::Top);
  toolBar("mainToolBar")->setBarPos(toolBarPos);
  QSize size=config->readSizeEntry("Geometry", new QSize(950,700));

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");
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

void KFLogApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.igc *.IGC", this,
      i18n("Select IGC-File"), true);
  IGCPreview* preview = new IGCPreview(dlg);
  dlg->setPreviewWidget(preview);
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

//  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName))
    {
      KNotifyClient::event(i18n("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  extern MapContents _globalMapContents;
  QFile file (fName);
  if(_globalMapContents.loadFlight(file))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  extern MapContents _globalMapContents;
  if(url.isLocalFile())
    {
      QFile file (url.path());
      if(_globalMapContents.loadFlight(file))
        {
          // Just a workaround. It's the only way to not have the item
          // checked after loading the flight. Otherwise we had to take
          // care that the item is unchecked, when the flight is closed ...
          fileOpenRecent->setCurrentItem(-1);
        }
    }

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

  // viewCenterFlight is enabled, when a flight is loaded ...
  MapPrint::MapPrint(viewCenterFlight->isEnabled());

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotOlcDeclaration()
{
  // currently not available ...

  extern MapContents _globalMapContents;
  Flight* f = (Flight*)_globalMapContents.getFlight();

  if(f == NULL)  return;

  if(f->getTypeID() != BaseMapElement::Flight)
    {
      KMessageBox::sorry(this, i18n("You can only send flights to the OLC!"),
          i18n("No flight found"));
      return;
    }

  OLCDialog* dlg = new OLCDialog(this, "olc-dialog", f);

  dlg->show();
}

void KFLogApp::slotFlightPrint()
{
  slotStatusMsg(i18n("Printing..."));

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
            tmp.sprintf(i18n("Not yet available for type : %d"), f->getTypeID());
            KMessageBox::sorry(0, tmp);
        }
    }
  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
  //saveOptions();   //Now done in queryClose

  close();
}

void KFLogApp::slotViewToolBar()
{
  slotStatusMsg(i18n("Toggling toolbar..."));

  if(!viewToolBar->isChecked())
      toolBar("mainToolBar")->hide();
  else
      toolBar("mainToolBar")->show();

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotViewStatusBar()
{
  slotStatusMsg(i18n("Toggle the statusbar..."));

  if(!viewStatusBar->isChecked())
      statusBar()->hide();
  else
      statusBar()->show();

  slotStatusMsg(i18n("Ready."));
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

void KFLogApp::slotHideWaypointsDock() { viewWaypoints->setChecked(false); }

void KFLogApp::slotHideTasksDock() { viewTasks->setChecked(false); }

void KFLogApp::slotHideLegendDock() { viewLegend->setChecked(false); }

void KFLogApp::slotCheckDockWidgetStatus()
{
  viewMapControl->setChecked(mapControlDock->isVisible());
  viewMap->setChecked(mapViewDock->isVisible());
  viewData->setChecked(dataViewDock->isVisible());
  viewHelpWindow->setChecked(helpWindowDock->isVisible());  
  viewWaypoints->setChecked(waypointsDock->isVisible());
  viewTasks->setChecked(tasksDock->isVisible());
  viewLegend->setChecked(legendDock->isVisible());
}

void KFLogApp::slotToggleDataView()  { dataViewDock->changeHideShowState(); }

void KFLogApp::slotToggleHelpWindow()  { helpWindowDock->changeHideShowState(); }

void KFLogApp::slotToggleMapControl() { mapControlDock->changeHideShowState(); }

void KFLogApp::slotToggleMap() { mapViewDock->changeHideShowState(); }

void KFLogApp::slotToggleWaypointsDock() { waypointsDock->changeHideShowState(); }

void KFLogApp::slotToggleTasksDock() { tasksDock->changeHideShowState(); }

void KFLogApp::slotToggleLegendDock() { legendDock->changeHideShowState(); }

void KFLogApp::slotSelectFlightData(int id)
{
  switch(id)
    {
      case 0:    // Altitude
        emit flightDataTypeChanged(MapConfig::Altitude);
        break;
      case 1:    // Cycling
        emit flightDataTypeChanged(MapConfig::Cycling);
        break;
      case 2:    // Speed
        emit flightDataTypeChanged(MapConfig::Speed);
        break;
      case 3:    // Vario
        emit flightDataTypeChanged(MapConfig::Vario);
        break;
      case 4:    // Solid color
        emit flightDataTypeChanged(MapConfig::Solid);
        break;
    }
  map->slotRedrawFlight();
}

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

void KFLogApp::slotCenterTo()
{
  CenterToDialog* center = new CenterToDialog(this, i18n("center-to-dialog"));

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
  QList<BaseFlightElement> flights = *(_globalMapContents.getFlightList());
  QListIterator<BaseFlightElement> it(flights);
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
            flightEvaluation->setEnabled(true);
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
//            mapPlanning->setEnabled(false);
            windowMenu->setEnabled(true);
            break;
          case BaseMapElement::Task:
            fileClose->setEnabled(true);
            flightPrint->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(false);
            flightEvaluation->setEnabled(false);
            flightEvaluation->setEnabled(false);
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
//            mapPlanning->setEnabled(true);
            windowMenu->setEnabled(true);
            break;
          case BaseMapElement::FlightGroup:
            fileClose->setEnabled(true);
            flightPrint->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
            flightEvaluation->setEnabled(true);
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
      flightEvaluation->setEnabled(false);
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
//     mapPlanning->setEnabled(false);
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

  surfaces.append(new TranslationElement(Airport::NotSet, i18n("Unknown")));
  surfaces.append(new TranslationElement(Airport::Grass, i18n("Grass")));
  surfaces.append(new TranslationElement(Airport::Asphalt, i18n("Asphalt")));
  surfaces.append(new TranslationElement(Airport::Concrete, i18n("Concrete")));

  surfaces.sort();
}

void KFLogApp::initTypes()
{
  waypointTypes.setAutoDelete(true);

  // don't know if we really need all of them
  waypointTypes.append(new TranslationElement(BaseMapElement::AerialRailway, i18n("Aerial railway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Airfield, i18n("Airfield")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Airport, i18n("Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::AmbHeliport, i18n("Ambul. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Ballon, i18n("Ballon")));
  waypointTypes.append(new TranslationElement(BaseMapElement::City, i18n("City")));
  waypointTypes.append(new TranslationElement(BaseMapElement::CivHeliport, i18n("Civil Heliport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::IntAirport, i18n("Int. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::MilAirport, i18n("Mil. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::CivMilAirport, i18n("Civil/Mil. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::ClosedAirfield, i18n("Closed Airfield")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Glidersite, i18n("Glider site")));
  waypointTypes.append(new TranslationElement(BaseMapElement::HangGlider, i18n("Hang glider")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Highway, i18n("Highway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Landmark, i18n("Landmark")));
  waypointTypes.append(new TranslationElement(BaseMapElement::MilHeliport, i18n("Mil. Heliport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::UltraLight, i18n("Ultralight")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Parachute, i18n("Parachute")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Outlanding, i18n("Outlanding")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Obstacle, i18n("Obstacle")));
  waypointTypes.append(new TranslationElement(BaseMapElement::ObstacleGroup, i18n("Obstacle group")));
  waypointTypes.append(new TranslationElement(BaseMapElement::LightObstacleGroup, i18n("Obstacle group (lighted)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::LightObstacle, i18n("Obstacle (lighted)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Railway, i18n("Railway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Road, i18n("Road")));
  waypointTypes.append(new TranslationElement(BaseMapElement::PopulationPlace, i18n("Village,City")));

  waypointTypes.sort();
}

/** No descriptions */
void KFLogApp::slotImportFlightGearFile(){
  slotStatusMsg(i18n("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.flightgear *.FLIGHTGEAR", this,
      i18n("Select FlightGear File"), true);
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

//  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName))
    {
      KNotifyClient::event(i18n("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  extern MapContents _globalMapContents;
  QFile file (fName);
  if(_globalMapContents.importFlightGearFile(file))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(i18n("Ready."));
}

/** Import a file from Gardown (DOS)  */
void KFLogApp::slotImportGardownFile(){
  slotStatusMsg(i18n("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.gdn *.GDN *.trk *TRK", this,
      i18n("Select Gardown File"), true);
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

//  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName))
    {
      KNotifyClient::event(i18n("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  extern MapContents _globalMapContents;
  QFile file (fName);
  if(_globalMapContents.importGardownFile(file))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(i18n("Ready."));
}
/** No descriptions */
void KFLogApp::slotSavePixmap( KURL url, int width, int height ){
  map->slotSavePixmap(url,width,height);
}

/** No descriptions */
void KFLogApp::slotTaskOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KFileDialog* dlg = new KFileDialog(flightDir, "*.kflogtsk *.KFLOGTSK", this,
      i18n("Select Task-File"), true);
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

//  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fName))
    {
      KNotifyClient::event(i18n("Can not download file %1").arg(fUrl.url()));
      return;
    }

  QFileInfo fInfo(fName);
  flightDir = fInfo.dirPath();
  extern MapContents _globalMapContents;
  QFile file(fName);
  _globalMapContents.loadTask(file);

  slotStatusMsg(i18n("Ready."));
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

  taskTypes.append(new TranslationElement(FlightTask::Route, i18n("Traditional Route")));
  taskTypes.append(new TranslationElement(FlightTask::FAIArea, i18n("FAI Area")));
  //taskTypes.append(new TranslationElement(FlightTask::AAT, i18n("Area Assigned")));

  taskTypes.sort();
}

/** Re-enables all messages turned off using "Don't show this dialog again" checkboxes. */
void KFLogApp::slotEnableMessages(){
  KMessageBox::enableAllMessages();
}

/** Connects the dialogs addWaypoint signal to the waypoint object. */
void KFLogApp::slotRegisterWaypointDialog(QWidget * dialog){
  connect(dialog, SIGNAL(addWaypoint(Waypoint *)), waypoints, SLOT(slotAddWaypoint(Waypoint *)));  
}
