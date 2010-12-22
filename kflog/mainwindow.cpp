/***********************************************************************
**
**   MainWindow.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <dlfcn.h>
#include <pwd.h>

#include <QtGui>

#include "airport.h"
#include "centertodialog.h"
#include "flightdataprint.h"
#include "flightloader.h"
#include "igc3ddialog.h"
#include "igcpreview.h"
#include "kflogconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "mapmatrix.h"
#include "mapprint.h"
#include "recorderdialog.h"
#include "taskdataprint.h"
#include "translationlist.h"
#include "wgspoint.h"

#include "mainwindow.h"

TranslationList surfaceTypes;
TranslationList waypointTypes;
TranslationList taskTypes;

extern QSettings _settings;

/**
 * Contains all map elements and takes control over drawing or printing
 * the elements.
 */
MapContents *_globalMapContents = static_cast<MapContents *> (0);

/**
 * Used for transforming the map items.
 */
MapMatrix *_globalMapMatrix =  static_cast<MapMatrix *> (0);

/**
 * Contains all configuration-info for drawing and printing the elements.
 */
MapConfig *_globalMapConfig = static_cast<MapConfig *> (0);

/**
 * The map widget.
 */
Map *_globalMap = static_cast<Map *> (0);


MainWindow::MainWindow() : Q3MainWindow(0, "KFLog main window")
{
  qDebug() << "MainWindow()";

  _globalMapMatrix   = new MapMatrix(this);
  _globalMapConfig   = new MapConfig(this);
  _globalMapContents = new MapContents(this);

  BaseMapElement::initMapElement( _globalMapMatrix, _globalMapConfig );

  showStartLogo=false;

  if(_settings.readBoolEntry("/GeneralOptions/Logo", true))
  {
    showStartLogo = true;
    startLogo = new KFLogStartLogo;
    startLogo->setVisible( true );
    startLogo->raise();
  }

  connect( _globalMapConfig, SIGNAL(configChanged()),
           _globalMapMatrix, SLOT(slotInitMatrix()) );

  _globalMapConfig->slotReadConfig();

  initTaskTypes();
  initSurfaceTypes();
  initWaypointTypes();

  if(showStartLogo && startLogo != 0)
    {
      startLogo->raise();
    }

  initDockWindows();
  initMenuBar();
  initStatusBar();
  initToolBar();
  readOptions();

  connect(map, SIGNAL(showFlightPoint(const QPoint&, const flightPoint&)), this, SLOT(slotSetPointInfo(const QPoint&, const flightPoint&)));
  connect(map, SIGNAL(taskPlanningEnd()), helpWindow, SLOT(slotClearView()) );
  connect(map, SIGNAL(showTaskText(FlightTask*)), dataView, SLOT(slotShowTaskText(FlightTask*)));
  connect(map, SIGNAL(taskPlanningEnd()), dataView, SLOT(setFlightData()));
  connect(map, SIGNAL(showPoint(const QPoint&)), this, SLOT(slotSetPointInfo(const QPoint&)));
  connect(map, SIGNAL(showPoint(const QPoint&)), evaluationWindow, SLOT(slotRemoveFlightPoint()));
  connect(map, SIGNAL(showFlightPoint(const QPoint&, const flightPoint&)), evaluationWindow, SLOT(slotShowFlightPoint(const QPoint&, const flightPoint&)));
  connect(map, SIGNAL(changed(QSize)), mapControl, SLOT(slotShowMapData(QSize)));
  connect(map, SIGNAL(waypointSelected(Waypoint *)), waypoints, SLOT(slotAddWaypoint(Waypoint *)));
  connect(map, SIGNAL(waypointDeleted(Waypoint *)), waypoints, SLOT(slotDeleteWaypoint(Waypoint *)));
  connect(map, SIGNAL(waypointEdited(Waypoint *)), waypoints, SLOT(slotEditWaypoint(Waypoint *)));
  connect(map, SIGNAL(elevation(int)), legend, SLOT(highlightLevel(int)));
  connect(map, SIGNAL(regWaypointDialog(QWidget *)), this, SLOT(slotRegisterWaypointDialog(QWidget *)));
  connect(map, SIGNAL(openFile(const char *)), this, SLOT(slotOpenFile(const char *)));
  connect(map, SIGNAL(setStatusBarProgress(int)), this, SLOT(slotSetProgress(int)));
  connect(map, SIGNAL(setStatusBarMsg(const QString&)), this, SLOT(slotSetStatusMsg(const QString&)));

  connect(mapControl, SIGNAL(scaleChanged(double)), _globalMapMatrix, SLOT(slotSetScale(double)));

  connect(dataView, SIGNAL(wpSelected(const int)), map, SLOT(slotCenterToWaypoint(const int)));
  connect(dataView, SIGNAL(flightSelected(BaseFlightElement *)), _globalMapContents, SLOT(slotSetFlight(BaseFlightElement *)));
  connect(dataView, SIGNAL(editFlightGroup()), _globalMapContents, SLOT(slotEditFlightGroup()));

  connect(_globalMapContents, SIGNAL(activatePlanning()), map,SLOT(slotActivatePlanning()));
  connect(_globalMapContents, SIGNAL(closingFlight(BaseFlightElement*)), objectTree, SLOT(slotCloseFlight(BaseFlightElement*)));
  connect(_globalMapContents, SIGNAL(contentsChanged()),map, SLOT(slotRedrawMap()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), this, SLOT(slotModifyMenu()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), dataView, SLOT(setFlightData()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), evaluationWindow, SLOT(slotShowFlightData()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), map, SLOT(slotShowCurrentFlight()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), objectTree, SLOT(slotSelectedFlightChanged()));
  connect(_globalMapContents, SIGNAL(errorOnMapLoading()), this, SLOT(slotStartComplete()));
  connect(_globalMapContents, SIGNAL(newFlightAdded(Flight*)), objectTree, SLOT(slotNewFlightAdded(Flight*)));
  connect(_globalMapContents, SIGNAL(newFlightGroupAdded(FlightGroup*)), objectTree, SLOT(slotNewFlightGroupAdded(FlightGroup*)));
  connect(_globalMapContents, SIGNAL(newTaskAdded(FlightTask*)), objectTree, SLOT(slotNewTaskAdded(FlightTask*)));
  connect(_globalMapContents, SIGNAL(taskHelp(QString)), helpWindow, SLOT(slotShowHelpText(QString)) );
  connect(_globalMapMatrix, SIGNAL(displayMatrixValues(int, bool)), _globalMapConfig, SLOT(slotSetMatrixValues(int, bool)));
  connect(_globalMapMatrix, SIGNAL(matrixChanged()), map, SLOT(slotRedrawMap()));
  connect(_globalMapMatrix, SIGNAL(printMatrixValues(int)), _globalMapConfig, SLOT(slotSetPrintMatrixValues(int)));
  connect(_globalMapMatrix, SIGNAL(projectionChanged()), _globalMapContents, SLOT(slotReloadMapData()));

  connect(waypoints, SIGNAL(copyWaypoint2Task(Waypoint *)), map, SLOT(slotAppendWaypoint2Task(Waypoint *)));
  connect(waypoints, SIGNAL(waypointCatalogChanged( WaypointCatalog * )), map, SLOT(slotWaypointCatalogChanged( WaypointCatalog * )));
  connect(waypoints, SIGNAL(centerMap(int, int)), _globalMapMatrix, SLOT(slotCenterTo(int, int)));

  connect(objectTree, SIGNAL(selectedFlight(BaseFlightElement *)), _globalMapContents, SLOT(slotSetFlight(BaseFlightElement *)));
  connect(objectTree, SIGNAL(newTask()), _globalMapContents, SLOT(slotNewTask()));
  connect(objectTree, SIGNAL(openTask()), this, SLOT(slotOpenTask()));
  connect(objectTree, SIGNAL(closeTask()), _globalMapContents, SLOT(closeFlight()));
  connect(objectTree, SIGNAL(newFlightGroup()), _globalMapContents, SLOT(slotNewFlightGroup()));
  connect(objectTree, SIGNAL(editFlightGroup()), _globalMapContents, SLOT(slotEditFlightGroup()));
  connect(objectTree, SIGNAL(openFlight()), this, SLOT(slotOpenFile()));
  connect(objectTree, SIGNAL(openFile(const char*)), this, SLOT(slotOpenFile(const char*)));
  connect(objectTree, SIGNAL(optimizeFlight()), this, SLOT(slotOptimizeFlight()));
  connect(objectTree, SIGNAL(optimizeFlightOLC()), this, SLOT(slotOptimizeFlightOLC()));

  connect(evaluationWindow, SIGNAL(showCursor(const QPoint&, const QPoint&)), map, SLOT(slotDrawCursor(const QPoint&, const QPoint&)));

  connect(this, SIGNAL(flightDataTypeChanged(int)), _globalMapConfig, SLOT(slotSetFlightDataType(int)));

  slotCheckDockWidgetStatus();
}

