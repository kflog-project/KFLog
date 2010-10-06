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

#include <dlfcn.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qapplication.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include "airport.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "recorderdialog.h"

RecorderDialog::RecorderDialog(QWidget *parent, const char *name)
  : QDialog(parent, "Flightrecorder-Dialog (RecorderDialog)"),
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

  configLayout = new QGridLayout(this, 2, 3, 1, -1, "main layout of RecorderDialog");

  setupTree = new QListView(this, "setupTree");
  setupTree->addColumn("Menu");
  setupTree->hideColumn(1);

  configLayout->addWidget(setupTree, 0, 0);
  connect(setupTree, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(slotPageChanged(QListViewItem *)));

  QPushButton *closeButton = new QPushButton(tr("&Close"), this);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
  configLayout->addWidget(closeButton, 1, 2);

  __addSettingsPage();
  __addFlightPage();
  __addDeclarationPage();
  __addTaskPage();
  __addWaypointPage();
  __addPilotPage();
  __addConfigPage();

  setupTree->hideColumn(1);
  setFixedWidth(830);
  setMinimumHeight(350);
  setupTree->setFixedWidth(145);
  setupTree->setColumnWidth(145, 1);
  setupTree->setResizeMode(QListView::NoColumn);
  settingsPage->show();
  activePage = settingsPage;
  activePage->setFixedWidth(685);

  slotEnablePages();
}

RecorderDialog::~RecorderDialog()
{
  extern QSettings _settings;

  _settings.writeEntry("/KFLog/RecorderDialog/Name", selectType->currentText());
  _settings.writeEntry("/KFLog/RecorderDialog/Port", selectPort->currentItem());
  _settings.writeEntry("/KFLog/RecorderDialog/Baud", _selectSpeed->currentItem());
  _settings.writeEntry("/KFLog/RecorderDialog/URL",  selectURL->text());

  slotCloseRecorder();
  delete waypoints;
  delete tasks;
}

