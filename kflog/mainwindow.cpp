/***********************************************************************
**
**   MainWindow.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**                   2010-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <dlfcn.h>

#include <QtGui>
#include <QSortFilterProxyModel>

#include "aboutwidget.h"
#include "airfield.h"
#include "centertodialog.h"
#include "dataview.h"
#include "distance.h"
#include "evaluationdialog.h"
#include "flightdataprint.h"
#include "flightloader.h"
#include "helpwindow.h"
#include "igc3ddialog.h"
#include "kflogconfig.h"
#include "map.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "mapcontrolview.h"
#include "mapdefaults.h"
#include "mapmatrix.h"
#include "objecttree.h"
#include "recorderdialog.h"
#include "taskdataprint.h"
#include "target.h"
#include "topolegend.h"
#include "wgspoint.h"
#include "waypointtreeview.h"

#include "mainwindow.h"


extern QSettings _settings;

/**
 * external reference, set by main to this class.
 */
extern MainWindow *_mainWindow;

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
#warning "This must be done once to get defined a new limit value."
#warning "Remove that after the next delivery!"
  _settings.setValue( "/Scale/LowerLimit", BORDER_L );

  setObjectName( "MainWindow" );
  QApplication::setStyle( "plastique" );

  // Initialize units to be used. Use the stored values from the configuration.
  int altUnit  = _settings.value( "/Units/Altitude", Altitude::meters ).toInt();
  int distUnit = _settings.value( "/Units/Distance", Distance::kilometers ).toInt();
  int posUnit  = _settings.value( "/Units/Position", WGSPoint::DMS ).toInt();

  Altitude::setUnit( static_cast<enum Altitude::altitudeUnit>(altUnit) );
  Distance::setUnit( static_cast<enum Distance::distanceUnit>(distUnit) );
  WGSPoint::setFormat( static_cast<enum WGSPoint::Format>(posUnit) );

  createApplicationDataDirectory();

  _globalMapMatrix   = new MapMatrix(this);
  _globalMapConfig   = new MapConfig(this);
  _globalMapContents = new MapContents(this);

  BaseMapElement::initMapElement( _globalMapMatrix, _globalMapConfig );

  connect( _globalMapConfig, SIGNAL(configChanged()),
           _globalMapMatrix, SLOT(slotInitMatrix()) );

  _globalMapConfig->slotReadConfig();

  toolBar = addToolBar( tr("Toolbar") );
  toolBar->setObjectName( "ToolBar");

  createDockWindows();
  createMenuBar();
  createStatusBar();
  createToolBar();
  readOptions();

  connect(map, SIGNAL(showFlightPoint(const QPoint&, const FlightPoint&)), this, SLOT(slotSetPointInfo(const QPoint&, const FlightPoint&)));
  connect(map, SIGNAL(taskPlanningEnd()), helpWindow, SLOT(slotClearView()) );
  connect(map, SIGNAL(showTaskText(FlightTask*)), dataView, SLOT(slotShowTaskText(FlightTask*)));
  connect(map, SIGNAL(taskPlanningEnd()), dataView, SLOT(slotSetFlightData()));
  connect(map, SIGNAL(showPoint(const QPoint&)), this, SLOT(slotSetPointInfo(const QPoint&)));
  connect(map, SIGNAL(showPoint(const QPoint&)), evaluationWindow, SLOT(slotRemoveFlightPoint()));
  connect(map, SIGNAL(showFlightPoint(const QPoint&, const FlightPoint&)), evaluationWindow, SLOT(slotShowFlightPoint(const QPoint&, const FlightPoint&)));
  connect(map, SIGNAL(changed(QSize)), mapControl, SLOT(slotShowMapData(QSize)));
  connect(map, SIGNAL(waypointSelected(Waypoint *)), waypointTreeView, SLOT(slotAddWaypoint(Waypoint *)));
  connect(map, SIGNAL(waypointDeleted(Waypoint *)), waypointTreeView, SLOT(slotDeleteWaypoint(Waypoint *)));
  connect(map, SIGNAL(waypointEdited(Waypoint *)), waypointTreeView, SLOT(slotEditWaypoint(Waypoint *)));
  connect(map, SIGNAL(elevation(int)), legend, SLOT(slotSelectElevation(int)));
  connect(map, SIGNAL(regWaypointDialog(QWidget *)), this, SLOT(slotRegisterWaypointDialog(QWidget *)));
  connect(map, SIGNAL(openFile(const QUrl&)), this, SLOT(slotOpenFile(const QUrl&)));
  connect(map, SIGNAL(setStatusBarProgress(int)), this, SLOT(slotSetProgress(int)));
  connect(map, SIGNAL(setStatusBarMsg(const QString&)), this, SLOT(slotSetStatusMsg(const QString&)));
  connect(map, SIGNAL(flightTaskModified()), objectTree, SLOT(slotFlightChanged()));

  connect(mapControl, SIGNAL(scaleChanged(double)), _globalMapMatrix, SLOT(slotSetScale(double)));

  connect(dataView, SIGNAL(wpSelected(const int)), map, SLOT(slotCenterToWaypoint(const int)));
  connect(dataView, SIGNAL(flightSelected(BaseFlightElement *)), _globalMapContents, SLOT(slotSetFlight(BaseFlightElement *)));
  connect(dataView, SIGNAL(editFlightGroup()), _globalMapContents, SLOT(slotEditFlightGroup()));

  connect(_globalMapContents, SIGNAL(activatePlanning()), map,SLOT(slotActivatePlanning()));
  connect(_globalMapContents, SIGNAL(closingFlight(BaseFlightElement*)), objectTree, SLOT(slotCloseFlight(BaseFlightElement*)));
  connect(_globalMapContents, SIGNAL(contentsChanged()),map, SLOT(slotScheduleRedrawMap()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), this, SLOT(slotModifyMenu()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), dataView, SLOT(slotSetFlightData()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), evaluationWindow, SLOT(slotShowFlightData()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), map, SLOT(slotShowCurrentFlight()));
  connect(_globalMapContents, SIGNAL(currentFlightChanged()), objectTree, SLOT(slotSelectedFlightChanged()));
  connect(_globalMapContents, SIGNAL(clearFlightCursor()), map, SLOT(slotClearCursor()));

  connect(_globalMapContents, SIGNAL(newFlightAdded(Flight*)), objectTree, SLOT(slotNewFlightAdded(Flight*)));
  connect(_globalMapContents, SIGNAL(newFlightGroupAdded(FlightGroup*)), objectTree, SLOT(slotNewFlightGroupAdded(FlightGroup*)));
  connect(_globalMapContents, SIGNAL(newTaskAdded(FlightTask*)), objectTree, SLOT(slotNewTaskAdded(FlightTask*)));
  connect(_globalMapContents, SIGNAL(taskHelp(QString&)), helpWindow, SLOT(slotShowHelpText(QString&)) );
  connect(_globalMapMatrix, SIGNAL(displayMatrixValues(int, bool)), _globalMapConfig, SLOT(slotSetMatrixValues(int, bool)));
  connect(_globalMapMatrix, SIGNAL(matrixChanged()), map, SLOT(slotScheduleRedrawMap()));
  connect(_globalMapMatrix, SIGNAL(printMatrixValues(int)), _globalMapConfig, SLOT(slotSetPrintMatrixValues(int)));
  connect(_globalMapMatrix, SIGNAL(projectionChanged()), _globalMapContents, SLOT(slotReloadMapData()));

  connect(waypointTreeView, SIGNAL(copyWaypoint2Task(Waypoint *)), map, SLOT(slotAppendWaypoint2Task(Waypoint *)));
  connect(waypointTreeView, SIGNAL(waypointCatalogChanged( WaypointCatalog * )), map, SLOT(slotWaypointCatalogChanged( WaypointCatalog * )));
  connect(waypointTreeView, SIGNAL(centerMap(int, int)), _globalMapMatrix, SLOT(slotCenterTo(int, int)));

  connect(objectTree, SIGNAL(newFlightSelected(BaseFlightElement *)), _globalMapContents, SLOT(slotSetFlight(BaseFlightElement *)));
  connect(objectTree, SIGNAL(newTask()), _globalMapContents, SLOT(slotNewTask()));
  connect(objectTree, SIGNAL(openTask()), this, SLOT(slotOpenTask()));
  connect(objectTree, SIGNAL(closeFlightElement()), _globalMapContents, SLOT(slotCloseFlight()));
  connect(objectTree, SIGNAL(newFlightGroup()), _globalMapContents, SLOT(slotNewFlightGroup()));
  connect(objectTree, SIGNAL(editFlightGroup()), _globalMapContents, SLOT(slotEditFlightGroup()));
  connect(objectTree, SIGNAL(openFlight()), this, SLOT(slotOpenFile()));
  connect(objectTree, SIGNAL(openFile(const QUrl&)), this, SLOT(slotOpenFile(const QUrl&)));
  connect(objectTree, SIGNAL(optimizeFlight()), this, SLOT(slotOptimizeFlight()));
  connect(objectTree, SIGNAL(optimizeFlightOLC()), this, SLOT(slotOptimizeFlightOLC()));

  connect( evaluationWindow, SIGNAL(showCursor(const QPoint&, const QPoint&)),
           map, SLOT(slotDrawCursor(const QPoint&, const QPoint&)) );
  connect( evaluationWindow, SIGNAL(evaluationHelp(QString&)),
           helpWindow, SLOT(slotShowHelpText(QString&)) );

  connect(this, SIGNAL(flightDataTypeChanged(int)), _globalMapConfig, SLOT(slotSetFlightDataType(int)));

  slotModifyMenu();
}