MainWindow::~MainWindow()
{
#if 0
  delete dataViewDock;
  delete dataView;
  delete evaluationWindowDock;
  delete evaluationWindow;
  delete helpWindowDock;
  delete helpWindow;
  delete legendDock;
  delete legend;
  delete mapViewDock;
  delete mapControlDock;
  delete mapControl;
  delete objectTreeDock;
  delete objectTree;
  delete waypointsDock;
  delete waypoints;
  delete fileNewWaypoint;
  delete fileNewTask;
  delete fileNewFlightGroup;
  delete fileOpenFlight;
  delete fileOpenTask;
  delete fileClose;
  delete fileSavePixmap;
  delete filePrint;
  delete filePrintFlight;
  delete fileOpenRecorder;
  delete fileQuit;
  delete viewCenterTask;
  delete viewCenterFlight;
  delete viewCenterHomesite;
  delete viewCenterTo;
  delete viewZoomIn;
  delete viewZoomOut;
  delete viewZoom;
  delete viewRedraw;
  delete viewMoveNW;
  delete viewMoveN;
  delete viewMoveNE;
  delete viewMoveW;
  delete viewMoveE;
  delete viewMoveSW;
  delete viewMoveS;
  delete viewMoveSE;
  delete flightEvaluationWindow;
  delete flightOptimization;
  delete flightOptimizationOLC;
  delete flightIgc3D;
  delete flightIgcOpenGL;
  delete flightAnimateStart;
  delete flightAnimateStop;
  delete flightAnimateNext;
  delete flightAnimatePrev;
  delete flightAnimate10Next;
  delete flightAnimate10Prev;
  delete flightAnimateHome;
  delete flightAnimateEnd;
  delete settingsEvaluationWindow;
  delete settingsFlightData;
  delete settingsHelpWindow;
  delete settingsLegend;
//  delete settingsMap;
  delete settingsMapControl;
  delete settingsObjectTree;
  delete settingsStatusBar;
  delete settingsToolBar;
  delete settingsWaypoints;
  delete fileOpenRecent;
  delete flightDataType;
  delete windowMenu;
  delete settings;
  delete statusLabel;
  delete statusTimeL;
  delete statusAltitudeL;
  delete statusVarioL;
  delete statusSpeedL;
  delete statusLatL;
  delete statusLonL;
  delete statusProgress;
  delete toolBar;
#endif
}

QPixmap MainWindow::getPixmap( const QString& pixmapName )
{
  QString path = _settings.value( "/Path/InstallRoot", ".." ).toString();

  path += "/pics/" + pixmapName;

  QPixmap pm;

  if( ! QPixmapCache::find( path, pm ) )
    {
      if( ! pm.load( path ) )
        {
          qWarning( "Could not load Pixmap file '%s'. Maybe it was not installed?",
                    path.toLatin1().data() );
        }

      QPixmapCache::insert( path, pm );
    }

  return pm;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  saveOptions();
  waypoints->saveChanges();

  _globalMapMatrix->writeMatrixOptions();
  e->accept();
}

