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
#include <taskandwaypoint.h>
#include <tasks.h>
#include <igc3ddialog.h>

#define STATUS_LABEL(a,b,c) \
  a = new KStatusBarLabel( "", 0, statusBar() ); \
  a->setFixedWidth( b ); \
  a->setFixedHeight( statusLabel->sizeHint().height() ); \
  a->setFrameStyle( QFrame::NoFrame | QFrame::Plain ); \
  a->setMargin(0); \
  a->setLineWidth(0); \
  a->setAlignment( c | AlignVCenter );

KFLogApp::KFLogApp()
  : KDockMainWindow(0, "KFLogMainWindow"), showStartLogo(false)
{
  extern MapConfig _globalMapConfig;
  extern MapMatrix _globalMapMatrix;

  config = kapp->config();

  config->setGroup("General Options");

  if (config->readBoolEntry("Logo", true) && (!kapp->isRestored() ) )
    {
      showStartLogo = true;
      startLogo = new KFLogStartLogo();
      startLogo->show();
    }

  connect(&_globalMapMatrix, SIGNAL(displayMatrixValues(int, bool)),
      &_globalMapConfig, SLOT(slotSetMatrixValues(int, bool)));
  connect(&_globalMapMatrix, SIGNAL(printMatrixValues(int)),
      &_globalMapConfig, SLOT(slotSetPrintMatrixValues(int)));
  connect(&_globalMapConfig, SIGNAL(configChanged()), &_globalMapMatrix,
      SLOT(slotInitMatrix()));
  connect(this, SIGNAL(flightDataTypeChanged(int)), &_globalMapConfig,
      SLOT(slotSetFlightDataType(int)));

  _globalMapConfig.slotReadConfig();

  initStatusBar();
  initView();
  initActions();

  if(showStartLogo)
      startLogo->raise();

  readOptions();

  fileClose->setEnabled(false);
  viewCenterTask->setEnabled(false);
  viewCenterFlight->setEnabled(false);
  flightEvaluation->setEnabled(false);
  flightPrint->setEnabled(false);
  animateFlightStart->setEnabled(false);
  animateFlightStop->setEnabled(false);
  stepFlightNext->setEnabled(false);
  stepFlightPrev->setEnabled(false);
  stepFlightStepNext->setEnabled(false);
  stepFlightStepPrev->setEnabled(false);
  stepFlightHome->setEnabled(false);
  stepFlightEnd->setEnabled(false);
  viewIgc3D->setEnabled(false);


  activateDock();

  slotCheckDockWidgetStatus();
  // Heavy workaround! MapConfig should tell KFLogApp, which type is selected!
  slotSelectFlightData(0);

  connect(&_globalMapMatrix, SIGNAL(matrixChanged()), map,
      SLOT(slotRedrawMap()));
  connect(map, SIGNAL(showFlightPoint(const QPoint, const struct flightPoint&)),
      this, SLOT(slotShowPointInfo(const QPoint, const struct flightPoint&)));
  // Plannung
  connect(map, SIGNAL(showTaskText(QArray<SinglePoint*>, QPoint)),
      dataView, SLOT(slotShowTaskText(QArray<SinglePoint*>, QPoint)));
  connect(map, SIGNAL(showPoint(const QPoint)),
      this, SLOT(slotShowPointInfo(const QPoint)));
}

KFLogApp::~KFLogApp()
{

}