MainWindow::~MainWindow()
{
}

MainWindow* MainWindow::instance()
{
  return _mainWindow;
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

QString MainWindow::getApplicationTaskDirectory()
{
  taskDir = _settings.value( "/Path/DefaultTaskDirectory",
                             getApplicationDataDirectory() ).toString();
  return taskDir;
}

QString MainWindow::getApplicationFlightDirectory()
{
  flightDir = _settings.value( "/Path/DefaultFlightDirectory",
                               getApplicationDataDirectory() ).toString();
  return flightDir;
}

void MainWindow::closeEvent( QCloseEvent *event )
{
  saveOptions();
  waypointTreeView->saveChanges();

  _globalMapMatrix->writeMatrixOptions();
  event->accept();
}

void MainWindow::createDockWindows()
{
  qDebug() << "MainWindow::initDockWindows()";

  // Reset animation of dock widget, it makes a lot of resizing.
  setAnimated( false );

  // First create the central widget. That is the Map.
  map = new Map(this);
  setCentralWidget(map);
  _globalMap = map;

  flightDataViewDock = new QDockWidget( tr("Flight Data"), this );
  flightDataViewDock->setObjectName( "FlightDataViewWindow");
  flightDataViewDock->setFloating( false );
  dataView = new DataView(flightDataViewDock);
  flightDataViewDock->setWidget(dataView);
  addDockWidget( Qt::RightDockWidgetArea, flightDataViewDock );
  flightDataViewDock->setVisible(false);

  evaluationWindowDock = new QDockWidget( tr("Flight Evaluation"), this );
  evaluationWindowDock->setObjectName( "EvaluationWindow");
  evaluationWindowDock->setVisible(false);
  evaluationWindowDock->setFloating( true );
  evaluationWindow = new EvaluationDialog(evaluationWindowDock);
  evaluationWindowDock->setWidget(evaluationWindow);
  addDockWidget( Qt::TopDockWidgetArea, evaluationWindowDock );

  helpWindowDock = new QDockWidget( tr("Help"), this );
  helpWindowDock->setObjectName( "HelpWindow");
  helpWindowDock->setVisible(false);
  helpWindowDock->setFloating( true );
  helpWindow = new HelpWindow(helpWindowDock);
  helpWindowDock->setWidget(helpWindow);
  addDockWidget( Qt::TopDockWidgetArea, helpWindowDock );

  legendDock = new QDockWidget( tr("Terrain Profile"), this );
  legendDock->setObjectName( "TerrainProfileWindow");
  legendDock->setFloating( false );
  legend = new TopoLegend(legendDock);
  legendDock->setWidget(legend);
  addDockWidget( Qt::RightDockWidgetArea, legendDock );
  legendDock->setVisible(false);

  mapControlDock = new QDockWidget( tr("Map Control"), this );
  mapControlDock->setObjectName( "MapControlWindow");
  mapControlDock->setFloating( false );
  mapControl = new MapControlView(mapControlDock);
  mapControlDock->setWidget(mapControl);
  addDockWidget( Qt::RightDockWidgetArea, mapControlDock );
  mapControlDock->setVisible(false);

  objectTreeDock = new QDockWidget( tr("Loaded Objects"), this );
  objectTreeDock->setObjectName( "ObjectTreeWindow");
  objectTreeDock->setFloating( false );
  objectTree = new ObjectTree(objectTreeDock);
  objectTreeDock->setWidget(objectTree);
  addDockWidget( Qt::LeftDockWidgetArea, objectTreeDock );
  objectTreeDock->setVisible(false);

  waypointsDock = new QDockWidget( tr("Waypoints"), this );
  waypointsDock->setObjectName( "WaypointsWindow");
  waypointsDock->setFloating( false );
  waypointTreeView = new WaypointTreeView(waypointsDock);
  waypointsDock->setWidget(waypointTreeView);
  addDockWidget( Qt::BottomDockWidgetArea, waypointsDock );
  waypointsDock->setVisible(false);
}

void MainWindow::createMenuBar()
{
  qDebug() << "MainWindow::initMenuBar()";

  //----------------------------------------------------------------------------
  // File menu actions
  //----------------------------------------------------------------------------
  fileNewWaypointAction = new QAction( getPixmap("waypoint_16.png"),
                                       tr("New &Waypoint"), this );
  fileNewWaypointAction->setEnabled(true);
  connect( fileNewWaypointAction, SIGNAL(triggered()),
           waypointTreeView, SLOT(slotNewWaypoint()) );

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
           _globalMapContents, SLOT(slotCloseFlight()) );

  fileSavePixmapAction = new QAction( getPixmap("kde_image_16.png"),
                                      tr("Save map to PNG..."), this );
  fileSavePixmapAction->setEnabled(true);
  connect( fileSavePixmapAction, SIGNAL(triggered()), this, SLOT(slotSaveMap2Image()) );

  filePrintAction = new QAction( getPixmap("kde_fileprint_16.png"),
                                 tr("Print Map"), this );
  filePrintAction->setShortcut( Qt::CTRL + Qt::Key_P );
  filePrintAction->setEnabled(true);
  connect( filePrintAction, SIGNAL(triggered()), this, SLOT(slotPrintMap()) );

  filePrintFlightAction = new QAction( getPixmap("kde_fileprint_16.png"),
                                       tr("Print Flight Data"), this );
  filePrintFlightAction->setEnabled(true);
  connect( filePrintFlightAction, SIGNAL(triggered()), this, SLOT(slotPrintFlight()) );

  filePrintTaskAction = new QAction( getPixmap("kde_fileprint_16.png"),
                                     tr("Print Task Data"), this );
  filePrintTaskAction->setEnabled(true);
  connect( filePrintTaskAction, SIGNAL(triggered()), this, SLOT(slotPrintFlight()) );

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

  // submenu recent files
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
    }

  connect( fileOpenRecentMenu, SIGNAL(triggered(QAction *)),
           this, SLOT(slotOpenRecentFile(QAction *)) );

  fileMenu->addAction( fileCloseAction );
  fileMenu->addSeparator();
  fileMenu->addAction( fileSavePixmapAction );
  fileMenu->addSeparator();
  fileMenu->addAction( filePrintAction );
  fileMenu->addAction( filePrintFlightAction );
  fileMenu->addAction( filePrintTaskAction );
  fileMenu->addSeparator();
  fileMenu->addAction( fileOpenRecorderAction );
  fileMenu->addSeparator();
  fileMenu->addAction( fileQuitAction );

  //----------------------------------------------------------------------------
  // View menu actions
  //----------------------------------------------------------------------------
  viewCenterTaskAction = new QAction( getPixmap("centertask_16.png"),
                                      tr("Center to &Task"), this );
  viewCenterTaskAction->setShortcut( Qt::Key_F6 );
  viewCenterTaskAction->setEnabled( true );
  connect( viewCenterTaskAction, SIGNAL(triggered()),
           map, SLOT(slotCenterToTask()) );

  viewCenterFlightAction = new QAction( getPixmap("centerflight_16.png"),
                                        tr("Center to &Flight"), this );
  viewCenterFlightAction->setShortcut( Qt::Key_F7 );
  viewCenterFlightAction->setEnabled( true );
  connect( viewCenterFlightAction, SIGNAL(triggered()),
           map, SLOT(slotCenterToFlight()));

  viewCenterHomesiteAction = new QAction( getPixmap("kde_gohome_16.png"),
                                          tr("Center to &Homesite"), this );
  viewCenterHomesiteAction->setShortcut( Qt::CTRL + Qt::Key_Home );
  viewCenterHomesiteAction->setEnabled( true );
  connect( viewCenterHomesiteAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotCenterToHome()) );

  viewCenterToAction = new QAction( getPixmap("centerto_16.png"),
                                    tr("&Center to..."), this );
  viewCenterToAction->setShortcut( Qt::Key_F8 );
  viewCenterToAction->setEnabled( true );
  connect( viewCenterToAction, SIGNAL(triggered()),
           this, SLOT(slotCenterTo()) );

  viewZoomInAction = new QAction( getPixmap("kde_viewmag+_16.png"),
                                  tr("Zoom &In"), this );
  viewZoomInAction->setShortcut( Qt::CTRL + Qt::Key_Plus );
  viewZoomInAction->setEnabled( true );
  connect( viewZoomInAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotZoomIn()) );

  viewZoomOutAction = new QAction( getPixmap("kde_viewmag-_16.png"),
                                   tr("Zoom &Out"), this );
  viewZoomOutAction->setShortcut( Qt::CTRL + Qt::Key_Minus );
  viewZoomOutAction->setEnabled( true );
  connect( viewZoomOutAction, SIGNAL(triggered()),
          _globalMapMatrix, SLOT(slotZoomOut()) );

  viewZoomAction = new QAction( getPixmap("kde_viewmagfit_16.png"),
                   tr("&Zoom into mouse rectangle"), this );
  viewZoomAction->setShortcut( Qt::Key_0 );
  viewZoomAction->setEnabled( true );
  connect( viewZoomAction, SIGNAL(triggered()), map, SLOT(slotZoomRect()) );

  viewRedrawAction = new QAction( getPixmap("kde_reload_16.png"),
                                 tr("&Redraw"), this );
  viewRedrawAction->setShortcut( Qt::Key_F5 );
  viewRedrawAction->setEnabled( true );
  connect( viewRedrawAction, SIGNAL(triggered()), map, SLOT(slotRedrawMap()) );

  viewMoveNWAction = new QAction( getPixmap("movemap_nw_22.png"),
                                  tr("Move map NW"), this );
  viewMoveNWAction->setShortcut( Qt::Key_7 );
  viewMoveNWAction->setEnabled( true );
  connect( viewMoveNWAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapNW()) );

  viewMoveNAction = new QAction( getPixmap("movemap_n_22.png"),
                                 tr("Move map N"), this );
  viewMoveNAction->setShortcut( Qt::Key_8 );
  viewMoveNAction->setEnabled( true );
  connect( viewMoveNAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapN()) );

  viewMoveNEAction = new QAction( getPixmap("movemap_ne_22.png"),
                                  tr("Move map NE"), this );
  viewMoveNEAction->setShortcut( Qt::Key_9 );
  viewMoveNEAction->setEnabled( true );
  connect( viewMoveNEAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapNE()) );

  viewMoveWAction = new QAction( getPixmap("movemap_w_22.png"),
                                 tr("Move map W"), this );
  viewMoveWAction->setShortcut( Qt::Key_4 );
  viewMoveWAction->setEnabled( true );
  connect( viewMoveWAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapW()) );

  viewMoveEAction = new QAction( getPixmap("movemap_e_22.png"),
                                tr("Move map W"), this );
  viewMoveEAction->setShortcut( Qt::Key_6 );
  viewMoveEAction->setEnabled( true );
  connect( viewMoveEAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapE()) );

  viewMoveSWAction = new QAction( getPixmap("movemap_sw_22.png"),
                                  tr("Move map SW"), this );
  viewMoveSWAction->setShortcut( Qt::Key_1 );
  viewMoveSWAction->setEnabled( true );
  connect( viewMoveSWAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapSW()) );

  viewMoveSAction = new QAction( getPixmap("movemap_s_22.png"),
                                 tr("Move map S"), this );
  viewMoveSAction->setShortcut( Qt::Key_2 );
  viewMoveSAction->setEnabled( true );
  connect( viewMoveSAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapS()) );

  viewMoveSEAction = new QAction( getPixmap("movemap_se_22.png"),
                                  tr("Move map SO"), this );
  viewMoveSEAction->setShortcut( Qt::Key_3 );
  viewMoveSEAction->setEnabled( true );
  connect( viewMoveSEAction, SIGNAL(triggered()),
           _globalMapMatrix, SLOT(slotMoveMapSE()) );

  viewMapDataUnderMouseCursor = new QAction( tr("Show map data touched by Mouse"),
                                             this );

  viewMapDataUnderMouseCursor->setEnabled( true );
  viewMapDataUnderMouseCursor->setCheckable( true );

  viewMapDataUnderMouseCursor->setChecked( _settings.value( "/MapData/ViewDataUnderMouseCursor", false ).toBool() );
  connect( viewMapDataUnderMouseCursor, SIGNAL(triggered(bool)),
           this, SLOT(slotViewMapDataUnderMouseCursor(bool)) );

  //----------------------------------------------------------------------------
  // View menu creation
  //----------------------------------------------------------------------------
  QMenu *vm = menuBar()->addMenu( tr("&View") );
  vm->addAction( viewCenterTaskAction );
  vm->addAction( viewCenterFlightAction );
  vm->addAction( viewCenterHomesiteAction );
  vm->addAction( viewCenterToAction );
  vm->addSeparator();
  vm->addAction( viewZoomInAction );
  vm->addAction( viewZoomOutAction );
  vm->addAction( viewZoomAction );
  vm->addAction( viewRedrawAction );
  vm->addSeparator();
  vm->addAction( viewMapDataUnderMouseCursor );
  vm->addSeparator();

  // Move map submenu
  QMenu *vmmMenu = vm->addMenu( getPixmap("kde_move_16.png"), tr("Move map") );
  vmmMenu->addAction( viewMoveNAction );
  vmmMenu->addAction( viewMoveNWAction );
  vmmMenu->addAction( viewMoveWAction );
  vmmMenu->addAction( viewMoveSWAction );
  vmmMenu->addAction( viewMoveSAction );
  vmmMenu->addAction( viewMoveSEAction );
  vmmMenu->addAction( viewMoveEAction );
  vmmMenu->addAction( viewMoveNEAction );

  //----------------------------------------------------------------------------
  // Flight menu actions
  //----------------------------------------------------------------------------
  flightEvaluationWindowAction = new QAction( getPixmap("kde_history_16.png"),
                                              tr("Show &Evaluation Window"), this );
  flightEvaluationWindowAction->setShortcut( Qt::CTRL + Qt::Key_E );
  flightEvaluationWindowAction->setEnabled(true);
  connect( flightEvaluationWindowAction, SIGNAL(triggered()),
           evaluationWindowDock, SLOT(show()) );

  connect( evaluationWindowDock, SIGNAL(visibilityChanged(bool)),
           flightEvaluationWindowAction, SLOT(setDisabled(bool)) );

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

  flightAnimatePauseAction = new QAction( getPixmap("kde_player_pause_16.png"),
                                    tr("&Pause Flight Animation"), this );
  flightAnimatePauseAction->setShortcut( Qt::Key_F11 );
  flightAnimatePauseAction->setEnabled(true);
  connect( flightAnimatePauseAction, SIGNAL(triggered()),
           map, SLOT(slotAnimateFlightPause()));

  flightAnimateStopAction = new QAction( getPixmap("kde_player_stop_16.png"),
                                   tr("Stop Flight &Animation"), this );
  flightAnimateStopAction->setShortcut( Qt::Key_F10 );
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
  fm->addAction( flightAnimatePauseAction );
  fm->addAction( flightAnimateStopAction );
  fm->addSeparator();
  fm->addAction( flightAnimateHomeAction );
  fm->addAction( flightAnimateNextAction );
  fm->addAction( flightAnimatePrevAction );
  fm->addAction( flightAnimate10NextAction );
  fm->addAction( flightAnimate10PrevAction );
  fm->addAction( flightAnimateEndAction );

  //----------------------------------------------------------------------------
  // Window menu creation
  //----------------------------------------------------------------------------
  windowMenu = menuBar()->addMenu( tr("&Window") );
  connect( windowMenu, SIGNAL(aboutToShow()),
           this, SLOT(slotWindowsMenuAboutToShow()) );
  connect( windowMenu, SIGNAL(triggered(QAction *)),
           _globalMapContents, SLOT(slotSetFlight(QAction *)) );

  windowMenu->setEnabled(false);

  //----------------------------------------------------------------------------
  // Settings menu creation
  //----------------------------------------------------------------------------
  settingsFlightDataAction = new QAction( getPixmap("kde_view_detailed_16.png"),
                                          tr("Show Flight &Data"), this );
  settingsFlightDataAction->setShortcut( Qt::CTRL + Qt::Key_D );
  settingsFlightDataAction->setEnabled( true );
  connect( settingsFlightDataAction, SIGNAL(triggered()),
           flightDataViewDock, SLOT(show()) );
  connect( flightDataViewDock, SIGNAL(visibilityChanged(bool)),
           settingsFlightDataAction, SLOT(setDisabled(bool)) );

  settingsHelpWindowAction = new QAction( getPixmap("kde_info_16.png"),
                                          tr("Show Help Window"), this );
  settingsHelpWindowAction->setShortcut( Qt::CTRL + Qt::Key_H );
  settingsHelpWindowAction->setEnabled( true );
  connect( settingsHelpWindowAction, SIGNAL(triggered()),
           helpWindowDock, SLOT(show()) );
  connect( helpWindowDock, SIGNAL(visibilityChanged(bool)),
           settingsHelpWindowAction, SLOT(setDisabled(bool)) );

  settingsObjectTreeAction = new QAction( getPixmap("kde_view_tree_16.png"),
                                          tr("Show KFLog &Browser"), this );
  settingsObjectTreeAction->setShortcut( Qt::CTRL + Qt::Key_B );
  settingsObjectTreeAction->setEnabled( true );
  connect( settingsObjectTreeAction, SIGNAL(triggered()),
           objectTreeDock, SLOT(show()) );
  connect( objectTreeDock, SIGNAL(visibilityChanged(bool)),
           settingsObjectTreeAction, SLOT(setDisabled(bool)) );

  settingsLegendAction = new QAction( getPixmap("kde_blend_16.png"),
                                      tr("Show Terrain Profile"), this );
  settingsLegendAction->setShortcut( Qt::CTRL + Qt::Key_L );
  settingsLegendAction->setEnabled( true );
  connect( settingsLegendAction, SIGNAL(triggered()),
           legendDock, SLOT(show()) );
  connect( legendDock, SIGNAL(visibilityChanged(bool)),
           settingsLegendAction, SLOT(setDisabled(bool)) );