void MainWindow::initDockWindows()
{
  qDebug() << "MainWindow::initDockWindows()";

  dataViewDock = new Q3DockWindow(Q3DockWindow::InDock, this, "DataView QDockWindow");
  dataViewDock->setResizeEnabled(true);
  dataView = new DataView(dataViewDock);
  dataViewDock->setWidget(dataView);
  moveDockWindow(dataViewDock, Qt::DockRight);
  setAppropriate(dataViewDock, true);
  dataViewDock->hide();

  evaluationWindowDock = new Q3DockWindow(Q3DockWindow::InDock, this, "EvaluationDialog QDockWindow");
  evaluationWindow = new EvaluationDialog(evaluationWindowDock);
  evaluationWindowDock->setWidget(evaluationWindow);
  moveDockWindow(evaluationWindowDock, Qt::DockTornOff);
  evaluationWindowDock->hide();

  helpWindowDock = new Q3DockWindow(Q3DockWindow::InDock, this, "HelpWindow QDockWindow");
  helpWindow = new HelpWindow(helpWindowDock);
  helpWindowDock->setWidget(helpWindow);
  moveDockWindow(helpWindowDock, Qt::DockTornOff);
  helpWindowDock->hide();

  legendDock = new Q3DockWindow(Q3DockWindow::InDock, this, "TopoLegend QDockWindow");
  legend = new TopoLegend(legendDock);
  legendDock->setWidget(legend);
  moveDockWindow(legendDock, Qt::DockRight);
  legendDock->hide();

//  mapViewDock = new QDockWindow(QDockWindow::InDock, this, "Map QDockWindow");
  map = new Map(this);
  setCentralWidget(map);
  _globalMap = map;
//  mapViewDock->setWidget(map);
//  moveDockWindow(mapViewDock, Qt::DockUnmanaged);

  mapControlDock = new Q3DockWindow(Q3DockWindow::InDock, this, "MapControlView QDockWindow");
  mapControl = new MapControlView(mapControlDock);
  mapControlDock->setWidget(mapControl);
  moveDockWindow(mapControlDock, Qt::DockBottom);
  mapControlDock->hide();

  objectTreeDock = new Q3DockWindow(Q3DockWindow::InDock, this, "ObjectTree QDockWindow");
  objectTree = new ObjectTree(objectTreeDock);
  objectTreeDock->setWidget(objectTree);
  moveDockWindow(objectTreeDock, Qt::DockBottom);
  objectTreeDock->hide();

  waypointsDock = new Q3DockWindow(Q3DockWindow::InDock, this, "Waypoint QDockWindow");
  waypoints = new Waypoints(waypointsDock);
  waypointsDock->setWidget(waypoints);
  moveDockWindow(waypointsDock, Qt::DockBottom);
  waypointsDock->hide();

//  connect(dataViewDock, SIGNAL(iMBeingClosed()), SLOT(slotHideDataViewDock()));
  connect(dataViewDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(evaluationWindowDock, SIGNAL(iMBeingClosed()), SLOT(slotHideEvaluationWindowDock()));
  connect(evaluationWindowDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(helpWindowDock, SIGNAL(iMBeingClosed()), SLOT(slotHideHelpWindowDock()));
  connect(helpWindowDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(legendDock, SIGNAL(iMBeingClosed()), SLOT(slotHideLegendDock()));
  connect(legendDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(mapViewDock, SIGNAL(iMBeingClosed()), SLOT(slotHideMapViewDock()));
//  connect(mapViewDock, SIGNAL(placeChanged(QDockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(mapControlDock, SIGNAL(iMBeingClosed()), SLOT(slotHideMapControlDock()));
  connect(mapControlDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(objectTreeDock, SIGNAL(iMBeingClosed()), SLOT(slotHideObjectTreeDock()));
  connect(objectTreeDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));
//  connect(waypointsDock, SIGNAL(iMBeingClosed()), SLOT(slotHideWaypointsDock()));
  connect(waypointsDock, SIGNAL(placeChanged(Q3DockWindow::Place)), SLOT(slotCheckDockWidgetStatus()));

}

void MainWindow::initMenuBar()
{
  // File menu
  fileNewWaypoint = new QAction(getPixmap("waypoint_16.png"), tr("New &Waypoint"), 0, this, "file_new_waypoint");
  connect(fileNewWaypoint, SIGNAL(activated()), waypoints, SLOT(slotNewWaypoint()));
  fileNewTask = new QAction(getPixmap("task_16.png"), tr("New &Task"), Qt::CTRL+Qt::Key_N, this, "file_new_task");
  connect(fileNewTask, SIGNAL(activated()), _globalMapContents, SLOT(slotNewTask()));
  fileNewFlightGroup = new QAction(tr("New &Flight group"), 0, this, "file_new_flight_group");
  connect(fileNewFlightGroup, SIGNAL(activated()), _globalMapContents, SLOT(slotNewFlightGroup()));
  Q3PopupMenu * fileNew = new Q3PopupMenu( this );
  fileNewWaypoint->addTo( fileNew );
  fileNewTask->addTo( fileNew );
  fileNewFlightGroup->addTo( fileNew );

  fileOpenFlight = new QAction(getPixmap("kde_fileopen_16.png"), tr("&Open Flight"), Qt::CTRL+Qt::Key_O, this, "file_open_flight");
  connect(fileOpenFlight, SIGNAL(activated()), this, SLOT(slotOpenFile()));
  fileOpenTask = new QAction(getPixmap("kde_fileopen_16.png"), tr("Open &Task"), Qt::CTRL+Qt::Key_T, this, "file_open_task");
  connect(fileOpenTask, SIGNAL(activated()), this, SLOT(slotOpenTask()));

  fileOpenRecent = new Q3PopupMenu( this );
  QStringList datalist;
  datalist = _settings.readListEntry("/GeneralOptions/RecentFiles");
  int size = std::min((int)datalist.size(), 5);
  QAction *recentFileActs[size];
  for (int i = 0; i < size; ++i) {
    recentFileActs[i] = new QAction(this);
    recentFileActs[i]->setToolTip(datalist[i]); // FIXME: in Qt4 setData must be used
    recentFileActs[i]->setText(tr("%1 %2").arg(i + 1).arg(QFileInfo(datalist[i]).fileName()));
    connect(recentFileActs[i], SIGNAL(activated()), SLOT(slotOpenRecentFile()));
    recentFileActs[i]->addTo(fileOpenRecent);
  }

  fileClose = new QAction(getPixmap("kde_cancel_16.png"), tr("&Close Flight/Task"), Qt::CTRL+Qt::Key_W, this, "file_close");
  connect(fileClose, SIGNAL(activated()), _globalMapContents, SLOT(closeFlight()));

  fileSavePixmap = new QAction(getPixmap("kde_image_16.png"), tr("Export to PNG..."), 0, this, "file_export_pixmap");
  connect(fileSavePixmap, SIGNAL(activated()), map, SLOT(slotSavePixmap()));
  filePrint = new QAction(getPixmap("kde_fileprint_16.png"), tr("Print..."), Qt::CTRL+Qt::Key_P, this, "file_print");
  connect(filePrint, SIGNAL(activated()), this, SLOT(slotFilePrint()));
  filePrintFlight = new QAction(getPixmap("kde_fileprint_16.png"), tr("Print Flightdata"), 0, this, "file_print_flightdata");
  connect(filePrintFlight, SIGNAL(activated()), this, SLOT(slotFlightPrint()));

  fileOpenRecorder = new QAction(getPixmap("kde_connect_no_16.png"), tr("Open Recorder"), 0, this, "file_open_recorder");
  connect(fileOpenRecorder, SIGNAL(activated()), this, SLOT(slotOpenRecorderDialog()));

  fileQuit = new QAction(getPixmap("kde_exit_16.png"), "&Quit", Qt::CTRL+Qt::Key_Q, this, "file_quit");
  connect(fileQuit, SIGNAL(activated()), qApp, SLOT(closeAllWindows()));
  connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );

  Q3PopupMenu * file = new Q3PopupMenu( this );
  menuBar()->insertItem( "&File", file );
  file->insertItem(getPixmap("kde_filenew_16.png"), "&New", fileNew);
  fileOpenFlight->addTo( file );
  fileOpenTask->addTo( file );
  file->insertItem(getPixmap("kde_fileopen_16.png"), "&Open recent files", fileOpenRecent);
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

  viewCenterTask = new QAction(getPixmap("centertask_16.png"), tr("Center to &Task"), Qt::Key_F6, this, "view_center_task");
  connect(viewCenterTask, SIGNAL(activated()), map, SLOT(slotCenterToTask()));
  viewCenterFlight = new QAction(getPixmap("centerflight_16.png"), tr("Center to &Flight"), Qt::Key_F7, this, "view_center_flight");
  connect(viewCenterFlight, SIGNAL(activated()), map, SLOT(slotCenterToFlight()));
  viewCenterHomesite = new QAction(getPixmap("kde_gohome_16.png"), tr("Center to &Homesite"), Qt::CTRL+Qt::Key_Home, this, "view_center_homeside");
  connect(viewCenterHomesite, SIGNAL(activated()), _globalMapMatrix, SLOT(slotCenterToHome()));
  viewCenterTo = new QAction(getPixmap("centerto_16.png"), tr("&Center to..."), Qt::Key_F8, this, "view_center_to");
  connect(viewCenterTo, SIGNAL(activated()), this, SLOT(slotCenterTo()));

  viewZoomIn = new QAction(getPixmap("kde_viewmag+_16.png"), tr("Zoom &In"), Qt::CTRL+Qt::Key_Plus, this, "view_zoom_in");
  connect(viewZoomIn, SIGNAL(activated()), _globalMapMatrix, SLOT(slotZoomIn()));
  viewZoomOut = new QAction(getPixmap("kde_viewmag-_16.png"), tr("Zoom &Out"), Qt::CTRL+Qt::Key_Minus, this, "view_zoom_out");
  connect(viewZoomOut, SIGNAL(activated()), _globalMapMatrix, SLOT(slotZoomOut()));
  viewZoom = new QAction(getPixmap("kde_viewmagfit_16.png"), tr("&Zoom..."), 0, this, "view_zoom");
  connect(viewZoom, SIGNAL(activated()), map, SLOT(slotZoomRect()));
  viewRedraw = new QAction(getPixmap("kde_reload_16.png"), tr("&Redraw"), Qt::Key_F5, this, "view_redraw");
  connect(viewRedraw, SIGNAL(activated()), map, SLOT(slotRedrawMap()));

  viewMoveNW = new QAction(getPixmap("movemap_nw_22.png"), tr("move map north-west"), Qt::Key_7, this, "view_move_nw");
  connect(viewMoveNW, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapNW()));
  viewMoveN = new QAction(getPixmap("movemap_n_22.png"), tr("move map north"), Qt::Key_8, this, "view_move_n");
  connect(viewMoveN, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapN()));
  viewMoveNE = new QAction(getPixmap("movemap_ne_22.png"), tr("move map north-east"), Qt::Key_9, this, "view_move_ne");
  connect(viewMoveNE, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapNE()));
  viewMoveW = new QAction(getPixmap("movemap_w_22.png"), tr("move map west"), Qt::Key_4, this, "view_move_w");
  connect(viewMoveW, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapW()));
  viewMoveE = new QAction(getPixmap("movemap_e_22.png"), tr("move map east"), Qt::Key_6, this, "view_move_e");
  connect(viewMoveE, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapE()));
  viewMoveSW = new QAction(getPixmap("movemap_sw_22.png"), tr("move map south-west"), Qt::Key_1, this, "view_move_sw");
  connect(viewMoveSW, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapSW()));
  viewMoveS = new QAction(getPixmap("movemap_s_22.png"), tr("move map south"), Qt::Key_2, this, "view_move_s");
  connect(viewMoveS, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapS()));
  viewMoveSE = new QAction(getPixmap("movemap_se_22.png"), tr("move map south-east"), Qt::Key_3, this, "view_move_se");
  connect(viewMoveSE, SIGNAL(activated()), _globalMapMatrix, SLOT(slotMoveMapSE()));

  Q3PopupMenu * viewMove = new Q3PopupMenu( this );
  viewMoveNW->addTo( viewMove );
  viewMoveN->addTo( viewMove );
  viewMoveNE->addTo( viewMove );
  viewMoveW->addTo( viewMove );
  viewMoveE->addTo( viewMove );
  viewMoveSW->addTo( viewMove );
  viewMoveS->addTo( viewMove );
  viewMoveSE->addTo( viewMove );

  Q3PopupMenu * view = new Q3PopupMenu( this );
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
  view->insertItem(getPixmap("kde_move_16.png"), "Move map", viewMove);

  // flight menu

  flightEvaluationWindow = new QAction(getPixmap("kde_history_16.png"), tr("Show &EvaluationWindow"), Qt::CTRL+Qt::Key_E, this, "toggle_evaluation_window");
  flightEvaluationWindow->setToggleAction(true);
  connect(flightEvaluationWindow, SIGNAL(activated()), this, SLOT(slotToggleEvaluationWindow()));
  flightOptimization = new QAction(getPixmap("kde_wizard_16.png"), tr("Optimize"), 0, this, "optimize_flight");
  connect(flightOptimization, SIGNAL(activated()), this, SLOT(slotOptimizeFlight()));
  flightOptimizationOLC = new QAction(getPixmap("kde_wizard_16.png"), tr("Optimize (OLC)"), 0, this, "optimize_flight_olc");
  connect(flightOptimizationOLC, SIGNAL(activated()), this, SLOT(slotOptimizeFlightOLC()));

  flightDataType = new Q3PopupMenu( this );
  QStringList dataList;
  dataList.append(tr("Altitude"));
  dataList.append(tr("Cycling"));
  dataList.append(tr("Speed"));
  dataList.append(tr("Vario"));
  dataList.append(tr("Solid"));
  for (int i = 0; i < 5; ++i)
    flightDataType->insertItem(dataList[i], i);
  connect(flightDataType, SIGNAL(activated(int)), this, SLOT(slotSelectFlightData(int)));
  flightDataType->setItemChecked(_settings.readNumEntry("/Flight/DrawType", MapConfig::Altitude), true);

  flightIgc3D = new QAction(getPixmap("kde_vectorgfx_16.png"), tr("View flight in 3D"), Qt::CTRL+Qt::Key_R, this, "view_flight_3D");
  connect(flightIgc3D, SIGNAL(activated()), this, SLOT(slotFlightViewIgc3D()));
  flightIgcOpenGL = new QAction(/*SmallIcon("openglgfx"),*/ tr("View flight in 3D (OpenGL)"), 0, this, "view_flight_opengl");
  connect(flightIgcOpenGL, SIGNAL(activated()), this, SLOT(slotFlightViewIgcOpenGL()));

  flightAnimateStart = new QAction(getPixmap("kde_1rightarrow_16.png"), tr("&Start Flight Animation"), Qt::Key_F12, this, "start_animate");
  connect(flightAnimateStart, SIGNAL(activated()), map, SLOT(slotAnimateFlightStart()));
  flightAnimateStop = new QAction(getPixmap("kde_player_stop_16.png"), tr("Stop Flight &Animation"), Qt::Key_F11, this, "stop_animate");
  connect(flightAnimateStop, SIGNAL(activated()), map, SLOT(slotAnimateFlightStop()));
  flightAnimateNext = new QAction(getPixmap("kde_forward_16.png"), tr("Next Flight Point"), Qt::CTRL+Qt::Key_Up, this, "next_flight_point");
  connect(flightAnimateNext, SIGNAL(activated()), map, SLOT(slotFlightNext()));
  flightAnimatePrev = new QAction(getPixmap("kde_back_16.png"), tr("Prev Flight Point"), Qt::CTRL+Qt::Key_Down, this, "prev_flight_point");
  connect(flightAnimatePrev, SIGNAL(activated()), map, SLOT(slotFlightPrev()));
  flightAnimate10Next = new QAction(getPixmap("kde_2rightarrow_16.png"), tr("Step +10 Flight Points"), Qt::Key_PageUp, this, "next_step_flight_point");
  connect(flightAnimate10Next, SIGNAL(activated()), map, SLOT(slotFlightStepNext()));
  flightAnimate10Prev = new QAction(getPixmap("kde_2leftarrow_16.png"),  tr("Step -10 Flight Points"), Qt::Key_PageDown, this, "prev_step_flight_point");
  connect(flightAnimate10Prev, SIGNAL(activated()), map, SLOT(slotFlightStepPrev()));
  flightAnimateHome = new QAction(getPixmap("kde_start_16.png"), tr("First Flight Point"), Qt::Key_Home, this, "first_flight_point");
  connect(flightAnimateHome, SIGNAL(activated()), map, SLOT(slotFlightHome()));
  flightAnimateEnd = new QAction(getPixmap("kde_finish_16.png"), tr("Last Flight Point"), Qt::Key_End, this, "last_flight_point");
  connect(flightAnimateEnd, SIGNAL(activated()), map, SLOT(slotFlightEnd()));

  Q3PopupMenu * flight = new Q3PopupMenu( this );
  menuBar()->insertItem( "F&light", flight );
  flightEvaluationWindow->addTo(flight);
  flightOptimization->addTo(flight);
  flightOptimizationOLC->addTo(flight);
  flight->insertItem(getPixmap("kde_idea_16.png"), tr("Show Flightdata"), flightDataType);
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

  windowMenu = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Window", windowMenu );
  windowMenu->setCheckable(true);
  connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(slotWindowsMenuAboutToShow()));

  // settings menu

  settingsEvaluationWindow = new QAction(getPixmap("kde_history_16.png"), tr("Show &EvaluationWindow"), Qt::CTRL+Qt::Key_E, this, "toggle_evaluation_window");
  settingsEvaluationWindow->setToggleAction(true);
  connect(settingsEvaluationWindow, SIGNAL(activated()), this, SLOT(slotToggleEvaluationWindow()));
  settingsFlightData = new QAction(getPixmap("kde_view_detailed_16.png"), tr("Show Flight&data"), Qt::CTRL+Qt::Key_E, this, "toggle_data_view");
  settingsFlightData->setToggleAction(true);
  connect(settingsFlightData, SIGNAL(activated()), this, SLOT(slotToggleDataView()));
  settingsHelpWindow = new QAction(getPixmap("kde_info_16.png"), tr("Show HelpWindow"), Qt::CTRL+Qt::Key_H, this, "toggle_help_window");
  settingsHelpWindow->setToggleAction(true);
  connect(settingsHelpWindow, SIGNAL(activated()), this, SLOT(slotToggleHelpWindow()));
  settingsObjectTree = new QAction(getPixmap("kde_view_tree_16.png"), tr("Show KFLog&Browser"), Qt::CTRL+Qt::Key_B, this, "view_tree");
  settingsObjectTree->setToggleAction(true);
  connect(settingsObjectTree, SIGNAL(activated()), this, SLOT(slotToggleObjectTreeDock()));
  settingsLegend = new QAction(getPixmap("kde_blend_16.png"), tr("Show Legend"), Qt::CTRL+Qt::Key_L, this, "toggle_legend");
  settingsLegend->setToggleAction(true);
  connect(settingsLegend, SIGNAL(activated()), this, SLOT(slotToggleLegendDock()));
