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
#include <dataview.h>
#include <evaluationdialog.h>
#include <flight.h>
#include <flightdataprint.h>
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
#include <igc3ddialog.h>
#include "basemapelement.h"
#include "airport.h"

#define STATUS_LABEL(a,b,c) \
  a = new KStatusBarLabel( "", 0, statusBar() ); \
  a->setFixedWidth( b ); \
  a->setFixedHeight( statusLabel->sizeHint().height() ); \
  a->setFrameStyle( QFrame::NoFrame | QFrame::Plain ); \
  a->setMargin(0); \
  a->setLineWidth(0); \
  a->setAlignment( c | AlignVCenter );

#define POS_STRINGS(point) \
  latitude = point.lat(); \
  longitude = point.lon(); \
  if(latitude < 0) latH = "S"; \
  if(longitude < 0) lonH = "W"; \
  degree = latitude / 600000; \
  min = (latitude - (degree * 600000)) / 10000; \
  min_deg = (latitude - (degree * 600000) - (min * 10000)); \
  min_deg = min_deg / 1000; \
  latG.sprintf("%d", degree); \
  latM.sprintf("%d", min); \
  latMD.sprintf("%d", min_deg); \
  degree = longitude / 600000; \
  min = (longitude - (degree * 600000)) / 10000; \
  min_deg = (longitude - (degree * 600000) - (min * 10000)); \
  min_deg = min_deg / 1000; \
  lonG.sprintf("%d", degree); \
  lonM.sprintf("%d", min); \
  lonMD.sprintf("%d", min_deg);

TranslationList surfaces;

TranslationList waypointTypes;

