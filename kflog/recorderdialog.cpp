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
#include <unistd.h>
#include <pwd.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <knotifyclient.h>

#include <qapplication.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include "mapcalc.h"
#include "mapcontents.h"
#include "airport.h"

RecorderDialog::RecorderDialog(QWidget *parent, KConfig* cnf, const char *name)
  : KDialogBase(IconList, i18n("Flightrecorder-Dialog"), Close, Close,
                parent, name, true, true),
    config(cnf),
    loggerConf(0),
    isOpen(false),
    isConnected(false),
    activeRecorder(NULL)
{
  extern MapContents _globalMapContents;
  BaseFlightElement *e;
  Waypoint *wp;
  QPtrList<Waypoint> *tmp;
  tmp = _globalMapContents.getWaypointList();
  waypoints = new WaypointList();
  for (wp = tmp->first(); wp; wp = tmp->next()) {
    waypoints->append(wp);
  }
  waypoints->sort();

  QPtrList<BaseFlightElement> *tList = _globalMapContents.getFlightList();
  tasks = new QPtrList<FlightTask>;

  for (e = tList->first(); e; e = tList->next()) {
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
}

RecorderDialog::~RecorderDialog()
{
  config->setGroup("Recorder Dialog");
  config->writeEntry("Name", selectType->currentText());
  config->writeEntry("Port", selectPort->currentItem());
  config->writeEntry("Baud", _selectSpeed->currentItem());
  config->writeEntry("URL",  selectURL->text());
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
  selectPortLabel = new QLabel(i18n("Port"), settingsPage);
  _selectSpeed = new KComboBox(settingsPage, "baud-selection");
  selectSpeedLabel = new QLabel(i18n("Transfer speed"), settingsPage);
  selectURL = new KLineEdit(settingsPage, "URL-selection");
  selectURLLabel = new QLabel(i18n("URL"), settingsPage);

  selectPort->insertItem("ttyS0");
  selectPort->insertItem("ttyS1");
  selectPort->insertItem("ttyS2");
  selectPort->insertItem("ttyS3");
  selectPort->insertItem("ttyUSB0");
  selectPort->insertItem("ttyUSB1");

  cmdConnect = new QPushButton(i18n("Connect recorder"), settingsPage);
  cmdConnect->setMaximumWidth(cmdConnect->sizeHint().width() + 5);

  QGroupBox* infoGroup = new QGroupBox(settingsPage, "infoGroup");
  infoGroup->setTitle(i18n("Info") + ":");

  lblApiID = new QLabel(i18n("API-Version"), settingsPage);
  apiID = new QLabel(settingsPage);
  apiID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  apiID->setBackgroundMode( PaletteLight );

  lblSerID = new QLabel(i18n("Serial-Nr."), settingsPage);
  serID = new QLabel(i18n("no recorder connected"), settingsPage);
  serID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  serID->setBackgroundMode( PaletteLight );

  lblRecType = new QLabel(i18n("Recorder Type"), settingsPage);
  recType = new QLabel (settingsPage);
  recType->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  recType->setBackgroundMode( PaletteLight );

  lblPltName = new QLabel(i18n("Pilot Name"), settingsPage);
  pltName = new QLabel (settingsPage);
  pltName->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  pltName->setBackgroundMode( PaletteLight );

  lblGldType = new QLabel(i18n("Glider Type"), settingsPage);
  gldType = new QLabel (settingsPage);
  gldType->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  gldType->setBackgroundMode( PaletteLight );

  lblGldID = new QLabel(i18n("Glider ID"), settingsPage);
  gldID = new QLabel (settingsPage);
  gldID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  gldID->setBackgroundMode( PaletteLight );

  compID = new QLabel (settingsPage);
  compID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  compID->setBackgroundMode( PaletteLight );

  sLayout->addMultiCellWidget(sGroup, 0, 6, 0, 7);
  sLayout->addWidget(new QLabel(i18n("Type"), settingsPage), 1, 1, AlignRight);
  sLayout->addMultiCellWidget(selectType, 1, 1, 2, 6);
  sLayout->addWidget(selectPortLabel, 3, 1, AlignRight);
  sLayout->addMultiCellWidget(selectPort, 3, 3, 2, 3);
  sLayout->addWidget(selectSpeedLabel, 3, 4, AlignRight);
  sLayout->addMultiCellWidget(_selectSpeed, 3, 3, 5, 6);
  sLayout->addWidget(selectURLLabel, 3, 1, AlignRight);
  sLayout->addMultiCellWidget(selectURL, 3, 3, 2, 6);
  __setRecorderConnectionType(FlightRecorderPluginBase::none);

  sLayout->addWidget(cmdConnect, 5, 6, AlignRight);

  sLayout->addMultiCellWidget(infoGroup, 8, 14, 0, 7);
  sLayout->addWidget(lblApiID, 9, 1, AlignRight);
  sLayout->addMultiCellWidget(apiID, 9, 9, 2, 3);
  sLayout->addWidget(lblSerID, 11, 1, AlignRight);
  sLayout->addMultiCellWidget(serID, 11, 11, 2, 3);
  sLayout->addWidget(lblRecType, 13, 1, AlignRight);
  sLayout->addMultiCellWidget(recType, 13, 13, 2, 3);

  sLayout->addWidget(lblPltName, 9, 4, AlignRight);
  sLayout->addMultiCellWidget(pltName, 9, 9, 5, 6);
  sLayout->addWidget(lblGldType, 11, 4, AlignRight);
  sLayout->addMultiCellWidget(gldType, 11, 11, 5, 6);
  sLayout->addWidget(lblGldID, 13, 4, AlignRight);
  sLayout->addMultiCellWidget(gldID, 13, 13, 5, 5);
  sLayout->addMultiCellWidget(compID, 13, 13, 6, 6);

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
  _selectSpeed->setCurrentItem(config->readNumEntry("Baud", 0));
  QString name(config->readEntry("Name", 0));
  QString pluginName;
  selectURL->setText(config->readEntry("URL", ""));
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
    pluginName=loggerConf->readEntry("Name");
    selectType->insertItem(pluginName);
    libNameList.insert(pluginName, new QString(loggerConf->readEntry("LibName")));
    typeLoop++;
  }

  //sort if this style uses a listbox for the combobox
  if (selectType->listBox()) selectType->listBox()->sort();

  selectType->setCurrentText(name);
  slotRecorderTypeChanged(selectType->text(typeID));

  connect(cmdConnect, SIGNAL(clicked()), SLOT(slotConnectRecorder()));
}