//  settingsMap = new QAction(tr("Show &Map"), 0, this, "toggle_map");
//  settingsMap->setToggleAction(true);
//  connect(settingsMap, SIGNAL(activated()), this, SLOT(slotToggleMap()));
  settingsMapControl = new QAction(tr("Show Map&control"), 0, this, "toggle_map_control");
  settingsMapControl->setToggleAction(true);
  connect(settingsMapControl, SIGNAL(activated()), this, SLOT(slotToggleMapControl()));
  settingsToolBar = new QAction(tr("Show Toolbar"), 0, this, "toggle_toolbar");
  settingsToolBar->setToggleAction(true);
  connect(settingsToolBar, SIGNAL(activated()), this, SLOT(slotToggleToolBar()));
  settingsStatusBar = new QAction(tr("Show Statusbar"), 0, this, "toggle_statusbar");
  settingsStatusBar->setToggleAction(true);
  connect(settingsStatusBar, SIGNAL(activated()), this, SLOT(slotToggleStatusBar()));
  settingsWaypoints = new QAction(getPixmap("waypoint_16.png"), tr("Show &Waypoints"), Qt::CTRL+Qt::Key_R, this, "toggle_waypoints");
  settingsWaypoints->setToggleAction(true);
  connect(settingsWaypoints, SIGNAL(activated()), this, SLOT(slotToggleWaypointsDock()));

  settings = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Settings", settings );
  settingsEvaluationWindow->addTo(settings);
  settingsFlightData->addTo(settings);
  settingsHelpWindow->addTo(settings);
  settingsObjectTree->addTo(settings);
  settingsLegend->addTo(settings);