void RecorderDialog::__addSettingsPage()
{
  int typeID(0), typeLoop(0);

  QListViewItem *item = new QListViewItem(setupTree, tr("Recorder"), "Recorder");
  item->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/kde_media-tape_48.png");

  settingsPage = new QFrame(this, "Recorder Settings");
  settingsPage->hide();
  configLayout->addMultiCellWidget(settingsPage, 0, 0, 1, 2);

  QGridLayout* sLayout = new QGridLayout(settingsPage, 15, 8, 10, 1);

  QGroupBox* sGroup = new QGroupBox(settingsPage, "homeGroup");
  sGroup->setTitle(tr("Settings") + ":");

  selectType = new QComboBox(settingsPage, "type-selection");
  connect(selectType, SIGNAL(activated(const QString &)), this, SLOT(slotRecorderTypeChanged(const QString &)));

  selectPort = new QComboBox(settingsPage, "port-selection");
  selectPortLabel = new QLabel(tr("Port"), settingsPage);
  _selectSpeed = new QComboBox(settingsPage, "baud-selection");
  selectSpeedLabel = new QLabel(tr("Transfer speed"), settingsPage);
  selectURL = new QLineEdit(settingsPage, "URL-selection");
  selectURLLabel = new QLabel(tr("URL"), settingsPage);

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

  cmdConnect = new QPushButton(tr("Connect recorder"), settingsPage);
  cmdConnect->setMaximumWidth(cmdConnect->sizeHint().width() + 5);

  QGroupBox* infoGroup = new QGroupBox(settingsPage, "infoGroup");
  infoGroup->setTitle(tr("Info") + ":");

  lblApiID = new QLabel(tr("API-Version"), settingsPage);
  apiID = new QLabel(settingsPage);
  apiID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  apiID->setBackgroundMode( PaletteLight );
  apiID->setEnabled(false);

  lblSerID = new QLabel(tr("Serial-Nr."), settingsPage);
  serID = new QLabel(tr("no recorder connected"), settingsPage);
  serID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  serID->setBackgroundMode( PaletteLight );
  serID->setEnabled(false);

  lblRecType = new QLabel(tr("Recorder Type"), settingsPage);
  recType = new QLabel (settingsPage);
  recType->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  recType->setBackgroundMode( PaletteLight );
  recType->setEnabled(false);

  lblPltName = new QLabel(tr("Pilot Name"), settingsPage);
  pltName = new QLineEdit (settingsPage, "pltName");
  pltName->setEnabled(false);

  lblGldType = new QLabel(tr("Glider Type"), settingsPage);
  gldType = new QLineEdit (settingsPage, "gldType");
  gldType->setEnabled(false);

  lblGldID = new QLabel(tr("Glider ID"), settingsPage);
  gldID = new QLineEdit (settingsPage, "gldID");
  gldID->setEnabled(false);

  cmdUploadBasicConfig = new QPushButton(tr("write config to recorder"), settingsPage);
  // disable this button until we read the information from the flight recorder:
  cmdUploadBasicConfig->setEnabled(false);
  connect(cmdUploadBasicConfig, SIGNAL(clicked()), SLOT(slotWriteConfig()));

  compID = new QLabel (settingsPage);
  compID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  compID->setBackgroundMode( PaletteLight );
  compID->setEnabled(false);

  sLayout->addMultiCellWidget(sGroup, 0, 6, 0, 7);
  sLayout->addWidget(new QLabel(tr("Type"), settingsPage), 1, 1, AlignRight);
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
  QDir *path = new QDir(QDir::homeDirPath() + "/.kflog/logger/");
  configRec = path->entryList("*.desktop");

  if(configRec.count() == 0) {
    QMessageBox::critical(this,
                       tr("No recorders installed."),
                       tr("There are no recorder-libraries installed."), QMessageBox::Ok, 0);
  }

  libNameList.clear();

  extern QSettings _settings;

  selectPort->setCurrentItem(_settings.readNumEntry("/KFLog/RecorderDialog/Port", 0));
  _selectSpeed->setCurrentItem(_settings.readNumEntry("/KFLog/RecorderDialog/Baud", 0));
  QString name(_settings.readEntry("/KFLog/RecorderDialog/Name", 0)), fileName;
  QString pluginName = "";
  QString currentLibName = "";
  QString lineStream;
  selectURL->setText(_settings.readEntry("/KFLog/RecorderDialog/URL", ""));
  QFile settingFile;

  for(QStringList::Iterator it = configRec.begin(); it != configRec.end(); it++) {
    QDir::setCurrent(QDir::homeDirPath() + "/.kflog/logger/");
    settingFile.setName(*it);
    if(!settingFile.exists())
      continue;
    if(!settingFile.open(IO_ReadOnly))
      continue;

    QTextStream stream(&settingFile);
    while (!stream.eof())
    {
      lineStream = stream.readLine();
      if(lineStream.mid(0,5) == "Name=")
        pluginName = lineStream.remove(0, 5);
      else if(lineStream.mid(0,8) == "LibName=")
        currentLibName = lineStream.remove(0, 8);
    }
    if(pluginName!="" && currentLibName!="")
    {
      selectType->insertItem(pluginName);
      libNameList.insert(pluginName, new QString(currentLibName));
      typeLoop++;
    }
    pluginName = "";
    currentLibName = "";
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
    selectSpeedLabel->setText(tr("Transfer speed:\n(automatic)"));
  else
    selectSpeedLabel->setText(tr("Transfer speed:"));
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
  QListViewItem *item = new QListViewItem(setupTree, tr("Flights"), "Flights");
  item->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/igc_48.png");

  flightPage = new QFrame(this, "Flights");
  flightPage->hide();
  configLayout->addMultiCellWidget(flightPage, 0, 0, 1, 2);

  QGridLayout* fLayout = new QGridLayout(flightPage, 13, 5, 10, 1);

  flightList = new KFLogListView("RecorderDialog/recorderFlightList", flightPage,
                                 "flightList");
  flightList->setShowSortIndicator(true);
  flightList->setAllColumnsShowFocus(true);

  colID = flightList->addColumn(tr("Nr"), 50);
  colDate = flightList->addColumn(tr("Date"));
  colPilot = flightList->addColumn(tr("Pilot"));
  colGlider = flightList->addColumn(tr("Glider"));
  colFirstPoint = flightList->addColumn(tr("first Point"));
  colLastPoint = flightList->addColumn(tr("last Point"));
  colDuration = flightList->addColumn(tr("Duration"));

//  flightList->setColumnAlignment(colID, AlignRight);

  flightList->loadConfig();

  QPushButton* listB = new QPushButton(tr("load list"), flightPage);
  QPushButton* fileB = new QPushButton(tr("save flight"), flightPage);
  useLongNames = new QCheckBox(tr("long filenames"), flightPage);
  // let's prefer short filenames. These are needed for OLC
  useLongNames->setChecked(false);
  useFastDownload = new QCheckBox(tr("fast download"), flightPage);
  useFastDownload->setChecked(true);

  QWhatsThis::add(useLongNames,
                  tr("If checked, the long filenames are used."));
  QWhatsThis::add(useFastDownload,
                  tr("If checked, the IGC-file will not be signed.<BR>"
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

  QListViewItem *item = new QListViewItem(setupTree, tr("Declaration"), "Declaration");
  item->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/declaration_48.png");

  declarationPage = new QFrame(this, "Flight Declaration");
  declarationPage->hide();
  configLayout->addMultiCellWidget(declarationPage, 0, 0, 1, 2);

  QVBoxLayout *top = new QVBoxLayout(declarationPage, 5);
  QGridLayout* tLayout = new QGridLayout(13, 5, 1);

  taskSelection = new QComboBox(false, declarationPage, "taskSelection");
  declarationList = new KFLogListView("RecorderDialog/recorderDeclarationList", declarationPage, "declarationList");

  declarationColID = declarationList->addColumn(tr("Nr"), 50);
  declarationColName = declarationList->addColumn(tr("Name"), 120);
  declarationColLat = declarationList->addColumn(tr("Latitude"), 140);
  declarationColLon = declarationList->addColumn(tr("Longitude"), 140);

  declarationList->setAllColumnsShowFocus(true);
  declarationList->setSorting(declarationColID, true);
  declarationList->setSelectionMode(QListView::NoSelection);

  declarationList->setColumnAlignment(declarationColID, AlignRight);
  declarationList->setColumnAlignment(declarationColLat, AlignRight);
  declarationList->setColumnAlignment(declarationColLon, AlignRight);

  declarationList->loadConfig();

  pilotName = new QLineEdit(declarationPage, "pilotName");
  copilotName = new QLineEdit(declarationPage, "copilotName");
  gliderID = new QLineEdit(declarationPage, "gliderID");
  gliderType = new QComboBox(declarationPage, "gliderType");
  gliderType->setEditable(true);
  editCompID = new QLineEdit(declarationPage, "compID");
  compClass = new QLineEdit(declarationPage, "compClass");

  QPushButton* writeDeclaration = new QPushButton(tr("write declaration to recorder"), declarationPage);
  writeDeclaration->setMaximumWidth(writeDeclaration->sizeHint().width() + 15);

  tLayout->addMultiCellWidget(declarationList, 0, 0, 0, 6);
  tLayout->addWidget(new QLabel(tr("Pilot") + " 1:", declarationPage), 2, 0,
                     AlignRight);
  tLayout->addWidget(pilotName, 2, 2);
  tLayout->addWidget(new QLabel(tr("Pilot") + " 2:", declarationPage), 2, 4,
                     AlignRight);
  tLayout->addWidget(copilotName, 2, 6);
  tLayout->addWidget(new QLabel(tr("Glider-ID") + ":", declarationPage), 4,
                     0, AlignRight);
  tLayout->addWidget(gliderID, 4, 2);
  tLayout->addWidget(new QLabel(tr("Glidertype") + ":", declarationPage), 4,
                     4, AlignRight);
  tLayout->addWidget(gliderType, 4, 6);
  tLayout->addWidget(new QLabel(tr("Comp. ID") + ":", declarationPage), 6,
                     0, AlignRight);
  tLayout->addWidget(editCompID, 6, 2);
  tLayout->addWidget(new QLabel(tr("Comp. Class") + ":", declarationPage), 6,
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

  top->addWidget(new QLabel(tr("Tasks"), declarationPage));
  top->addWidget(taskSelection);
  top->addLayout(tLayout);

  int idx = 0;
#include <gliders.h>
  while(gliderList[idx].index != -1) {
    gliderType->insertItem(QString(gliderList[idx++].name));
  }

  extern QSettings _settings;

  pilotName->setText(_settings.readEntry("/KFLog/PersonalData/PilotName", ""));

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
  QListViewItem *item = new QListViewItem(setupTree, tr("Tasks"), "Tasks");
  item->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/task_48.png");

  taskPage = new QFrame(this, "Tasks");
  taskPage->hide();
  configLayout->addMultiCellWidget(taskPage, 0, 0, 1, 2);

  QVBoxLayout *top = new QVBoxLayout(taskPage, 5);
  QHBoxLayout *buttons = new QHBoxLayout(10);

  taskList = new KFLogListView("RecorderDialog/recorderTaskList", taskPage, "taskList");

  taskColID = taskList->addColumn(tr("Nr"), 50);
  taskColName = taskList->addColumn(tr("Name"), 120);
  taskColDesc = taskList->addColumn(tr("Description"), 120);
  taskColTask = taskList->addColumn(tr("Task dist."), 120);
  taskColTotal = taskList->addColumn(tr("Total dist."), 120);

  taskList->setSorting(taskColID, true);
  taskList->setAllColumnsShowFocus(true);

  taskList->setSelectionMode(QListView::NoSelection);
  taskList->setColumnAlignment(taskColID, AlignRight);

  taskList->loadConfig();

  buttons->addStretch();
  cmdUploadTasks = new QPushButton(tr("write tasks to recorder"), taskPage);
  connect(cmdUploadTasks, SIGNAL(clicked()), SLOT(slotWriteTasks()));
  buttons->addWidget(cmdUploadTasks);

  cmdDownloadTasks = new QPushButton(tr("read tasks from recorder"), taskPage);
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

  QListViewItem *itemMenu = new QListViewItem(setupTree, tr("Waypoints"), "Waypoints");
  itemMenu->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/waypoint_48.png");

  waypointPage = new QFrame(this, "Waypoints");
  waypointPage->hide();
  configLayout->addMultiCellWidget(waypointPage, 0, 0, 1, 2);

  QVBoxLayout *top = new QVBoxLayout(waypointPage, 5);
  QHBoxLayout *buttons = new QHBoxLayout(10);
//  QPushButton *b;

  waypointList = new KFLogListView("RecorderDialog/recorderWaypointList", waypointPage,
                                   "waypointList");

  waypointColID = waypointList->addColumn(tr("Nr"), 50);
  waypointColName = waypointList->addColumn(tr("Name"), 120);
  waypointColLat = waypointList->addColumn(tr("Latitude"), 140);
  waypointColLon = waypointList->addColumn(tr("Longitude"), 140);

  waypointList->setAllColumnsShowFocus(true);
  waypointList->setSorting(waypointColID, true);
  waypointList->setSelectionMode(QListView::NoSelection);

  waypointList->setColumnAlignment(waypointColID, AlignRight);
  waypointList->setColumnAlignment(waypointColLat, AlignRight);
  waypointList->setColumnAlignment(waypointColLon, AlignRight);

  waypointList->loadConfig();

  buttons->addStretch();
  cmdUploadWaypoints = new QPushButton(tr("write waypoints to recorder"), waypointPage);
  connect(cmdUploadWaypoints, SIGNAL(clicked()), SLOT(slotWriteWaypoints()));
  buttons->addWidget(cmdUploadWaypoints);

  cmdDownloadWaypoints = new QPushButton(tr("read waypoints from recorder"), waypointPage);
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
    warning("%s", (const char*)tr("Could not open lib!"));
    return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  //check if we have valid parameters, is so, try to connect!
  switch (activeRecorder->getTransferMode()) {
  case FlightRecorderPluginBase::serial:
    if(portName.isEmpty()) {
      warning("%s", (const char*)tr("No port given!"));
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
      warning("%s", (const char*)tr("No URL entered!"));
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
      QMessageBox::warning(this,
                         tr("Recorder Connection"),
                         tr("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings.") + errorDetails, QMessageBox::Ok, 0); //Using the Sorry box is a bit friendlier than Error...
    } else {
      QMessageBox::warning(this,
                         tr("Recorder Connection"),
                         tr("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."), QMessageBox::Ok, 0); //Using the Sorry box is a bit friendlier than Error...
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

void RecorderDialog::slotPageChanged(QListViewItem *currentItem)
{
  if(currentItem->text(1)=="Configuration")
  {
    activePage->hide();
    configPage->show();
    activePage = configPage;
  }
  else if(currentItem->text(1)=="Declaration")
  {
    activePage->hide();
    declarationPage->show();
    activePage = declarationPage;
  }
  else if(currentItem->text(1)=="Flights")
  {
    activePage->hide();
    flightPage->show();
    activePage = flightPage;
  }
//  else if(currentItem->text(1)=="Pilots")
//  {
//    activePage->hide();
//    pilotPage->show();
//    activePage = pilotPage;
//  }
  else if(currentItem->text(1)=="Recorder")
  {
    activePage->hide();
    settingsPage->show();
    activePage = settingsPage;
  }
  else if(currentItem->text(1)=="Tasks")
  {
    activePage->hide();
    taskPage->show();
    activePage = taskPage;
  }
  else if(currentItem->text(1)=="Waypoints")
  {
    activePage->hide();
    waypointPage->show();
    activePage = waypointPage;
  }
  activePage->setFixedWidth(685);
  setupTree->hideColumn(1);
}

void RecorderDialog::slotReadFlightList()
{

  if (!activeRecorder)
    return;

    QMessageBox* statusDlg = new QMessageBox ( tr("downloading flightlist"), tr("downloading flightlist"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);

  statusDlg->show();

  // Now we need to read the flight list from the logger!

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  flightList->clear();

  qApp->processEvents();
  if (__fillDirList() == FR_ERROR) {
    delete statusDlg;  
    QApplication::restoreOverrideCursor();
    QString errorDetails = activeRecorder->lastError();
    if (!errorDetails.isEmpty()) {
      QMessageBox::critical(this,
           tr("Library Error"),
           tr("There was an error reading the flight list!") + errorDetails, QMessageBox::Ok, 0);
    } else {
      QMessageBox::critical(this,
                         tr("Library Error"),
                         tr("There was an error reading the flight list!"), QMessageBox::Ok, 0);
    }
    return;
  }

  QString day;
  QString idS;

  if (dirList.count()==0) {
    delete statusDlg;
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this,
                            tr("Download result"),
                            tr("There were no flights recorded in the recorder."), QMessageBox::Ok, 0);
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
    item->setText(colFirstPoint, time.toString(Qt::LocalDate));
    time = QTime(e->lastTime.tm_hour, e->lastTime.tm_min,e->lastTime.tm_sec);
    item->setText(colLastPoint, time.toString(Qt::LocalDate));
    time = QTime().addSecs (e->duration);
    item->setText(colDuration, time.toString(Qt::LocalDate));
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
  QString flightDir = _settings.readEntry("/KFLog/Path/DefaultFlightDirectory", QDir::homeDirPath());

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
//         tr("Select IGC File"), true);
  QFileDialog* dlg = new QFileDialog(flightDir, tr("*.igc *.IGC|IGC files"), this, "Select IGC File");
  dlg->setSelection(fileName);
  dlg->setMode(QFileDialog::AnyFile);
  dlg->setCaption(tr("Select IGC file to save to"));
  dlg->show();

  QUrl fUrl = dlg->url();

  if(fUrl.isLocalFile())
    fileName = fUrl.path();
  else
    return;

  QMessageBox* statusDlg = new QMessageBox ( tr("downloading flight"), tr("downloading flight"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  qApp->processEvents();

  warning("%s", (const char*)fileName);

  if (!activeRecorder) return;


  qApp->processEvents();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  ret = activeRecorder->downloadFlight(flightID,!useFastDownload->isChecked(),fileName);

  qApp->processEvents();
  QApplication::restoreOverrideCursor();
  if (ret == FR_ERROR) {
    warning("ERROR");
    if ((errorDetails = activeRecorder->lastError()) != "") {
      warning("%s", (const char*)errorDetails);
      QMessageBox::critical(this,
          tr("Library Error"),
          tr("There was an error downloading the flight!") + errorDetails, QMessageBox::Ok, 0);
    } else {
      QMessageBox::critical(this,
                         tr("Library Error"),
                         tr("There was an error downloading the flight!"), QMessageBox::Ok, 0);
    }
  }
  //TODO: handle returnvalues!

  delete statusDlg;

  slotEnablePages();
}

void RecorderDialog::slotWriteDeclaration()
{
  QMessageBox* statusDlg = new QMessageBox ( tr("send declaration"), tr("send flightdeclaration to the recorder"),
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
  qApp->processEvents();
  if (!activeRecorder->capabilities().supUlDeclaration) {
    QMessageBox::warning(this,
                       tr("Declaration upload"),
                       tr("Function not implemented"), QMessageBox::Ok, 0);
    return;
  }


  if (taskSelection->currentItem() >= 0) {
    QPtrList<Waypoint> wpList = tasks->at(taskSelection->currentItem())->getWPList();

    warning("Writing task to logger...");

    ret=activeRecorder->writeDeclaration(&taskDecl,&wpList);

    //evaluate result
    if (ret==FR_NOTSUPPORTED) {
      QMessageBox::warning(this,
                         tr("Declaration upload"),
                         tr("Function not implemented"), QMessageBox::Ok, 0);
      return;
    }

    if (ret==FR_ERROR) {
      if ((errorDetails=activeRecorder->lastError())!="") {
        QMessageBox::critical(this,
            tr("Library Error"),
            tr("There was an error writing the declaration!") + errorDetails, QMessageBox::Ok, 0);
      } else {
        QMessageBox::critical(this,
                           tr("Library Error"),
                           tr("There was an error writing the declaration!"), QMessageBox::Ok, 0);
      }
      return;
    }

    warning("   ... ready (%d)", ret);
    QMessageBox::information(this,
        tr("Declaration upload"),
        tr("The declaration was uploaded to the recorder."), QMessageBox::Ok, 0);
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

//  libHandle = dlopen(KGlobal::dirs()->findResource("lib", libN), RTLD_NOW);
  //FIXME: path to lib is not always /usr/lib
  libHandle = dlopen("/usr/lib/"+libN, RTLD_NOW);

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
  QMessageBox* statusDlg = new QMessageBox ( tr("Downloading tasks"), tr("Downloading tasks"),
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
                       tr("Task download"),
                       tr("Function not implemented"), QMessageBox::Ok, 0);
    return;
  }

  qApp->processEvents();
  ret = activeRecorder->readTasks(tasks);
  if (ret<FR_OK) {
    if ((errorDetails=activeRecorder->lastError())!="") {
      QMessageBox::critical(this,
          tr("Library Error"),
          tr("Cannot read tasks from recorder") + errorDetails, QMessageBox::Ok, 0);
    } else {
      QMessageBox::critical(this,
                         tr("Library Error"),
                         tr("Cannot read tasks from recorder"), QMessageBox::Ok, 0);
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
    QMessageBox::information(this,
        tr("Task download"),
        tr("%1 tasks were downloaded from the recorder.").arg(cnt), QMessageBox::Ok, 0);
  }

  delete statusDlg;
}

void RecorderDialog::slotWriteTasks()
{
  QMessageBox* statusDlg = new QMessageBox ( tr("send tasks to recorder"), tr("send tasks to recorder"),
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
                       tr("Task upload"),
                       tr("Function not implemented"), QMessageBox::Ok, 0);
    delete statusDlg;
    return;
  }

  maxNrTasks = activeRecorder->capabilities().maxNrTasks;

  maxNrWayPointsPerTask = activeRecorder->capabilities().maxNrWaypointsPerTask;

  if (maxNrTasks == 0) {
    QMessageBox::critical(this,
                       tr("Library Error"),
                       tr("Cannot obtain max number of tasks!"), QMessageBox::Ok, 0);
  }
  else if (maxNrWayPointsPerTask == 0) {
    QMessageBox::critical(this,
                       tr("Library Error"),
                       tr("Cannot obtain max number of waypoints per task!"), QMessageBox::Ok, 0);
  }
  else {
    for (task = tasks->first(); task; task = tasks->next()) {
      if (frTasks.count() > maxNrTasks) {
        e.sprintf(tr("Maximum number of %d tasks reached!\n"
                       "Further tasks will be ignored."), maxNrTasks);

        if (QMessageBox::warning(this, tr("Recorder Warning"), e, tr("Continue"), tr("Cancel"))==1) {
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
          e.sprintf(tr("Maximum number of turnpoints/task %d in %s reached!\n"
                         "Further turnpoints will be ignored."),
                    maxNrWayPointsPerTask, task->getFileName().latin1());
          if (QMessageBox::warning(this, tr("Recorder Warning"), e, tr("Continue"), tr("Cancel"))==1) {
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
        QMessageBox::critical(this,
            tr("Library Error"),
            tr("Cannot write tasks to recorder") + errorDetails, QMessageBox::Ok, 0);
      } else {
        QMessageBox::critical(this,
                           tr("Library Error"),
                           tr("Cannot write tasks to recorder"), QMessageBox::Ok, 0);
      }
    } else {
      QMessageBox::information(this,
          tr("Task upload"),
          tr("%1 tasks were uploaded to the recorder.").arg(cnt), QMessageBox::Ok, 0);
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
                       tr("Waypoint download"),
                       tr("Function not implemented"), QMessageBox::Ok, 0);
    return;
  }

  QMessageBox* statusDlg = new QMessageBox (tr("Reading waypoints"), tr("Reading Waypoints"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  qApp->processEvents();

  ret = activeRecorder->readWaypoints(&frWaypoints);
  if (ret<FR_OK) {
    QApplication::restoreOverrideCursor();

    if ((errorDetails=activeRecorder->lastError())!="") {
      QMessageBox::critical(this,
          tr("Library Error"),
          tr("Cannot read waypoints from recorder") + errorDetails, QMessageBox::Ok, 0);
    } else {
      QMessageBox::critical(this,
                         tr("Library Error"),
                         tr("Cannot read waypoints from recorder"), QMessageBox::Ok, 0);
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
    QMessageBox::information(this,
        tr("Waypoint download"),
        tr("%1 waypoints have been downloaded from the recorder.").arg(cnt), QMessageBox::Ok, 0);

  }

  delete statusDlg;
}

void RecorderDialog::slotWriteWaypoints()
{
  QMessageBox* statusDlg = new QMessageBox ( tr("send waypoints"), tr("send waypoints"),
      QMessageBox::Information, QMessageBox::NoButton, QMessageBox::NoButton,
      QMessageBox::NoButton, this, "statusDialog", true);
  statusDlg->show();

  unsigned int maxNrWaypoints;
  QString e;
  Waypoint *wp;
  QPtrList<Waypoint> frWaypoints;
  int cnt=0;
  QString errorDetails;

  if (QMessageBox::warning(this,
      tr("Waypoints upload"),
      tr("Uploading waypoints to the recorder will overwrite existing waypoints on the recorder. Do want to continue uploading?"),
      tr("Continue"), tr("Cancel")) == 1)
    return;

  if (!activeRecorder) return;

  if (!activeRecorder->capabilities().supUlWaypoint) {
    QMessageBox::warning(this,
                       tr("Waypoint upload"),
                       tr("Function not implemented"), QMessageBox::Ok, 0);
    return;
  }

  maxNrWaypoints = activeRecorder->capabilities().maxNrWaypoints;

  if (maxNrWaypoints == 0) {
    QMessageBox::critical(this,
                       tr("Library Error"),
                       tr("Cannot obtain maximum number of waypoints from lib."), QMessageBox::Ok, 0);
  }
  else {
    for (wp = waypoints->first(); wp; wp = waypoints->next()){
      if (frWaypoints.count() > maxNrWaypoints) {
        e.sprintf(tr("Maximum number of %d waypoints reached!\n"
                       "Further waypoints will be ignored."), maxNrWaypoints);
        if (QMessageBox::warning(this, tr("Recorder Warning"), e, tr("Continue"), tr("Cancel")) ==1) {
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
        QMessageBox::critical(this,
            tr("Library Error"),
            tr("Cannot write waypoints to recorder.") + errorDetails, QMessageBox::Ok, 0);
      } else {
        QMessageBox::critical(this,
                           tr("Library Error"),
                           tr("Cannot write waypoints to recorder."), QMessageBox::Ok, 0);
      }
    } else {
      QApplication::restoreOverrideCursor();

      QMessageBox::information(this,
          tr("Waypoint upload"),
          QString(tr("%1 waypoints have been uploaded to the recorder.")).arg(cnt), QMessageBox::Ok, 0);
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
      QMessageBox::warning(this,
          tr("Recorder Connection"),
          tr("Sorry, could not connect to recorder.\n"
               "Please check connections and settings.") + errorDetails, QMessageBox::Ok, 0);       //Using the Sorry box is a bit friendlier than Error...
    } else {
      QMessageBox::warning(this,
                         tr("Recorder Connection"),
                         tr("Sorry, could not connect to recorder.\n"
                              "Please check connections and settings."));       //Using the Sorry box is a bit friendlier than Error...
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
      QMessageBox::warning(this,
          tr("Recorder Connection"),
          tr("Sorry, could not write configuration to recorder.\n"
               "Please check connections and settings.") + errorDetails, QMessageBox::Ok, 0);       //Using the Sorry box is a bit friendlier than Error...
    } else {
      QMessageBox::warning(this,
                         tr("Recorder Connection"),
                         tr("Sorry, could not write configuration to recorder.\n"
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
    warning("%s", (const char*)tr("Could not open lib!"));
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
//  QListViewItem *item = new QListViewItem(setupTree, tr("Pilots"), "Pilots");
//  item->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/kde_identity_48.png");

//  pilotPage = new QFrame(this, "List of pilots");
//  pilotPage->hide();
//  configLayout->addMultiCellWidget(pilotPage, 0, 0, 1, 2);
}

/** No descriptions */
void RecorderDialog::__addConfigPage()
{
  QListViewItem *item = new QListViewItem(setupTree, tr("Configuration"), "Configuration");
  item->setPixmap(0, QDir::homeDirPath() + "/.kflog/pics/kde_configure_48.png");

  configPage = new QFrame(this, "Recorder configuration");
  configPage->hide();
  configLayout->addMultiCellWidget(configPage, 0, 0, 1, 2);

  QGridLayout* configLayout = new QGridLayout(configPage, 16, 8, 10, 1);

  QGroupBox* gGroup = new QGroupBox(configPage, "gliderGroup");
  gGroup->setTitle(tr("Glider Settings") + ":");

  QGroupBox* vGroup = new QGroupBox(configPage, "varioGroup");
  vGroup->setTitle(tr("Variometer Settings") + ":");

  configLayout->addMultiCellWidget(gGroup, 0, 4, 0, 7);
  configLayout->addMultiCellWidget(vGroup, 5, 14, 0, 7);

  QGridLayout *ggrid = new QGridLayout(gGroup, 3, 4, 15, 1);

  ggrid->addWidget(new QLabel(tr("Best L/D:"), gGroup), 0, 0);
  LD = new QSpinBox (gGroup, "LD");
  LD->setRange(0, 255);
  LD->setLineStep(1);
  LD->setButtonSymbols(QSpinBox::PlusMinus);
  LD->setEnabled(true);
  LD->setValue(0);
  ggrid->addWidget(LD, 0, 1);

  ggrid->addWidget(new QLabel(tr("Best L/D speed (km/h):"), gGroup), 1, 0);
  speedLD = new QSpinBox (gGroup, "speedLD");
  speedLD->setRange(0, 255);
  speedLD->setLineStep(1);
  speedLD->setButtonSymbols(QSpinBox::PlusMinus);
  speedLD->setEnabled(true);
  speedLD->setValue(0);
  ggrid->addWidget(speedLD, 1, 1);

  ggrid->addWidget(new QLabel(tr("2 m/s sink speed (km/h):"), gGroup), 2, 0);
  speedV2 = new QSpinBox (gGroup, "speedV2");
  speedV2->setRange(0, 255);
  speedV2->setLineStep(1);
  speedV2->setButtonSymbols(QSpinBox::PlusMinus);
  speedV2->setEnabled(true);
  speedV2->setValue(0);
  ggrid->addWidget(speedV2, 2, 1);

  ggrid->addWidget(new QLabel(tr("Dry weight (kg):"), gGroup), 0, 2);
  dryweight = new QSpinBox (gGroup, "dryweight");
  dryweight->setRange(0, 1000);
  dryweight->setLineStep(1);
  dryweight->setButtonSymbols(QSpinBox::PlusMinus);
  dryweight->setEnabled(true);
  dryweight->setValue(0);
  ggrid->addWidget(dryweight, 0, 3);

  ggrid->addWidget(new QLabel(tr("Max. water ballast (l):"), gGroup), 1, 2);
  maxwater = new QSpinBox (gGroup, "maxwater");
  maxwater->setRange(0, 500);
  maxwater->setLineStep(1);
  maxwater->setButtonSymbols(QSpinBox::PlusMinus);
  maxwater->setEnabled(true);
  maxwater->setValue(0);
  ggrid->addWidget(maxwater, 1, 3);


  QGridLayout *vgrid = new QGridLayout(vGroup, 9, 7, 15, 1);

  vgrid->addWidget(new QLabel(tr("Approach radius (m):"), vGroup), 0, 0);
  approachradius = new QSpinBox (vGroup, "approachradius");
  approachradius->setRange(0, 65535);
  approachradius->setLineStep(10);
  approachradius->setButtonSymbols(QSpinBox::PlusMinus);
  approachradius->setEnabled(true);
  approachradius->setValue(0);
  vgrid->addWidget(approachradius, 0, 1);

  vgrid->addWidget(new QLabel(tr("Arrival radius (m):"), vGroup), 2, 0);
  arrivalradius = new QSpinBox (vGroup, "arrivalradius");
  arrivalradius->setRange(0, 65535);
  arrivalradius->setLineStep(10);
  arrivalradius->setButtonSymbols(QSpinBox::PlusMinus);
  arrivalradius->setEnabled(true);
  arrivalradius->setValue(0);
  vgrid->addWidget(arrivalradius, 2, 1);

  vgrid->addWidget(new QLabel(tr("Goal altitude (m):"), vGroup), 4, 0);
  goalalt = new QSpinBox (vGroup, "goalalt");
  goalalt->setRange(0, 6553);
  goalalt->setLineStep(1);
  goalalt->setButtonSymbols(QSpinBox::PlusMinus);
  goalalt->setEnabled(true);
  goalalt->setValue(0);
  vgrid->addWidget(goalalt, 4, 1);

  vgrid->addWidget(new QLabel(tr("Min. flight time (min):"), vGroup), 6, 0);
  gaptime = new QSpinBox (vGroup, "gaptime");
  gaptime->setRange(0, 600);
  gaptime->setLineStep(1);
  gaptime->setButtonSymbols(QSpinBox::PlusMinus);
  gaptime->setEnabled(true);
  gaptime->setValue(0);
  vgrid->addWidget(gaptime, 6, 1);

  vgrid->addWidget(new QLabel(tr("Slow log interval (s):"), vGroup), 0, 2);
  sloginterval = new QSpinBox (vGroup, "sloginterval");
  sloginterval->setRange(0, 600);
  sloginterval->setLineStep(1);
  sloginterval->setButtonSymbols(QSpinBox::PlusMinus);
  sloginterval->setEnabled(true);
  sloginterval->setValue(0);
  vgrid->addWidget(sloginterval, 0, 3);

  vgrid->addWidget(new QLabel(tr("Fast log interval (s):"), vGroup), 2, 2);
  floginterval = new QSpinBox (vGroup, "floginterval");
  floginterval->setRange(0, 600);
  floginterval->setLineStep(1);
  floginterval->setButtonSymbols(QSpinBox::PlusMinus);
  floginterval->setEnabled(true);
  floginterval->setValue(0);
  vgrid->addWidget(floginterval, 2, 3);

  vgrid->addWidget(new QLabel(tr("Min. logging spd (km/h):"), vGroup), 4, 2);
  minloggingspd = new QSpinBox (vGroup, "minloggingspd");
  minloggingspd->setRange(0, 100);
  minloggingspd->setLineStep(1);
  minloggingspd->setButtonSymbols(QSpinBox::PlusMinus);
  minloggingspd->setEnabled(true);
  minloggingspd->setValue(0);
  vgrid->addWidget(minloggingspd, 4, 3);

  vgrid->addWidget(new QLabel(tr("Audio dead-band (km/h):"), vGroup), 6, 2);
  stfdeadband = new QSpinBox (vGroup, "stfdeadband");
  stfdeadband->setRange(0, 90);
  stfdeadband->setLineStep(1);
  stfdeadband->setButtonSymbols(QSpinBox::PlusMinus);
  stfdeadband->setEnabled(true);
  stfdeadband->setValue(0);
  vgrid->addWidget(stfdeadband, 6, 3);

  vgrid->addWidget(new QLabel("     ", vGroup), 0, 4);  // Just a filler

  vgrid->addWidget(new QLabel(tr("Altitude:"), vGroup), 0, 5);
  unitAltButtonGroup = new QButtonGroup(vGroup);
  unitAltButtonGroup-> hide();
  unitAltButtonGroup-> setExclusive(true);
  QRadioButton *rb = new QRadioButton(tr("m"), vGroup);
  unitAltButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 1, 5);
  rb = new QRadioButton(tr("ft"), vGroup);
  unitAltButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Alt_ft);
  vgrid->addWidget(rb, 2, 5);

  vgrid->addWidget(new QLabel(tr("QNH:"), vGroup), 5, 5);
  unitQNHButtonGroup = new QButtonGroup(vGroup);
  unitQNHButtonGroup-> hide();
  unitQNHButtonGroup-> setExclusive(true);
  rb = new QRadioButton(tr("mbar"), vGroup);
  unitQNHButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 6, 5);
  rb = new QRadioButton(tr("inHg"), vGroup);
  unitQNHButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Baro_inHg);
  vgrid->addWidget(rb, 7, 5);

  vgrid->addWidget(new QLabel(tr("Speed:"), vGroup), 0, 6);
  unitSpeedButtonGroup = new QButtonGroup(vGroup);
  unitSpeedButtonGroup-> hide();
  unitSpeedButtonGroup-> setExclusive(true);
  rb = new QRadioButton(tr("km/h"), vGroup);
  unitSpeedButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 1, 6);
  rb = new QRadioButton(tr("kts"), vGroup);
  unitSpeedButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Spd_kts);
  vgrid->addWidget(rb, 2, 6);
  rb = new QRadioButton(tr("mph"), vGroup);
  unitSpeedButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Spd_mph);
  vgrid->addWidget(rb, 3, 6);

  vgrid->addWidget(new QLabel(tr("Vario:"), vGroup), 5, 6);
  unitVarioButtonGroup = new QButtonGroup(vGroup);
  unitVarioButtonGroup-> hide();
  unitVarioButtonGroup-> setExclusive(true);
  rb = new QRadioButton(tr("m/s"), vGroup);
  unitVarioButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 6, 6);
  rb = new QRadioButton(tr("kts"), vGroup);
  unitVarioButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Vario_kts);
  vgrid->addWidget(rb, 7, 6);

  vgrid->addWidget(new QLabel(tr("Distance:"), vGroup), 0, 7);
  unitDistButtonGroup = new QButtonGroup(vGroup);
  unitDistButtonGroup-> hide();
  unitDistButtonGroup-> setExclusive(true);
  rb = new QRadioButton(tr("km"), vGroup);
  unitDistButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 1, 7);
  rb = new QRadioButton(tr("nm"), vGroup);
  unitDistButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Dist_nm);
  vgrid->addWidget(rb, 2, 7);
  rb = new QRadioButton(tr("sm"), vGroup);
  unitDistButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Dist_sm);
  vgrid->addWidget(rb, 3, 7);

  vgrid->addWidget(new QLabel(tr("Temp.:"), vGroup), 5, 7);
  unitTempButtonGroup = new QButtonGroup(vGroup);
  unitTempButtonGroup-> hide();
  unitTempButtonGroup-> setExclusive(true);
  rb = new QRadioButton(tr("C"), vGroup);
  unitTempButtonGroup-> insert(rb, 0);
  vgrid->addWidget(rb, 6, 7);
  rb = new QRadioButton(tr("F"), vGroup);
  unitTempButtonGroup-> insert(rb, FlightRecorderPluginBase::FR_Unit_Temp_F);
  vgrid->addWidget(rb, 7, 7);

  sinktone = new QCheckBox(tr("sink tone"), vGroup);
  sinktone->setChecked(true);
  vgrid->addWidget(sinktone, 8, 2);


  cmdUploadConfig = new QPushButton(tr("write config to recorder"), configPage);
  // disable this button until we read the information from the flight recorder:
  cmdUploadConfig->setEnabled(false);
  connect(cmdUploadConfig, SIGNAL(clicked()), SLOT(slotWriteConfig()));
  configLayout->addWidget(cmdUploadConfig, 15, 6);

}