#if 0
  settingsMapAction = new QAction(tr("Show &Map"), this );
  settingsMapAction->setCheckable( true );
  connect(settingsMapAction, SIGNAL(triggered()), this, SLOT(slotToggleMap()) );
#endif

  settingsMapControlAction = new QAction( getPixmap("kde_move_16.png"),
                                          tr("Show Map& Control"), this );
  connect( settingsMapControlAction, SIGNAL(triggered()),
           mapControlDock, SLOT(show()) );
  connect( mapControlDock, SIGNAL(visibilityChanged(bool)),
           settingsMapControlAction, SLOT(setDisabled(bool)) );

  settingsToolBarAction = new QAction( tr("Show Toolbar"), this );
  settingsToolBarAction->setCheckable( true );
  settingsToolBarAction->setChecked( true );
  connect( settingsToolBarAction, SIGNAL(triggered(bool)),
           toolBar, SLOT(setVisible(bool)) );
  connect( toolBar, SIGNAL(visibilityChanged(bool)),
           settingsToolBarAction, SLOT( setChecked(bool)) );

  settingsStatusBarAction = new QAction( tr("Show Statusbar"), this );
  settingsStatusBarAction->setCheckable( true );
  settingsStatusBarAction->setChecked( true );
  connect( settingsStatusBarAction, SIGNAL(triggered(bool)),
           statusBar(), SLOT(setVisible(bool)) );

  settingsWaypointsAction = new QAction( getPixmap("waypoint_16.png"),
                                         tr("Show &Waypoints"), this );
  // We can't use CTRL-W, because this shortcut is reserved for closing a file ...
  settingsWaypointsAction->setShortcut( Qt::CTRL + Qt::Key_V );
  settingsWaypointsAction->setEnabled( true );
  connect( settingsWaypointsAction, SIGNAL(triggered()),
           waypointsDock, SLOT(show()) );
  connect( waypointsDock, SIGNAL(visibilityChanged(bool)),
           settingsWaypointsAction, SLOT(setDisabled(bool)) );

  QMenu * sm = menuBar()->addMenu( tr("&Settings") );
  sm->addAction( flightEvaluationWindowAction );
  sm->addAction( settingsFlightDataAction );
  sm->addAction( settingsHelpWindowAction );
  sm->addAction( settingsObjectTreeAction );
  sm->addAction( settingsLegendAction );
  // sm->addAction( settingsMapAction );
  sm->addAction( settingsMapControlAction );
  sm->addAction( settingsToolBarAction );
  sm->addAction( settingsStatusBarAction );
  sm->addAction( settingsWaypointsAction );
  sm->addSeparator();
  sm->addAction( getPixmap("kde_configure_16.png"),
                 tr("KFLog &Setup..."),
                 this,
                 SLOT(slotConfigureKFLog()) );

  //  settings->insertItem(getPixmap("kde_configure_shortcuts_16.png"), tr("Configure Shortcuts...") );//, this, SLOT(slotConfigureKeyBindings()));
  //  settings->insertItem(getPixmap("kde_configure_toolbars_16.png"), tr("Configure Toolbars...") );//, this, SLOT(slotConfigureToolbars()));

  //----------------------------------------------------------------------------
  // Help menu creation
  //----------------------------------------------------------------------------
  QMenu *help = menuBar()->addMenu( tr("&Help") );

  help->addAction( getPixmap("kde_contexthelp_16.png"),
                             tr("What's This?"),
                             this,
                             SLOT(slotWhatsThis()), Qt::CTRL + Qt::Key_F1 );

  help->addAction( getPixmap("qt_logo_32x32.png"), tr("About &Qt"),
                   qApp, SLOT(aboutQt()), Qt::Key_Q );

  help->addAction( getPixmap("kflog_16.png"), tr("About KFLog"),
                   this, SLOT(slotShowAbout()) );


  //FIXME: link to manual must be added
  //FIXME: dialog to switch application language must be added
  //help->insertItem(getPixmap("kde_idea_16.png"), tr("Tip of the day") );//, this, SLOT(slotTipOfDay()));
}