//  settingsMap->addTo(settings);
  settingsMapControl->addTo(settings);
  settingsToolBar->addTo(settings);
  settingsStatusBar->addTo(settings);
  // We can't use CTRL-W, because this shortcut is reserved for closing a file ...
  settingsWaypoints->addTo(settings);
  settings->insertSeparator();
//  settings->insertItem(getPixmap("kde_configure_shortcuts_16.png"), tr("Configure Shortcuts...") );//, this, SLOT(slotConfigureKeyBindings()));
//  settings->insertItem(getPixmap("kde_configure_toolbars_16.png"), tr("Configure Toolbars...") );//, this, SLOT(slotConfigureToolbars()));
  settings->insertItem(getPixmap("kde_configure_16.png"), tr("KFLog &Setup..."), this, SLOT(slotConfigureKFLog()));

  // help menu

  Q3PopupMenu * help = new Q3PopupMenu( this );
  menuBar()->insertItem( "&Help", help );
  //FIXME: link to manual must be added
  help->insertItem(getPixmap("kde_contexthelp_16.png"), tr("What's This?"), this, SLOT(slotWhatsThis()), Qt::CTRL+Qt::Key_F1);
  //FIXME: dialog to swith application language must be added
//  help->insertItem(getPixmap("kde_idea_16.png"), tr("Tip of the day") );//, this, SLOT(slotTipOfDay()));
//  help->insertItem(getPixmap("kflog_16.png"), tr("About KFLog") );//, this, SLOT(slotShowAbout()));

  menuBar()->insertItem("Dock Windows", createDockWindowMenu());
}

