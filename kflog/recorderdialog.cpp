/***********************************************************************
**
**   recorderdialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "recorderdialog.h"

#include <dlfcn.h>

#include <sys/stat.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qwhatsthis.h>

#include <mapcalc.h>
#include <mapcontents.h>
#include <airport.h>

#define CHECK_ERROR_EXIT  error = (char *)dlerror(); \
  if(error != NULL) \
    { \
      warning(error); \
      return; \
    }

#define CHECK_ERROR_RETURN  error = (char *)dlerror(); \
  if(error != NULL) \
    { \
      warning(error); \
      return 0; \
    }

#define CHECK_ERROR  error = (char *)dlerror(); \
  if(error != NULL) \
    { \
      warning(error); \
    }

RecorderDialog::RecorderDialog(QWidget *parent, KConfig* cnf, const char *name)
  : KDialogBase(IconList, i18n("Flightrecorder-Dialog"), Close, Close,
                parent, name, true, true),
    config(cnf),
    loggerConf(0),
    isOpen(false),
    isConnected(false)
{
  extern MapContents _globalMapContents;
  BaseFlightElement *e;
  Waypoint *wp;
  QList<Waypoint> *tmp;
  tmp = _globalMapContents.getWaypointList();
  waypoints = new WaypointList;
  for (wp = tmp->first(); wp != 0; wp = tmp->next()) {
    waypoints->append(wp);
  }
  waypoints->sort();

  QList<BaseFlightElement> *tList = _globalMapContents.getFlightList();
  tasks = new QList<FlightTask>;

  for (e = tList->first(); e != 0; e = tList->next()) {
    if (e->getTypeID() == BaseMapElement::Task) {
      tasks->append((FlightTask*)e);
    }
  }

  __addSettingsPage();
  __addFlightPage();
  __addDeclarationPage();
  __addTaskPage();
  __addWaypointPage();
  __addPilotPage();
  __addConfigPage();

  slotEnablePages();

  setMinimumWidth(500);
  setMinimumHeight(350);

  // First: disable all pages but the first, for we must connect
  // to the recorder prior any other action

  flightPage->setEnabled( false );
  waypointPage->setEnabled( false );
  taskPage->setEnabled( false );
  declarationPage->setEnabled( false );
}

RecorderDialog::~RecorderDialog()
{
  config->setGroup("Recorder Dialog");
  config->writeEntry("Name", selectType->currentText());
  config->writeEntry("Port", selectPort->currentItem());
  config->writeEntry("Baud", selectBaud->currentItem());
  config->setGroup(0);
  slotCloseRecorder();
  delete waypoints;
  delete tasks;
}

void RecorderDialog::__addSettingsPage()
{
  int typeID(0), typeLoop(0);

  settingsPage = addPage(i18n("Recorder"), i18n("Recorder Settings"),
                         KGlobal::instance()->iconLoader()->loadIcon("connect_no", KIcon::NoGroup,
                                                                     KIcon::SizeLarge));

  QGridLayout* sLayout = new QGridLayout(settingsPage, 15, 8, 10, 1);

  QGroupBox* sGroup = new QGroupBox(settingsPage, "homeGroup");
  sGroup->setTitle(i18n("Settings") + ":");

  selectType = new KComboBox(settingsPage, "type-selection");
  connect(selectType, SIGNAL(activated(const QString &)), this,
    SLOT(slotRecorderTypeChanged(const QString &)));

  selectPort = new KComboBox(settingsPage, "port-selection");
  selectBaud = new KComboBox(settingsPage, "baud-selection");

  selectPort->insertItem("ttyS0");
  selectPort->insertItem("ttyS1");
  selectPort->insertItem("ttyS2");
  selectPort->insertItem("ttyS3");

  selectBaud->insertItem("115200");
  selectBaud->insertItem("57600");
  selectBaud->insertItem("38400");
  selectBaud->insertItem("19200");
  selectBaud->insertItem("9600");

  QPushButton* connectB = new QPushButton(i18n("Connect recorder"), settingsPage);
  connectB->setMaximumWidth(connectB->sizeHint().width() + 5);

  QGroupBox* infoGroup = new QGroupBox(settingsPage, "infoGroup");
  infoGroup->setTitle(i18n("Info") + ":");
  apiID = new QLabel(settingsPage);
  apiID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  apiID->setBackgroundMode( PaletteLight );

  serID = new QLabel(i18n("no recorder connected"), settingsPage);
  serID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  serID->setBackgroundMode( PaletteLight );

  sLayout->addMultiCellWidget(sGroup, 0, 6, 0, 7);
  sLayout->addWidget(new QLabel(i18n("Type"), settingsPage), 1, 1, AlignRight);
  sLayout->addMultiCellWidget(selectType, 1, 1, 2, 6);
  sLayout->addWidget(new QLabel(i18n("Port"), settingsPage), 3, 1, AlignRight);
  sLayout->addMultiCellWidget(selectPort, 3, 3, 2, 3);
  sLayout->addWidget(new QLabel(i18n("Baud"), settingsPage), 3, 4, AlignRight);
  sLayout->addMultiCellWidget(selectBaud, 3, 3, 5, 6);

  sLayout->addWidget(connectB, 5, 6, AlignRight);

  sLayout->addMultiCellWidget(infoGroup, 8, 14, 0, 7);
  sLayout->addWidget(new QLabel(i18n("API-Version"), settingsPage), 9, 1,
                     AlignRight);
  sLayout->addMultiCellWidget(apiID, 9, 9, 2, 3);
  sLayout->addWidget(new QLabel(i18n("Serial-Nr."), settingsPage), 11, 1);
  sLayout->addMultiCellWidget(serID, 11, 11, 2, 3);

  sLayout->setColStretch(0, 1);
  sLayout->setColStretch(7, 1);
  for (int i = 1; i < 7; i++) {
    sLayout->setColStretch(i, 10);
  }

  sLayout->addRowSpacing(0, 20);
  sLayout->addRowSpacing(2, 5);
  sLayout->addRowSpacing(4, 5);
  sLayout->addRowSpacing(6, 10);

  sLayout->addRowSpacing(7, 5);
  sLayout->addRowSpacing(9, 0);

  sLayout->addRowSpacing(8, 20);
  sLayout->addRowSpacing(10, 5);
  sLayout->addRowSpacing(12, 5);
  sLayout->addRowSpacing(14, 10);

  QStringList configRec;
  configRec = KGlobal::dirs()->findAllResources("appdata", "logger/*.desktop");

  if(configRec.count() == 0) {
    KMessageBox::error(this,
                       i18n("There are no recorder-libraries installed."),
                       i18n("No recorders installed."));
  }

  libNameList.clear();

  config->setGroup("Recorder Dialog");
  selectPort->setCurrentItem(config->readNumEntry("Port", 0));
  selectBaud->setCurrentItem(config->readNumEntry("Baud", 0));
  QString name(config->readEntry("Name", 0));
  config->setGroup(0);

  for(QStringList::Iterator it = configRec.begin(); it != configRec.end(); it++) {
    if(loggerConf != NULL) {
      delete loggerConf;
    }

    loggerConf = new KConfig((*it).latin1());
    if(!loggerConf->hasGroup("Logger Data")) {
      warning(i18n("Configfile %1 is corrupt!").arg((*it).latin1()));
    }
    loggerConf->setGroup("Logger Data");
    selectType->insertItem(loggerConf->readEntry("Name"));
    if(loggerConf->readEntry("Name") == name) {
      typeID = typeLoop;
    }
    libNameList.append(loggerConf->readEntry("LibName"));
    typeLoop++;
  }

  selectType->setCurrentItem(typeID);

  connect(connectB, SIGNAL(clicked()), SLOT(slotConnectRecorder()));
}

void RecorderDialog::__addFlightPage()
{
  flightPage = addPage(i18n("Flights"), i18n("Flights"),
                       KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup,
                                                                   KIcon::SizeLarge));

  QGridLayout* fLayout = new QGridLayout(flightPage, 13, 5, 10, 1);

  flightList = new KFLogListView("recorderFlightList", flightPage,
                                 "flightList");
  flightList->setShowSortIndicator(true);
  flightList->setAllColumnsShowFocus(true);

  colID = flightList->addColumn(i18n("Nr"), 50);
  colDate = flightList->addColumn(i18n("Date"));
  colPilot = flightList->addColumn(i18n("Pilot"));
  colGlider = flightList->addColumn(i18n("Glider"));
  colFirstPoint = flightList->addColumn(i18n("first Point"));
  colLastPoint = flightList->addColumn(i18n("last Point"));

  flightList->setColumnAlignment(colID, AlignRight);

  flightList->loadConfig();

  QPushButton* listB = new QPushButton(i18n("load list"), flightPage);
  QPushButton* fileB = new QPushButton(i18n("save flight"), flightPage);
  useLongNames = new QCheckBox(i18n("long filenames"), flightPage);
  useLongNames->setChecked(true);
  useFastDownload = new QCheckBox(i18n("fast download"), flightPage);
  useFastDownload->setChecked(true);

  QWhatsThis::add(useLongNames,
                  i18n("If checked, the long filenames are used."));
  QWhatsThis::add(useFastDownload,
                  i18n("If checked, the IGC-file will not be signed.<BR>"
                       "Note: Do not use fast download when "
                       "using the file for competitions."));

  fLayout->addMultiCellWidget(flightList, 0, 0, 0, 6);
  fLayout->addWidget(listB, 2, 0);
  fLayout->addWidget(fileB, 2, 2);
  fLayout->addWidget(useLongNames, 2, 4);
  fLayout->addWidget(useFastDownload, 2, 6);

  fLayout->addColSpacing(1, 5);

  fLayout->addRowSpacing(1, 5);

  connect(listB, SIGNAL(clicked()), SLOT(slotReadFlightList()));
  connect(fileB, SIGNAL(clicked()), SLOT(slotDownloadFlight()));
}

void RecorderDialog::__addDeclarationPage()
{
  FlightTask *e;

  declarationPage = addPage(i18n("Declaration"), i18n("Flight Declaration"),
                            KGlobal::instance()->iconLoader()->loadIcon("declaration",
                                                                        KIcon::NoGroup,
                                                                        KIcon::SizeLarge));
  QVBoxLayout *top = new QVBoxLayout(declarationPage, 5);
  QGridLayout* tLayout = new QGridLayout(13, 5, 1);

  taskSelection = new KComboBox(false, declarationPage, "taskSelection");
  declarationList = new KFLogListView("recorderDeclarationList",
                                      declarationPage, "declarationList");

  declarationColID = declarationList->addColumn(i18n("Nr"), 50);
  declarationColName = declarationList->addColumn(i18n("Name"), 120);
  declarationColLat = declarationList->addColumn(i18n("Latitude"), 140);
  declarationColLon = declarationList->addColumn(i18n("Longitude"), 140);

  declarationList->setAllColumnsShowFocus(true);
  declarationList->setSorting(declarationColID, true);
  declarationList->setSelectionMode(QListView::NoSelection);

  declarationList->setColumnAlignment(declarationColID, AlignRight);
  declarationList->setColumnAlignment(declarationColLat, AlignRight);
  declarationList->setColumnAlignment(declarationColLon, AlignRight);

  declarationList->loadConfig();

  pilotName = new KLineEdit(declarationPage, "pilotName");
  copilotName = new KLineEdit(declarationPage, "copilotName");
  gliderID = new KComboBox(declarationPage, "gliderID");
  gliderID->setEditable(true);
  gliderType = new KLineEdit(declarationPage, "gliderType");
  compID = new KLineEdit(declarationPage, "compID");
  compClass = new KLineEdit(declarationPage, "compClass");

  QPushButton* writeDeclaration = new QPushButton(i18n("write declaration to recorder"), declarationPage);
  writeDeclaration->setMaximumWidth(writeDeclaration->sizeHint().width() + 15);

  tLayout->addMultiCellWidget(declarationList, 0, 0, 0, 6);
  tLayout->addWidget(new QLabel(i18n("Pilot") + "1:", declarationPage), 2, 0,
                     AlignRight);
  tLayout->addWidget(pilotName, 2, 2);
  tLayout->addWidget(new QLabel(i18n("Pilot") + "2:", declarationPage), 2, 4,
                     AlignRight);
  tLayout->addWidget(copilotName, 2, 6);
  tLayout->addWidget(new QLabel(i18n("Glider-ID") + ":", declarationPage), 4,
                     0, AlignRight);
  tLayout->addWidget(gliderID, 4, 2);
  tLayout->addWidget(new QLabel(i18n("Glidertype") + ":", declarationPage), 4,
                     4, AlignRight);
  tLayout->addWidget(gliderType, 4, 6);
  tLayout->addWidget(new QLabel(i18n("Comp. ID") + ":", declarationPage), 6,
                     0, AlignRight);
  tLayout->addWidget(compID, 6, 2);
  tLayout->addWidget(new QLabel(i18n("Comp. Class") + ":", declarationPage), 6,
                     4, AlignRight);
  tLayout->addWidget(compClass, 6, 6);
  tLayout->addMultiCellWidget(writeDeclaration, 8, 8, 4, 6, AlignRight);

  tLayout->addColSpacing(1, 5);
  tLayout->addColSpacing(3, 10);
  tLayout->addColSpacing(5, 5);

  tLayout->addRowSpacing(1, 10);
  tLayout->addRowSpacing(3, 5);
  tLayout->addRowSpacing(5, 5);
  tLayout->addRowSpacing(7, 10);

  top->addWidget(new QLabel(i18n("Tasks"), declarationPage));
  top->addWidget(taskSelection);
  top->addLayout(tLayout);

  config->setGroup("Personal Data");
  pilotName->setText(config->readEntry("PilotName", ""));

  config->setGroup(0);

  for (e = tasks->first(); e != 0; e = tasks->next()) {
    taskSelection->insertItem(e->getFileName() + " " + e->getTaskTypeString());
  }

  if (tasks->count()) {
    slotSwitchTask(0);
  }
  else {
    warning("Keine Aufgaben geplant ...");
    writeDeclaration->setEnabled(false);
  }

  connect(taskSelection, SIGNAL(activated(int)), SLOT(slotSwitchTask(int)));
  connect(writeDeclaration, SIGNAL(clicked()), SLOT(slotWriteDeclaration()));
}

void RecorderDialog::__addTaskPage()
{
  taskPage = addPage(i18n("Tasks"), i18n("Tasks"),
                     KGlobal::instance()->iconLoader()->loadIcon("task",
                                                                 KIcon::NoGroup,
                                                                 KIcon::SizeLarge));

  QVBoxLayout *top = new QVBoxLayout(taskPage, 5);
  QHBoxLayout *buttons = new QHBoxLayout(10);
  QPushButton *b;

  taskList = new KFLogListView("recorderTaskList", taskPage, "taskList");

  taskColID = taskList->addColumn(i18n("Nr"), 50);
  taskColName = taskList->addColumn(i18n("Name"), 120);
  taskColDesc = taskList->addColumn(i18n("Description"), 120);
  taskColTask = taskList->addColumn(i18n("Task dist."), 120);
  taskColTotal = taskList->addColumn(i18n("Total dist."), 120);

  taskList->setSorting(taskColID, true);
  taskList->setAllColumnsShowFocus(true);

  taskList->setSelectionMode(QListView::NoSelection);
  taskList->setColumnAlignment(taskColID, AlignRight);

  taskList->loadConfig();

  buttons->addStretch();
  b = new QPushButton(i18n("write tasks to recorder"), taskPage);
  connect(b, SIGNAL(clicked()), SLOT(slotWriteTasks()));
  buttons->addWidget(b);

  b = new QPushButton(i18n("read tasks from recorder"), taskPage);
  connect(b, SIGNAL(clicked()), SLOT(slotReadTasks()));
  buttons->addWidget(b);

  top->addWidget(taskList);
  top->addLayout(buttons);

  fillTaskList();
}

void RecorderDialog::fillTaskList()
{
  FlightTask *task;
  int loop = 1;
  QString idS;
  QListViewItem *item;

  taskList->clear();
  for (task = tasks->first(); task != 0; task = tasks->next()){
    item = new QListViewItem(taskList);
    idS.sprintf("%.3d", loop++);
    item->setText(taskColID, idS);
    item->setText(taskColName, task->getFileName());
    item->setText(taskColDesc, task->getTaskTypeString());
    item->setText(taskColTask, task->getTaskDistanceString());
    item->setText(taskColTotal, task->getTotalDistanceString());
  }
}

void RecorderDialog::__addWaypointPage()
{
  Waypoint *wp;
  int loop = 1;
  QString idS;
  QListViewItem *item;

  waypointPage = addPage(i18n("Waypoints"), i18n("Waypoints"),
                         KGlobal::instance()->iconLoader()->loadIcon("waypoint",
                                                                 KIcon::NoGroup,
                                                                 KIcon::SizeLarge));
  QVBoxLayout *top = new QVBoxLayout(waypointPage, 5);
  QHBoxLayout *buttons = new QHBoxLayout(10);
  QPushButton *b;

  waypointList = new KFLogListView("recorderWaypointList", waypointPage,
                                   "waypointList");

  waypointColID = waypointList->addColumn(i18n("Nr"), 50);
  waypointColName = waypointList->addColumn(i18n("Name"), 120);
  waypointColLat = waypointList->addColumn(i18n("Latitude"), 140);
  waypointColLon = waypointList->addColumn(i18n("Longitude"), 140);

  waypointList->setAllColumnsShowFocus(true);
  waypointList->setSorting(waypointColID, true
  );
  waypointList->setSelectionMode(QListView::NoSelection);

  waypointList->setColumnAlignment(waypointColID, AlignRight);
  waypointList->setColumnAlignment(waypointColLat, AlignRight);
  waypointList->setColumnAlignment(waypointColLon, AlignRight);

  waypointList->loadConfig();

  buttons->addStretch();
  b = new QPushButton(i18n("write waypoints to recorder"), waypointPage);
  connect(b, SIGNAL(clicked()), SLOT(slotWriteWaypoints()));
  buttons->addWidget(b);

  b = new QPushButton(i18n("read waypoints from recorder"), waypointPage);
  connect(b, SIGNAL(clicked()), SLOT(slotReadWaypoints()));
  buttons->addWidget(b);

  top->addWidget(waypointList);
  top->addLayout(buttons);

  for (wp = waypoints->first(); wp != 0; wp = waypoints->next()){
    item = new QListViewItem(waypointList);
    idS.sprintf("%.3d", loop++);
    item->setText(waypointColID, idS);
    item->setText(waypointColName, wp->name);
    item->setText(waypointColLat, printPos(wp->origP.lat()));
    item->setText(waypointColLon, printPos(wp->origP.lon(), false));
  }
}

void RecorderDialog::slotConnectRecorder()
{
  portName = "/dev/" + selectPort->currentText();
  QStringList::Iterator it = libNameList.at(selectType->currentItem());
  QString name = (*it).latin1();
  int baud = selectBaud->currentText().toInt();
  void* funcH;
  char* error;

  if(portName == 0) {
    warning(i18n("No port given!"));
    return;
  }

  if(!__openLib(name)) {
    warning(i18n("Could not open lib!"));
    return;
  }

  setCursor(WaitCursor);

  funcH = dlsym(libHandle, "openRecorder");
  CHECK_ERROR_EXIT

  if (((int (*)(char *, int))funcH)(qstrdup(portName), baud)) {
    funcH = dlsym(libHandle, "getRecorderSerialNo");
    CHECK_ERROR_EXIT

    serID->setText(((QString (*)())funcH)());
    isConnected = true;
    slotEnablePages();
    slotReadDatabase();
  }
  else {
    setCursor(ArrowCursor);
    KMessageBox::error(this,
                       i18n("Could not connect to recorder!\n"
                            "Check connections and settings."),
                       i18n("Recorder Error"));
  }
  setCursor(ArrowCursor);
}

void RecorderDialog::slotCloseRecorder()
{
  void* funcH;
  char* error;
  int err;

  if (isOpen) {
    funcH = dlsym(libHandle, "closeRecorder");
    CHECK_ERROR_EXIT

    err = ((int (*)())funcH)();
    libName = "";
    apiID->setText(libName);
    serID->clear();
    isOpen = false;
  }
}

void RecorderDialog::slotReadFlightList()
{
  // Jetzt muss das Flugverzeichnis vom Logger gelesen werden!
  __fillDirList();

  flightList->clear();

  QListViewItem* item;
  FRDirEntry* e;
  QTime time;
  QString day;
  QString idS;

  for(unsigned int loop = 0; loop < dirList.count(); loop++) {
    e = dirList.at(loop);
    item = new QListViewItem(flightList);
    idS.sprintf("%.3d", loop + 1);
    item->setText(colID, idS);
    day.sprintf("%d-%.2d-%.2d", e->firstTime.tm_year + 1900,
                e->firstTime.tm_mon + 1, e->firstTime.tm_mday);
    item->setText(colDate, day);
    item->setText(colPilot, e->pilotName);
    item->setText(colGlider, e->gliderID);
    time = QTime(e->firstTime.tm_hour, e->firstTime.tm_min,
                 e->firstTime.tm_sec);
    item->setText(colFirstPoint, KGlobal::locale()->formatTime(time, true));
    time = QTime(e->lastTime.tm_hour, e->lastTime.tm_min,
                 e->lastTime.tm_sec);
    item->setText(colLastPoint, KGlobal::locale()->formatTime(time, true));
  }
}

void RecorderDialog::slotDownloadFlight()
{
  QListViewItem *item = flightList->currentItem();
  char* error;
  void* funcH;
  int ret;
  QString fileName;

  if(item == 0) {
    return;
  }

  config->setGroup("Path");
  fileName = config->readEntry("DefaultFlightDirectory") + "/";

  int flightID(item->text(colID).toInt() - 1);

  warning("Lade Flug %d (%d)", flightID, flightList->itemPos(item));
  warning(dirList.at(flightID)->longFileName);
  warning(dirList.at(flightID)->shortFileName);

  if(useLongNames->isChecked()) {
    fileName += dirList.at(flightID)->longFileName;
  }
  else {
    fileName += dirList.at(flightID)->shortFileName;
  }
  warning(fileName);

  funcH = dlsym(libHandle, "downloadFlight");

  CHECK_ERROR

  ret = ((int (*)(int, int, char*))funcH)(flightID, !useFastDownload->isChecked(), (char *)(const char*)fileName);

}

void RecorderDialog::slotWriteDeclaration()
{
  void* funcH;
  int ret;
  FRTaskDeclaration taskDecl;
  taskDecl.pilotA = pilotName->text();
  taskDecl.pilotB = copilotName->text();
  taskDecl.gliderID = gliderID->currentText();
  taskDecl.gliderType = gliderType->text();
  taskDecl.compID = compID->text();
  taskDecl.compClass = compClass->text();

  if (taskSelection->currentItem() >= 0) {
    QList<Waypoint> wpList = tasks->at(taskSelection->currentItem())->getWPList();

    funcH = dlsym(libHandle, "writeDeclaration");
    if (funcH == 0) {
      KMessageBox::error(this,
                         i18n("Function not implemented"),
                         i18n("Library Error"));
    }
    else {
      warning("Schreibe Aufgabe auf Logger ...");

      ret = ((int (*)(FRTaskDeclaration*, QList<Waypoint>*))funcH)(&taskDecl, &wpList);

      warning("   ... fertig (%d)", ret);
    }
  }
}

int RecorderDialog::__fillDirList()
{
  char* error;
  void* funcH;
  int ret;

  funcH = dlsym(libHandle, "getFlightDir");

  CHECK_ERROR

  ret = ((int (*)(QList<FRDirEntry>*)) funcH)(&dirList);

  return ret;
}

int RecorderDialog::__openLib(QString libN)
{
  char* error;
  void* funcH;

  libName = "";
  apiID->setText(libName);

  libHandle = dlopen(KGlobal::dirs()->findResource("lib", libN), RTLD_NOW);

  CHECK_ERROR_RETURN

  funcH = dlsym(libHandle, "getLibName");

  CHECK_ERROR_RETURN

  libName = ((QString (*)())funcH)();
  apiID->setText(libName);

  isOpen = true;

  return 1;
}

void RecorderDialog::slotSwitchTask(int idx)
{
  FlightTask *task = tasks->at(idx);
  QListViewItem* item;
  QString idS;
  Waypoint *wp;

  declarationList->clear();
  if(task) {
    QList<Waypoint> wpList = ((FlightTask*)task)->getWPList();
    int loop = 1;
    for(wp = wpList.first(); wp != 0; wp = wpList.next()) {
      item = new QListViewItem(declarationList);
      idS.sprintf("%2d", loop++);
      item->setText(declarationColID, idS);
      item->setText(declarationColName, wp->name);
      item->setText(declarationColLat, printPos(wp->origP.lat()));
      item->setText(declarationColLon, printPos(wp->origP.lon(), false));
    }
  }
}

void RecorderDialog::slotReadTasks()
{
  char* error;
  void* funcH;
  FlightTask *task;
  Waypoint *wp;
  QList<Waypoint> wpList;
  extern MapMatrix _globalMapMatrix;
  int ret;

  funcH = dlsym(libHandle, "readTasks");
  CHECK_ERROR

  if (funcH == 0) {
    KMessageBox::error(this,
                       i18n("Function not implemented"),
                       i18n("Library Error"));
  }
  else {
    ret = ((int(*)(QList<FlightTask> *))funcH)(tasks);
    if (!ret) {
      KMessageBox::error(this,
                         i18n("Cannot read tasks from recorder"),
                         i18n("Library Error"));
    }
    else {
      for (task = tasks->first(); task != 0; task = tasks->next()) {
        wpList = task->getWPList();
        for (wp = wpList.first(); wp != 0; wp = wpList.next()) {
          wp->projP = _globalMapMatrix.wgsToMap(wp->origP);
        }
        task->setWaypointList(wpList);
        emit addTask(task);
      }
      // fill task list with new tasks
      fillTaskList();
    }
  }
}

void RecorderDialog::slotWriteTasks()
{
  unsigned int *maxNrWayPointsPerTask;
  unsigned int *maxNrTasks;
  char* error;
  void* funcH;
  QString e;
  FlightTask *task;
  Waypoint *wp;
  QList<FlightTask> frTasks;
  QList<Waypoint> wpListOrig;
  QList<Waypoint> wpListCopy;
  frTasks.setAutoDelete(true);

  funcH = dlsym(libHandle, "writeTasks");
  CHECK_ERROR

  maxNrTasks = (unsigned int *)dlsym(libHandle, "maxNrTasks");
  CHECK_ERROR

  maxNrWayPointsPerTask = (unsigned int *)dlsym(libHandle, "maxNrWaypointsPerTask");
  CHECK_ERROR

  if (funcH == 0) {
    KMessageBox::error(this,
                       i18n("Function not implemented"),
                       i18n("Library Error"));
  }
  else if (maxNrTasks == 0) {
    KMessageBox::error(this,
                       i18n("Cannot obtain max number of tasks!"),
                       i18n("Library Error"));
  }
  else if (maxNrTasks == 0) {
    KMessageBox::error(this,
                       i18n("Cannot obtain max number of waypoints per task!"),
                       i18n("Library Error"));
  }
  else {
    for (task = tasks->first(); task != 0; task = tasks->next()) {
      if (frTasks.count() > *maxNrTasks) {
        e.sprintf(i18n("Maximum number of %d tasks reached!\n"
                       "Further tasks will be ignored."), *maxNrTasks);
        if (KMessageBox::warningContinueCancel(this, e, i18n("Recorder Warning"))
            == KMessageBox::Cancel) {
          return;
        }
        else {
          break;
        }
      }

      wpListOrig = task->getWPList();
      wpListCopy.clear();
      for (wp = wpListOrig.first(); wp != 0; wp = wpListOrig.next()){
        if (wpListCopy.count() > *maxNrWayPointsPerTask) {
          e.sprintf(i18n("Maximum number of turnpoints/task %d in %s reached!\n"
                         "Further turnpoints will be ignored."),
                    *maxNrWayPointsPerTask, task->getFileName().latin1());
          if (KMessageBox::warningContinueCancel(this, e, i18n("Recorder Warning"))
              == KMessageBox::Cancel) {
            return;
          }
          else {
            break;
          }
        }

        wpListCopy.append(wp);
      }
      frTasks.append(new FlightTask(wpListCopy, true, task->getFileName()));
    }

    if (!((int (*)(QList<FlightTask>*))funcH)(&frTasks)) {
      KMessageBox::error(this,
                         i18n("Cannot write tasks to recorder"),
                         i18n("Library Error"));
    }
  }
}

void RecorderDialog::slotReadWaypoints()
{
  char* error;
  void* funcH;
  int ret;
  QString e;
  QList<Waypoint> frWaypoints;
  Waypoint *wp;

  funcH = dlsym(libHandle, "readWaypoints");
  CHECK_ERROR

  if (funcH == 0) {
    KMessageBox::error(this,
                       i18n("Function not implemented"),
                       i18n("Library Error"));
  }
  else {
    ret = ((int(*)(QList<Waypoint> *))funcH)(&frWaypoints);
    if (!ret) {
      KMessageBox::error(this,
                         i18n("Cannot read waypoints from recorder"),
                         i18n("Library Error"));
    }
    else {
      WaypointCatalog *w = new WaypointCatalog(selectType->currentText() + "_" + serID->text());
      w->modified = true;
      for (wp = frWaypoints.first(); wp != 0; wp = frWaypoints.next()) {
        w->wpList.insertItem(wp);
      }

      emit addCatalog(w);
    }
  }
}

void RecorderDialog::slotWriteWaypoints()
{
  unsigned int *maxNrWaypoints;
  char* error;
  void* funcH;
  QString e;
  Waypoint *wp;
  QList<Waypoint> frWaypoints;

  funcH = dlsym(libHandle, "writeWaypoints");
  CHECK_ERROR

  maxNrWaypoints = (unsigned int *)dlsym(libHandle, "maxNrWaypoints");
  CHECK_ERROR

  if (funcH == 0) {
    KMessageBox::error(this,
                       i18n("Function not implemented"),
                       i18n("Library Error"));
  }
  else if (maxNrWaypoints == 0) {
    KMessageBox::error(this,
                       i18n("Cannot obtain max number of waypoints from lib"),
                       i18n("Library Error"));
  }
  else {
    for (wp = waypoints->first(); wp != 0; wp = waypoints->next()){
      if (frWaypoints.count() > *maxNrWaypoints) {
        e.sprintf(i18n("Maximum number of %d waypoints reached!\n"
                       "Further waypoints will be ignored."), *maxNrWaypoints);
        if (KMessageBox::warningContinueCancel(this, e, i18n("Recorder Warning"))
            == KMessageBox::Cancel) {
          return;
        }
        else {
          break;
        }
      }

      frWaypoints.append(wp);
    }

    if (!((int (*)(QList<Waypoint>*))funcH)(&frWaypoints)) {
      KMessageBox::error(this,
                         i18n("Cannot write waypoints to recorder"),
                         i18n("Library Error"));
    }
  }
}

void RecorderDialog::slotReadDatabase()
{
  void* funcH;
  char* error;
  int ret;

  funcH = dlsym(libHandle, "readDatabase");
  CHECK_ERROR

  if (funcH != 0) {
    warning("read database");
    ret = ((int (*)())funcH)();
    if (!ret) {
      KMessageBox::error(this,
                         i18n("Cannot read recorder database"),
                         i18n("Recorder Error"));
    }
    warning("read database finish");
  }
}

/** Enable/Disable pages when not connected to a recorder */
void RecorderDialog::slotEnablePages()
{
  flightPage->setEnabled(isConnected);
  waypointPage->setEnabled(isConnected);
  taskPage->setEnabled(isConnected);
  declarationPage->setEnabled(isConnected);
  pilotPage->setEnabled(isConnected);
  configPage->setEnabled(isConnected);
}

/** No descriptions */
void RecorderDialog::slotRecorderTypeChanged(const QString &name)
{
  if(isOpen && libName != name) {
    // closing old lib
    dlclose(libHandle);
    slotCloseRecorder();
    isConnected = isOpen = false;
    slotEnablePages();
  }
}

/** No descriptions */
void RecorderDialog::__addPilotPage()
{
  pilotPage = addPage(i18n("Pilots"), i18n("List of pilots"),
                      KGlobal::instance()->iconLoader()->loadIcon("pilot",
                                                                  KIcon::NoGroup,
                                                                  KIcon::SizeLarge));
}

/** No descriptions */
void RecorderDialog::__addConfigPage()
{
  configPage = addPage(i18n("Configuration"), i18n("Recorder configuration"),
                       KGlobal::instance()->iconLoader()->loadIcon("configure",
                                                                   KIcon::Panel,
                                                                   KIcon::SizeLarge));
}
