/***********************************************************************
**
**   kflog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <stdlib.h>

// include files for QT
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qprinter.h>
#include <qpainter.h>

// include files for KDE
#include <kedittoolbar.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>

// application specific includes
#include "kflog.h"
#include <dataview.h>
#include <kflogstartlogo.h>
#include <map.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapcontrolview.h>
#include <mapmatrix.h>

#define STATUS_LABEL(a,b,c) \
  a = new KStatusBarLabel( "", 0, statusBar() ); \
  a->setFixedWidth( b ); \
  a->setFixedHeight( statusLabel->sizeHint().height() ); \
  a->setFrameStyle( QFrame::NoFrame | QFrame::Plain ); \
  a->setMargin(0); \
  a->setLineWidth(0); \
  a->setAlignment( c | AlignVCenter );
//  a->setIndent(10);

KFLogApp::KFLogApp(QWidget* , const char* name)
  : KDockMainWindow(0, name), showStartLogo(false)
{
  config = kapp->config();

  config->setGroup("General Options");

  if (config->readBoolEntry("Logo",true) && (!kapp->isRestored() ) )
    {
      showStartLogo = true;
      startLogo = new KFLogStartLogo();
      startLogo->show();
    }

  initStatusBar();
  initView();
  initActions();
	
  if(showStartLogo)
      startLogo->raise();

  readOptions();

  filePrint->setEnabled(false);
  viewData->setChecked(true);
  viewMapControl->setChecked(true);
  viewCenterTask->setEnabled(false);
  viewCenterFlight->setEnabled(false);
}

KFLogApp::~KFLogApp()
{

}

void KFLogApp::initActions()
{
  fileOpen = new KAction(i18n("&Open Flight"), SmallIcon("fileopen"),
      KStdAccel::key(KStdAccel::Open), this, SLOT(slotFileOpen()),
      actionCollection(), "file_open");
  fileOpenRecent = KStdAction::openRecent(this,
      SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileClose = new KAction(i18n("Close Flight"), SmallIcon("fileclose"),
      KStdAccel::key(KStdAccel::Close), this, SLOT(slotFileClose()),
      actionCollection(), "file_close");
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()),
      actionCollection());
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

  viewRedraw = KStdAction::redisplay(map, SLOT(slotRedrawMap()),
      actionCollection());
  viewCenterTask = new KAction(i18n("Center to &Task"), 0, 0, map,
      SLOT(slotCenterToTask()), actionCollection(), "view_center_task");
  viewCenterFlight = new KAction(i18n("Center to &Flight"), 0, 0, map,
      SLOT(slotCenterToFlight()), actionCollection(), "view_center_flight");
  viewCenterHome = new KAction(i18n("Center to &Homesite"),
      SmallIcon("gohome"), 0, map, SLOT(slotCenterToHome()),
      actionCollection(), "view_center_home");
  viewZoomIn = KStdAction::zoomIn(map, SLOT(slotZoomIn()),
      actionCollection());
  viewZoomOut = KStdAction::zoomOut(map, SLOT(slotZoomOut()),
      actionCollection());
  /*
   * Wir brauchen dringend Icons für diese beiden Aktionen, damit man
   * es auch in die Werkzeugleisten packen kann!
   */
  viewData = new KToggleAction(i18n("Show Flightdata"), 0, 0, this,
      SLOT(slotToggleDataView()), actionCollection(), "toggle_data_view");
  viewMapControl = new KToggleAction(i18n("Show Mapcontrol"), 0, 0, this,
      SLOT(slotToggleMapControl()), actionCollection(), "toggle_map_control");

  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());
  configToolBar = KStdAction::configureToolbars(this,
      SLOT(slotConfigureToolbars()), actionCollection());
  configMap = new KAction(i18n("Configure &Map"), SmallIcon("configure"), 0,
      map, SLOT(slotConfigureMap()), actionCollection(), "configure_map");

  fileOpen->setStatusText(i18n("Opens an existing flight"));
  fileOpenRecent->setStatusText(i18n("Opens a recently used flight"));
  filePrint ->setStatusText(i18n("Prints out the actual map"));
  fileQuit->setStatusText(i18n("Quits KFLog"));
  viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  // use the absolute path to your kflogui.rc file for testing purpose in createGUI();
  createGUI("/home/heiner/Entwicklung/kflog2/kflog/kflogui.rc");
}

