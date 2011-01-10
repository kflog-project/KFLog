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

#include <QtGui>
#include <Qt3Support>

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


MainWindow::MainWindow( QWidget *parent, Qt::WindowFlags flags ) :
  QMainWindow( parent, flags )
{
  qDebug() << "MainWindow()";

  createApplicationDataDirectory();

  _globalMapMatrix   = new MapMatrix(this);
  _globalMapConfig   = new MapConfig(this);
  _globalMapContents = new MapContents(this);

  BaseMapElement::initMapElement( _globalMapMatrix, _globalMapConfig );

  connect( _globalMapConfig, SIGNAL(configChanged()),
           _globalMapMatrix, SLOT(slotInitMatrix()) );

  _globalMapConfig->slotReadConfig();

  initTaskTypes();
  initSurfaceTypes();
  initWaypointTypes();
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
  qDebug() << "~MainWindow()";
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

bool MainWindow::createApplicationDataDirectory()
{
  qDebug() << "MainWindow::createApplicationDataDirectory()";

  QString path = getApplicationDataDirectory();

  QDir dir( path );

  if( ! dir.exists() )
    {
      dir.mkpath( path );

      if( ! dir.exists() )
        {
          qWarning() << "MainWindow: Cannot create Application data directory:" << path;
          return false;
        }
    }

  if( ! dir.isReadable() )
    {
      qWarning() << "MainWindow: Application data directory:" << path << "not readable!";
      return false;
    }

  return true;
}

QString MainWindow::getApplicationDataDirectory()
{
  QString path = _settings.value( "/Path/ApplicationDataDirectory",
                                  QDir::homePath() + "/KFLog" ).toString();
  return path;
}

void MainWindow::closeEvent( QCloseEvent *event )
{
  saveOptions();
  waypoints->saveChanges();

  _globalMapMatrix->writeMatrixOptions();
  event->accept();
}

void MainWindow::initDockWindows()
{
  qDebug() << "MainWindow::initDockWindows() Begin";

  // First create the central widget. That is the Map.
  map = new Map(this);
  setCentralWidget(map);
  _globalMap = map;

  dataViewDock = new QDockWidget( tr("Flight Data"), this );
  dataViewDock->setFloating( false );
  dataView = new DataView(dataViewDock);
  dataViewDock->setWidget(dataView);
  addDockWidget( Qt::RightDockWidgetArea, dataViewDock );
  dataViewDock->setVisible(false);

  qDebug() << "DataView Begin";
  evaluationWindowDock = new QDockWidget( tr("Evaluation"), this );
  evaluationWindowDock->setVisible(false);
  evaluationWindowDock->setFloating( true );
  evaluationWindow = new EvaluationDialog(evaluationWindowDock);
  evaluationWindowDock->setWidget(evaluationWindow);
  addDockWidget( Qt::AllDockWidgetAreas, evaluationWindowDock );

  qDebug() << "DataView End";

  helpWindowDock = new QDockWidget( tr("Help"), this );
  helpWindowDock->setVisible(false);
  helpWindowDock->setFloating( true );
  helpWindow = new HelpWindow(helpWindowDock);
  helpWindowDock->setWidget(helpWindow);
  addDockWidget( Qt::AllDockWidgetAreas, helpWindowDock );

  legendDock = new QDockWidget( tr("Terrain Profile"), this );
  legendDock->setFloating( false );
  legend = new TopoLegend(legendDock);
  legendDock->setWidget(legend);
  addDockWidget( Qt::RightDockWidgetArea, legendDock );
  legendDock->setVisible(false);

  mapControlDock = new QDockWidget( tr("Map Control"), this );
  mapControlDock->setFloating( false );
  mapControl = new MapControlView(mapControlDock);
  mapControlDock->setWidget(mapControl);
  addDockWidget( Qt::BottomDockWidgetArea, mapControlDock );
  mapControlDock->setVisible(false);

  objectTreeDock = new QDockWidget( tr("Loaded Objects"), this );
  objectTreeDock->setFloating( false );
  objectTree = new ObjectTree(objectTreeDock);
  objectTreeDock->setWidget(objectTree);
  addDockWidget( Qt::BottomDockWidgetArea, objectTreeDock );
  objectTreeDock->setVisible(false);

  waypointsDock = new QDockWidget( tr("Waypoints"), this );
  waypointsDock->setFloating( false );
  waypoints = new Waypoints(waypointsDock);
  waypointsDock->setWidget(waypoints);
  addDockWidget( Qt::BottomDockWidgetArea, waypointsDock );
  waypointsDock->setVisible(false);

//  connect(dataViewDock, SIGNAL(iMBeingClosed()), SLOT(slotHideDataViewDock()));
  connect(dataViewDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));

  connect(evaluationWindowDock, SIGNAL(visibilityChanged(bool)), SLOT(slotEvaluationWindowVisibilityChanged(bool)));
  connect(evaluationWindowDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));

//  connect(helpWindowDock, SIGNAL(iMBeingClosed()), SLOT(slotHideHelpWindowDock()));
  connect(helpWindowDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));
//  connect(legendDock, SIGNAL(iMBeingClosed()), SLOT(slotHideLegendDock()));
  connect(legendDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));