KFLogApp::KFLogApp()
  : KDockMainWindow(0, "KFLogMainWindow"), showStartLogo(false)
{
  extern MapConfig _globalMapConfig;
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  config = kapp->config();

  config->setGroup("General Options");

  if (config->readBoolEntry("Logo", true) && (!kapp->isRestored() ) )
    {
      showStartLogo = true;
      startLogo = new KFLogStartLogo();
      startLogo->show();
    }

  // initialize internal translation lists
  initSurfaces();
  initTypes();

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
  initActions();

  if(showStartLogo)
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

  connect(map, SIGNAL(showTaskText(FlightTask*)),
      dataView, SLOT(slotShowTaskText(FlightTask*)));
  connect(map, SIGNAL(taskPlanningEnd()), dataView, SLOT(setFlightData()));
  connect(map, SIGNAL(showPoint(const QPoint)),
      this, SLOT(slotShowPointInfo(const QPoint)));

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

  fileRecorder = new KAction(i18n("Open Recorder"), "connect_no",
      0, this, SLOT(slotOpenRecorderDialog()), actionCollection(),
      "recorderdialog");

  fileImportFlightGearFile = new KAction(i18n("Import FlightGear File"), "import_flightgear",
      0, this, SLOT(slotImportFlightGearFile()), actionCollection(),
      "flightgear");

  fileImportGardownFile = new KAction(i18n("Import Gardown File"), "import_gardown",
      0, this, SLOT(slotImportGardownFile()), actionCollection(),
      "gardown");


  KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());

  flightPrint = new KAction(i18n("Print Flightdata"), "fileprint",
      0, this, SLOT(slotFlightPrint()), actionCollection(), "flight_print");

  KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

  KActionMenu* mapMoveMenu = new KActionMenu(i18n("Move map"), "move",
      actionCollection(), "move_map");
  mapMoveMenu->setDelayed(false);

  KAction* viewRedraw = KStdAction::redisplay(map, SLOT(slotRedrawMap()),
      actionCollection());
  viewRedraw->setAccel(Key_F5);

  viewCenterTask = new KAction(i18n("Center to &Task"), "centertask",
      Key_F6, map,
      SLOT(slotCenterToTask()), actionCollection(), "view_center_task");
  viewCenterFlight = new KAction(i18n("Center to &Flight"), "centerflight",
      Key_F7, map,
      SLOT(slotCenterToFlight()), actionCollection(), "view_center_flight");

  new KAction(i18n("Center to &Homesite"), "gohome",
      KStdAccel::key(KStdAccel::Home), &_globalMapMatrix,
      SLOT(slotCenterToHome()), actionCollection(), "view_center_home");

  mapMoveMenu->insert(new KAction(i18n("move map north-west"), "movemap_nw", 0,
      &_globalMapMatrix, SLOT(slotMoveMapNW()), actionCollection(), "move_map_nw"));
  mapMoveMenu->insert(new KAction(i18n("move map north"), "movemap_n", 0,
      &_globalMapMatrix, SLOT(slotMoveMapN()), actionCollection(), "move_map_n"));
  mapMoveMenu->insert(new KAction(i18n("move map northeast"), "movemap_ne", 0,
      &_globalMapMatrix, SLOT(slotMoveMapNE()), actionCollection(), "move_map_ne"));
  mapMoveMenu->insert(new KAction(i18n("move map west"), "movemap_w", 0,
      &_globalMapMatrix, SLOT(slotMoveMapW()), actionCollection(), "move_map_w"));
  mapMoveMenu->insert(new KAction(i18n("move map east"), "movemap_e", 0,
      &_globalMapMatrix, SLOT(slotMoveMapE()), actionCollection(), "move_map_e"));
  mapMoveMenu->insert(new KAction(i18n("move map south-west"), "movemap_sw", 0,
      &_globalMapMatrix, SLOT(slotMoveMapSW()), actionCollection(), "move_map_sw"));
  mapMoveMenu->insert(new KAction(i18n("move map south"), "movemap_s", 0,
      &_globalMapMatrix, SLOT(slotMoveMapS()), actionCollection(), "move_map_s"));
  mapMoveMenu->insert(new KAction(i18n("move map south-east"), "movemap_se", 0,
      &_globalMapMatrix, SLOT(slotMoveMapSE()), actionCollection(), "move_map_se"));

  KStdAction::zoomIn(&_globalMapMatrix, SLOT(slotZoomIn()), actionCollection());
  KStdAction::zoomOut(&_globalMapMatrix, SLOT(slotZoomOut()), actionCollection());
  /*
   * Wir brauchen dringend Icons für diese beiden Aktionen, damit man
   * es auch in die Werkzeugleisten packen kann!
   */
  viewData = new KToggleAction(i18n("Show Flightdata"), 0, this,
      SLOT(slotToggleDataView()), actionCollection(), "toggle_data_view");
  viewMapControl = new KToggleAction(i18n("Show Mapcontrol"), 0, this,
      SLOT(slotToggleMapControl()), actionCollection(), "toggle_map_control");
  viewMap = new KToggleAction(i18n("Show Map"), 0, this,
      SLOT(slotToggleMap()), actionCollection(), "toggle_map");

  /**
    * Graphical Planning
    */