void MainWindow::createStatusBar()
{
  statusProgress = new QProgressBar( statusBar() );
  statusProgress->setFixedWidth(120);
  statusProgress->setFixedHeight( statusProgress->sizeHint().height() - 4 );

  statusLabel = new QLabel( statusBar() );
  statusLabel->setFixedHeight( statusLabel->sizeHint().height() );
  statusLabel->setFrameStyle( QFrame::NoFrame |QFrame::Plain );
  statusLabel->setMargin(0);
  statusLabel->setLineWidth(0);
  statusLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusTimeL = new QLabel(statusBar());
  statusTimeL->setFixedWidth( 80 );
  statusTimeL->setFixedHeight( statusLabel->sizeHint().height() );
  statusTimeL->setFrameStyle(QFrame::NoFrame |QFrame::Plain );
  statusTimeL->setMargin(0);
  statusTimeL->setLineWidth(0);
  statusTimeL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusAltitudeL = new QLabel(statusBar());
  statusAltitudeL->setFixedWidth( 80 );
  statusAltitudeL->setFixedHeight( statusLabel->sizeHint().height() );
  statusAltitudeL->setFrameStyle(QFrame::NoFrame |QFrame::Plain );
  statusAltitudeL->setMargin(0);
  statusAltitudeL->setLineWidth(0);
  statusAltitudeL->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  statusVarioL = new QLabel(statusBar());
  statusVarioL->setFixedWidth( 80 );
  statusVarioL->setFixedHeight( statusLabel->sizeHint().height() );
  statusVarioL->setFrameStyle(QFrame::NoFrame |QFrame::Plain );
  statusVarioL->setMargin(0);
  statusVarioL->setLineWidth(0);
  statusVarioL->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  statusSpeedL = new QLabel(statusBar());
  statusSpeedL->setFixedWidth( 100 );
  statusSpeedL->setFixedHeight( statusLabel->sizeHint().height() );
  statusSpeedL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusSpeedL->setMargin(0);
  statusSpeedL->setLineWidth(0);
  statusSpeedL->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

  statusLatL = new QLabel(statusBar());
  statusLatL->setFixedHeight( statusLabel->sizeHint().height() );
  statusLatL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusLatL->setMargin(0);
  statusLatL->setLineWidth(0);
  statusLatL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusLonL = new QLabel(statusBar());
  statusLonL->setFixedHeight( statusLabel->sizeHint().height() );
  statusLonL->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statusLonL->setMargin(0);
  statusLonL->setLineWidth(0);
  statusLonL->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  statusBar()->addWidget( statusLabel, 1 );
  statusBar()->addWidget( statusTimeL, 0 );
  statusBar()->addWidget( statusAltitudeL, 0 );
  statusBar()->addWidget( statusSpeedL, 0 );
  statusBar()->addWidget( statusVarioL, 0 );
  statusBar()->addWidget( statusProgress, 0 );
  statusBar()->addWidget( statusLatL, 0 );
  statusBar()->addWidget( statusLonL, 0 );
}

