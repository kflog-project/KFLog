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
//#include <sys/types.h>
#include <stdlib.h>

// include files for QT
#include <qdir.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
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
#include <kflogconfig.h>
#include <kflogstartlogo.h>
#include <map.h>
#include <mapcalc.h>
#include <mapconfig.h>
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

  if (config->readBoolEntry("Logo",false) && (!kapp->isRestored() ) )
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

//  filePrint->setEnabled(true);
  viewData->setChecked(true);
  viewMapControl->setChecked(true);
  viewCenterTask->setEnabled(false);
  viewCenterFlight->setEnabled(false);
  flightEvaluation->setEnabled(false);
}

KFLogApp::~KFLogApp()
{

}

void KFLogApp::initActions()
{
  fileOpen = new KAction(i18n("&Open Flight"), "fileopen",
      KStdAccel::key(KStdAccel::Open), this, SLOT(slotFileOpen()),
      actionCollection(), "file_open");
  fileOpenRecent = KStdAction::openRecent(this,
      SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileClose = new KAction(i18n("Close Flight"), "fileclose",
      KStdAccel::key(KStdAccel::Close), this, SLOT(slotFileClose()),
      actionCollection(), "file_close");
/*
 * Printing not available yet ...
 *  filePrint = KStdAction::print(this, SLOT(slotFilePrint()),
 *      actionCollection());
 */
  KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

  viewRedraw = KStdAction::redisplay(map, SLOT(slotRedrawMap()),
      actionCollection());
  viewRedraw->setAccel(Key_F5);

  viewCenterTask = new KAction(i18n("Center to &Task"), "centertask",
      Key_F6, map,
      SLOT(slotCenterToTask()), actionCollection(), "view_center_task");
  viewCenterFlight = new KAction(i18n("Center to &Flight"), "centerflight",
      Key_F7, map,
      SLOT(slotCenterToFlight()), actionCollection(), "view_center_flight");

  new KAction(i18n("Center to &Homesite"), "gohome",
      KStdAccel::key(KStdAccel::Home), map,
      SLOT(slotCenterToHome()), actionCollection(), "view_center_home");

  KStdAction::zoomIn(map, SLOT(slotZoomIn()),
      actionCollection());
  KStdAction::zoomOut(map, SLOT(slotZoomOut()),
      actionCollection());
  /*
   * Wir brauchen dringend Icons für diese beiden Aktionen, damit man
   * es auch in die Werkzeugleisten packen kann!
   */
  viewData = new KToggleAction(i18n("Show Flightdata"), 0, this,
      SLOT(slotToggleDataView()), actionCollection(), "toggle_data_view");
  viewMapControl = new KToggleAction(i18n("Show Mapcontrol"), 0, this,
      SLOT(slotToggleMapControl()), actionCollection(), "toggle_map_control");

  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()),
      actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()),
      actionCollection());

  KStdAction::configureToolbars(this,
      SLOT(slotConfigureToolbars()), actionCollection());
  KStdAction::keyBindings(this,
      SLOT(slotConfigureKeyBindings()), actionCollection());

  KStdAction::preferences(this, SLOT(slotConfigureKFLog()), actionCollection());

  flightEvaluation = new KAction(i18n("Evaluation"), "flightevaluation",
      CTRL+Key_E, this, SLOT(slotEvaluateFlight()), actionCollection(),
      "evaluate_flight");

//  new KAction(i18n("Optimize"), 0, 0, map,
//      SLOT(slotOptimzeFlight()), actionCollection(), "optimize_flight");

  fileOpen->setStatusText(i18n("Open flight"));
//  filePrint ->setStatusText(i18n("Print map"));

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
  map = new Map(this, mapViewFrame, "KFLog-Map");

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

//void KFLogApp::showCoords(QPoint pos)
//{
//  statusBar()->clear();
//  statusLatL->setText(printPos(pos.y()));
//  statusLonL->setText(printPos(pos.x(), false));
//}

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
  QSize size=config->readSizeEntry("Geometry");

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");
  config->setGroup("Path");
  flightDir = config->readEntry("DefaultFlightDirectory",
      getpwuid(getuid())->pw_dir);

  if(!size.isEmpty())  resize(size);

  mapConfig = new MapConfig(config);

  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.initMatrix(mapConfig);

  BaseMapElement::initMapElement(&_globalMapMatrix, mapConfig);
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

void KFLogApp::slotFileClose()
{
  map->slotDeleteFlightLayer();
  extern MapContents _globalMapContents;
  if(_globalMapContents.getFlightList()->count() == 0)
    {
      viewCenterTask->setEnabled(false);
      viewCenterFlight->setEnabled(false);
      flightEvaluation->setEnabled(false);
    }
}

void KFLogApp::slotFileOpen()
{
  slotStatusMsg(i18n("Opening file..."));

  KURL fUrl = KFileDialog::getOpenURL(flightDir, "*.igc *.IGC", this);

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
      viewCenterTask->setEnabled(true);
      viewCenterFlight->setEnabled(true);
      flightEvaluation->setEnabled(true);

      fileOpenRecent->addURL(fUrl);

      // Hier wird die Karte leider 2x neu gezeichnet, denn erst
      // beim ersten Zeichnen werden die Rahmen von Flug und Aufgabe
      // bestimmt.
      map->slotRedrawMap();
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
          viewCenterTask->setEnabled(true);
          viewCenterFlight->setEnabled(true);
          flightEvaluation->setEnabled(true);
          map->slotRedrawMap();
          map->slotCenterToFlight();
        }
    }

  slotStatusMsg(i18n("Ready."));
}


void KFLogApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

//  QPrinter printer;
//  if (printer.setup(this))
      map->slotPrintMap();

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

void KFLogApp::slotEvaluateFlight()
{
  extern MapContents _globalMapContents;
  EvaluationDialog* eval =
    new EvaluationDialog(_globalMapContents.getFlightList());

  map->connect(eval, SIGNAL(showCursor(QPoint, QPoint)),
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
  if(KFLogConfig(this, config, "kflogconfig").exec())
    {
      mapConfig->readConfig();
      map->slotRedrawMap();
    }
}

void KFLogApp::slotNewToolbarConfig()
{
//   ...if you use any action list, use plugActionList on each here...
   applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KFLogApp::slotStartComplete()
{
  if(showStartLogo)  delete startLogo;
}