void RecorderDialog::__setRecorderConnectionType(FlightRecorderPluginBase::TransferMode mode){
  selectPort->hide();
  selectPortLabel->hide();
  _selectSpeed->hide();
  selectSpeedLabel->hide();
  selectURL->hide();
  selectURLLabel->hide();
  cmdConnect->setEnabled(false);

  switch(mode){
    case FlightRecorderPluginBase::serial:
      selectPort->show();
      selectPortLabel->show();
      _selectSpeed->show();
      selectSpeedLabel->show();
      cmdConnect->setEnabled(true);
      break;
    case FlightRecorderPluginBase::URL:
      selectURL->show();
      selectURLLabel->show();
      cmdConnect->setEnabled(true);
      break;
    default:
      break; //nothing to be done.
  }
}

void RecorderDialog::__setRecorderCapabilities()
{
  FlightRecorderPluginBase::FR_Capabilities cap = activeRecorder->capabilities();
  if (cap.supDspSerialNumber) {
    serID->show();
    lblSerID->show();
  } else {
    serID->hide();
    lblSerID->hide();
  }
  if (cap.supDspRecorderType) {
    recType->show();
    lblRecType->show();
  } else {
    recType->hide();
    lblRecType->hide();
  }
  if (cap.supDspPilotName) {
    pltName->show();
    lblPltName->show();
  } else {
    pltName->hide();
    lblPltName->hide();
  }
  if (cap.supDspGliderType) {
    gldType->show();
    lblGldType->show();
  } else {
    gldType->hide();
    lblGldType->hide();
  }
  if (cap.supDspGliderID) {
    gldID->show();
    lblGldID->show();
  } else {
    gldID->hide();
    lblGldID->hide();
  }
  if (cap.supDspCompetitionID) {
    compID->show();
  } else {
    compID->hide();
  }

  if (cap.supAutoSpeed)
    selectSpeedLabel->setText(i18n("Transfer speed:\n(automatic)"));
  else
    selectSpeedLabel->setText(i18n("Transfer speed:"));
  _selectSpeed->setEnabled (!cap.supAutoSpeed);

  _selectSpeed->clear();
  // insert highest speed first
  for (int i = FlightRecorderPluginBase::transferDataMax-1; i >= 0; i--)
  {
    if ((FlightRecorderPluginBase::transferData[i]._bps & cap.transferSpeeds) ||
        (cap.transferSpeeds == FlightRecorderPluginBase::bps00000))
      _selectSpeed->insertItem(QString("%1").arg(FlightRecorderPluginBase::transferData[i]._speed));
  }
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
  colDuration = flightList->addColumn(i18n("Duration"));

//  flightList->setColumnAlignment(colID, AlignRight);

  flightList->loadConfig();

  QPushButton* listB = new QPushButton(i18n("load list"), flightPage);
  QPushButton* fileB = new QPushButton(i18n("save flight"), flightPage);
  useLongNames = new QCheckBox(i18n("long filenames"), flightPage);
  // let's prefer short filenames. These are needed for OLC
  useLongNames->setChecked(false);
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
  gliderID = new KLineEdit(declarationPage, "gliderID");
  gliderType = new KComboBox(declarationPage, "gliderType");
  gliderType->setEditable(true);
  editCompID = new KLineEdit(declarationPage, "compID");
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
  tLayout->addWidget(editCompID, 6, 2);
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

  int idx = 0;
#include <gliders.h>
  while(gliderList[idx].index != -1) {
    gliderType->insertItem(QString(gliderList[idx++].name));
  }

  config->setGroup("Personal Data");
  pilotName->setText(config->readEntry("PilotName", ""));

  config->setGroup(0);

  for (e = tasks->first(); e; e = tasks->next()) {
    taskSelection->insertItem(e->getFileName() + " " + e->getTaskTypeString());
  }

  if (tasks->count()) {
    slotSwitchTask(0);
  }
  else {
    warning("No tasks planned ...");
// Isn't it possible to write an declaration without a task?
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
  cmdUploadTasks = new QPushButton(i18n("write tasks to recorder"), taskPage);
  connect(cmdUploadTasks, SIGNAL(clicked()), SLOT(slotWriteTasks()));
  buttons->addWidget(cmdUploadTasks);

  cmdDownloadTasks = new QPushButton(i18n("read tasks from recorder"), taskPage);
  connect(cmdDownloadTasks, SIGNAL(clicked()), SLOT(slotReadTasks()));
  buttons->addWidget(cmdDownloadTasks);

  top->addWidget(taskList);
  top->addLayout(buttons);

  fillTaskList();
}

void RecorderDialog::fillTaskList()
{
  int loop = 1;
  QString idS;

  taskList->clear();
  for (FlightTask* task = tasks->first(); task; task = tasks->next()){
    QListViewItem *item = new QListViewItem(taskList);
    idS.sprintf("%3d", loop++);
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
//  QPushButton *b;

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
  cmdUploadWaypoints = new QPushButton(i18n("write waypoints to recorder"), waypointPage);
  connect(cmdUploadWaypoints, SIGNAL(clicked()), SLOT(slotWriteWaypoints()));
  buttons->addWidget(cmdUploadWaypoints);

  cmdDownloadWaypoints = new QPushButton(i18n("read waypoints from recorder"), waypointPage);
  connect(cmdDownloadWaypoints, SIGNAL(clicked()), SLOT(slotReadWaypoints()));
  buttons->addWidget(cmdDownloadWaypoints);

  top->addWidget(waypointList);
  top->addLayout(buttons);

  for (wp = waypoints->first(); wp; wp = waypoints->next()){
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
  if (!activeRecorder)
    return;
  portName = "/dev/" + selectPort->currentText();
  //QStringList::Iterator it = libNameList.at(selectType->currentItem());
  //QString name = (*it).latin1();
  QString name=*libNameList[selectType->currentText()];
  int speed = _selectSpeed->currentText().toInt();

  if(!__openLib(name)) {
    warning(i18n("Could not open lib!"));
    return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  //check if we have valid parameters, is so, try to connect!
  switch (activeRecorder->getTransferMode()) {
  case FlightRecorderPluginBase::serial:
    if(portName.isEmpty()) {
      warning(i18n("No port given!"));
      isConnected=false;
      break;
    }
    isConnected=(activeRecorder->openRecorder(portName.latin1(),speed)>=FR_OK);
    break;
  case FlightRecorderPluginBase::URL:
  {
    selectURL->setText(selectURL->text().stripWhiteSpace());
    QString URL = selectURL->text();
    if (URL.isEmpty()) {
      warning(i18n("No URL entered!"));
      QApplication::restoreOverrideCursor();
      isConnected=false;
      break;
    };
    isConnected=(activeRecorder->openRecorder(URL)>=FR_OK);
    break;
  }
  default:
    QApplication::restoreOverrideCursor();
    isConnected=false;
    return; //If it's not one of the above, we don't know the connection method, so how can we connect?!
  }

  if (isConnected)
  {
    connect (activeRecorder, SIGNAL(newSpeed(int)),this,SLOT(slotNewSpeed(int)));
    slotEnablePages();
    slotReadDatabase();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    QApplication::restoreOverrideCursor();
    QString errorDetails = errorDetails=activeRecorder->lastError();

    if (!errorDetails.isEmpty()) {
      KMessageBox::detailedSorry(this,
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."),
                         errorDetails,
                         i18n("Recorder Connection"));       //Using the Sorry box is a bit friendlier than Error...
    } else {
      KMessageBox::sorry(this,
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."),
                         i18n("Recorder Connection"));       //Using the Sorry box is a bit friendlier than Error...
    }
  }
}


void RecorderDialog::slotCloseRecorder()
{
  if(activeRecorder) {
    qDebug("A recorder is active. Checking connection.");
    if (activeRecorder->isConnected()) {
      qDebug("Recorder is connected. Closing...");
      if (activeRecorder)
        activeRecorder->closeRecorder();
    }
    qDebug("Going to close recorder object...");
    delete activeRecorder;  // -> seems to cause a segfault? But won't leaving it cause a memoryleak?!
    // it seems to be important to do a make install after the recorder libraries have been changed.
    // I could not detect a segfault. Works fine now. Eggert
    activeRecorder = NULL;
    qDebug("Done.");
  }
}

void RecorderDialog::slotReadFlightList()
{

  if (!activeRecorder)
    return;

    QMessageBox* statusDlg = new QMessageBox ( i18n("downloading flightlist"), i18n("downloading flightlist"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);

  statusDlg->show();

  // Jetzt muss das Flugverzeichnis vom Logger gelesen werden!
  // Now we need to read the flightdeclaration from the logger!

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  flightList->clear();

  kapp->processEvents();
  if (__fillDirList() == FR_ERROR) {
    QApplication::restoreOverrideCursor();
    QString errorDetails = activeRecorder->lastError();
    if (!errorDetails.isEmpty()) {
      KMessageBox::detailedError(this,
                                 i18n("There was an error reading the flight list!"),
                                 errorDetails,
                                 i18n("Library Error"));
    } else {
      KMessageBox::error(this,
                         i18n("There was an error reading the flight list!"),
                         i18n("Library Error"));
    }
    return;
  }

  QString day;
  QString idS;

  if (dirList.count()==0) {
    delete statusDlg;
    QApplication::restoreOverrideCursor();
    KMessageBox::sorry(this,
                            i18n("There were no flights recorded in the recorder."),
                            i18n("Download result"));
    return;
  }

  for(unsigned int loop = 0; loop < dirList.count(); loop++) {
    FRDirEntry* e = dirList.at(loop);
    QListViewItem* item = new QListViewItem(flightList);
    idS.sprintf("%3d", loop + 1);
    item->setText(colID, idS);
    day.sprintf("%d-%.2d-%.2d", e->firstTime.tm_year + 1900,
                e->firstTime.tm_mon + 1, e->firstTime.tm_mday);
    item->setText(colDate, day);
    item->setText(colPilot, e->pilotName);
    item->setText(colGlider, e->gliderID);
    QTime time (e->firstTime.tm_hour, e->firstTime.tm_min,e->firstTime.tm_sec);
    item->setText(colFirstPoint, KGlobal::locale()->formatTime(time, true));
    time = QTime(e->lastTime.tm_hour, e->lastTime.tm_min,e->lastTime.tm_sec);
    item->setText(colLastPoint, KGlobal::locale()->formatTime(time, true));
    time = QTime().addSecs (e->duration);
    item->setText(colDuration, KGlobal::locale()->formatTime(time, true));
  }
  QApplication::restoreOverrideCursor();

  delete statusDlg;
}

void RecorderDialog::slotDownloadFlight()
{
  QListViewItem *item = flightList->currentItem();
  int ret;
  QString errorDetails;

  if(item == 0) {
    return;
  }

  config->setGroup("Path");
  // If no DefaultFlightDirectory is configured, we must use $HOME instead of the root-directory
  QString flightDir = config->readEntry("DefaultFlightDirectory", getpwuid(getuid())->pw_dir) + "/";

  QString fileName = flightDir;

  int flightID(item->text(colID).toInt() - 1);

  warning("Loading flight %d (%d)", flightID, flightList->itemPos(item));
  warning(dirList.at(flightID)->longFileName);
  warning(dirList.at(flightID)->shortFileName);

//  QTimer::singleShot( 0, this, SLOT(slotDisablePages()) );


  if(useLongNames->isChecked()) {
    fileName += dirList.at(flightID)->longFileName.upper();
  }
  else {
    fileName += dirList.at(flightID)->shortFileName.upper();
  }

  KFileDialog* dlg = new KFileDialog(flightDir, "*.igc *.IGC ", this,
         i18n("Select IGC File"), true);
  dlg->setSelection( fileName );
  dlg->setOperationMode( KFileDialog::Saving );
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

  if(fUrl.isLocalFile())
    fileName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fileName))
   {
     KNotifyClient::event(i18n("Can not download file %1").arg(fUrl.url()));
     return;
   }


  QMessageBox* statusDlg = new QMessageBox ( i18n("downloading flight"), i18n("downloading flight"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  kapp->processEvents();

  warning(fileName);

  if (!activeRecorder) return;


  kapp->processEvents();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  ret = activeRecorder->downloadFlight(flightID,!useFastDownload->isChecked(),fileName);

  kapp->processEvents();
  QApplication::restoreOverrideCursor();
  if (ret == FR_ERROR) {
    warning("ERROR");
    if ((errorDetails = activeRecorder->lastError()) != "") {
      warning(errorDetails);
      KMessageBox::detailedError(this,
                                 i18n("There was an error downloading the flight!"),
                                 errorDetails,
                                 i18n("Library Error"));
    } else {
      KMessageBox::error(this,
                         i18n("There was an error downloading the flight!"),
                         i18n("Library Error"));
    }
  }
  //TODO: handle returnvalues!

  delete statusDlg;

  slotEnablePages();
}

void RecorderDialog::slotWriteDeclaration()
{
  QMessageBox* statusDlg = new QMessageBox ( i18n("send declaration"), i18n("send flightdeclaration to the recorder"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  int ret;
  FRTaskDeclaration taskDecl;
  taskDecl.pilotA = pilotName->text();
  taskDecl.pilotB = copilotName->text();
  taskDecl.gliderID = gliderID->text();
  taskDecl.gliderType = gliderType->currentText();
  taskDecl.compID = editCompID->text();
  taskDecl.compClass = compClass->text();
  QString errorDetails;

  if (!activeRecorder) return;
  kapp->processEvents();
  if (!activeRecorder->capabilities().supUlDeclaration) {
    KMessageBox::sorry(this,
                       i18n("Function not implemented"),
                       i18n("Declaration upload"));
    return;
  }


  if (taskSelection->currentItem() >= 0) {
    QPtrList<Waypoint> wpList = tasks->at(taskSelection->currentItem())->getWPList();

    warning("Writing task to logger...");

    ret=activeRecorder->writeDeclaration(&taskDecl,&wpList);

    //evaluate result
    if (ret==FR_NOTSUPPORTED) {
      KMessageBox::sorry(this,
                         i18n("Function not implemented"),
                         i18n("Declaration upload"));
      return;
    }

    if (ret==FR_ERROR) {
      if ((errorDetails=activeRecorder->lastError())!="") {
        KMessageBox::detailedError(this,
                           i18n("There was an error writing the declaration!"),
                           errorDetails,
                           i18n("Library Error"));
      } else {
        KMessageBox::error(this,
                           i18n("There was an error writing the declaration!"),
                           i18n("Library Error"));
      }
      return;
    }

    warning("   ... ready (%d)", ret);
    KMessageBox::information(this,
                       i18n("The declaration was uploaded to the recorder."),
                       i18n("Declaration upload"),
                       "ShowDeclarationUploadSuccesReport");
  }

  delete statusDlg;

}

int RecorderDialog::__fillDirList()
{
  if (!activeRecorder) return FR_ERROR;
  if (activeRecorder->capabilities().supDlFlight) {
    return activeRecorder->getFlightDir(&dirList);
  } else {
    return FR_NOTSUPPORTED;
  }
}

bool RecorderDialog::__openLib(const QString& libN)
{
  warning("__openLib(%s)", (const char*) libN);
  char* error;

  if (libName==libN) {
    warning("OK, Lib allready open.");
    return true;
  }
  qDebug("Opening lib %s...",libN.latin1());

  libName = "";
  apiID->setText("");
  serID->setText("");
  recType->setText("");
  pltName->setText("");
  gldType->setText("");
  gldID->setText("");
  compID->setText("");

  libHandle = dlopen(KGlobal::dirs()->findResource("lib", libN), RTLD_NOW);

  error = (char *)dlerror();
  if (error != NULL)
  {
    warning(error);
    return false;
  }

  FlightRecorderPluginBase* (*getRecorder)();
  getRecorder = (FlightRecorderPluginBase* (*) ()) dlsym(libHandle, "getRecorder");
  if (!getRecorder) {
    warning("getRecorder funtion not defined in lib. Lib can't be used.");
    return false;
  }

  activeRecorder=getRecorder();

  if(!activeRecorder) {
    warning("No recorder object returned!");
    return false;
  }

  apiID->setText(activeRecorder->getLibName());

  isOpen = true;
  libName=libN;

  return true;
}

void RecorderDialog::slotSwitchTask(int idx)
{
  FlightTask *task = tasks->at(idx);
  QString idS;

  declarationList->clear();
  if(task) {
    QPtrList<Waypoint> wpList = ((FlightTask*)task)->getWPList();
    int loop = 1;
    for(Waypoint *wp = wpList.first(); wp; wp = wpList.next()) {
      QListViewItem* item = new QListViewItem(declarationList);
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
  QMessageBox* statusDlg = new QMessageBox ( "Lade die Aufgaben herunter", "Lade die Aufgaben herunter",
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  FlightTask *task;
  Waypoint *wp;
  QPtrList<Waypoint> wpList;
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;
  int ret;
  int cnt=0;
  QString errorDetails;

  if (!activeRecorder) return;
  if (!activeRecorder->capabilities().supDlTask) {
    KMessageBox::sorry(this,
                       i18n("Function not implemented"),
                       i18n("Task download"));
    return;
  }

  kapp->processEvents();
  ret = activeRecorder->readTasks(tasks);
  if (ret<FR_OK) {
    if ((errorDetails=activeRecorder->lastError())!="") {
      KMessageBox::detailedError(this,
                         i18n("Cannot read tasks from recorder"),
                         errorDetails,
                         i18n("Library Error"));
    } else {
      KMessageBox::error(this,
                         i18n("Cannot read tasks from recorder"),
                         i18n("Library Error"));
    }
  }
  else {
    for (task = tasks->first(); task; task = tasks->next()) {
      wpList = task->getWPList();
      // here we overwrite the original task name to get a unique internal name
      task->setTaskName(_globalMapContents.genTaskName());
      for (wp = wpList.first(); wp; wp = wpList.next()) {
        wp->projP = _globalMapMatrix.wgsToMap(wp->origP);
      }
      task->setWaypointList(wpList);
      emit addTask(task);
      cnt++;
    }
    // fill task list with new tasks
    fillTaskList();
    KMessageBox::information(this,
                       i18n("%1 tasks were downloaded from the recorder.").arg(cnt),
                       i18n("Task download"),
                       "ShowTaskDownloadSuccesReport");

  }



  delete statusDlg;
}

void RecorderDialog::slotWriteTasks()
{
  QMessageBox* statusDlg = new QMessageBox ( i18n("send tasks to recorder"), i18n("send tasks to recorder"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();


  unsigned int maxNrWayPointsPerTask;
  unsigned int maxNrTasks;
  QString e;
  FlightTask *task;
  Waypoint *wp;
  QPtrList<FlightTask> frTasks;
  QPtrList<Waypoint> wpListOrig;
  QPtrList<Waypoint> wpListCopy;
  frTasks.setAutoDelete(true);
  int cnt=0;
  QString errorDetails;

  if (!activeRecorder) return;
  if (!activeRecorder->capabilities().supUlTask) {
    KMessageBox::sorry(this,
                       i18n("Function not implemented"),
                       i18n("Task upload"));
    return;
  }

  maxNrTasks = activeRecorder->capabilities().maxNrTasks;

  maxNrWayPointsPerTask = activeRecorder->capabilities().maxNrWaypointsPerTask;

  if (maxNrTasks == 0) {
    KMessageBox::error(this,
                       i18n("Cannot obtain max number of tasks!"),
                       i18n("Library Error"));
  }
  else if (maxNrWayPointsPerTask == 0) {
    KMessageBox::error(this,
                       i18n("Cannot obtain max number of waypoints per task!"),
                       i18n("Library Error"));
  }
  else {
    for (task = tasks->first(); task; task = tasks->next()) {
      if (frTasks.count() > maxNrTasks) {
        e.sprintf(i18n("Maximum number of %d tasks reached!\n"
                       "Further tasks will be ignored."), maxNrTasks);
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
      for (wp = wpListOrig.first(); wp; wp = wpListOrig.next()){
        if (wpListCopy.count() > maxNrWayPointsPerTask) {
          e.sprintf(i18n("Maximum number of turnpoints/task %d in %s reached!\n"
                         "Further turnpoints will be ignored."),
                    maxNrWayPointsPerTask, task->getFileName().latin1());
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
      cnt++;
      frTasks.append(new FlightTask(wpListCopy, true, task->getFileName()));

    }

    if (!activeRecorder->writeTasks(&frTasks)) {
      if ((errorDetails=activeRecorder->lastError())!="") {
        KMessageBox::detailedError(this,
                           i18n("Cannot write tasks to recorder"),
                           errorDetails,
                           i18n("Library Error"));
      } else {
        KMessageBox::error(this,
                           i18n("Cannot write tasks to recorder"),
                           i18n("Library Error"));
      }
    } else {
      KMessageBox::information(this,
                         i18n("%1 tasks were uploaded to the recorder.").arg(cnt),
                         i18n("Task upload"),
                         "ShowTaskUploadSuccesReport");
    }
  }

  delete statusDlg;
}

void RecorderDialog::slotReadWaypoints()
{
  int ret;
  int cnt=0;
  QString e;
  QPtrList<Waypoint> frWaypoints;
  Waypoint *wp;
  QString errorDetails;

  if (!activeRecorder) return;
  if (!activeRecorder->capabilities().supDlWaypoint) {
    KMessageBox::sorry(this,
                       i18n("Function not implemented"),
                       i18n("Waypoint download"));
    return;
  }

  QMessageBox* statusDlg = new QMessageBox (i18n("Reading waypoints"), i18n("Reading Waypoints"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  kapp->processEvents();

  ret = activeRecorder->readWaypoints(&frWaypoints);
  if (ret<FR_OK) {
    QApplication::restoreOverrideCursor();

    if ((errorDetails=activeRecorder->lastError())!="") {
      KMessageBox::detailedError(this,
                         i18n("Cannot read waypoints from recorder"),
                         errorDetails,
                         i18n("Library Error"));
    } else {
      KMessageBox::error(this,
                         i18n("Cannot read waypoints from recorder"),
                         i18n("Library Error"));
    }

  } else {
    WaypointCatalog *w = new WaypointCatalog(selectType->currentText() + "_" + serID->text());
    w->modified = true;
    for (wp = frWaypoints.first(); wp; wp = frWaypoints.next()) {
      w->wpList.insertItem(wp);
      cnt++;
    }

    QApplication::restoreOverrideCursor();
    emit addCatalog(w);
    KMessageBox::information(this,
                       i18n("%1 waypoints have been downloaded from the recorder.").arg(cnt),
                       i18n("Waypoint download"),
                       "ShowWaypointDownloadSuccesReport");

  }

  delete statusDlg;
}

void RecorderDialog::slotWriteWaypoints()
{
  QMessageBox* statusDlg = new QMessageBox ( i18n("send waypoints"), i18n("send waypoints"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  unsigned int maxNrWaypoints;
  QString e;
  Waypoint *wp;
  QPtrList<Waypoint> frWaypoints;
  int cnt=0;
  QString errorDetails;

  if (KMessageBox::warningContinueCancel(this,
                                         i18n("Uploading waypoints to the recorder will overwrite existing waypoints on the recorder. Do want to continue uploading?"),
                                         i18n("Waypoints upload"),
                                         i18n("Upload"),
                                         "showWaypointUploadConfirmation") == KMessageBox::Cancel)
    return;

  if (!activeRecorder) return;

  if (!activeRecorder->capabilities().supUlWaypoint) {
    KMessageBox::sorry(this,
                       i18n("Function not implemented"),
                       i18n("Waypoint upload"));
    return;
  }

  maxNrWaypoints = activeRecorder->capabilities().maxNrWaypoints;

  if (maxNrWaypoints == 0) {
    KMessageBox::error(this,
                       i18n("Cannot obtain maximum number of waypoints from lib."),
                       i18n("Library Error"));
  }
  else {
    for (wp = waypoints->first(); wp; wp = waypoints->next()){
      if (frWaypoints.count() > maxNrWaypoints) {
        e.sprintf(i18n("Maximum number of %d waypoints reached!\n"
                       "Further waypoints will be ignored."), maxNrWaypoints);
        if (KMessageBox::warningContinueCancel(this, e, i18n("Recorder Warning"))
            == KMessageBox::Cancel) {
          return;
        }
        else {
          break;
        }
      }

      frWaypoints.append(wp);
      cnt++;
    }

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    if (!activeRecorder->writeWaypoints(&frWaypoints)) {
      QApplication::restoreOverrideCursor();
      if ((errorDetails=activeRecorder->lastError())!="") {
        KMessageBox::detailedError(this,
                           i18n("Cannot write waypoints to recorder."),
                           errorDetails,
                           i18n("Library Error"));
      } else {
        KMessageBox::error(this,
                           i18n("Cannot write waypoints to recorder."),
                           i18n("Library Error"));
      }
    } else {
      QApplication::restoreOverrideCursor();

      KMessageBox::information(this,
                         QString(i18n("%1 waypoints have been uploaded to the recorder.")).arg(cnt),
                         i18n("Waypoint upload"),
                         "ShowWaypointUploadSuccesReport");
    }
  }

  delete statusDlg;
}

void RecorderDialog::slotReadDatabase()
{
  if (!activeRecorder) return;
  FlightRecorderPluginBase::FR_Capabilities cap = activeRecorder->capabilities();

  FlightRecorderPluginBase::FR_BasicData dat;
  int ret = activeRecorder->getBasicData(dat);
  if (ret == FR_OK)
  {
    if (cap.supDspSerialNumber)
      serID->setText(dat.serialNumber);
    if (cap.supDspRecorderType)
      recType->setText(dat.recorderType);
    if (cap.supDspPilotName)
      pltName->setText(dat.pilotName);
    if (cap.supDspGliderType)
      gldType->setText(dat.gliderType);
    if (cap.supDspGliderID)
      gldID->setText(dat.gliderID);
    if (cap.supDspCompetitionID)
      compID->setText(dat.competitionID);
  }
  else
  {
    QString errorDetails=activeRecorder->lastError();
    if (!errorDetails.isEmpty()) {
      KMessageBox::detailedSorry(this,
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."),
                         errorDetails,
                         i18n("Recorder Connection"));       //Using the Sorry box is a bit friendlier than Error...
    } else {
      KMessageBox::sorry(this,
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."),
                         i18n("Recorder Connection"));       //Using the Sorry box is a bit friendlier than Error...
    }
  }
}

void RecorderDialog::slotDisablePages()
{
  flightPage->setEnabled(false);
  waypointPage->setEnabled(false);
  taskPage->setEnabled(false);
  declarationPage->setEnabled(false);
}

/** Enable/Disable pages when (not) connected to a recorder */
void RecorderDialog::slotEnablePages()
{
  //first, disable all pages
  flightPage->setEnabled(false);
  waypointPage->setEnabled(false);
  taskPage->setEnabled(false);
  declarationPage->setEnabled(false);
  //pilotPage->setEnabled(false);
  //configPage->setEnabled(false);

  //Then, if there is an active recorder, and that recorder is connected,
  //  selectively re-activate them.
  if (!activeRecorder) return;
  FlightRecorderPluginBase::FR_Capabilities cap=activeRecorder->capabilities();
  if (isConnected) {
    //flightpage
    if (cap.supDlFlight) {
      flightPage->setEnabled(true);
      useFastDownload->setEnabled (cap.supSignedFlight);
    }

    //waypointpage
    if (cap.supDlWaypoint || cap.supUlWaypoint ) {
      waypointPage->setEnabled(true);
      cmdUploadWaypoints->setEnabled (cap.supUlWaypoint);
      cmdDownloadWaypoints->setEnabled (cap.supDlWaypoint);
    }

    //taskpage
    if (cap.supDlTask || cap.supUlTask ) {
      taskPage->setEnabled(true);
      cmdUploadTasks->setEnabled (cap.supUlTask);
      cmdDownloadTasks->setEnabled (cap.supDlTask);
    }

    //declarationpage
    if (cap.supUlDeclaration) {
      declarationPage->setEnabled(true);
    }

    //pilotpage  -  not available yet
    //configpage -  not available yet
  }

}

/** No descriptions */
void RecorderDialog::slotRecorderTypeChanged(const QString&) // name)
{
  if(selectType->currentText().isEmpty())  return;

  QString name = *(libNameList[selectType->currentText()]);

  if(isOpen && libName != name) {
    // closing old lib
    dlclose(libHandle);
    slotCloseRecorder();
    isConnected = isOpen = false;
    slotEnablePages();

  }
  if(!__openLib(name)) {
    warning(i18n("Could not open lib!"));
    __setRecorderConnectionType(FlightRecorderPluginBase::none);
    return;
  }
  __setRecorderConnectionType(activeRecorder->getTransferMode());
  __setRecorderCapabilities();

}

/**
  *  If the recorder supports auto-detection of transfer speed,
  *  it will signal the new speed to adjust the gui
  */
void RecorderDialog::slotNewSpeed (int speed)
{
  _selectSpeed->setCurrentText(QString("%1").arg(speed));
}

/** No descriptions */
void RecorderDialog::__addPilotPage()
{

  return; //We are not going to do that now, since there is nothing to display

/*  pilotPage = addPage(i18n("Pilots"), i18n("List of pilots"),
                      KGlobal::instance()->iconLoader()->loadIcon("pilot", KIcon::NoGroup, KIcon::SizeLarge));
*/
  pilotPage = addPage(i18n("Pilots"), i18n("List of pilots"),
                      KGlobal::instance()->iconLoader()->loadIcon("identity",
                                                                  KIcon::NoGroup,
                                                                  KIcon::SizeLarge));
}

/** No descriptions */
void RecorderDialog::__addConfigPage()
{
  return; //We are not going to do that now, since there is nothing to display

  configPage = addPage(i18n("Configuration"), i18n("Recorder configuration"),
                       KGlobal::instance()->iconLoader()->loadIcon("configure",
                                                                   KIcon::Panel,
                                                                   KIcon::SizeLarge));
}