void MainWindow::createToolBar()
{
  toolBar->addAction( fileOpenFlightAction );
  toolBar->addSeparator();
  toolBar->addAction( viewZoomAction );
  toolBar->addAction( viewZoomInAction );
  toolBar->addAction( viewZoomOutAction );
  toolBar->addAction( viewRedrawAction );
  toolBar->addSeparator();
  toolBar->addAction( viewCenterTaskAction );
  toolBar->addAction( viewCenterFlightAction );
  toolBar->addAction( viewCenterHomesiteAction );
  toolBar->addSeparator();
  toolBar->addAction( flightEvaluationWindowAction );
}

void MainWindow::readOptions()
{
  qDebug() << "reading options...";

  bool visible = _settings.value( "/MainWindow/ShowToolbar", true ).toBool();
  toolBar->setVisible( visible );
  settingsToolBarAction->setChecked( visible );

  visible = _settings.value( "/MainWindow/ShowStatusbar", true ).toBool();
  statusBar()->setVisible( visible );
  settingsStatusBarAction->setChecked( visible );

  restoreState( _settings.value( "/MainWindow/State" ).toByteArray() );

  setMinimumSize( QSize(900, 500) );
  bool ok = restoreGeometry( _settings.value( "/MainWindow/Geometry" ).toByteArray() );

  if( ! ok )
    {
      // use default window size
      QSize size( _settings.value( "/MainWindow/DefaultWidth",  900 ).toInt(),
                  _settings.value( "/MainWindow/DefaultHeight", 500 ).toInt() );

      setMinimumSize( size );

      _settings.setValue( "/MainWindow/DefaultWidth",  900 );
      _settings.setValue( "/MainWindow/DefaultHeight", 500 );
    }

  // initialize the recent file list
  flightDir = _settings.value( "/Path/DefaultFlightDirectory",
                               getApplicationDataDirectory() ).toString();

  taskDir   = _settings.value( "/Path/DefaultTaskDirectory",
                               getApplicationDataDirectory() ).toString();

  mapControl->slotSetMinMaxValue( _settings.value( "/Scale/LowerLimit", BORDER_L ).toInt(),
                                  _settings.value( "/Scale/UpperLimit", BORDER_U ).toInt() );
}