void MainWindow::initStatusBar()
{
  /* Alternativ könnte der Balken auch nur während des Zeichnens erscheinen */
  statusProgress = new QProgressBar( statusBar() );
  statusProgress->setFixedWidth(120);
  statusProgress->setFixedHeight( statusProgress->sizeHint().height() - 4 );
  statusProgress->setBackgroundMode( Qt::PaletteBackground );

  statusLabel = new QLabel( statusBar() );
  statusLabel->setFixedHeight( statusLabel->sizeHint().height() );
  statusLabel->setFrameStyle( QFrame::NoFrame |QFrame::Plain );
  statusLabel->setMargin(0);
  statusLabel->setLineWidth(0);

  statusTimeL = new QLabel(statusBar(), "time_label_status_bar");
  statusTimeL->setFixedWidth( 80 );
  statusTimeL->setFixedHeight( statusLabel->sizeHint().height() );
  statusTimeL->setFrameStyle(QFrame::NoFrame |QFrame::Plain );
  statusTimeL->setMargin(0);
  statusTimeL->setLineWidth(0);
  statusTimeL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusAltitudeL = new QLabel(statusBar(), "altitude_label_status_bar");
  statusAltitudeL->setFixedWidth( 80 );
  statusAltitudeL->setFixedHeight( statusLabel->sizeHint().height() );
  statusAltitudeL->setFrameStyle(QFrame::NoFrame |QFrame::Plain );
  statusAltitudeL->setMargin(0);
  statusAltitudeL->setLineWidth(0);
  statusAltitudeL->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  statusVarioL = new QLabel(statusBar(), "vario_label_status_bar");
  statusVarioL->setFixedWidth( 80 );
  statusVarioL->setFixedHeight( statusLabel->sizeHint().height() );
  statusVarioL->setFrameStyle(QFrame::NoFrame |QFrame::Plain );
  statusVarioL->setMargin(0);
  statusVarioL->setLineWidth(0);
  statusVarioL->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  statusSpeedL = new QLabel(statusBar(), "speed_label_status_bar");
  statusSpeedL->setFixedWidth( 100 );
  statusSpeedL->setFixedHeight( statusLabel->sizeHint().height() );
  statusSpeedL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusSpeedL->setMargin(0);
  statusSpeedL->setLineWidth(0);
  statusSpeedL->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  statusLatL = new QLabel(statusBar(), "lat_label_status_bar");
  statusLatL->setFixedWidth( 110 );
  statusLatL->setFixedHeight( statusLabel->sizeHint().height() );
  statusLatL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusLatL->setMargin(0);
  statusLatL->setLineWidth(0);
  statusLatL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusLonL = new QLabel(statusBar(), "lon_label_status_bar");
  statusLonL->setFixedWidth( 110 );
  statusLonL->setFixedHeight( statusLabel->sizeHint().height() );
  statusLonL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusLonL->setMargin(0);
  statusLonL->setLineWidth(0);
  statusLonL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusBar()->addWidget( statusLabel, 1, false );
  statusBar()->addWidget( statusTimeL, 0, false );
  statusBar()->addWidget( statusAltitudeL, 0, false );
  statusBar()->addWidget( statusSpeedL, 0, false );
  statusBar()->addWidget( statusVarioL, 0, false );
  statusBar()->addWidget( statusProgress, 0,  false );
  statusBar()->addWidget( statusLatL, 0, false );
  statusBar()->addWidget( statusLonL, 0, false );
}

void MainWindow::initSurfaceTypes()
{
  surfaceTypes.setAutoDelete(true);

  surfaceTypes.append(new TranslationElement(Airport::Unknown, tr("Unknown")));
  surfaceTypes.append(new TranslationElement(Airport::Grass, tr("Grass")));
  surfaceTypes.append(new TranslationElement(Airport::Asphalt, tr("Asphalt")));
  surfaceTypes.append(new TranslationElement(Airport::Concrete, tr("Concrete")));

  surfaceTypes.sort();
}

void MainWindow::initTaskTypes()
{
  taskTypes.setAutoDelete(true);

  taskTypes.append(new TranslationElement(FlightTask::Route, tr("Traditional Route")));
  taskTypes.append(new TranslationElement(FlightTask::FAIArea, tr("FAI Area")));
  //taskTypes.append(new TranslationElement(FlightTask::AAT, tr("Area Assigned")));

  taskTypes.sort();
}

void MainWindow::initToolBar()
{
  toolBar = new Q3ToolBar(this, "main toolbar");
  fileOpenFlight->addTo(toolBar);
  toolBar->addSeparator();
  viewZoom->addTo(toolBar);
  viewZoomIn->addTo(toolBar);
  viewZoomOut->addTo(toolBar);
  viewRedraw->addTo(toolBar);
  toolBar->addSeparator();
  viewCenterTask->addTo(toolBar);
  viewCenterFlight->addTo(toolBar);
  viewCenterHomesite->addTo(toolBar);
  toolBar->addSeparator();
  flightEvaluationWindow->addTo(toolBar);
}

void MainWindow::initWaypointTypes()
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

void MainWindow::readOptions()
{
  bool bViewToolbar = _settings.readBoolEntry("/GeneralOptions/ShowToolbar", true);
  if(toolBar->isShown()!=bViewToolbar)
    slotToggleToolBar();

  bool bViewStatusbar = _settings.readBoolEntry("/GeneralOptions/ShowStatusbar", true);
  if(statusBar()->isShown()!=bViewStatusbar)
    slotToggleStatusBar();

  // bar position settings
  QSize size(_settings.readNumEntry("/GeneralOptions/GeometryWidth", 950),
             _settings.readNumEntry("/GeneralOptions/GeometryHeight", 700));

  if(!size.isEmpty())
    resize(size);

  // initialize the recent file list
  flightDir = _settings.readEntry("/Path/DefaultFlightDirectory", getpwuid(getuid())->pw_dir);
  taskDir = _settings.readEntry("/Path/DefaultWaypointDirectory", getpwuid(getuid())->pw_dir);

  mapControl->slotSetMinMaxValue(_settings.readNumEntry("/Scale/Lower Limit", 10),
                                 _settings.readNumEntry("/Scale/Upper Limit", 1500));

// FIXME: use QMainWindow::restoreState in Qt4
// bar position settings
//  KToolBar::BarPosition toolBarPos;
//  toolBarPos=(KToolBar::BarPosition) _settings.readNumEntry("/GeneralOptions/ToolBarPos", KToolBar::Top);
//  toolBar("mainToolBar")->setBarPos(toolBarPos);
//  readDockConfig(config, "Window Layout");
}