void KFLogApp::initActions()
{
  extern MapMatrix _globalMapMatrix;

  new KAction(i18n("&Open Flight"), "fileopen",
      KStdAccel::key(KStdAccel::Open), this, SLOT(slotFileOpen()),
      actionCollection(), "file_open");

  fileOpenRecent = KStdAction::openRecent(this,
      SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileClose = new KAction(i18n("Close Flight"), "fileclose",
      KStdAccel::key(KStdAccel::Close), this, SLOT(slotFileClose()),
      actionCollection(), "file_close");

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
  mapPlanning = new KToggleAction(i18n("graphical Taskplanning"), Key_F4,
      map, SLOT(slotActivatePlanning()), actionCollection(), "activate_planning");

  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());

  flightEvaluation = new KAction(i18n("Evaluation"), "flightevaluation",
      CTRL+Key_E, this, SLOT(slotEvaluateFlight()), actionCollection(),
      "evaluate_flight");

  viewTaskAndWaypoint = new KToggleAction(i18n("Task && Waypoints"), "waypoint",
      CTRL+Key_T, this, SLOT(slotToggleTaskAndWaypointDock()), actionCollection(),
      "task_and_waypoint");

  flightOptimization = new KAction(i18n("Optimize"), "wizard", 0,
      this, SLOT(slotOptimizeFlight()), actionCollection(), "optimize_flight");

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
  flightMenu->insert(viewTaskAndWaypoint);
  flightMenu->insert(viewFlightDataType);
  flightMenu->insert(viewIgc3D);
  flightMenu->insert(mapPlanning);
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
  taskAndWaypointDock = createDockWidget("Waypoints", 0, 0, i18n("Waypoints"));
  taskDock = createDockWidget("Tasks", 0, 0, i18n("Tasks"));

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
  connect(taskAndWaypointDock, SIGNAL(iMBeingClosed()),
      SLOT(slotHideTaskAndWaypointDock()));
  connect(taskAndWaypointDock, SIGNAL(hasUndocked()),
      SLOT(slotHideTaskAndWaypointDock()));

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

  taskAndWaypoint = new TaskAndWaypoint(taskAndWaypointDock);
  taskAndWaypointDock->setWidget(taskAndWaypoint);

  taskView = new Tasks(taskDock);
  taskDock->setWidget(taskView);

  /* Argumente für manualDock():
   * dock target, dock side, relation target/this (in percent)
   */
  dataViewDock->manualDock( mapViewDock, KDockWidget::DockRight, 71 );
  taskDock->manualDock( dataViewDock, KDockWidget::DockBottom, 20);
  mapControlDock->manualDock( dataViewDock, KDockWidget::DockBottom, 75 );
  taskAndWaypointDock->manualDock(mapViewDock, KDockWidget::DockBottom, 71);

  connect(map, SIGNAL(changed(QSize)), mapControl,
      SLOT(slotShowMapData(QSize)));

  connect(map, SIGNAL(waypointSelected(WaypointElement *)), taskAndWaypoint,
    SLOT(slotAddWaypoint(WaypointElement *)));

  extern MapMatrix _globalMapMatrix;
  connect(mapControl, SIGNAL(scaleChanged(double)), &_globalMapMatrix,
      SLOT(slotSetScale(double)));

  connect(dataView, SIGNAL(wpSelected(const unsigned int)), map,
      SLOT(slotCenterToWaypoint(const unsigned int)));
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

void KFLogApp::slotFileClose()
{
  map->slotDeleteFlightLayer();
  extern MapContents _globalMapContents;
  if(_globalMapContents.getFlightList()->count() == 0)
    {
      fileClose->setEnabled(false);
      viewCenterTask->setEnabled(false);
      viewCenterFlight->setEnabled(false);
      flightEvaluation->setEnabled(false);
      flightPrint->setEnabled(false);
      animateFlightStart->setEnabled(false);
      animateFlightStop->setEnabled(false);
			stepFlightNext->setEnabled(false);
		  stepFlightPrev->setEnabled(false);
		  stepFlightStepNext->setEnabled(false);
		  stepFlightStepPrev->setEnabled(false);
		  stepFlightHome->setEnabled(false);
		  stepFlightEnd->setEnabled(false);
		  viewIgc3D->setEnabled(false);
    }
}

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
    {
      dataView->setFlightData(_globalMapContents.getFlight());
      fileClose->setEnabled(true);
      viewCenterTask->setEnabled(true);
      viewCenterFlight->setEnabled(true);
      flightEvaluation->setEnabled(true);
      flightPrint->setEnabled(true);
      animateFlightStart->setEnabled(true);
      animateFlightStop->setEnabled(true);
		  stepFlightNext->setEnabled(true);
		  stepFlightPrev->setEnabled(true);
		  stepFlightStepNext->setEnabled(true);
		  stepFlightStepPrev->setEnabled(true);
		  stepFlightHome->setEnabled(true);
		  stepFlightEnd->setEnabled(true);
		  viewIgc3D->setEnabled(true);

      fileOpenRecent->addURL(fUrl);

      // Hier wird der Flug 2x neu gezeichnet, denn erst beim
      // ersten Zeichnen werden die Rahmen von Flug und Aufgabe
      // bestimmt.
      map->slotRedrawFlight();
      map->slotCenterToFlight();
    }

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
          dataView->setFlightData(_globalMapContents.getFlight());
          fileClose->setEnabled(true);
          viewCenterTask->setEnabled(true);
          viewCenterFlight->setEnabled(true);
          flightEvaluation->setEnabled(true);
          flightPrint->setEnabled(true);
          animateFlightStart->setEnabled(true);
          animateFlightStop->setEnabled(true);
				  stepFlightNext->setEnabled(true);
				  stepFlightPrev->setEnabled(true);
				  stepFlightStepNext->setEnabled(true);
				  stepFlightStepPrev->setEnabled(true);
				  stepFlightHome->setEnabled(true);
				  stepFlightEnd->setEnabled(true);
				  viewIgc3D->setEnabled(true);

          map->slotRedrawFlight();
          map->slotCenterToFlight();

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

void KFLogApp::slotFlightPrint()
{
  slotStatusMsg(i18n("Printing..."));

  extern MapContents _globalMapContents;
  FlightDataPrint::FlightDataPrint(_globalMapContents.getFlight());

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

void KFLogApp::slotHideTaskAndWaypointDock() { viewTaskAndWaypoint->setChecked(false); }

void KFLogApp::slotCheckDockWidgetStatus()
{
  viewMapControl->setChecked(mapControlDock->isVisible());
  viewMap->setChecked(mapViewDock->isVisible());
  viewData->setChecked(dataViewDock->isVisible());
  viewTaskAndWaypoint->setChecked(taskAndWaypointDock->isVisible());
}

void KFLogApp::slotToggleDataView()  { dataViewDock->changeHideShowState(); }

void KFLogApp::slotToggleMapControl() { mapControlDock->changeHideShowState(); }

void KFLogApp::slotToggleMap() { mapViewDock->changeHideShowState(); }

void KFLogApp::slotToggleTaskAndWaypointDock() { taskAndWaypointDock->changeHideShowState(); }

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
  extern MapContents _globalMapContents;
  EvaluationDialog* eval =
      new EvaluationDialog(_globalMapContents.getFlightList());

  connect(eval, SIGNAL(showCursor(QPoint, QPoint)), map,
      SLOT(slotDrawCursor(QPoint, QPoint)));

  eval->slotShowFlightData(0);  // We assume, that at least one flight
                                // is loaded ...
}

void KFLogApp::slotOptimizeFlight()
{
  extern MapContents _globalMapContents;
  if(_globalMapContents.getFlightList()->count() &&
      _globalMapContents.getFlightList()->current()->optimizeTask())
    {
      // Okay, update flightdata and redraw map
      dataView->setFlightData(_globalMapContents.getFlight());
      map->slotRedrawFlight();
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

void KFLogApp::slotStartComplete()  { if(showStartLogo)  delete startLogo; }

void KFLogApp::slotFlightViewIgc3D()
{
  extern MapContents _globalMapContents;

  if(_globalMapContents.getFlightList()->count()){
	  Igc3DDialog * igc3d = new Igc3DDialog(_globalMapContents.getFlightList());
    igc3d->show();
  }
}

bool KFLogApp::queryClose()
{
  return taskAndWaypoint->saveChanges();
}