//  connect(mapViewDock, SIGNAL(iMBeingClosed()), SLOT(slotHideMapViewDock()));
//  connect(mapViewDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));
//  connect(mapControlDock, SIGNAL(iMBeingClosed()), SLOT(slotHideMapControlDock()));
  connect(mapControlDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));
//  connect(objectTreeDock, SIGNAL(iMBeingClosed()), SLOT(slotHideObjectTreeDock()));
  connect(objectTreeDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));
//  connect(waypointsDock, SIGNAL(iMBeingClosed()), SLOT(slotHideWaypointsDock()));
  connect(waypointsDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotCheckDockWidgetStatus()));

  qDebug() << "MainWindow::initDockWindows() End";
}

void MainWindow::initMenuBar()
{
  qDebug() << "MainWindow::initMenuBar() Begin";

  //----------------------------------------------------------------------------
  // File menu actions
  //----------------------------------------------------------------------------
  fileNewWaypointAction = new QAction( getPixmap("waypoint_16.png"),
                                       tr("New &Waypoint"), this );
  fileNewWaypointAction->setEnabled(true);
  connect( fileNewWaypointAction, SIGNAL(triggered()),
           waypoints, SLOT(slotNewWaypoint()) );

  fileNewTaskAction = new QAction( getPixmap("task_16.png"), tr("New &Task"), this );
  fileNewTaskAction->setShortcut( Qt::CTRL + Qt::Key_N );
  fileNewTaskAction->setEnabled(true);
  connect( fileNewTaskAction, SIGNAL(triggered()),
           _globalMapContents, SLOT(slotNewTask()) );

  fileNewFlightGroupAction = new QAction( tr("New &Flight Group"), this );
  fileNewFlightGroupAction->setEnabled(true);
  connect( fileNewFlightGroupAction, SIGNAL(triggered()),
           _globalMapContents, SLOT(slotNewFlightGroup()) );

  fileOpenFlightAction = new QAction( getPixmap("kde_fileopen_16.png"),
                                     tr("&Open Flight"), this );
  fileOpenFlightAction->setShortcut( Qt::CTRL + Qt::Key_O );
  fileOpenFlightAction->setEnabled(true);
  connect( fileOpenFlightAction, SIGNAL(triggered()), this, SLOT(slotOpenFile()) );

  fileOpenTaskAction = new QAction( getPixmap("kde_fileopen_16.png"),
                                    tr("Open &Task"), this );
  fileOpenTaskAction->setShortcut( Qt::CTRL + Qt::Key_T );
  fileOpenTaskAction->setEnabled(true);
  connect( fileOpenTaskAction, SIGNAL(triggered()), this, SLOT(slotOpenTask()) );

  fileCloseAction = new QAction(getPixmap( "kde_cancel_16.png"),
                                           tr("&Close Flight/Task"), this );
  fileCloseAction->setShortcut( Qt::CTRL + Qt::Key_W );
  fileCloseAction->setEnabled(true);
  connect( fileCloseAction, SIGNAL(triggered()),
           _globalMapContents, SLOT(closeFlight()) );

  fileSavePixmapAction = new QAction( getPixmap("kde_image_16.png"),
                                      tr("Export to PNG..."), this );
  fileSavePixmapAction->setEnabled(true);
  connect( fileSavePixmapAction, SIGNAL(triggered()), map, SLOT(slotSavePixmap()) );

  filePrintAction = new QAction( getPixmap("kde_fileprint_16.png"),
                                 tr("Print..."), this );
  filePrintAction->setShortcut( Qt::CTRL + Qt::Key_P );
  filePrintAction->setEnabled(true);
  connect( filePrintAction, SIGNAL(triggered()), this, SLOT(slotFilePrint()) );

  filePrintFlightAction = new QAction( getPixmap("kde_fileprint_16.png"),
                                       tr("Print Flight Data"), this );
  filePrintFlightAction->setEnabled(true);
  connect( filePrintFlightAction, SIGNAL(triggered()), this, SLOT(slotFlightPrint()) );

  fileOpenRecorderAction = new QAction( getPixmap("kde_connect_no_16.png"),
                                        tr("Open Recorder"), this );
  fileOpenRecorderAction->setEnabled(true);
  connect( fileOpenRecorderAction, SIGNAL(triggered()),
           this, SLOT(slotOpenRecorderDialog()) );

  fileQuitAction = new QAction( getPixmap("kde_exit_16.png"),
                                tr("&Quit"), this );
  fileQuitAction->setShortcut( Qt::CTRL + Qt::Key_Q );
  fileQuitAction->setEnabled(true);
  connect(fileQuitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()) );
  connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT(quit()) );

  //----------------------------------------------------------------------------
  // File menu creation
  //----------------------------------------------------------------------------
  QMenu *fileMenu = menuBar()->addMenu( tr("&File") );
  QMenu *fileNewMenu = fileMenu->addMenu( getPixmap("kde_filenew_16.png"), tr("&New") );
  fileNewMenu->addAction( fileNewWaypointAction );
  fileNewMenu->addAction( fileNewTaskAction );
  fileNewMenu->addAction( fileNewFlightGroupAction );

  fileMenu->addAction( fileOpenFlightAction );
  fileMenu->addAction( fileOpenTaskAction );

  // recent files submenu
  fileOpenRecentMenu = fileMenu->addMenu( getPixmap( "kde_fileopen_16.png"),
                                          tr("&Open recent files") );

  QStringList datalist = _settings.value("/GeneralOptions/RecentFiles").toStringList();

  int size = qMin( datalist.size(), 5 );

  for( int i = 0; i < size; i++ )
    {
      QAction *action = new QAction( this );
      action->setEnabled( true );
      action->setToolTip( datalist[i] );
      action->setText( QFileInfo( datalist[i]).fileName() );
      fileOpenRecentMenu->addAction( action );

      connect( action, SIGNAL(triggered(QAction *)),
               this, SLOT(slotOpenRecentFile(QAction *)) );
    }

  fileMenu->addAction( fileCloseAction );
  fileMenu->addSeparator();
  fileMenu->addAction( fileSavePixmapAction );
  fileMenu->addSeparator();
  fileMenu->addAction( filePrintAction );
  fileMenu->addAction( filePrintFlightAction );
  fileMenu->addSeparator();
  fileMenu->addAction( fileOpenRecorderAction );
  fileMenu->addSeparator();
  fileMenu->addAction( fileQuitAction );

  //----------------------------------------------------------------------------
  // View menu actions
  //----------------------------------------------------------------------------
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

  //----------------------------------------------------------------------------
  // Flight menu actions
  //----------------------------------------------------------------------------
  flightEvaluationWindowAction = new QAction( getPixmap("kde_history_16.png"),
                                              tr("Show &Evaluation Window"), this );
  flightEvaluationWindowAction->setShortcut( Qt::CTRL + Qt::Key_E );
  flightEvaluationWindowAction->setEnabled(true);
  connect( flightEvaluationWindowAction, SIGNAL(triggered(bool)),
           this, SLOT(slotToggleEvaluationWindow(bool)) );

  flightOptimizationAction = new QAction( getPixmap("kde_wizard_16.png"),
                                          tr("Optimize"), this );
  flightOptimizationAction->setEnabled(true);
  connect( flightOptimizationAction, SIGNAL(triggered()),
           this, SLOT(slotOptimizeFlight()) );

  flightOptimizationOLCAction = new QAction( getPixmap("kde_wizard_16.png"),
                                             tr("Optimize for OLC"), this );
  flightOptimizationOLCAction->setEnabled(true);
  connect( flightOptimizationOLCAction, SIGNAL(triggered()),
           this, SLOT(slotOptimizeFlightOLC()) );

  // flight data type actions
  altitudeAction = new QAction( tr("Altitude"), this );
  altitudeAction->setCheckable( true );
  altitudeAction->setData( 0 );

  cyclingAction = new QAction( tr("Cycling"), this );
  cyclingAction->setCheckable( true );
  cyclingAction->setData( 1 );

  speedAction = new QAction( tr("Speed"), this );
  speedAction->setCheckable( true );
  speedAction->setData( 2 );

  varioAction = new QAction( tr("Vario"), this );
  varioAction->setCheckable( true );
  varioAction->setData( 3 );

  solidAction = new QAction( tr("Solid"), this );
  solidAction->setCheckable( true );
  solidAction->setData( 4 );

  flightDataTypeGroupAction = new QActionGroup( this );
  flightDataTypeGroupAction->addAction( altitudeAction );
  flightDataTypeGroupAction->addAction( cyclingAction );
  flightDataTypeGroupAction->addAction( speedAction );
  flightDataTypeGroupAction->addAction( varioAction );
  flightDataTypeGroupAction->addAction( solidAction );

  selectFlightDataAction( _settings.value( "/Flight/DrawType",
                                           MapConfig::Altitude).toInt() );

  connect( flightDataTypeGroupAction, SIGNAL(triggered(QAction *)),
           this, SLOT(slotFlightDataTypeGroupAction(QAction *)) );

  flightIgc3DAction = new QAction( getPixmap("kde_vectorgfx_16.png"),
                                   tr("View flight in 3D"), this );
  flightIgc3DAction->setShortcut( Qt::CTRL + Qt::Key_R );
  flightIgc3DAction->setEnabled(true);
  connect( flightIgc3DAction, SIGNAL(triggered()),
           this, SLOT(slotFlightViewIgc3D()) );

  // FIXME: icons is missing /*SmallIcon("openglgfx"),*/
  flightIgcOpenGLAction = new QAction( tr("View flight in 3D (OpenGL)"), this );
  flightIgcOpenGLAction->setEnabled(true);
  connect( flightIgcOpenGLAction, SIGNAL(triggered()),
           this, SLOT(slotFlightViewIgcOpenGL()) );

  flightAnimateStartAction = new QAction( getPixmap("kde_1rightarrow_16.png"),
                                    tr("&Start Flight Animation"), this );
  flightAnimateStartAction->setShortcut( Qt::Key_F12 );
  flightAnimateStartAction->setEnabled(true);
  connect( flightAnimateStartAction, SIGNAL(triggered()),
           map, SLOT(slotAnimateFlightStart()));

  flightAnimateStopAction = new QAction( getPixmap("kde_player_stop_16.png"),
                                   tr("Stop Flight &Animation"), this );
  flightAnimateStopAction->setShortcut( Qt::Key_F11 );
  flightAnimateStopAction->setEnabled(true);
  connect( flightAnimateStopAction, SIGNAL(triggered()),
           map, SLOT(slotAnimateFlightStop()));

  flightAnimateNextAction = new QAction( getPixmap("kde_forward_16.png"),
                                   tr("Next Flight Point"), this );
  flightAnimateNextAction->setShortcut( Qt::CTRL+Qt::Key_Up );
  flightAnimateNextAction->setEnabled(true);
  connect(flightAnimateNextAction, SIGNAL(triggered()), map, SLOT(slotFlightNext()));

  flightAnimatePrevAction = new QAction( getPixmap("kde_back_16.png"),
                      tr("Prev Flight Point"), this );
  flightAnimatePrevAction->setShortcut( Qt::CTRL+Qt::Key_Down );
  flightAnimatePrevAction->setEnabled(true);
  connect(flightAnimatePrevAction, SIGNAL(triggered()), map, SLOT(slotFlightPrev()));

  flightAnimate10NextAction = new QAction( getPixmap("kde_2rightarrow_16.png"),
                                    tr("Step +10 Flight Points"), this );
  flightAnimate10NextAction->setShortcut( Qt::Key_PageUp );
  flightAnimate10NextAction->setEnabled(true);
  connect(flightAnimate10NextAction, SIGNAL(triggered()), map, SLOT(slotFlightStepNext()));

  flightAnimate10PrevAction = new QAction( getPixmap("kde_2leftarrow_16.png"),
                                    tr("Step -10 Flight Points"), this);
  flightAnimate10PrevAction->setShortcut( Qt::Key_PageDown );
  flightAnimate10PrevAction->setEnabled(true);
  connect(flightAnimate10PrevAction, SIGNAL(triggered()), map, SLOT(slotFlightStepPrev()));

  flightAnimateHomeAction = new QAction( getPixmap("kde_start_16.png"),
                                   tr("First Flight Point"), this );
  flightAnimateHomeAction->setShortcut( Qt::Key_Home );
  flightAnimateHomeAction->setEnabled(true);
  connect(flightAnimateHomeAction, SIGNAL(triggered()), map, SLOT(slotFlightHome()));

  flightAnimateEndAction = new QAction( getPixmap("kde_finish_16.png"),
                                  tr("Last Flight Point"), this );
  flightAnimateEndAction->setShortcut( Qt::Key_End );
  flightAnimateEndAction->setEnabled(true);
  connect(flightAnimateEndAction, SIGNAL(triggered()), map, SLOT(slotFlightEnd()));

  //----------------------------------------------------------------------------
  // Flight menu creation
  //----------------------------------------------------------------------------
  QMenu *fm = menuBar()->addMenu( tr("F&light") );
  fm->addAction( flightEvaluationWindowAction );
  fm->addAction( flightOptimizationAction );
  fm->addAction( flightOptimizationOLCAction );

  QMenu *fdtMenu = fm->addMenu( getPixmap("kde_idea_16.png"), tr("Show Flight Data") );
  fdtMenu->addAction( altitudeAction );
  fdtMenu->addAction( cyclingAction );
  fdtMenu->addAction( speedAction );
  fdtMenu->addAction( varioAction );
  fdtMenu->addAction( solidAction );

  fm->addAction( flightIgc3DAction );
  fm->addAction( flightIgcOpenGLAction );
  fm->addSeparator();
  fm->addAction( flightAnimateStartAction );
  fm->addAction( flightAnimateStopAction );
  fm->addAction( flightAnimateNextAction );
  fm->addAction( flightAnimatePrevAction );
  fm->addAction( flightAnimate10NextAction );
  fm->addAction( flightAnimate10PrevAction );
  fm->addAction( flightAnimateHomeAction );
  fm->addAction( flightAnimateEndAction );

  //----------------------------------------------------------------------------
  // Window menu creation
  //----------------------------------------------------------------------------
  windowMenu = menuBar()->addMenu( tr("&Window") );
  connect( windowMenu, SIGNAL(aboutToShow()),
           this, SLOT(slotWindowsMenuAboutToShow()) );

  // settings menu

  settingsEvaluationWindow = new QAction(getPixmap("kde_history_16.png"), tr("Show &Evaluation Window"), Qt::CTRL+Qt::Key_E, this, "toggle_evaluation_window");
  settingsEvaluationWindow->setToggleAction(true);
  connect(settingsEvaluationWindow, SIGNAL(activated()), this, SLOT(slotToggleEvaluationWindow()));


  settingsFlightData = new QAction(getPixmap("kde_view_detailed_16.png"), tr("Show Flight &Data"), Qt::CTRL+Qt::Key_E, this, "toggle_data_view");
  settingsFlightData->setToggleAction(true);
  connect(settingsFlightData, SIGNAL(activated()), this, SLOT(slotToggleDataView()));
  settingsHelpWindow = new QAction(getPixmap("kde_info_16.png"), tr("Show Help Window"), Qt::CTRL+Qt::Key_H, this, "toggle_help_window");
  settingsHelpWindow->setToggleAction(true);
  connect(settingsHelpWindow, SIGNAL(activated()), this, SLOT(slotToggleHelpWindow()));
  settingsObjectTree = new QAction(getPixmap("kde_view_tree_16.png"), tr("Show KFLog &Browser"), Qt::CTRL+Qt::Key_B, this, "view_tree");
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

  //----------------------------------------------------------------------------
  // Help menu creation
  //----------------------------------------------------------------------------
  QMenu *help = menuBar()->addMenu( tr("&Help") );

  help->addAction( getPixmap("kde_contexthelp_16.png"),
                             tr("What's This?"),
                             this,
                             SLOT(slotWhatsThis()), Qt::CTRL + Qt::Key_F1 );

  help->addAction( getPixmap("qt-logo-16x16.jpeg"), tr("About &Qt"),
                   qApp, SLOT(aboutQt()), Qt::Key_Q );

  //FIXME: link to manual must be added
  //FIXME: dialog to swith application language must be added