void KFLogApp::initMenuBar()
{
//  file_menu = new QPopupMenu;

/*
  mapMenu = new QPopupMenu();
  mapMenu->setCheckable(true);
  mapMenu->insertItem(LAYER_ICON_HYDRO,
            i18n("H&ydrography"), ID_LAYER_HYDRO);
  mapMenu->insertItem(LAYER_ICON_TOPO,
            i18n("&Topography"), ID_LAYER_TOPO);
  mapMenu->insertItem(LAYER_ICON_ROAD,
            i18n("&Highways and roads"), ID_LAYER_ROAD);
  mapMenu->insertItem(LAYER_ICON_RAIL,
            i18n("&Railways and stations"), ID_LAYER_RAIL);
  mapMenu->insertItem(LAYER_ICON_CITY,
            i18n("&Cities and villages"), ID_LAYER_CITY);
  mapMenu->insertItem(LAYER_ICON_LAND,
            i18n("&Landmarks"), ID_LAYER_LAND);
  mapMenu->insertItem(LAYER_ICON_NAV,
            i18n("&Navigation facilities"), ID_LAYER_NAV);
  mapMenu->insertItem(LAYER_ICON_ADDSITES,
            i18n("Additional &sites"), ID_LAYER_ADDSITES);
  mapMenu->insertItem(LAYER_ICON_AIRPORT,
            i18n("&Airports"), ID_LAYER_AIRPORT);
  mapMenu->insertItem(LAYER_ICON_GLIDER,
            i18n("&Glider-sites"), ID_LAYER_GLIDER);
  mapMenu->insertItem(LAYER_ICON_AIRSPACE,
            i18n("A&irspace-structure"), ID_LAYER_AIRSPACE);
  mapMenu->insertItem(LAYER_ICON_OUT,
            i18n("&Outlandingfields"), ID_LAYER_OUT);
  mapMenu->insertItem(LAYER_ICON_WAYPOINT,
            i18n("&Waypoints"), ID_LAYER_WAYPOINT);
  mapMenu->insertItem(LAYER_ICON_FLIGHT,
            i18n("&Flights"), ID_LAYER_FLIGHT);

  QObject::connect(mapMenu, SIGNAL(activated(int)), SLOT(slotMenuLayer(int)));
*/
//  viewMenu = new QPopupMenu();
//  viewMenu->setCheckable(true);
//  viewMenu->insertItem(SmallIcon("reload"), i18n("Redraw Map"), ID_REDRAW);
//  viewMenu->insertItem(i18n("center to &task"), ID_CENTER_TASK);
//  viewMenu->insertItem(i18n("center to &flight"), ID_CENTER_FLIGHT);
//  viewMenu->insertItem(i18n("&Layer"), mapMenu);
//  viewMenu->insertItem(SmallIcon("viewmag+"), i18n("Zoom in"), ID_ZOOM_IN);
//  viewMenu->insertItem(SmallIcon("viewmag-"), i18n("Zoom out"), ID_ZOOM_OUT);
//  viewMenu->insertSeparator();
//  viewMenu->insertItem(i18n("Tool&bar"), ID_VIEW_TOOLBAR);
//  viewMenu->insertItem(i18n("&Statusbar"), ID_VIEW_STATUSBAR );

//  viewMenu->setItemChecked(ID_VIEW_TOOLBAR, bViewToolbar);
//  viewMenu->setItemChecked(ID_VIEW_STATUSBAR, bViewStatusbar);

//  viewMenu->setAccel(CTRL+Key_R, ID_REDRAW);
//  viewMenu->setAccel(CTRL+Key_F, ID_CENTER_FLIGHT);
//  viewMenu->setAccel(CTRL+Key_T, ID_CENTER_TASK);
//  viewMenu->setAccel(Key_Plus, ID_ZOOM_IN);
//  viewMenu->setAccel(Key_Minus, ID_ZOOM_OUT);

//  confMenu = new QPopupMenu();
//  confMenu->insertItem(i18n("Configure &map"), ID_CONF_MAP);
//  confMenu->insertItem(i18n("KFLog-&setup"), ID_CONF_KFLOG);

//  flightMenu = new QPopupMenu();
//  flightMenu->insertItem(i18n("Auswertung"), ID_FLIGHT_EVALUATION);
//  flightMenu->insertItem(i18n("Optimize task"), ID_FLIGHT_OPTIMIZE);
//  flightMenu->insertSeparator();
//  flightMenu->insertItem(SmallIcon("fileprint"), i18n("&Print flightdata"),
//            ID_FLIGHT_PRINT);
//  flightMenu->setAccel(Key_F9, ID_FLIGHT_EVALUATION);
//  flightMenu->setAccel(Key_F10, ID_FLIGHT_OPTIMIZE);
//  flightMenu->setAccel(Key_F8, ID_FLIGHT_PRINT);

//  helpMenu = new QPopupMenu();
//  helpMenu->insertItem(SmallIcon("contents"),
//        i18n("User &Manual"), this, SLOT(slotHelpContents()),0,
//            ID_HELP_CONTENTS);
//  helpMenu->insertSeparator();
  /* KFLog-Icon einsetzen ...*/
//  helpMenu->insertItem(i18n("&About KFLog..."), this, SLOT(slotHelpAbout()),0,
//            ID_HELP_ABOUT);

//  menuBar()->insertItem(i18n("&File"), file_menu);
//  menuBar()->insertItem(i18n("&View"), viewMenu);
//  menuBar()->insertItem(i18n("&Options"), confMenu);
//  menuBar()->insertItem(i18n("F&light"), flightMenu);
//  menuBar()->insertSeparator();
//  menuBar()->insertItem(i18n("&Help"), helpMenu);

//	KAccel* key_accel = new KAccel(this);
//	key_accel->connectItem(KAccel::Help, kapp, SLOT(appHelpActivated()));

//  CONNECT_CMD(file_menu);
//  CONNECT_CMD(viewMenu);
//  CONNECT_CMD(confMenu);
//  CONNECT_CMD(flightMenu);
}