void MainWindow::saveOptions()
{
  qDebug() << "saving options...";

  _settings.setValue( "/MainWindow/Geometry", saveGeometry() );
  _settings.setValue( "/MainWindow/State", saveState() );
  _settings.setValue( "/MainWindow/ShowToolbar", toolBar->isVisible() );
  _settings.setValue( "/MainWindow/ShowStatusbar", statusBar()->isVisible() );

  if( _settings.value("/Waypoints/DefaultWaypointCatalog", KFLogConfig::LastUsed).toInt() ==
      KFLogConfig::LastUsed && waypointTreeView->getCurrentCatalog() != static_cast<WaypointCatalog *> (0) )
    {
      // Only write the path, if a waypoint-catalog is opened.
      _settings.setValue( "/Waypoints/DefaultCatalogName",
                          waypointTreeView->getCurrentCatalog()->path );
    }
  else
    {
      // Reset the path, if no catalog is opened. Otherwise the next startup
      // tries to open the last used catalog.
      _settings.setValue( "/Waypoints/DefaultCatalogName", "" );
    }
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
  connect( igc3d, SIGNAL(igc3dHelp(QString&)),
           helpWindow, SLOT(slotShowHelpText(QString&)) );

  igc3d->setVisible( true );
}

void MainWindow::slotFlightViewIgc3DClosed()
{
  flightIgc3DAction->setEnabled(true);
}

void MainWindow::slotFlightViewIgcOpenGL()
{
  // Note, that this function does not only delivers Flight objects!!!
  Flight *flight = dynamic_cast<Flight *> (_globalMapContents->getFlight());

  if( flight == static_cast<Flight *> (0) )
    {
      return;
    }

  QString root = _settings.value( "/Path/InstallRoot", ".." ).toString();

  QString libPath = root + "/lib/libopengl_igc.so";

  void* libHandle = dlopen( libPath.toAscii().data(), RTLD_NOW );

  char *error = (char *) dlerror();

  if( libHandle == 0 )
    {
      QString errMsg;

      if( error != 0 )
        {
          qWarning() << "MainWindow::slotFlightViewIgcOpenGL() Error:" << error;

          errMsg = QString(error);
        }

      QMessageBox::critical( this,
                             tr("Plugin is missing!"),
                             tr("Cannot open plugin library:") +
                             "\n\n" + libPath + "\n\n" + errMsg,
                             QMessageBox::Ok );

      return;
    }


  QWidget* (*getMainWidget)(QWidget *);

  getMainWidget = (QWidget* (*) (QWidget *)) dlsym(libHandle, "getMainWidget");

  if( !getMainWidget( this ) )
    {
      qWarning( "getMainWidget function not defined in library!" );
      return;
    }

  void (*addFlight)(Flight*);

  addFlight = (void (*) (Flight*)) dlsym(libHandle, "addFlight");

  if( !addFlight )
    {
      qWarning( "addFlight function not defined in library!" );
      return;
    }

  (void)(*addFlight)( flight );

  // FIXME: Memory leak from libHandle is to fix here!
}

/** set menu items enabled/disabled */
void MainWindow::slotModifyMenu()
{
  if( _globalMapContents->getFlightList()->count() > 0 &&
      _globalMapContents->getFlight() != 0 )
    {
      // Note AP: The flight list number is incremented when a new task is
      // created but the current flight maybe NULL!!! I got a core dump here.
      // Added check above to if clause.
      windowMenu->setEnabled(true);

      switch(_globalMapContents->getFlight()->getObjectType())
        {
          case BaseMapElement::Flight:
            fileCloseAction->setEnabled(true);
            filePrintFlightAction->setEnabled(true);
            filePrintTaskAction->setEnabled(false);
            viewCenterTaskAction->setEnabled(true);
            viewCenterFlightAction->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimizationAction->setEnabled(true);
            flightOptimizationOLCAction->setEnabled(true);
            flightIgc3DAction->setEnabled(true);
            flightIgcOpenGLAction->setEnabled(true);
            flightAnimateStartAction->setEnabled(true);
            flightAnimatePauseAction->setEnabled(true);
            flightAnimateStopAction->setEnabled(true);
            flightAnimateNextAction->setEnabled(true);
            flightAnimatePrevAction->setEnabled(true);
            flightAnimate10NextAction->setEnabled(true);
            flightAnimate10PrevAction->setEnabled(true);
            flightAnimateHomeAction->setEnabled(true);
            flightAnimateEndAction->setEnabled(true);
            break;

          case BaseMapElement::Task:
            fileCloseAction->setEnabled(true);
            filePrintFlightAction->setEnabled(false);
            filePrintTaskAction->setEnabled(true);
            viewCenterTaskAction->setEnabled(true);
            viewCenterFlightAction->setEnabled(false);
//            flightEvaluation->setEnabled(false);
            flightOptimizationAction->setEnabled(false);
            flightOptimizationOLCAction->setEnabled(false);
            flightIgc3DAction->setEnabled(false);
            flightIgcOpenGLAction->setEnabled(false);
            flightAnimateStartAction->setEnabled(false);
            flightAnimatePauseAction->setEnabled(true);
            flightAnimateStopAction->setEnabled(false);
            flightAnimateNextAction->setEnabled(false);
            flightAnimatePrevAction->setEnabled(false);
            flightAnimate10NextAction->setEnabled(false);
            flightAnimate10PrevAction->setEnabled(false);
            flightAnimateHomeAction->setEnabled(false);
            flightAnimateEndAction->setEnabled(false);
            break;

          case BaseMapElement::FlightGroup:
            fileCloseAction->setEnabled(true);
            filePrintFlightAction->setEnabled(true);
            filePrintTaskAction->setEnabled(false);
            viewCenterTaskAction->setEnabled(true);
            viewCenterFlightAction->setEnabled(true);
//            flightEvaluation->setEnabled(true);
            flightOptimizationAction->setEnabled(true);
            flightOptimizationOLCAction->setEnabled(true);
            flightIgc3DAction->setEnabled(true);
            flightIgcOpenGLAction->setEnabled(true);
            flightAnimateStartAction->setEnabled(true);
            flightAnimatePauseAction->setEnabled(true);
            flightAnimateStopAction->setEnabled(true);
            flightAnimateNextAction->setEnabled(true);
            flightAnimatePrevAction->setEnabled(true);
            flightAnimate10NextAction->setEnabled(true);
            flightAnimate10PrevAction->setEnabled(true);
            flightAnimateHomeAction->setEnabled(true);
            flightAnimateEndAction->setEnabled(true);
            break;

          default:
            break;
        }
    }
  else
    {
      fileCloseAction->setEnabled( false );
      filePrintFlightAction->setEnabled( false );
      filePrintTaskAction->setEnabled( false );

      viewCenterTaskAction->setEnabled( false );
      viewCenterFlightAction->setEnabled( false );
      // flightEvaluation->setEnabled(false);
      flightOptimizationAction->setEnabled( false );
      flightOptimizationOLCAction->setEnabled( false );
      flightIgc3DAction->setEnabled( false );
      flightIgcOpenGLAction->setEnabled( false );
      flightAnimateStartAction->setEnabled( false );
      flightAnimatePauseAction->setEnabled( false );
      flightAnimateStopAction->setEnabled( false );
      flightAnimateNextAction->setEnabled( false );
      flightAnimatePrevAction->setEnabled( false );
      flightAnimate10NextAction->setEnabled( false );
      flightAnimate10PrevAction->setEnabled( false );
      flightAnimateHomeAction->setEnabled( false );
      flightAnimateEndAction->setEnabled( false );
      windowMenu->setEnabled(false);
    }
}