//  help->insertItem(getPixmap("kde_idea_16.png"), tr("Tip of the day") );//, this, SLOT(slotTipOfDay()));
//  help->insertItem(getPixmap("kflog_16.png"), tr("About KFLog") );//, this, SLOT(slotShowAbout()));

  // FIXME: menuBar()->insertItem("Dock Windows", createDockWindowMenu());

  qDebug() << "MainWindow::initMenuBar() End";
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
  statusLatL->setFixedHeight( statusLabel->sizeHint().height() );
  statusLatL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusLatL->setMargin(0);
  statusLatL->setLineWidth(0);
  statusLatL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusLonL = new QLabel(statusBar(), "lon_label_status_bar");
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
  toolBar = addToolBar( tr("Toolbar") );

  toolBar->addAction( fileOpenFlightAction );
  toolBar->addSeparator();
  toolBar->addAction( viewZoom );
  toolBar->addAction( viewZoomIn );
  toolBar->addAction( viewZoomOut );
  toolBar->addAction( viewRedraw );
  toolBar->addSeparator();
  toolBar->addAction( viewCenterTask );
  toolBar->addAction( viewCenterFlight );
  toolBar->addAction( viewCenterHomesite );
  toolBar->addSeparator();
  toolBar->addAction( flightEvaluationWindowAction );
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
  if(toolBar->isVisible()!=bViewToolbar)
    slotToggleToolBar();

  bool bViewStatusbar = _settings.readBoolEntry("/GeneralOptions/ShowStatusbar", true);
  if(statusBar()->isVisible()!=bViewStatusbar)
    slotToggleStatusBar();

  // bar position settings
  QSize size(_settings.readNumEntry("/GeneralOptions/GeometryWidth", 950),
             _settings.readNumEntry("/GeneralOptions/GeometryHeight", 700));

  if(!size.isEmpty())
    resize(size);

  // initialize the recent file list
  flightDir = _settings.value( "/Path/DefaultFlightDirectory",
                               getApplicationDataDirectory() ).toString();
  taskDir   = _settings.value( "/Path/DefaultWaypointDirectory",
                               getApplicationDataDirectory() ).toString();

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
  qDebug("saving options...");

  _settings.setValue("/GeneralOptions/GeometryWidth", size().width());
  _settings.setValue("/GeneralOptions/GeometryHeight", size().height());
  _settings.setValue("/GeneralOptions/ShowToolbar", toolBar->isVisible());
  _settings.setValue("/GeneralOptions/ShowStatusbar", statusBar()->isVisible());

  if( _settings.value("/Waypoints/DefaultWaypointCatalog", KFLogConfig::LastUsed).toInt() ==
      KFLogConfig::LastUsed && waypoints->getCurrentCatalog() != static_cast<WaypointCatalog *> (0) )
    {
      // Only write the path, if a waypoint-catalog is opened.
      // Otherwise KFLog crashes on a clean installation.
      //qDebug("saving catalog name");
      _settings.setValue( "/Waypoints/DefaultCatalogName", waypoints->getCurrentCatalog()->path );
    }