void MainWindow::saveOptions()
{
  _settings.setValue("/GeneralOptions/GeometryWidth", size().width());
  _settings.setValue("/GeneralOptions/GeometryHeight", size().height());
  _settings.setValue("/GeneralOptions/ShowToolbar", toolBar->isShown());
  _settings.setValue("/GeneralOptions/ShowStatusbar", statusBar()->isShown());

  qDebug("saving options...");
  if (_settings.readNumEntry("/Waypoints/DefaultWaypointCatalog", KFLogConfig::LastUsed) ==
      KFLogConfig::LastUsed && waypoints->getCurrentCatalog() != NULL)
    {
      // Only write the path, if a waypoint-catalog is opened.
      // Otherwise KFLog crashes on a clean installation.
      //qDebug("saving catalog name");
      _settings.setValue("/Waypoints/DefaultCatalogName", waypoints->getCurrentCatalog()->path);
    }

// FIXME: use QMainWindow::saveState in Qt4
//  _settings.setValue("/GeneralOptions/ToolBarPos", (int) toolBar("mainToolBar")->barPos());
//  writeDockConfig(config, "Window Layout");
}

void MainWindow::slotCenterTo()
{
  CenterToDialog* center = new CenterToDialog(this, tr("center-to-dialog"));

  connect(center, SIGNAL(centerTo(int,int)), _globalMapMatrix, SLOT(slotCenterTo(int, int)));

  center->show();
}

void MainWindow::slotFlightViewIgc3D()
{
  Igc3DDialog * igc3d = new Igc3DDialog(this);

  connect(_globalMapContents, SIGNAL(currentFlightChanged()), igc3d, SLOT(slotShowFlightData()));
}

