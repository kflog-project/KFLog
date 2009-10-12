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
#include <qdir.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include "mapcalc.h"
#include "mapcontents.h"
#include "airport.h"

RecorderDialog::RecorderDialog(QWidget *parent, const char *name)
  : KDialogBase(IconList, i18n("Flightrecorder-Dialog"), Close, Close,
                parent, name, true, true),
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
  extern QSettings _settings;

  _settings.writeEntry("/RecorderDialog/Name", selectType->currentText());
  _settings.writeEntry("/RecorderDialog/Port", selectPort->currentItem());
  _settings.writeEntry("/RecorderDialog/Baud", _selectSpeed->currentItem());
  _settings.writeEntry("/RecorderDialog/URL",  selectURL->text());

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
  // the following devices are used for usb adapters
  selectPort->insertItem("ttyUSB0");   // classical device
  selectPort->insertItem("tts/USB0");  // devfs
  selectPort->insertItem("usb/tts/0"); // udev
  // we never know if the device name will change again; let the user have a chance 
  selectPort->setEditable(true);

  cmdConnect = new QPushButton(i18n("Connect recorder"), settingsPage);
  cmdConnect->setMaximumWidth(cmdConnect->sizeHint().width() + 5);

  QGroupBox* infoGroup = new QGroupBox(settingsPage, "infoGroup");
  infoGroup->setTitle(i18n("Info") + ":");

  lblApiID = new QLabel(i18n("API-Version"), settingsPage);
  apiID = new QLabel(settingsPage);
  apiID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  apiID->setBackgroundMode( PaletteLight );
  apiID->setEnabled(false);

  lblSerID = new QLabel(i18n("Serial-Nr."), settingsPage);
  serID = new QLabel(i18n("no recorder connected"), settingsPage);
  serID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  serID->setBackgroundMode( PaletteLight );
  serID->setEnabled(false);

  lblRecType = new QLabel(i18n("Recorder Type"), settingsPage);
  recType = new QLabel (settingsPage);
  recType->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  recType->setBackgroundMode( PaletteLight );
  recType->setEnabled(false);

  lblPltName = new QLabel(i18n("Pilot Name"), settingsPage);
  pltName = new KLineEdit (settingsPage, "pltName");
  pltName->setEnabled(false);

  lblGldType = new QLabel(i18n("Glider Type"), settingsPage);
  gldType = new KLineEdit (settingsPage, "gldType");
  gldType->setEnabled(false);

  lblGldID = new QLabel(i18n("Glider ID"), settingsPage);
  gldID = new KLineEdit (settingsPage, "gldID");
  gldID->setEnabled(false);

  cmdUploadBasicConfig = new QPushButton(i18n("write config to recorder"), settingsPage);
  // disable this button until we read the information from the flight recorder:
  cmdUploadBasicConfig->setEnabled(false);
  connect(cmdUploadBasicConfig, SIGNAL(clicked()), SLOT(slotWriteConfig()));

  compID = new QLabel (settingsPage);
  compID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  compID->setBackgroundMode( PaletteLight );
  compID->setEnabled(false);

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

  sLayout->addWidget(cmdUploadBasicConfig, 15, 5, 6);

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
    QMessageBox::critical(this,
                       i18n("No recorders installed."),
                       i18n("There are no recorder-libraries installed."), QMessageBox::Ok, 0);
  }

  libNameList.clear();

  extern QSettings _settings;

  selectPort->setCurrentItem(_settings.readNumEntry("/RecorderDialog/Port", 0));
  _selectSpeed->setCurrentItem(_settings.readNumEntry("/RecorderDialog/Baud", 0));
  QString name(_settings.readEntry("/RecorderDialog/Name", 0));
  QString pluginName;
  selectURL->setText(_settings.readEntry("/RecorderDialog/URL", ""));

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

  if (cap.supEditGliderID     ||
      cap.supEditGliderType   ||
      cap.supEditPilotName) {
    cmdUploadBasicConfig->show();
  } else {
    cmdUploadBasicConfig->hide();
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
  tLayout->addWidget(new QLabel(i18n("Pilot") + " 1:", declarationPage), 2, 0,
                     AlignRight);
  tLayout->addWidget(pilotName, 2, 2);
  tLayout->addWidget(new QLabel(i18n("Pilot") + " 2:", declarationPage), 2, 4,
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

  extern QSettings _settings;

  pilotName->setText(_settings.readEntry("/PersonalData/PilotName", ""));

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
    warning("%s", (const char*)i18n("Could not open lib!"));
    return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  //check if we have valid parameters, is so, try to connect!
  switch (activeRecorder->getTransferMode()) {
  case FlightRecorderPluginBase::serial:
    if(portName.isEmpty()) {
      warning("%s", (const char*)i18n("No port given!"));
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
      warning("%s", (const char*)i18n("No URL entered!"));
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
    QString errorDetails=activeRecorder->lastError();

    if (!errorDetails.isEmpty()) {
      KMessageBox::detailedSorry(this,
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."),
                         errorDetails,
                         i18n("Recorder Connection"));       //Using the Sorry box is a bit friendlier than Error...
    } else {
      QMessageBox::warning(this,
                         i18n("Recorder Connection"),
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."), QMessageBox::Ok, 0);       //Using the Sorry box is a bit friendlier than Error...
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

  // Now we need to read the flight list from the logger!

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  flightList->clear();

  kapp->processEvents();
  if (__fillDirList() == FR_ERROR) {
    delete statusDlg;  
    QApplication::restoreOverrideCursor();
    QString errorDetails = activeRecorder->lastError();
    if (!errorDetails.isEmpty()) {
      KMessageBox::detailedError(this,
                                 i18n("There was an error reading the flight list!"),
                                 errorDetails,
                                 i18n("Library Error"));
    } else {
      QMessageBox::critical(this,
                         i18n("Library Error"),
                         i18n("There was an error reading the flight list!"), QMessageBox::Ok, 0);
    }
    return;
  }

  QString day;
  QString idS;

  if (dirList.count()==0) {
    delete statusDlg;
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,
                            i18n("Download result"),
                            i18n("There were no flights recorded in the recorder."), QMessageBox::Ok, 0);
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

  extern QSettings _settings;
  // If no DefaultFlightDirectory is configured, we must use $HOME instead of the root-directory
  QString flightDir = _settings.readEntry("/Path/DefaultFlightDirectory", QDir::homeDirPath());

  QString fileName = flightDir + "/";

  int flightID(item->text(colID).toInt() - 1);

  //warning("Loading flight %d (%d)", flightID, flightList->itemPos(item));
  warning("%s", (const char*)dirList.at(flightID)->longFileName);
  warning("%s", (const char*)dirList.at(flightID)->shortFileName);

//  QTimer::singleShot( 0, this, SLOT(slotDisablePages()) );

  warning("fileName: %s", fileName.latin1());
  if(useLongNames->isChecked()) {
    fileName += dirList.at(flightID)->longFileName.upper();
  }
  else {
    fileName += dirList.at(flightID)->shortFileName.upper();
  }
  warning("flightdir: %s, filename: %s", flightDir.latin1(), fileName.latin1());
//  KFileDialog* dlg = new KFileDialog(flightDir, "*.igc *.IGC ", this,
//         i18n("Select IGC File"), true);
  KFileDialog* dlg = new KFileDialog(flightDir, i18n("*.igc *.IGC|IGC files"), this,
         i18n("Select IGC File"), true);
  dlg->setSelection( fileName );
  dlg->setOperationMode( KFileDialog::Saving );
  dlg->setCaption(i18n("Select IGC file to save to"));
  dlg->exec();

  KURL fUrl = dlg->selectedURL();

  if(fUrl.isLocalFile())
    fileName = fUrl.path();
  else if(!KIO::NetAccess::download(fUrl, fileName, this))
   {
     KNotifyClient::event(this->winId(), i18n("Can not download file %1").arg(fUrl.url()));
     return;
   }


  QMessageBox* statusDlg = new QMessageBox ( i18n("downloading flight"), i18n("downloading flight"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  kapp->processEvents();

  warning("%s", (const char*)fileName);

  if (!activeRecorder) return;


  kapp->processEvents();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  ret = activeRecorder->downloadFlight(flightID,!useFastDownload->isChecked(),fileName);

  kapp->processEvents();
  QApplication::restoreOverrideCursor();
  if (ret == FR_ERROR) {
    warning("ERROR");
    if ((errorDetails = activeRecorder->lastError()) != "") {
      warning("%s", (const char*)errorDetails);
      KMessageBox::detailedError(this,
                                 i18n("There was an error downloading the flight!"),
                                 errorDetails,
                                 i18n("Library Error"));
    } else {
      QMessageBox::critical(this,
                         i18n("Library Error"),
                         i18n("There was an error downloading the flight!"), QMessageBox::Ok, 0);
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
    QMessageBox::warning(this,
                       i18n("Declaration upload"),
                       i18n("Function not implemented"), QMessageBox::Ok, 0);
    return;
  }


  if (taskSelection->currentItem() >= 0) {
    QPtrList<Waypoint> wpList = tasks->at(taskSelection->currentItem())->getWPList();

    warning("Writing task to logger...");

    ret=activeRecorder->writeDeclaration(&taskDecl,&wpList);

    //evaluate result
    if (ret==FR_NOTSUPPORTED) {
      QMessageBox::warning(this,
                         i18n("Declaration upload"),
                         i18n("Function not implemented"), QMessageBox::Ok, 0);
      return;
    }

    if (ret==FR_ERROR) {
      if ((errorDetails=activeRecorder->lastError())!="") {
        KMessageBox::detailedError(this,
                           i18n("There was an error writing the declaration!"),
                           errorDetails,
                           i18n("Library Error"));
      } else {
        QMessageBox::critical(this,
                           i18n("Library Error"),
                           i18n("There was an error writing the declaration!"), QMessageBox::Ok, 0);
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
    warning("%s", (const char*)error);
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

  activeRecorder->setParent(this);
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
  QMessageBox* statusDlg = new QMessageBox ( i18n("Downloading tasks"), i18n("Downloading tasks"),
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
    QMessageBox::warning(this,
                       i18n("Task download"),
                       i18n("Function not implemented"), QMessageBox::Ok, 0);
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
      QMessageBox::critical(this,
                         i18n("Library Error"),
                         i18n("Cannot read tasks from recorder"), QMessageBox::Ok, 0);
    }
  }
  else {
    for (task = tasks->first(); task; task = tasks->next()) {
      wpList = task->getWPList();
      // here we overwrite the original task name (if needed) to get a unique internal name
      task->setTaskName(_globalMapContents.genTaskName(task->getFileName()));
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
    QMessageBox::warning(this,
                       i18n("Task upload"),
                       i18n("Function not implemented"), QMessageBox::Ok, 0);
    delete statusDlg;
    return;
  }

  maxNrTasks = activeRecorder->capabilities().maxNrTasks;

  maxNrWayPointsPerTask = activeRecorder->capabilities().maxNrWaypointsPerTask;

  if (maxNrTasks == 0) {
    QMessageBox::critical(this,
                       i18n("Library Error"),
                       i18n("Cannot obtain max number of tasks!"), QMessageBox::Ok, 0);
  }
  else if (maxNrWayPointsPerTask == 0) {
    QMessageBox::critical(this,
                       i18n("Library Error"),
                       i18n("Cannot obtain max number of waypoints per task!"), QMessageBox::Ok, 0);
  }
  else {
    for (task = tasks->first(); task; task = tasks->next()) {
      if (frTasks.count() > maxNrTasks) {
        e.sprintf(i18n("Maximum number of %d tasks reached!\n"
                       "Further tasks will be ignored."), maxNrTasks);
        if (KMessageBox::warningContinueCancel(this, e, i18n("Recorder Warning"))
            == KMessageBox::Cancel) {
          delete statusDlg;
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
            delete statusDlg;
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
        QMessageBox::critical(this,
                           i18n("Library Error"),
                           i18n("Cannot write tasks to recorder"), QMessageBox::Ok, 0);
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
    QMessageBox::warning(this,
                       i18n("Waypoint download"),
                       i18n("Function not implemented"), QMessageBox::Ok, 0);
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
      QMessageBox::critical(this,
                         i18n("Library Error"),
                         i18n("Cannot read waypoints from recorder"), QMessageBox::Ok, 0);
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
    QMessageBox::warning(this,
                       i18n("Waypoint upload"),
                       i18n("Function not implemented"), QMessageBox::Ok, 0);
    return;
  }

  maxNrWaypoints = activeRecorder->capabilities().maxNrWaypoints;

  if (maxNrWaypoints == 0) {
    QMessageBox::critical(this,
                       i18n("Library Error"),
                       i18n("Cannot obtain maximum number of waypoints from lib."), QMessageBox::Ok, 0);
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
        QMessageBox::critical(this,
                           i18n("Library Error"),
                           i18n("Cannot write waypoints to recorder."), QMessageBox::Ok, 0);
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

  int ret = activeRecorder->getBasicData(basicdata);
  if (ret == FR_OK)
  {
    if (cap.supDspSerialNumber)
      serID->setText(basicdata.serialNumber);
    if (cap.supDspRecorderType)
      recType->setText(basicdata.recorderType);
    if (cap.supDspPilotName)
      pltName->setText(basicdata.pilotName.stripWhiteSpace());
    if (cap.supDspGliderType)
      gldType->setText(basicdata.gliderType.stripWhiteSpace());
    if (cap.supDspGliderID)
      gldID->setText(basicdata.gliderID.stripWhiteSpace());
    if (cap.supDspCompetitionID)
      compID->setText(basicdata.competitionID.stripWhiteSpace());
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
      QMessageBox::warning(this,
                         i18n("Recorder Connection"),
                         i18n("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."), QMessageBox::Ok, 0);       //Using the Sorry box is a bit friendlier than Error...
    }
  }
  if (cap.supEditGliderID     ||
      cap.supEditGliderType   ||
      cap.supEditGliderPolar  ||
      cap.supEditPilotName    ||
      cap.supEditUnits        ||
      cap.supEditGoalAlt      ||
      cap.supEditArvRadius    ||
      cap.supEditAudio        ||
      cap.supEditLogInterval) {
    int ret = activeRecorder->getConfigData(configdata);
    if (ret == FR_OK)
    {
      // now that we read the information from the logger, we can enable the write button:
      cmdUploadConfig->setEnabled(true);
      LD->setValue(configdata.LD);
      speedLD->setValue(configdata.speedLD);
      speedV2->setValue(configdata.speedV2);
      dryweight->setValue(configdata.dryweight);
      maxwater->setValue(configdata.maxwater);
      sinktone->setChecked(configdata.sinktone);
      approachradius->setValue(configdata.approachradius);
      arrivalradius->setValue(configdata.arrivalradius);
      goalalt->setValue(configdata.goalalt);
      sloginterval->setValue(configdata.sloginterval);
      floginterval->setValue(configdata.floginterval);
      gaptime->setValue(configdata.gaptime);
      minloggingspd->setValue(configdata.minloggingspd);
      stfdeadband->setValue(configdata.stfdeadband);
      unitVarioButtonGroup->setButton(configdata.units & FlightRecorderPluginBase::FR_Unit_Vario_kts);
      unitAltButtonGroup->setButton(configdata.units & FlightRecorderPluginBase::FR_Unit_Alt_ft);
      unitTempButtonGroup->setButton(configdata.units & FlightRecorderPluginBase::FR_Unit_Temp_F);
      unitQNHButtonGroup->setButton(configdata.units & FlightRecorderPluginBase::FR_Unit_Baro_inHg);
      unitDistButtonGroup->setButton(configdata.units & (FlightRecorderPluginBase::FR_Unit_Dist_nm|FlightRecorderPluginBase::FR_Unit_Dist_sm));
      unitSpeedButtonGroup->setButton(configdata.units & (FlightRecorderPluginBase::FR_Unit_Spd_kts|FlightRecorderPluginBase::FR_Unit_Spd_mph));
    }
  }
  if (cap.supEditGliderID     ||
      cap.supEditGliderType   ||
      cap.supEditPilotName) {
      cmdUploadBasicConfig->setEnabled(true);
      pltName->setEnabled(true);
      gldType->setEnabled(true);
      gldID->setEnabled(true);
  }
}


void RecorderDialog::slotWriteConfig()
{
  if (!activeRecorder) return;

  basicdata.pilotName = pltName->text();
  basicdata.gliderType = gldType->text();
  basicdata.gliderID = gldID->text();
  basicdata.competitionID = compID->text();

  configdata.LD = atoi(LD->text());
  configdata.speedLD = atoi(speedLD->text());
  configdata.speedV2 = atoi(speedV2->text());
  configdata.dryweight = atoi(dryweight->text());
  configdata.maxwater = atoi(maxwater->text());

  configdata.sinktone = (int)sinktone->isChecked();

  configdata.approachradius = atoi(approachradius->text());
  configdata.arrivalradius = atoi(arrivalradius->text());
  configdata.goalalt = atoi(goalalt->text());
  configdata.sloginterval = atoi(sloginterval->text());
  configdata.floginterval = atoi(floginterval->text());
  configdata.gaptime = atoi(gaptime->text());
  configdata.minloggingspd = atoi(minloggingspd->text());
  configdata.stfdeadband = atoi(stfdeadband->text());

  configdata.units = unitAltButtonGroup->selectedId()   |
                     unitVarioButtonGroup->selectedId() |
                     unitSpeedButtonGroup->selectedId() |
                     unitQNHButtonGroup->selectedId()   |
                     unitTempButtonGroup->selectedId()  |
                     unitDistButtonGroup->selectedId();
  int ret = activeRecorder->writeConfigData(basicdata, configdata);
  if (ret != FR_OK)
  {
    QString errorDetails=activeRecorder->lastError();
    if (!errorDetails.isEmpty()) {
      KMessageBox::detailedSorry(this,
                         i18n("Sorry, could not write configuration to recorder.\n"
                              "Please check connections and settings."),
                         errorDetails,
                         i18n("Recorder Connection"));       //Using the Sorry box is a bit friendlier than Error...
    } else {
      QMessageBox::warning(this,
                         i18n("Recorder Connection"),
                         i18n("Sorry, could not write configuration to recorder.\n"
                              "Please check connections and settings."), QMessageBox::Ok, 0);       //Using the Sorry box is a bit friendlier than Error...
    }
  }
}

void RecorderDialog::slotDisablePages()
{
  flightPage->setEnabled(false);
  waypointPage->setEnabled(false);
  taskPage->setEnabled(false);
  declarationPage->setEnabled(false);
  //pilotPage->setEnabled(false);
  configPage->setEnabled(false);
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
  configPage->setEnabled(false);

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

    //configpage
    if (cap.supEditGliderPolar  ||
        cap.supEditUnits        ||
        cap.supEditGoalAlt      ||
        cap.supEditArvRadius    ||
        cap.supEditAudio        ||
        cap.supEditLogInterval) {
      configPage->setEnabled(true);
    }
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
    warning("%s", (const char*)i18n("Could not open lib!"));
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
  return; // We don't need the page (yet).
  pilotPage = addPage(i18n("Pilots"), i18n("List of pilots"),
                      KGlobal::instance()->iconLoader()->loadIcon("identity",
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

  QGridLayout* configLayout = new QGridLayout(configPage, 16, 8, 10, 1);

  QGroupBox* gGroup = new QGroupBox(configPage, "gliderGroup");
  gGroup->setTitle(i18n("Glider Settings") + ":");

  QGroupBox* vGroup = new QGroupBox(configPage, "varioGroup");
  vGroup->setTitle(i18n("Variometer Settings") + ":");

  configLayout->addMultiCellWidget(gGroup, 0, 4, 0, 7);
  configLayout->addMultiCellWidget(vGroup, 5, 14, 0, 7);

  QGridLayout *ggrid = new QGridLayout(gGroup, 3, 4, 15, 1);

  ggrid->addWidget(new QLabel(i18n("Best L/D:"), gGroup), 0, 0);
  LD = new QSpinBox (gGroup, "LD");
  LD->setRange(0, 255);
  LD->setLineStep(1);
  LD->setButtonSymbols(QSpinBox::PlusMinus);
  LD->setEnabled(true);
  LD->setValue(0);
  ggrid->addWidget(LD, 0, 1);

  ggrid->addWidget(new QLabel(i18n("Best L/D speed (km/h):"), gGroup), 1, 0);
  speedLD = new QSpinBox (gGroup, "speedLD");
  speedLD->setRange(0, 255);
  speedLD->setLineStep(1);
  speedLD->setButtonSymbols(QSpinBox::PlusMinus);
  speedLD->setEnabled(true);
  speedLD->setValue(0);
  ggrid->addWidget(speedLD, 1, 1);

  ggrid->addWidget(new QLabel(i18n("2 m/s sink speed (km/h):"), gGroup), 2, 0);
  speedV2 = new QSpinBox (gGroup, "speedV2");
  speedV2->setRange(0, 255);
  speedV2->setLineStep(1);
  speedV2->setButtonSymbols(QSpinBox::PlusMinus);
  speedV2->setEnabled(true);
  speedV2->setValue(0);
  ggrid->addWidget(speedV2, 2, 1);

  ggrid->addWidget(new QLabel(i18n("Dry weight (kg):"), gGroup), 0, 2);
  dryweight = new QSpinBox (gGroup, "dryweight");
  dryweight->setRange(0, 1000);
  dryweight->setLineStep(1);
  dryweight->setButtonSymbols(QSpinBox::PlusMinus);
  dryweight->setEnabled(true);
  dryweight->setValue(0);
  ggrid->addWidget(dryweight, 0, 3);

  ggrid->addWidget(new QLabel(i18n("Max. water ballast (l):"), gGroup), 1, 2);
  maxwater = new QSpinBox (gGroup, "maxwater");
  maxwater->setRange(0, 500);
  maxwater->setLineStep(1);
  maxwater->setButtonSymbols(QSpinBox::PlusMinus);
  maxwater->setEnabled(true);
  maxwater->setValue(0);
  ggrid->addWidget(maxwater, 1, 3);


  QGridLayout *vgrid = new QGridLayout(vGroup, 9, 7, 15, 1);

  vgrid->addWidget(new QLabel(i18n("Approach radius (m):"), vGroup), 0, 0);
  approachradius = new QSpinBox (vGroup, "approachradius");
  approachradius->setRange(0, 65535);
  approachradius->setLineStep(10);
  approachradius->setButtonSymbols(QSpinBox::PlusMinus);
  approachradius->setEnabled(true);
  approachradius->setValue(0);
  vgrid->addWidget(approachradius, 0, 1);

  vgrid->addWidget(new QLabel(i18n("Arrival radius (m):"), vGroup), 2, 0);
  arrivalradius = new QSpinBox (vGroup, "arrivalradius");
  arrivalradius->setRange(0, 65535);
  arrivalradius->setLineStep(10);
  arrivalradius->setButtonSymbols(QSpinBox::PlusMinus);
  arrivalradius->setEnabled(true);
  arrivalradius->setValue(0);
  vgrid->addWidget(arrivalradius, 2, 1);

  vgrid->addWidget(new QLabel(i18n("Goal altitude (m):"), vGroup), 4, 0);
  goalalt = new QSpinBox (vGroup, "goalalt");
  goalalt->setRange(0, 6553);
  goalalt->setLineStep(1);
  goalalt->setButtonSymbols(QSpinBox::PlusMinus);
  goalalt->setEnabled(true);
  goalalt->setValue(0);
  vgrid->addWidget(goalalt, 4, 1);

  vgrid->addWidget(new QLabel(i18n("Min. flight time (min):"), vGroup), 6, 0);
  gaptime = new QSpinBox (vGroup, "gaptime");
  gaptime->setRange(0, 600);
  gaptime->setLineStep(1);
  gaptime->setButtonSymbols(QSpinBox::PlusMinus);
  gaptime->setEnabled(true);
  gaptime->setValue(0);
  vgrid->addWidget(gaptime, 6, 1);

  vgrid->addWidget(new QLabel(i18n("Slow log interval (s):"), vGroup), 0, 2);
  sloginterval = new QSpinBox (vGroup, "sloginterval");
  sloginterval->setRange(0, 600);
  sloginterval->setLineStep(1);
  sloginterval->setButtonSymbols(QSpinBox::PlusMinus);
  sloginterval->setEnabled(true);
  sloginterval->setValue(0);
  vgrid->addWidget(sloginterval, 0, 3);

  vgrid->addWidget(new QLabel(i18n("Fast log interval (s):"), vGroup), 2, 2);
  floginterval = new QSpinBox (vGroup, "floginterval");
  floginterval->setRange(0, 600);
  floginterval->setLineStep(1);
  floginterval->setButtonSymbols(QSpinBox::PlusMinus);
  floginterval->setEnabled(true);
  floginterval->setValue(0);
  vgrid->addWidget(floginterval, 2, 3);

  vgrid->addWidget(new QLabel(i18n("Min. logging spd (km/h):"), vGroup), 4, 2);
  minloggingspd = new QSpinBox (vGroup, "minloggingspd");
  minloggingspd->setRange(0, 100);
  minloggingspd->setLineStep(1);
  minloggingspd->setButtonSymbols(QSpinBox::PlusMinus);
  minloggingspd->setEnabled(true);
  minloggingspd->setValue(0);
  vgrid->addWidget(minloggingspd, 4, 3);

  vgrid->addWidget(new QLabel(i18n("Audio dead-band (km/h):"), vGroup), 6, 2);
  stfdeadband = new QSpinBox (vGroup, "stfdeadband");
  stfdeadband->setRange(0, 90);
  stfdeadband->setLineStep(1);
  stfdeadband->setButtonSymbols(QSpinBox::PlusMinus);
  stfdeadband->setEnabled(true);
  stfdeadband->setValue(0);
  vgrid->addWidget(stfdeadband, 6, 3);

  vgrid->addWidget(new QLabel("     ", vGroup), 0, 4);  // Just a filler

  vgrid->addWidget(new QLabel(i18n("Altitude:"), vGroup), 0, 5);
  unitAltButtonGroup = new QButtonGroup(vGroup);
  unitAltButtonGroup-> hide();
  unitAltButtonGroup-> setExclusive(true);
  QRadioButton *rb = new QRadioButton(i18n("m"), vGroup);
  unitAltButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 1, 5);
  rb = new QRadioButton(i18n("ft"), vGroup);
  unitAltButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Alt_ft);
  vgrid->addWidget(rb, 2, 5);

  vgrid->addWidget(new QLabel(i18n("QNH:"), vGroup), 5, 5);
  unitQNHButtonGroup = new QButtonGroup(vGroup);
  unitQNHButtonGroup-> hide();
  unitQNHButtonGroup-> setExclusive(true);
  rb = new QRadioButton(i18n("mbar"), vGroup);
  unitQNHButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 6, 5);
  rb = new QRadioButton(i18n("inHg"), vGroup);
  unitQNHButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Baro_inHg);
  vgrid->addWidget(rb, 7, 5);

  vgrid->addWidget(new QLabel(i18n("Speed:"), vGroup), 0, 6);
  unitSpeedButtonGroup = new QButtonGroup(vGroup);
  unitSpeedButtonGroup-> hide();
  unitSpeedButtonGroup-> setExclusive(true);
  rb = new QRadioButton(i18n("km/h"), vGroup);
  unitSpeedButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 1, 6);
  rb = new QRadioButton(i18n("kts"), vGroup);
  unitSpeedButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Spd_kts);
  vgrid->addWidget(rb, 2, 6);
  rb = new QRadioButton(i18n("mph"), vGroup);
  unitSpeedButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Spd_mph);
  vgrid->addWidget(rb, 3, 6);

  vgrid->addWidget(new QLabel(i18n("Vario:"), vGroup), 5, 6);
  unitVarioButtonGroup = new QButtonGroup(vGroup);
  unitVarioButtonGroup-> hide();
  unitVarioButtonGroup-> setExclusive(true);
  rb = new QRadioButton(i18n("m/s"), vGroup);
  unitVarioButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 6, 6);
  rb = new QRadioButton(i18n("kts"), vGroup);
  unitVarioButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Vario_kts);
  vgrid->addWidget(rb, 7, 6);

  vgrid->addWidget(new QLabel(i18n("Distance:"), vGroup), 0, 7);
  unitDistButtonGroup = new QButtonGroup(vGroup);
  unitDistButtonGroup-> hide();
  unitDistButtonGroup-> setExclusive(true);
  rb = new QRadioButton(i18n("km"), vGroup);
  unitDistButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 1, 7);
  rb = new QRadioButton(i18n("nm"), vGroup);
  unitDistButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Dist_nm);
  vgrid->addWidget(rb, 2, 7);
  rb = new QRadioButton(i18n("sm"), vGroup);
  unitDistButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Dist_sm);
  vgrid->addWidget(rb, 3, 7);

  vgrid->addWidget(new QLabel(i18n("Temp.:"), vGroup), 5, 7);
  unitTempButtonGroup = new QButtonGroup(vGroup);
  unitTempButtonGroup-> hide();
  unitTempButtonGroup-> setExclusive(true);
  rb = new QRadioButton(i18n("C"), vGroup);
  unitTempButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 6, 7);
  rb = new QRadioButton(i18n("F"), vGroup);
  unitTempButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Temp_F);
  vgrid->addWidget(rb, 7, 7);

  sinktone = new QCheckBox(i18n("sink tone"), vGroup);
  sinktone->setChecked(true);
  vgrid->addWidget(sinktone, 8, 2);


  cmdUploadConfig = new QPushButton(i18n("write config to recorder"), configPage);
  // disable this button until we read the information from the flight recorder:
  cmdUploadConfig->setEnabled(false);
  connect(cmdUploadConfig, SIGNAL(clicked()), SLOT(slotWriteConfig()));
  configLayout->addWidget(cmdUploadConfig, 15, 6);

}