// FIXME: use QMainWindow::saveState in Qt4
//  _settings.setValue("/GeneralOptions/ToolBarPos", (int) toolBar("mainToolBar")->barPos());
//  writeDockConfig(config, "Window Layout");
}

void MainWindow::slotCenterTo()
{
  CenterToDialog* center = new CenterToDialog( this );

  connect( center, SIGNAL(centerTo(int,int)), _globalMapMatrix,
           SLOT(slotCenterTo(int, int)));
  center->setVisible(true);
}

void MainWindow::slotFlightViewIgc3D()
{
  flightIgc3DAction->setEnabled(false);

  Igc3DDialog *igc3d = new Igc3DDialog(this);

  connect( _globalMapContents, SIGNAL(currentFlightChanged()),
           igc3d, SLOT(slotShowFlightData()) );

  connect( igc3d, SIGNAL(accepted()), this, SLOT(slotFlightViewIgc3DClosed()) );
  connect( igc3d, SIGNAL(rejected()), this, SLOT(slotFlightViewIgc3DClosed()) );
}

void MainWindow::slotFlightViewIgc3DClosed()
{
  flightIgc3DAction->setEnabled(true);
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

  qDebug() << "KFLogApp::slotFlightViewIgcOpenGL()";

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
            fileCloseAction->setEnabled(true);
            filePrintFlightAction->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimizationAction->setEnabled(true);
            flightOptimizationOLCAction->setEnabled(true);
            flightIgc3DAction->setEnabled(true);
            flightIgcOpenGLAction->setEnabled(true);
            flightAnimateStartAction->setEnabled(true);
            flightAnimateStopAction->setEnabled(true);
            flightAnimateNextAction->setEnabled(true);
            flightAnimatePrevAction->setEnabled(true);
            flightAnimate10NextAction->setEnabled(true);
            flightAnimate10PrevAction->setEnabled(true);
            flightAnimateHomeAction->setEnabled(true);
            flightAnimateEndAction->setEnabled(true);
            windowMenu->setEnabled(true);
            break;

          case BaseMapElement::Task:
            fileCloseAction->setEnabled(true);
            filePrintFlightAction->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(false);
//            flightEvaluation->setEnabled(false);
            flightOptimizationAction->setEnabled(false);
            flightOptimizationOLCAction->setEnabled(false);
            flightIgc3DAction->setEnabled(false);
            flightIgcOpenGLAction->setEnabled(false);
            flightAnimateStartAction->setEnabled(false);
            flightAnimateStopAction->setEnabled(false);
            flightAnimateNextAction->setEnabled(false);
            flightAnimatePrevAction->setEnabled(false);
            flightAnimate10NextAction->setEnabled(false);
            flightAnimate10PrevAction->setEnabled(false);
            flightAnimateHomeAction->setEnabled(false);
            flightAnimateEndAction->setEnabled(false);
            windowMenu->setEnabled(true);
            break;

          case BaseMapElement::FlightGroup:
            fileCloseAction->setEnabled(true);
            filePrintFlightAction->setEnabled(true);
            viewCenterTask->setEnabled(true);
            viewCenterFlight->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimizationAction->setEnabled(true);
            flightOptimizationOLCAction->setEnabled(true);
            flightIgc3DAction->setEnabled(true);
            flightIgcOpenGLAction->setEnabled(true);
            flightAnimateStartAction->setEnabled(true);
            flightAnimateStopAction->setEnabled(true);
            flightAnimateNextAction->setEnabled(true);
            flightAnimatePrevAction->setEnabled(true);
            flightAnimate10NextAction->setEnabled(true);
            flightAnimate10PrevAction->setEnabled(true);
            flightAnimateHomeAction->setEnabled(true);
            flightAnimateEndAction->setEnabled(true);
            windowMenu->setEnabled(true);
            break;

          default:
            break;
        }
  }
  else
  {
      fileCloseAction->setEnabled(false);
      filePrintFlightAction->setEnabled(false);
      viewCenterTask->setEnabled(false);
      viewCenterFlight->setEnabled(false);
//      flightEvaluation->setEnabled(false);
      flightOptimizationAction->setEnabled(false);
      flightOptimizationOLCAction->setEnabled(false);
      flightIgc3DAction->setEnabled(false);
      flightIgcOpenGLAction->setEnabled(false);
      flightAnimateStartAction->setEnabled(false);
      flightAnimateStopAction->setEnabled(false);
      flightAnimateNextAction->setEnabled(false);
      flightAnimatePrevAction->setEnabled(false);
      flightAnimate10NextAction->setEnabled(false);
      flightAnimate10PrevAction->setEnabled(false);
      flightAnimateHomeAction->setEnabled(false);
      flightAnimateEndAction->setEnabled(false);
      windowMenu->setEnabled(false);
  }
}