void MainWindow::slotFlightViewIgcOpenGL()
{
  #define CHECK_ERROR_EXIT  error = (char *)dlerror(); \
    if(error != NULL) \
      { \
        qWarning("%s", error); \
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

  (void)(*addFlight)((Flight*)_globalMapContents->getFlight());
}

/** set menu items enabled/disabled */
void MainWindow::slotModifyMenu()
{
  if (_globalMapContents->getFlightList()->count() > 0)
  {
      switch(_globalMapContents->getFlight()->getObjectType())
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

          default:
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

void MainWindow::slotOpenFile()
{
  Q3FileDialog* fd = new Q3FileDialog(this);
  fd->setCaption(tr("Open flight"));
  fd->setDir(flightDir);

  QString filter;
  filter.append(tr("All flight type files")+" (*.igc *.flightgear *.trk *.gdn);;");
  filter.append(tr("IGC")+" (*.igc);;");
  filter.append(tr("Garmin")+" (*.trk *.gdn)");
  fd->setFilters(filter);

  IGCPreview* preview = new IGCPreview(fd);
  fd->setContentsPreviewEnabled( TRUE );
  fd->setContentsPreview(preview, preview);
  fd->setPreviewMode( Q3FileDialog::Contents );

  if(fd->exec()==QDialog::Accepted)
  {
    QString fName = fd->selectedFile();
    Q3Url fUrl = Q3Url(fName);
    flightDir = fd->dirPath();

    if(!fUrl.isValid())
      return;

    if(fUrl.protocol()!="file")
      return;

    FlightLoader flightLoader;
    QFile file (fName);
    if(flightLoader.openFlight(file))
      slotSetCurrentFile(fName);
  }

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotOpenFile(const char* surl)
{
  slotSetStatusMsg(tr("Opening file..."));

  Q3Url url = Q3Url(surl);

  if(url.isLocalFile())
  {
    QFile file (url.path());
    if (url.fileName().right(9).lower()==".kflogtsk")
    {
      //this is probably a taskfile. Try to open it as a task
      if (_globalMapContents->loadTask(file))
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

  slotSetStatusMsg(tr("Ready."));
}

/**
 * Opens a task-file-open-dialog.
 */
void MainWindow::slotOpenTask()
{
  slotSetStatusMsg(tr("Opening file..."));

  Q3FileDialog* fd = new Q3FileDialog(this);
  fd->setCaption(tr("Open task"));
  fd->setDir(taskDir);

  QString filter;
  filter.append(tr("KFLog tasks")+"(*.kflogtsk *.KFLOGTSK)");
  fd->setFilters(filter);

  if(fd->exec()==QDialog::Accepted)
  {
    QString fName = fd->selectedFile();
    Q3Url fUrl = Q3Url(fName);
    taskDir = fd->dirPath();

    if(!fUrl.isValid())
      return;

    if(!fUrl.isLocalFile())
      return;

    QFile file(fName);

    if (_globalMapContents->loadTask(file))
        slotSetCurrentFile(fName);
  }

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotOpenRecentFile()
{
  QString fileName;
  QAction *action = (QAction*)(sender());
  if (action)
    fileName = action->toolTip();
  else
    return;

  slotSetStatusMsg(tr("Opening file..."));

  FlightLoader flightLoader;
  Q3Url url (fileName);
  if(url.isLocalFile())
  {
    QFile file (url.path());
    if (url.fileName().right(9).lower()==".kflogtsk")
    {
      //this is probably a taskfile. Try to open it as a task
      if (_globalMapContents->loadTask(file))
        slotSetCurrentFile(url.path());
    }
    else
    {
      //try to open as flight
      if(flightLoader.openFlight(file))
        slotSetCurrentFile(url.path());
    } // .kflogtsk
  } //isLocalFile

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotOpenRecorderDialog()
{
  RecorderDialog* dlg = new RecorderDialog(this, "recorderDialog");
  connect(dlg, SIGNAL(addCatalog(WaypointCatalog *)), waypoints, SLOT(slotAddCatalog(WaypointCatalog *)));
  connect(dlg, SIGNAL(addTask(FlightTask *)), _globalMapContents, SLOT(slotAppendTask(FlightTask *)));
  dlg->exec();
  delete dlg;
}

void MainWindow::slotOptimizeFlight()
{
  Flight *f = (Flight *)_globalMapContents->getFlight();

  if(f && f->getObjectType() == BaseMapElement::Flight)
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

void MainWindow::slotOptimizeFlightOLC()
{
  Flight *f = (Flight *)_globalMapContents->getFlight();

  if(f && f->getObjectType() == BaseMapElement::Flight){
      if(f->optimizeTaskOLC(map))
        {
          // Okay, update flightdata and redraw map
          dataView->setFlightData();
          map->slotRedrawFlight();
          objectTree->slotFlightChanged();
        }
  }
}

/** Connects the dialogs addWaypoint signal to the waypoint object. */
void MainWindow::slotRegisterWaypointDialog(QWidget * dialog){
  connect(dialog, SIGNAL(addWaypoint(Waypoint *)), waypoints, SLOT(slotAddWaypoint(Waypoint *)));
}

void MainWindow::slotSavePixmap(QUrl url, int width, int height){
  map->slotSavePixmap(url,width,height);
}

void MainWindow::slotSelectFlightData(int id)
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

void MainWindow::slotSetCurrentFile(const QString &fileName)
{
  QStringList files = _settings.readListEntry("/GeneralOptions/RecentFiles");
  QStringList newFiles;
  int recentFilesMax = _settings.readNumEntry("/GeneralOptions/RecentFilesMax", 5);
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

  _settings.setValue("/GeneralOptions/RecentFiles", newFiles);

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

void MainWindow::slotSetPointInfo(const QPoint& pos, const flightPoint& point)
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

  statusLatL->setText(WGSPoint::printPos(pos.y()));
  statusLonL->setText(WGSPoint::printPos(pos.x(), false));
}

void MainWindow::slotSetPointInfo(const QPoint& pos)
{
  statusBar()->clear();
  statusTimeL->setText("");
  statusAltitudeL->setText("");
  statusSpeedL->setText("");
  statusVarioL->setText("");

  statusLatL->setText(WGSPoint::printPos(pos.y()));
  statusLonL->setText(WGSPoint::printPos(pos.x(), false));
}

void MainWindow::slotSetProgress(int value)
{
  statusProgress->setValue(value);
}

void MainWindow::slotSetStatusMsg(const QString &text)
{
  statusBar()->clear();
//  statusBar()->message(text);
  statusLabel->setText(text);
}

/* Slot to set filename for WaypointCatalog */
void MainWindow::slotSetWaypointCatalog(QString catalog)
{
  waypoints->slotSetWaypointCatalogName(catalog);
}

void MainWindow::slotCheckDockWidgetStatus()
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
//  settingsMap->setOn(mapViewDock->isShown());
  settingsMapControl->setOn(mapControlDock->isShown());
  settingsStatusBar->setOn(statusBar()->isShown());
  settingsToolBar->setOn(toolBar->isShown());
  settingsWaypoints->setOn(waypointsDock->isShown());
}

void MainWindow::slotConfigureKFLog()
{
  KFLogConfig* confDlg = new KFLogConfig(this);

  connect(confDlg, SIGNAL(scaleChanged(int, int)), mapControl, SLOT(slotSetMinMaxValue(int, int)));

  connect(confDlg, SIGNAL(configOk()), _globalMapConfig, SLOT(slotReadConfig()));

  connect(confDlg, SIGNAL(configOk()), _globalMapContents, SLOT(reProject()));

  connect(confDlg, SIGNAL(configOk()), map, SLOT(slotRedrawMap()));

  connect(confDlg, SIGNAL(newDrawType(int)), this, SLOT(slotSelectFlightData(int)));

  confDlg->exec();

  delete confDlg;
}

void MainWindow::slotFilePrint()
{
  slotSetStatusMsg(tr("Printing..."));

  // viewCenterFlight is enabled, when a flight is loaded ...
  MapPrint::MapPrint(viewCenterFlight->isEnabled());

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotFlightPrint()
{
  slotSetStatusMsg(tr("Printing..."));

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      switch (f->getObjectType())
        {
          case BaseMapElement::Flight:
            FlightDataPrint::FlightDataPrint((Flight *)f);
            break;
          case BaseMapElement::Task:
            TaskDataPrint::TaskDataPrint((FlightTask*)f);
            break;
          default:
            QString tmp;
            tmp.sprintf(tr("Not yet available for type: %d"), f->getObjectType());
            QMessageBox::warning(0, tr("Type not available"), tmp, QMessageBox::Ok);
        }
    }
  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotStartComplete()
{
  if( showStartLogo && startLogo != static_cast<KFLogStartLogo *> (0) )
    {
      startLogo->close();
      startLogo = static_cast<KFLogStartLogo *> (0);
      showStartLogo = false;
    }
}

void MainWindow::slotToggleDataView()
{
  if(dataViewDock->isVisible())
    dataViewDock->hide();
  else
    dataViewDock->show();
  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleEvaluationWindow()
{
  if(evaluationWindowDock->isVisible())
    evaluationWindowDock->hide();
  else
    evaluationWindowDock->show();
  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleHelpWindow()
{
  if(helpWindowDock->isVisible())
    helpWindowDock->hide();
  else
    helpWindowDock->show();
  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleLegendDock()
{
  if(legendDock->isVisible())
    legendDock->hide();
  else
    legendDock->show();
  slotCheckDockWidgetStatus();
}

// Map is now a background widget instead of a dock
//void KFLog::slotToggleMap()
//{
//  if(mapViewDock->isVisible())
//    mapViewDock->hide();
//  else
//    mapViewDock->show();
////  slotCheckDockWidgetStatus();
//}

void MainWindow::slotToggleMapControl()
{
  if(mapControlDock->isVisible())
    mapControlDock->hide();
  else
    mapControlDock->show();
  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleObjectTreeDock()
{
  if(objectTreeDock->isVisible())
    objectTreeDock->hide();
  else
    objectTreeDock->show();
  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleStatusBar()
{
  slotSetStatusMsg(tr("Toggle the statusbar..."));

  if(statusBar()->isShown())
    statusBar()->hide();
  else
    statusBar()->show();

  slotCheckDockWidgetStatus();
  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotToggleToolBar()
{
  slotSetStatusMsg(tr("Toggling toolbar..."));

  if(toolBar->isShown())
      toolBar->hide();
  else
      toolBar->show();

  slotCheckDockWidgetStatus();
  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotToggleWaypointsDock()
{
  if(waypointsDock->isVisible())
    waypointsDock->hide();
  else
    waypointsDock->show();
  slotCheckDockWidgetStatus();
}

/** Called to the What's This? mode. */
void MainWindow::slotWhatsThis()
{
  QWhatsThis::enterWhatsThisMode();
}

/** insert available flights into menu */
void MainWindow::slotWindowsMenuAboutToShow()
{
  QList<BaseFlightElement*> flights = *(_globalMapContents->getFlightList());
//  Q3PtrListIterator<BaseFlightElement> it(flights);
  BaseFlightElement *flight;

  windowMenu->clear();

  int i = 0;
  foreach(flight, flights)
//  for (int i = 0 ; it.current(); ++it , i++)
    {
//      flight = it.current();
      int id = windowMenu->insertItem(flight->getFileName(), _globalMapContents, SLOT(slotSetFlight(int)));

      windowMenu->setItemParameter(id, i);
      windowMenu->setItemChecked(id, _globalMapContents->getFlightIndex() == i);
      i++;
    }
}