void MainWindow::slotOpenFile()
{
  /*
   * Please note remark from Qt: The QFileDialog class in Qt 4 has been totally
   * rewritten. It provides most of the functionality of the old QFileDialog
   * class, but with a different API. Some functionality, such as the ability
   * to preview files, is expected to be added in a later Qt 4 release.
   *
   * Therefore we fall back to the provided features of the Qt4 FileDialog
   * without a preview.
   */
  slotSetStatusMsg(tr("Opening file..."));

  QFileDialog* fd = new QFileDialog( this );
  fd->setWindowTitle( tr( "Open Flight" ) );
  fd->setDirectory( getApplicationFlightDirectory() );
  fd->setFileMode( QFileDialog::ExistingFile );

  QStringList filter;
  filter.append(tr("All types") + " (*.igc *.flightgear *.trk *.gdn)");
  filter.append(tr("IGC") + " (*.igc)");
  filter.append(tr("Garmin") + " (*.trk *.gdn)");
  fd->setFilters( filter );

  // We need this to sort the file names alphabetically
  QSortFilterProxyModel *sorter = new QSortFilterProxyModel();
  sorter->setDynamicSortFilter(true);
  fd->setProxyModel(sorter);

  if( fd->exec() == QDialog::Accepted )
    {
      QStringList fNames = fd->selectedFiles();

      if( fNames.size() == 0 )
        {
          return;
        }

      QFile file( fNames[0] );

      flightDir = fd->directory().canonicalPath();

      FlightLoader flightLoader;

      if( flightLoader.openFlight(file) )
        {
          slotSetCurrentFile( fNames[0] );
        }
    }

  slotSetStatusMsg( tr( "Ready." ) );
}

void MainWindow::slotOpenFile( const QUrl& url )
{
  slotSetStatusMsg(tr("Opening file..."));

  if( url.scheme() == "file" || url.isRelative() )
    {
      QFile file( url.path() );

      if( url.path().right( 9 ).toLower() == ".kflogtsk" )
        {
          // this is probably a taskfile. Try to open it as a task
          if( _globalMapContents->loadTask( file ) )
            {
              slotSetCurrentFile( url.path() );
            }
        }
      else
        {
          // try to open as flight
          FlightLoader flightLoader;

          if( flightLoader.openFlight( file ) )
            {
              slotSetCurrentFile( url.path() );
            }
        }
    }

  slotSetStatusMsg( tr( "Ready." ) );
}

/**
 * Opens a task-file-open-dialog.
 */
void MainWindow::slotOpenTask()
{
  slotSetStatusMsg(tr("Opening file..."));

  QFileDialog* fd = new QFileDialog( this );
  fd->setWindowTitle( tr( "Open Task" ) );
  fd->setDirectory( getApplicationTaskDirectory() );
  fd->setFileMode( QFileDialog::ExistingFile );

  QStringList filters;
  filters.append( tr( "KFLog tasks" ) + "(*.kflogtsk *.KFLOGTSK)" );
  fd->setFilters( filters );

  // We need this to sort the file names alphabetically
  QSortFilterProxyModel *sorter = new QSortFilterProxyModel();
  sorter->setDynamicSortFilter(true);
  fd->setProxyModel(sorter);

  if( fd->exec() == QDialog::Accepted )
    {
      QStringList fNames = fd->selectedFiles();

      if( fNames.size() == 0 )
        {
          return;
        }

      QString fName = fNames[0];

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

  slotSetStatusMsg( tr( "Opening File..." ) );

  FlightLoader flightLoader;

  QString fileName = action->toolTip();

  QFile file( fileName );

  if( fileName.endsWith(".kflogtsk", Qt::CaseInsensitive) )
    {
      // this is probably a task file. Try to open it as a task
      if( _globalMapContents->loadTask( file ) )
        {
          slotSetCurrentFile( fileName );
        }
    }
  else
    {
      //try to open as flight
      if( flightLoader.openFlight( file ) )
        {
          slotSetCurrentFile( fileName );
        }
    }

  slotSetStatusMsg(tr("Ready."));
}

void MainWindow::slotOpenRecorderDialog()
{
  RecorderDialog* dlg = new RecorderDialog(this);

  connect( dlg, SIGNAL(addCatalog(WaypointCatalog *)),
           waypointTreeView, SLOT(slotAddCatalog(WaypointCatalog *)) );

  connect( dlg, SIGNAL(addTask(FlightTask *)),
           _globalMapContents, SLOT(slotAppendTask(FlightTask *)) );

  dlg->exec();
}

void MainWindow::slotOptimizeFlight()
{
  Flight *flight = dynamic_cast<Flight *> (_globalMapContents->getFlight());

  if( flight != 0 && flight->getObjectType() == BaseMapElement::Flight )
    {
      if( flight->optimizeTask() )
        {
          // Okay, update flight data and redraw the map
          dataView->slotSetFlightData();
          evaluationWindow->slotShowFlightData();
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
          dataView->slotSetFlightData();
          evaluationWindow->slotShowFlightData();
          map->slotRedrawFlight();
          objectTree->slotFlightChanged();
        }
    }
}

/** Connects the dialogs addWaypoint signal to the waypoint object. */
void MainWindow::slotRegisterWaypointDialog(QWidget * dialog)
{
  connect( dialog, SIGNAL(addWaypoint(Waypoint *)), waypointTreeView,
           SLOT(slotAddWaypoint(Waypoint *)));
}

void MainWindow::slotSaveMap2Image()
{
  slotSavePixmap( QUrl() );
}

void MainWindow::slotSavePixmap(QUrl url, int width, int height)
{
  QString fileName;

  if( url.isEmpty() || ! url.isValid() )
    {
      fileName = QFileDialog::getSaveFileName( this,
                                               tr("Save map as image"),
                                               getApplicationDataDirectory() + "/kflog_map.png",
                                               tr("Image (*.png)") );
    }
  if( fileName.isEmpty() )
    {
      return;
    }

  url.setPath( fileName );
  url.setScheme( "file" );

  map->slotSavePixmap(url, width, height);
}

void MainWindow::slotFlightDataTypeGroupAction( QAction *action )
{
  // Get index from action
  int index = action->data().toInt();

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
    }
}

void MainWindow::slotSetPointInfo(const QPoint& pos, const FlightPoint& point)
{
  statusBar()->clearMessage();
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
  statusBar()->clearMessage();
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
  waypointTreeView->slotSetWaypointCatalogName(catalog);
}

void MainWindow::slotCheckDockWidgetStatus()
{
  flightEvaluationWindowAction->setDisabled(evaluationWindowDock->isVisible());
  settingsFlightDataAction->setDisabled(flightDataViewDock->isVisible());
  settingsHelpWindowAction->setDisabled(helpWindowDock->isVisible());
  settingsObjectTreeAction->setDisabled(objectTreeDock->isVisible());
  settingsLegendAction->setDisabled(legendDock->isVisible());
//  settingsMapAction->setDisabled(mapViewDock->isVisible());
  settingsMapControlAction->setDisabled(mapControlDock->isVisible());
  settingsStatusBarAction->setDisabled(statusBar()->isVisible());
  settingsToolBarAction->setDisabled(toolBar->isVisible());
  settingsWaypointsAction->setDisabled(waypointsDock->isVisible());
}