void MainWindow::slotOpenFile()
{
  Q3FileDialog* fd = new Q3FileDialog(this);
  fd->setCaption(tr("Open flight"));
  fd->setDir(flightDir);

  QString filter;
  filter.append(tr("All flight type files") +" (*.igc *.flightgear *.trk *.gdn)");
  filter.append(tr("IGC") +" (*.igc)");
  filter.append(tr("Garmin") +" (*.trk *.gdn)");
  fd->setFilters(filter);

  IGCPreview* preview = new IGCPreview(fd);
  fd->setContentsPreviewEnabled( true );
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

  QFileDialog* fd = new QFileDialog( this );
  fd->setWindowTitle( tr( "Open Task" ) );
  fd->setDirectory( taskDir );
  fd->setFileMode( QFileDialog::ExistingFile );

  QStringList filters;
  filters.append( tr( "KFLog tasks" ) + "(*.kflogtsk *.KFLOGTSK)" );
  fd->setFilters( filters );

  if( fd->exec() == QDialog::Accepted )
    {
      QStringList fNames = fd->selectedFiles();

      if( fNames.size() == 0 )
        {
          return;
        }

      QString fName = fNames[0];

      QUrl fUrl = QUrl( fName );
      taskDir = fd->directory().path();

      if( !fUrl.isValid() )
        {
          return;
        }

      QString scheme = fUrl.scheme();

#warning "Check, if that is right proted to Qt4"

      if( scheme != "file" )
        {
          return;
        }

      QFile file( fName );

      if( _globalMapContents->loadTask( file ) )
        {
          slotSetCurrentFile( fName );
        }
    }

  slotSetStatusMsg( tr( "Ready." ) );
}

