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
#include <igcpreview.h>
#include <kflogconfig.h>
#include <kflogstartlogo.h>
#include <map.h>
#include <mapcalc.h>
#include <mapconfig.h>
#include <mapcontents.h>
#include <mapcontrolview.h>
#include <printdialog.h>
#include <taskandwaypoint.h>

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
  config = kapp->config();

  config->setGroup("General Options");

  if (config->readBoolEntry("Logo",false) && (!kapp->isRestored() ) )
    {
      showStartLogo = true;
      startLogo = new KFLogStartLogo();
      startLogo->show();
    }

  extern MapConfig _globalMapConfig;
  extern MapMatrix _globalMapMatrix;

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

  activateDock();

  slotCheckDockWidgetStatus();
  // Heavy workaround! MapConfig should tell KFLogApp, which type is selected!
  slotSelectFlightData(0);

  connect(&_globalMapMatrix, SIGNAL(matrixChanged()), map,
      SLOT(slotRedrawMap()));
  connect(map, SIGNAL(showFlightPoint(const QPoint, const struct flightPoint&)),
      this, SLOT(slotShowPointInfo(const QPoint, const struct flightPoint&)));
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

  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());

  flightEvaluation = new KAction(i18n("Evaluation"), "flightevaluation",
      CTRL+Key_E, this, SLOT(slotEvaluateFlight()), actionCollection(),
      "evaluate_flight");

  taskAndWaypoint = new KAction(i18n("Task && Waypoints"), "waypoint",
      CTRL+Key_T, this, SLOT(slotTaskAndWaypoint()), actionCollection(),
      "task_and_waypoint");

//  new KAction(i18n("Optimize"), "flight_optimize", 0, map,
//      SLOT(slotOptimzeFlight()), actionCollection(), "optimize_flight");

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

  viewFlightDataType->setItems(dataList);
  // Heavy workaround! MapConfig should tell KFLogApp, which type is selected!
  viewFlightDataType->setCurrentItem(0);

  KActionMenu* flightMenu = new KActionMenu(i18n("F&light"),
      actionCollection(), "flight");
  flightMenu->insert(flightEvaluation);
  flightMenu->insert(taskAndWaypoint);
  flightMenu->insert(viewFlightDataType);

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

  /* Argumente für manualDock():
   * dock target, dock side, relation target/this (in percent)
   */
  dataViewDock->manualDock( mapViewDock, KDockWidget::DockRight, 71 );
  mapControlDock->manualDock( dataViewDock, KDockWidget::DockBottom, 75 );

  connect(map, SIGNAL(changed(QSize)), mapControl,
      SLOT(slotShowMapData(QSize)));

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

void KFLogApp::slotShowPointInfo(QPoint pos)
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

//  QPrinter printer;
//  if (printer.setup(this))
  PrintDialog pD(this, true);
  pD.openMapPrintDialog();

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

void KFLogApp::slotCheckDockWidgetStatus()
{
  viewMapControl->setChecked(mapControlDock->isVisible());
  viewMap->setChecked(mapViewDock->isVisible());
  viewData->setChecked(dataViewDock->isVisible());
}

void KFLogApp::slotToggleDataView()  { dataViewDock->changeHideShowState(); }

void KFLogApp::slotToggleMapControl() { mapControlDock->changeHideShowState(); }

void KFLogApp::slotToggleMap() { mapViewDock->changeHideShowState(); }

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

  eval->slotShowFlightData(0);  // <- hier wird angenommen, dass mindestens
                                // ein Element vorhanden ist ...
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

void KFLogApp::slotTaskAndWaypoint()
{
  TaskAndWaypoint d(this);
  d.exec();
}