void MainWindow::slotConfigureKFLog()
{
  KFLogConfig* confDlg = new KFLogConfig( this );

  connect( confDlg, SIGNAL(scaleChanged(int, int)),
           mapControl, SLOT(slotSetMinMaxValue(int, int)) );

  connect(confDlg, SIGNAL(configOk()), _globalMapConfig, SLOT(slotReadConfig()));

  connect(confDlg, SIGNAL(configOk()), _globalMapContents, SLOT(reProject()));

  connect(confDlg, SIGNAL(configOk()), map, SLOT(slotRedrawMap()));

  connect(confDlg, SIGNAL(configOk()), waypointTreeView, SLOT(slotFillWaypoints()));

  connect(confDlg, SIGNAL(newDrawType(int)), this, SLOT(slotSelectFlightData(int)));

  connect( confDlg, SIGNAL(downloadWelt2000()),
           _globalMapContents, SLOT(slotDownloadWelt2000()) );

  connect( confDlg, SIGNAL(reloadWelt2000Data()),
           _globalMapContents, SLOT(slotReloadWelt2000Data()) );

  connect( confDlg, SIGNAL(airspaceFileListChanged()),
          _globalMapContents, SLOT(slotReloadAirspaceData()) );

  confDlg->setVisible( true );
}

void MainWindow::slotPrintMap()
{
  slotSetStatusMsg( tr( "Printing map ..." ) );

  QPrinter printer( QPrinter::HighResolution );

  printer.setDocName( "kflog-map" );
  printer.setCreator( QString( "KFLog " ) + KFLOG_VERSION );
  printer.setOutputFileName( getApplicationDataDirectory() + "/kflog-map.pdf" );

  QPrintDialog dialog( &printer, this );

  dialog.setWindowTitle( tr("Print Map") );
  dialog.setSizeGripEnabled ( true );
  dialog.setOptions( QAbstractPrintDialog::PrintToFile |
                     QAbstractPrintDialog::PrintSelection |
                     QAbstractPrintDialog::PrintPageRange |
                     QAbstractPrintDialog::PrintShowPageSize );

  if( dialog.exec() != QDialog::Accepted )
    {
      slotSetStatusMsg( tr( "" ) );
      return;
    }

  QPainter painter( &printer );

  // We print the current content of the map into a file.
  QWidget *map = _globalMap;

  double xscale = printer.pageRect().width() / double( map->width() );
  double yscale = printer.pageRect().height() / double( map->height() );
  double scale = qMin( xscale, yscale );

  painter.translate( printer.paperRect().x() + printer.pageRect().width() / 2,
                     printer.paperRect().y() + printer.pageRect().height() / 2 );

  painter.scale( scale, scale );

  painter.translate( -map->width() / 2, -map->height() / 2 );

  map->render( &painter );

  slotSetStatusMsg( tr( "Ready." ) );
}

void MainWindow::slotPrintFlight()
{
  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      switch (f->getObjectType())
        {
          case BaseMapElement::Flight:
            slotSetStatusMsg(tr("Printing flight ..."));
            FlightDataPrint((Flight *)f);
            break;
          case BaseMapElement::Task:
            slotSetStatusMsg(tr("Printing task ..."));
            TaskDataPrint((FlightTask*)f);
            break;
          default:
            QString tmp(tr("Not yet available for type: %1").arg(f->getObjectType()));
            QMessageBox::warning(0, tr("Type not available"), tmp, QMessageBox::Ok);
        }
    }

  slotSetStatusMsg(tr("Ready."));
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

  for( int i = 0; i < flights->size(); i++ )
    {
      QAction *action = new QAction( this );
      action->setText( flights->at(i)->getFileName() );
      action->setEnabled( true );
      action->setData( i );
      action->setChecked( _globalMapContents->getFlightIndex() == i );
      windowMenu->addAction( action );
    }
}

/**
 * Toggles the view map data action on/off.
 */
void MainWindow::slotViewMapDataUnderMouseCursor( bool checked )
{
  viewMapDataUnderMouseCursor->setChecked( checked );
  _settings.setValue( "/MapData/ViewDataUnderMouseCursor", checked );
}

void MainWindow::slotShowAbout()
{
  AboutWidget *aw = new AboutWidget( this );

  aw->setWindowIcon( QIcon(getPixmap("kflog_16.png")) );
  aw->setWindowTitle( tr( "About KFLog") );
  aw->setHeaderIcon( getPixmap("kflog_16.png") );

  QString header( tr("<html>KFLog %1, &copy; 2000-2011, The KFLog-Team</html>").arg( QCoreApplication::applicationVersion() ) );

  aw->setHeaderText( header );

  QString about( tr(
          "<hml>"
          "KFLog %1, compiled at %2 with QT %3<br><br>"
          "Homepage: <a href=\"http://www.kflog.org/kflog/\">www.kflog.org/kflog/</a><br><br>"
          "Software Repository: <a href=\"https://svn.kflog.org/svn/repos/kflog2/qt4\">https://svn.kflog.org/svn/repos/kflog2/qt4</a><br><br>"
          "Report bugs to: <a href=\"mailto:kflog-user&#64;kflog.org\">kflog.user&#64;kflog.org</a><br><br>"
          "Published under the <a href=\"http://www.gnu.org/licenses/licenses.html#GPL\">GPL</a>"
          "</html>" ).arg( QCoreApplication::applicationVersion() )
                     .arg( _settings.value( "/Main/CompileDate", "" ).toString() )
                     .arg( QT_VERSION_STR ) );

  aw->setAboutText( about );

  QString team( tr(
      "<hml>"
      "<b>Current Maintainer</b>"
      "<blockquote>"
      "Hendrik Hoeth &lt;<a href=\"mailto:hoeth&#64;linta.de\">hoeth&#64;linta.de</a>&gt;"
      "</blockquote>"
      "<b>Developers, Maintainers</b>"
      "<blockquote>"
      "Axel Pauli (Developer, Portage to Qt4)<br>"
      "Constantijn Neeteson (Maintenance, Core-developer)<br>"
      "Florian Ehinger (Maintenance, Core-developer, Mapdata)<br>"
      "Heiner Lamprecht (Maintenance, Core-developer)<br>"
      "Andr&eacute; Somers (Developer, Waypoint-handling, Plugin architecture, ...)<br>"
      "Christof Bodner (Developer, OLC Optimization)<br>"
      "Eggert Ehmke (Developer)<br>"
      "Harald Maier (Developer, Waypoint-Dialog, Task-handling)<br>"
      "Thomas Nielsen (Developer, 3D-Dialog)<br>"
      "Jan Kr&uuml;ger (Developer, 3D-Dialog)"
      "</blockquote>"
      "<b>Server Sponsor</b>"
      "<blockquote>"
      "Heiner Lamprecht &lt;<a href=\"mailto:heiner&#64;kflog.org\">heiner&#64;kflog.org</a>&gt;"
      "</blockquote>"
      "Thanks to all, who have made available this software!"
      "<br></html>" ));

  aw->setTeamText( team );
  aw->resize( 600, 500 );
  aw->setVisible( true );
}