void MainWindow::slotOpenRecentFile( QAction *action )
{
  if( !action )
    {
      return;
    }

  QString fileName = action->toolTip();

  slotSetStatusMsg( tr( "Opening File..." ) );

  FlightLoader flightLoader;

  Q3Url url( fileName );

  if( url.isLocalFile() )
    {
      QFile file( url.path() );

      if( url.fileName().right( 9 ).lower() == ".kflogtsk" )
        {
          // this is probably a task file. Try to open it as a task
          if( _globalMapContents->loadTask( file ) )
            {
              slotSetCurrentFile( url.path() );
            }
        }
      else
        {
          //try to open as flight
          if( flightLoader.openFlight( file ) )
            {
              slotSetCurrentFile( url.path() );
            }
        } // .kflogtsk
    } //isLocalFile

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotOpenRecorderDialog()
{
  RecorderDialog* dlg = new RecorderDialog(this);
  connect(dlg, SIGNAL(addCatalog(WaypointCatalog *)), waypoints, SLOT(slotAddCatalog(WaypointCatalog *)));
  connect(dlg, SIGNAL(addTask(FlightTask *)), _globalMapContents, SLOT(slotAppendTask(FlightTask *)));
  dlg->exec();
  delete dlg;
}

void MainWindow::slotOptimizeFlight()
{
  Flight *flight = dynamic_cast<Flight *> (_globalMapContents->getFlight());

  if( flight != 0 && flight->getObjectType() == BaseMapElement::Flight )
    {
      if( flight->optimizeTask() )
        {
          // Okay, update flight data and redraw the map
          dataView->setFlightData();
          map->slotRedrawFlight();
          objectTree->slotFlightChanged();
        }
    }
}

void MainWindow::slotOptimizeFlightOLC()
{
  Flight *flight = dynamic_cast<Flight *> (_globalMapContents->getFlight());

  if( flight != 0 && flight->getObjectType() == BaseMapElement::Flight )
    {
      if( flight->optimizeTaskOLC(map) )
        {
          // Okay, update flight data and redraw map
          dataView->setFlightData();
          map->slotRedrawFlight();
          objectTree->slotFlightChanged();
        }
    }
}

/** Connects the dialogs addWaypoint signal to the waypoint object. */
void MainWindow::slotRegisterWaypointDialog(QWidget * dialog)
{
  connect(dialog, SIGNAL(addWaypoint(Waypoint *)), waypoints, SLOT(slotAddWaypoint(Waypoint *)));
}

void MainWindow::slotSavePixmap(QUrl url, int width, int height)
{
  map->slotSavePixmap(url, width, height);
}

void MainWindow::slotFlightDataTypeGroupAction( QAction *action )
{
  // Get index from action
  int index = action->data().toInt();

  qDebug() << " MainWindow::slotFlightDataTypeGroupAction: ID=" << index;

  // Select indexed action
  slotSelectFlightData( index );
}

void MainWindow::selectFlightDataAction( const int index )
{
  switch( index )
    {
      case MapConfig::Altitude:    // Altitude
        altitudeAction->setChecked( true );
        cyclingAction->setChecked( false );
        speedAction->setChecked( false );
        varioAction->setChecked( false );
        solidAction->setChecked( false );
        break;
      case MapConfig::Cycling:     // Cycling
        altitudeAction->setChecked( false );
        cyclingAction->setChecked( true );
        speedAction->setChecked( false );
        varioAction->setChecked( false );
        solidAction->setChecked( false );
        break;
      case MapConfig::Speed:       // Speed
        altitudeAction->setChecked( false );
        cyclingAction->setChecked( false );
        speedAction->setChecked( true );
        varioAction->setChecked( false );
        solidAction->setChecked( false );
        break;
      case MapConfig::Vario:       // Vario
        altitudeAction->setChecked( false );
        cyclingAction->setChecked( false );
        speedAction->setChecked( false );
        varioAction->setChecked( true );
        solidAction->setChecked( false );
        break;
      case MapConfig::Solid:       // Solid color
        altitudeAction->setChecked( false );
        cyclingAction->setChecked( false );
        speedAction->setChecked( false );
        varioAction->setChecked( false );
        solidAction->setChecked( true );
        break;

      default:
        qWarning() << "MainWindow::selectFlightDataAction: Unknown identifier"
                   << index;
        break;
    }
}

void MainWindow::slotSelectFlightData( const int index )
{
  qDebug() << "MainWindow::slotSelectFlightData: index=" << index;

  selectFlightDataAction( index );

  switch( index )
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

      default:
        qWarning() << "MainWindow::slotSelectFlightData: Unknown identifier"
                   << index;
        break;
    }

  map->slotRedrawFlight();
}