void KFLogApp::initStatusBar()
{
  /* Alternativ könnte der Balken auch nur während des Zeichnens erscheinen */
  statusProgress = new KProgress(statusBar());
  statusProgress->setFixedWidth(120);
  statusProgress->setFixedHeight( statusProgress->sizeHint().height() - 4 );
  statusProgress->setBarStyle( KProgress::Blocked );
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
  STATUS_LABEL(statusHeightL, 80, AlignRight);
  STATUS_LABEL(statusVarioL, 80, AlignRight);
  STATUS_LABEL(statusSpeedL, 100, AlignRight);
  STATUS_LABEL(statusLatL, 110, AlignHCenter);
  STATUS_LABEL(statusLonL, 110, AlignHCenter);

  statusBar()->addWidget( statusLabel, 1, false );
  statusBar()->addWidget( statusTimeL, 0, false );
  statusBar()->addWidget( statusHeightL, 0, false );
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

  setView(mapViewDock);
  setMainDockWidget(mapViewDock);

  QFrame* mapViewFrame = new QFrame(mapViewDock);
  map = new Map(this, mapViewFrame);

  QVBoxLayout* mapLayout = new QVBoxLayout(mapViewFrame,2,1);
  mapLayout->addWidget(map);
  mapLayout->activate();

  QFrame* mapControlFrame = new QFrame(mapControlDock);
  mapControl = new MapControlView(mapControlFrame, map);
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
  connect(mapControl, SIGNAL(scaleChanged()), map,
      SLOT(slotRedrawMap()));
}

void KFLogApp::showCoords(QPoint pos)
{
  statusBar()->clear();
  statusLatL->setText(printPos(pos.y()));
  statusLonL->setText(printPos(pos.x(), false));
}