/*
  mapPlanning = new KToggleAction(i18n("graphical Taskplanning"), Key_F4,
      map, SLOT(slotActivatePlanning()), actionCollection(), "activate_planning");
*/
  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());

  flightEvaluation = new KAction(i18n("Evaluation"), "flightevaluation",
      CTRL+Key_E, this, SLOT(slotEvaluateFlight()), actionCollection(),
      "evaluate_flight");

  viewWaypoints = new KToggleAction(i18n("Waypoints"), "waypoint",
      CTRL+Key_T, this, SLOT(slotToggleWaypointsDock()), actionCollection(),
      "waypoints");

  flightOptimization = new KAction(i18n("Optimize"), "wizard", 0,
      this, SLOT(slotOptimizeFlight()), actionCollection(), "optimize_flight");

  olcDeclaration = new KAction(i18n("send OLC-Declaration"), 0,
      this, SLOT(slotOlcDeclaration()), actionCollection(), "olc_declaration");

  //Animation actions
  animateFlightStart = new KAction(i18n("&Start Flight Animation"), "1rightarrow",
			Key_F12, map, SLOT(slotAnimateFlightStart()), actionCollection(),
			"start_animate");
  animateFlightStop = new KAction(i18n("Stop Flight &Animation"), "player_stop",
			Key_F11, map, SLOT(slotAnimateFlightStop()), actionCollection(),
			"stop_animate");
	//Stepping actions
	stepFlightNext = new KAction(i18n("Next Flight Point"), "forward",
			Key_Up, map, SLOT(slotFlightNext()), actionCollection(),
		  "next_flight_point");
	stepFlightPrev = new KAction(i18n("Prev Flight Point"), "back",
			Key_Down, map, SLOT(slotFlightPrev()), actionCollection(),
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
  flightMenu->insert(viewWaypoints);
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
  m->popupMenu()->insertItem(i18n("&Task"), &_globalMapContents, SLOT(slotNewTask()));
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
  mapControlDock = createDockWidget("Map-Control", 0, 0, i18n("Map-Control"));
  waypointsDock = createDockWidget("Waypoints", 0, 0, i18n("Waypoints"));

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
  connect(waypointsDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideWaypointsDock()));
  connect(waypointsDock, SIGNAL(hasUndocked()),
      SLOT(slotHideWaypointsDock()));

  setView(mapViewDock);
  setMainDockWidget(mapViewDock);

  QFrame* mapViewFrame = new QFrame(mapViewDock);
  map = new Map(this, mapViewFrame, "KFLog-Map");

  QVBoxLayout* mapLayout = new QVBoxLayout(mapViewFrame,2,1);
  mapLayout->addWidget(map);
  mapLayout->activate();

  QFrame* mapControlFrame = new QFrame(mapControlDock);
  mapControl = new MapControlView(mapControlFrame);
  mapControlDock->setWidget(mapControlFrame);

  mapViewDock->setWidget(mapViewFrame);

  dataView = new DataView(dataViewDock);
  dataViewDock->setWidget(dataView);

  waypoints = new Waypoints(waypointsDock);
  waypointsDock->setWidget(waypoints);

  /* Argumente für manualDock():
   * dock target, dock side, relation target/this (in percent)
   */
  dataViewDock->manualDock( mapViewDock, KDockWidget::DockRight, 71 );
  mapControlDock->manualDock( dataViewDock, KDockWidget::DockBottom, 75 );
  waypointsDock->manualDock(mapViewDock, KDockWidget::DockBottom, 71);

  connect(map, SIGNAL(changed(QSize)), mapControl,
      SLOT(slotShowMapData(QSize)));

  connect(map, SIGNAL(waypointSelected(wayPoint *)), waypoints,
    SLOT(slotAddWaypoint(wayPoint *)));

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

  connect(waypoints, SIGNAL(copyWaypoint2Task(wayPoint *)), map,
      SLOT(slotAppendWaypoint2Task(wayPoint *)));
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
  QSize size=config->readSizeEntry("Geometry");

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");
  config->setGroup("Path");
  flightDir = config->readEntry("DefaultFlightDirectory",
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
  if(_globalMapContents.loadFlight(fName))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  extern MapContents _globalMapContents;
  if(url.isLocalFile())
    {
      if(_globalMapContents.loadFlight(url.path()))
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

  OLCDialog* dlg = new OLCDialog(this, "olc-dialog");

  dlg->show();

  return;

  extern MapContents _globalMapContents;
  Flight* f = (Flight*)_globalMapContents.getFlight();

  if(f == NULL)  return;

  if(f->getTypeID() != BaseMapElement::Flight)
    {
      KMessageBox::sorry(this, i18n("You can only send flights to the OLC!"),
          i18n("No flight found"));
      return;
    }

  FlightTask t = f->getTask();

  config->setGroup("Personal Data");

  QString link;

  // ungeklärte Felder:
  QString index("100");     // Sollte sowieso mal in eine Config-Datei ...
  QString glider("1");      // "1" für reine Segelflugzeuge

  QString dateString;
  dateString.sprintf("%d", f->getDate().year() + f->getDate().dayOfYear());

  QString compClass("0");
  switch(f->getCompetitionClass())
    {
      case Flight::PW5:
        compClass = "1";
        break;
      case Flight::Club:
        compClass = "2";
        break;
      case Flight::Standard:
        compClass = "3";
        break;
      case Flight::FifteenMeter:
        compClass = "4";
        break;
      case Flight::EightteenMeter:
        compClass = "5";
        break;
      case Flight::DoubleSitter:
        compClass = "6";
        break;
      case Flight::OpenClass:
        compClass = "7";
        break;
      case Flight::HGFlexWing:
        compClass = "1";
        break;
      case Flight::HGRigidWing:
        compClass = "2";
        break;
      case Flight::ParaGlider:
        compClass = "10";
        break;
      case Flight::ParaOpen:
        compClass = "3";
        break;
      case Flight::ParaSport:
        compClass = "4";
        break;
      case Flight::ParaTandem:
        compClass = "5";
        break;
    }

  // personal info
  link = "OLCvnolc=" + config->readEntry("PreName", "")
      + "&na=" + config->readEntry("SurName", "")
      + "&geb=" + config->readEntry("Birthday", "");

  // glider info
  link = link + "&gty=" + f->getHeader().at(2)
      + "&gid=" + f->getHeader().at(1)
      + "&ind=" + index
      + "&klasse=" + compClass + "&flugzeug=" + glider;

  // The olc need an "offical" filename. So we have to create it here ...
  link += "&igcfn=" + f->getFileName() + "&sta=" + t.getWPList().first()->name +
          "&ft=" + dateString +  "&s0=" + printTime(f->getStartTime(), true);

  QString latH("N"), latG, latM, latMD;
  QString lonH("E"), lonG, lonM, lonMD;

  int latitude, longitude;
  int degree, min, min_deg;

  // the beginning of the task should allways be the second point ...
  POS_STRINGS(t.getWPList().at(1)->origP)

  // Abflugpunkt
  link += "&w0bh=" + latH + "&w0bg=" + latG + "&w0bm=" + latM + "&w0bmd=" + latMD
      + "&w0lh=" + lonH + "&w0lg=" + lonG + "&w0lm=" + lonM + "&w0lmd=" + lonMD;

  if(t.getTaskType() == FlightTask::FAI || t.getTaskType() == FlightTask::Dreieck ||
      t.getTaskType() == FlightTask::FAI_S || t.getTaskType() == FlightTask::Dreieck_S)
    {
      // we have a triangle ...
      POS_STRINGS(t.getWPList().at(2)->origP)

      link += "&w1bh=" + latH + "&w1bg=" + latG + "&w1bm=" + latM + "&w1bmd=" + latMD
          + "&w1lh=" + lonH + "&w1lg=" + lonG + "&w1lm=" + lonM + "&w1lmd=" + lonMD;

      POS_STRINGS(t.getWPList().at(3)->origP)
      link += "&w2bh=" + latH + "&w2bg=" + latG + "&w2bm=" + latM + "&w2bmd=" + latMD
          + "&w2lh=" + lonH + "&w2lg=" + lonG + "&w2lm=" + lonM + "&w2lmd=" + lonMD;

      POS_STRINGS(t.getWPList().at(4)->origP)
      link += "&w3bh=" + latH + "&w3bg=" + latG + "&w3bm=" + latM + "&w3bmd=" + latMD
          + "&w3lh=" + lonH + "&w3lg=" + lonG + "&w3lm=" + lonM + "&w3lmd=" + lonMD;
    }

  // Endpunkt
  POS_STRINGS(t.getWPList().at(t.getWPList().count() - 2)->origP)
  link += "&w4bh=" + latH + "&w4bg=" + latG + "&w4bm=" + latM + "&w4bmd=" + latMD
      + "&w4lh=" + lonH + "&w4lg=" + lonG + "&w4lm=" + lonM + "&w4lmd=" + lonMD;

  link += "&s4=" + printTime(f->getLandTime(), true);

  QFile igcFile(f->getFileName());

//  if(!igcFile.open(IO_ReadOnly))
//    {
//      KMessageBox::error(0,
//          i18n("You don't have permission to access file<BR><B>%1</B>").arg(igcFile.name()),
//          i18n("No permission"));
//      return;
//    }

//  QTextStream igcStream(&igcFile);
  QString igcString;

//  while(!igcStream.eof())
//    {
//      igcString += igcStream.readLine();
//    }

  // IGC File
  link = link + "&software=" + "kflog-" + VERSION + "&IGCigcIGC=" + igcString;

  // Link für Hängegleiter:
  //   http://www.segelflugszene.de/olc-cgi/holc-d/olc
  // Link für Segelflüge:
  //   http://www.segelflugszene.de/olc-cgi/olc-d/olc
  // Ausserdem muss das Land noch konfigurierbar sein.
  link = "http://www.segelflugszene.de/olc-cgi/olc-d/olc?" + link;

//  warning(link);

  // Because "%" is used in placeholder in a string, we have to add it this way ...
  char prozent = 0x25;
  QString spaceString = QString(QChar(prozent)) + "20";
  link.replace(QRegExp("[ ]"), spaceString);

  browser.clearArguments();
  browser << "konqueror" << link;
  browser.start();
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
  saveOptions();

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

void KFLogApp::slotHideWaypointsDock() { viewWaypoints->setChecked(false); }

void KFLogApp::slotCheckDockWidgetStatus()
{
  viewMapControl->setChecked(mapControlDock->isVisible());
  viewMap->setChecked(mapViewDock->isVisible());
  viewData->setChecked(dataViewDock->isVisible());
  viewWaypoints->setChecked(waypointsDock->isVisible());
}

void KFLogApp::slotToggleDataView()  { dataViewDock->changeHideShowState(); }

void KFLogApp::slotToggleMapControl() { mapControlDock->changeHideShowState(); }

void KFLogApp::slotToggleMap() { mapViewDock->changeHideShowState(); }

void KFLogApp::slotToggleWaypointsDock() { waypointsDock->changeHideShowState(); }

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
  EvaluationDialog* evaluation =  new EvaluationDialog(this);
  extern MapContents _globalMapContents;
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), evaluation,
      SLOT(slotShowFlightData()));
  connect(evaluation, SIGNAL(showCursor(QPoint, QPoint)), map,
      SLOT(slotDrawCursor(QPoint, QPoint)));

//  evaluation->show();
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
  if(showStartLogo && startLogo != NULL)
    {
      delete startLogo;
      startLogo = 0L;
    }
}

void KFLogApp::slotFlightViewIgc3D()
{
  Igc3DDialog * igc3d = new Igc3DDialog(this);
  extern MapContents _globalMapContents;
  connect(&_globalMapContents, SIGNAL(currentFlightChanged()), igc3d,
      SLOT(slotShowFlightData()));
}

bool KFLogApp::queryClose()
{
  return waypoints->saveChanges();
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
            flightOptimization->setEnabled(false);
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
  RecorderDialog* dlg = new RecorderDialog(this, config, "recorderDialog");
  dlg->exec();
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
  waypointTypes.append(new TranslationElement(BaseMapElement::Village, i18n("Village")));

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
  if(_globalMapContents.importFlightGearFile(fName))
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
  if(_globalMapContents.importGardownFile(fName))
      fileOpenRecent->addURL(fUrl);

  slotStatusMsg(i18n("Ready."));
}