void MainWindow::slotSetCurrentFile( const QString &fileName )
{
  QStringList files = _settings.value("/GeneralOptions/RecentFiles").toStringList();
  QStringList newFiles;

  int recentFilesMax = _settings.value("/GeneralOptions/RecentFilesMax", 5).toInt();
  int index = 0;

  newFiles.append(fileName);

  for( int i = 0; i < files.size() && index <= recentFilesMax; i++ )
  {
    if( files[i] != fileName )
    {
      newFiles.append( files[i] );
      index++;
    }
  }

  _settings.setValue( "/GeneralOptions/RecentFiles", newFiles );

  // rebuild recent file menu
  fileOpenRecentMenu->clear();

  int size = qMin( newFiles.size(), 5 );

  for( int i = 0; i < size; i++ )
    {
      QAction *action = new QAction( this );
      action->setEnabled( true );
      action->setToolTip( newFiles[i] );
      action->setText( QFileInfo( newFiles[i]).fileName() );
      fileOpenRecentMenu->addAction( action );

      connect( action, SIGNAL(triggered(QAction *)),
               this, SLOT(slotOpenRecentFile(QAction *)) );
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
  statusBar()->clearMessage();
  statusLabel->setText(text);
}

/* Slot to set filename for WaypointCatalog */
void MainWindow::slotSetWaypointCatalog(QString catalog)
{
  waypoints->slotSetWaypointCatalogName(catalog);
}

void MainWindow::slotCheckDockWidgetStatus()
{
  flightEvaluationWindowAction->setDisabled(evaluationWindowDock->isVisible());
  settingsEvaluationWindow->setDisabled(evaluationWindowDock->isVisible());
  settingsFlightData->setDisabled(dataViewDock->isVisible());
  settingsHelpWindow->setDisabled(helpWindowDock->isVisible());
  settingsObjectTree->setDisabled(objectTreeDock->isVisible());
  settingsLegend->setDisabled(legendDock->isVisible());
//  settingsMap->setDisabled(mapViewDock->isVisible());
  settingsMapControl->setDisabled(mapControlDock->isVisible());
  settingsStatusBar->setDisabled(statusBar()->isVisible());
  settingsToolBar->setDisabled(toolBar->isVisible());
  settingsWaypoints->setDisabled(waypointsDock->isVisible());
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

void MainWindow::slotToggleDataView()
{
  if(dataViewDock->isVisible())
    dataViewDock->setVisible(false);
  else
    dataViewDock->setVisible(true);

  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleEvaluationWindow( bool flag )
{
  evaluationWindowDock->setVisible( ! flag );
  slotCheckDockWidgetStatus();
}

void MainWindow::slotEvaluationWindowVisibilityChanged( bool flag )
{
  // Toggle access to menu action according to visibility of related widget.
  flightEvaluationWindowAction->setEnabled( ! flag );
}

void MainWindow::slotToggleHelpWindow()
{
  if(helpWindowDock->isVisible())
    helpWindowDock->setVisible(false);
  else
    helpWindowDock->setVisible(true);

  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleLegendDock()
{
  if(legendDock->isVisible())
    legendDock->setVisible(false);
  else
    legendDock->setVisible(true);

  slotCheckDockWidgetStatus();
}

// Map is now a background widget instead of a dock
//void KFLog::slotToggleMap()
//{
//  if(mapViewDock->isVisible())
//    mapViewDock->setVisible(false);
//  else
//    mapViewDock->setVisible(true);
////  slotCheckDockWidgetStatus();
//}

void MainWindow::slotToggleMapControl()
{
  if(mapControlDock->isVisible())
    mapControlDock->setVisible(false);
  else
    mapControlDock->setVisible(true);

  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleObjectTreeDock()
{
  if(objectTreeDock->isVisible())
    objectTreeDock->setVisible(false);
  else
    objectTreeDock->setVisible(true);

  slotCheckDockWidgetStatus();
}

void MainWindow::slotToggleStatusBar()
{
  slotSetStatusMsg(tr("Toggle the statusbar..."));

  if(statusBar()->isVisible())
    statusBar()->setVisible(false);
  else
    statusBar()->setVisible(true);

  slotCheckDockWidgetStatus();

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotToggleToolBar()
{
  slotSetStatusMsg(tr("Toggling toolbar..."));

  if(toolBar->isVisible())
    {
      toolBar->setVisible(false);
    }
  else
    {
      toolBar->setVisible(true);
    }

  slotCheckDockWidgetStatus();
  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotToggleWaypointsDock()
{
  if(waypointsDock->isVisible())
    waypointsDock->setVisible(false);
  else
    waypointsDock->setVisible(true);

  slotCheckDockWidgetStatus();
}

/** Called to the What's This? mode. */
void MainWindow::slotWhatsThis()
{
  QWhatsThis::enterWhatsThisMode();
}

/** Insert available flights into menu. */
void MainWindow::slotWindowsMenuAboutToShow()
{
  QList<BaseFlightElement *> *flights = _globalMapContents->getFlightList();

  windowMenu->clear();

  for( int i = 0; flights->size(); i++ )
    {
      QAction *action = new QAction( this );
      action->setText( flights->at(i)->getFileName() );
      action->setEnabled( true );
      action->setData( i );
      action->setChecked( _globalMapContents->getFlightIndex() == i );
      windowMenu->addAction( action );

      connect( action, SIGNAL(triggered(QAction *)),
               _globalMapContents, SLOT(slotSetFlight(QAction *)) );
    }
}