void KFLogApp::showPointInfo(QPoint pos, struct flightPoint* point)
{
  statusBar()->clear();
  QString text;
  text.sprintf("%s", (const char*)printTime(point->time, true));
  statusTimeL->setText(text);
  text.sprintf("%4d m  ", point->height);
  statusHeightL->setText(text);
  text.sprintf("%3.1f km/h  ", (float)point->dS / (float)point->dT);
  statusSpeedL->setText(text);
  text.sprintf("%2.1f m/s  ", (float)point->dH / (float)point->dT);
  statusVarioL->setText(text);

  statusLatL->setText(printPos(pos.y()));
  statusLonL->setText(printPos(pos.x(), false));
}

void KFLogApp::clearPointInfo(QPoint pos)
{
  statusBar()->clear();
  statusTimeL->setText("");
  statusHeightL->setText("");
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

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");
  flightDir = config->readEntry("FlightDir", getenv("HOME"));

  QSize size=config->readSizeEntry("Geometry");

  if(!size.isEmpty())  resize(size);

  // initialize the mapmatrix
  config->setGroup("Map Data");
  int mapCenterLat = config->readNumEntry("Center Latitude", 29100000);
  int mapCenterLon = config->readNumEntry("Center Longitude", 5400000);
  double scale = config->readDoubleNumEntry("Map Scale", 200);
  double v1 = config->readDoubleNumEntry("Parallel1", 32400000);
  double v2 = config->readDoubleNumEntry("Parallel2", 30000000);
  int homeLat = config->readNumEntry("Home Latitude", 29125200);
  int homeLon = config->readNumEntry("Home Longitude", 5364500);

  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.initMatrix(mapCenterLat, mapCenterLon, scale, v1, v2,
      homeLat, homeLon);
}

bool KFLogApp::queryExit()
{
  saveOptions();
  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KFLogApp::slotSetProgress(int value)  { statusProgress->setValue(value); }

void KFLogApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  QString fName = KFileDialog::getOpenFileName(flightDir, "*.igc *.IGC", this);

  if(fName != NULL)
    {
      QFileInfo fInfo(fName);
      flightDir = fInfo.dirPath();
      extern MapContents _globalMapContents;
      if(_globalMapContents.loadFlight(fName))
        {
          dataView->setFlightData(_globalMapContents.getFlight());
          viewCenterTask->setEnabled(true);
          viewCenterFlight->setEnabled(true);
          // Hier wird die Karte leider 2x neu gezeichnet, denn erst
          // beim ersten Zeichnen werden die Rahmen von Flug und Aufgabe
          // bestimmt.
          map->slotRedrawMap();
          map->slotCenterToFlight();
        }
    }

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  extern MapContents _globalMapContents;
  if(url.isLocalFile())
      _globalMapContents.loadFlight(url.fileName());

  slotStatusMsg(i18n("Ready."));
}

void KFLogApp::slotFileClose()
{

}

void KFLogApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

  QPrinter printer;
  if (printer.setup(this)) {  }

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

void KFLogApp::slotToggleDataView()
{
  if(dataViewDock->isVisible()){
//    view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
//    toolBar()->setButton(ID_VIEW_TREEVIEW,false);
  }
  else{
//    view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
//    toolBar()->setButton(ID_VIEW_TREEVIEW,true);
  }
  dataViewDock->changeHideShowState();
}

void KFLogApp::slotToggleMapControl()
{
  if(mapControlDock->isVisible()){
//    view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
//    toolBar()->setButton(ID_VIEW_TREEVIEW,false);
  }
  else{
//    view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
//    toolBar()->setButton(ID_VIEW_TREEVIEW,true);
  }
  mapControlDock->changeHideShowState();
}

void KFLogApp::slotConfigureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );
  KEditToolbar dlg(actionCollection());
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));

  if (dlg.exec())  createGUI();
}

void KFLogApp::slotNewToolbarConfig()
{
//   ...if you use any action list, use plugActionList on each here...
   applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KFLogApp::slotStartComplete() { if(showStartLogo)  delete startLogo; }
