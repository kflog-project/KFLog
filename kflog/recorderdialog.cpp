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

#include <flighttask.h>
#include <mapcalc.h>
#include <mapcontents.h>

#define CHECK_ERROR_EXIT  error = dlerror(); \
  if(error != NULL) \
    { \
      warning(error); \
      return; \
    }

#define CHECK_ERROR_RETURN  error = dlerror(); \
  if(error != NULL) \
    { \
      warning(error); \
      return -1; \
    }

#define CHECK_ERROR  error = dlerror(); \
  if(error != NULL) \
    { \
      warning(error); \
    }

RecorderDialog::RecorderDialog(QWidget *parent, KConfig* cnf, const char *name)
 : KDialogBase(IconList, i18n("Flightrecorder-Dialog"), Close, Close,
      parent, name, true, true),
    config(cnf),
    loggerConf(0),
    isOpen(false)
{
  __addSettingsPage();
  __addFlightPage();
  __addTaskPage();
}

RecorderDialog::~RecorderDialog()
{
  config->setGroup("Recorder Dialog");
  config->writeEntry("Name", selectType->currentText());
  config->writeEntry("Port", selectPort->currentItem());
  config->setGroup(0);
}

void RecorderDialog::__addSettingsPage()
{
  settingsPage = addPage(i18n("Recorder"), i18n("Recorder Settings"),
      KGlobal::instance()->iconLoader()->loadIcon("connect_no", KIcon::NoGroup,
          KIcon::SizeLarge));

  settingsPage->setMinimumWidth(500);
  settingsPage->setMinimumHeight(300);

  QGridLayout* sLayout = new QGridLayout(settingsPage, 15, 5, 10, 1);

  QGroupBox* sGroup = new QGroupBox(settingsPage, "homeGroup");
  sGroup->setTitle(i18n("Settings") + ":");

  selectType = new KComboBox(settingsPage, "type-selection");
  selectPort = new KComboBox(settingsPage, "port-selection");

  selectPort->insertItem("ttyS0");
  selectPort->insertItem("ttyS1");
  selectPort->insertItem("ttyS2");
  selectPort->insertItem("ttyS3");

//  QPushButton* connectB = new QPushButton(i18n("Connect and read flight-list"), settingsPage);
//  connectB->setMaximumWidth(connectB->sizeHint().width() + 5);

  QGroupBox* infoGroup = new QGroupBox(settingsPage, "infoGroup");
  infoGroup->setTitle(i18n("Info") + ":");
  apiID = new QLabel(settingsPage);
  apiID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  apiID->setBackgroundMode( PaletteLight );

  serID = new QLabel(i18n("no recorder connected"), settingsPage);
  serID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  serID->setBackgroundMode( PaletteLight );

  sLayout->addMultiCellWidget(sGroup, 0, 6, 0, 4);
  sLayout->addWidget(new QLabel(i18n("Type"), settingsPage), 1, 1);
  sLayout->addWidget(selectType, 1, 3);
  sLayout->addWidget(new QLabel(i18n("Port"), settingsPage), 3, 1);
  sLayout->addWidget(selectPort, 3, 3);

//  sLayout->addWidget(connectB, 8, 3, AlignRight);

  sLayout->addMultiCellWidget(infoGroup, 8, 14, 0, 4);
  sLayout->addWidget(new QLabel(i18n("API-Version"), settingsPage), 9, 1);
  sLayout->addWidget(apiID, 9, 3);
  sLayout->addWidget(new QLabel(i18n("Serial-Nr."), settingsPage), 11, 1);
  sLayout->addWidget(serID, 11, 3);

  sLayout->setColStretch(1,1);
  sLayout->setColStretch(3,2);

  sLayout->addColSpacing(0, 10);
  sLayout->addColSpacing(2, 10);
  sLayout->addColSpacing(4, 10);

  sLayout->addRowSpacing(0, 20);
  sLayout->addRowSpacing(2, 5);
  sLayout->addRowSpacing(4, 5);
  sLayout->addRowSpacing(6, 10);

  sLayout->addRowSpacing(7, 5);
//  sLayout->addRowSpacing(9, 0);

  sLayout->addRowSpacing(8, 20);
  sLayout->addRowSpacing(10, 5);
  sLayout->addRowSpacing(12, 5);
  sLayout->addRowSpacing(14, 10);

  QStringList configRec;
  configRec = KGlobal::dirs()->findAllResources("appdata",
      "logger/*.desktop");

  if(configRec.count() == 0)
    {
      KMessageBox::error(this,
        i18n("There are no recorder-libraries installed."),
        i18n("No recorders installed."));
    }

  libNameList.clear();

  config->setGroup("Recorder Dialog");
  selectPort->setCurrentItem(config->readNumEntry("Port", 0));
  QString name(config->readEntry("Name", 0));
  config->setGroup(0);

  int typeID(0), typeLoop(0);

  for(QStringList::Iterator it = configRec.begin();
      it != configRec.end(); it++)
    {
      if(loggerConf != NULL)  delete loggerConf;

      loggerConf = new KConfig((*it).latin1());
      if(!loggerConf->hasGroup("Logger Data"))
          warning(i18n("Configfile %1 is corrupt!").arg((*it).latin1()));

      loggerConf->setGroup("Logger Data");
      selectType->insertItem(loggerConf->readEntry("Name"));
      if(loggerConf->readEntry("Name") == name)
          typeID = typeLoop;

      libNameList.append(loggerConf->readEntry("LibName"));
      typeLoop++;
    }

  selectType->setCurrentItem(typeID);

//  connect(connectB, SIGNAL(clicked()), SLOT(slotConnectRecorder()));
}

void RecorderDialog::__addFlightPage()
{
  flightPage = addPage(i18n("Flights"), i18n("Flights"),
      KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* fLayout = new QGridLayout(flightPage, 13, 5, 10, 1);

  flightList = new KListView(flightPage, "flightList");
  flightList->setShowSortIndicator(true);
  flightList->setAllColumnsShowFocus(true);
  colID = flightList->addColumn(i18n("Nr"));
  colDate = flightList->addColumn(i18n("Date"));
  colPilot = flightList->addColumn(i18n("Pilot"));
  colGlider = flightList->addColumn(i18n("Glider"));
  colFirstPoint = flightList->addColumn(i18n("first Point"));
  colLastPoint = flightList->addColumn(i18n("last Point"));

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

void RecorderDialog::__addTaskPage()
{
  taskPage = addPage(i18n("Task"), i18n("Task"),
      KGlobal::instance()->iconLoader()->loadIcon("waypoint", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* tLayout = new QGridLayout(taskPage, 13, 5, 10, 1);

  taskList = new KListView(taskPage, "flightList");
  taskList->setShowSortIndicator(true);
  taskList->setAllColumnsShowFocus(true);
  taskColID = taskList->addColumn(i18n("Nr"));
  taskColName = taskList->addColumn(i18n("Name"));
  taskColPosition = taskList->addColumn(i18n("Position"));

  pilotName = new KLineEdit(taskPage, "pilotName");
  copilotName = new KLineEdit(taskPage, "copilotName");
  gliderID = new KComboBox(taskPage, "gliderID");
  gliderID->setEditable(true);
  gliderType = new KLineEdit(taskPage, "gliderType");
  compClass = new KLineEdit(taskPage, "compClass");
  compID = new KLineEdit(taskPage, "compID");

  QPushButton* writeTask = new QPushButton(i18n("write task to recorder"), taskPage);
  writeTask->setMaximumWidth(writeTask->sizeHint().width() + 15);

  tLayout->addMultiCellWidget(taskList, 0, 0, 0, 6);
  tLayout->addWidget(new QLabel(i18n("Pilot") + "1:", taskPage), 2, 0);
  tLayout->addWidget(pilotName, 2, 2);
  tLayout->addWidget(new QLabel(i18n("Pilot") + "2:", taskPage), 2, 4);
  tLayout->addWidget(copilotName, 2, 6);
  tLayout->addWidget(new QLabel(i18n("Glider-ID"), taskPage), 4, 0);
  tLayout->addWidget(gliderID, 4, 2);
  tLayout->addWidget(new QLabel(i18n("Glidertype"), taskPage), 4, 4);
  tLayout->addWidget(gliderType, 4, 6);
  tLayout->addWidget(new QLabel(i18n("Comp. ID"), taskPage), 6, 0);
  tLayout->addWidget(compClass, 6, 2);
  tLayout->addWidget(new QLabel(i18n("Comp. Class"), taskPage), 6, 4);
  tLayout->addWidget(compID, 6, 6);
  tLayout->addMultiCellWidget(writeTask, 8, 8, 4, 6, Qt::AlignRight);

  tLayout->addColSpacing(1, 5);
  tLayout->addColSpacing(3, 10);
  tLayout->addColSpacing(5, 5);

  tLayout->addRowSpacing(1, 10);
  tLayout->addRowSpacing(3, 5);
  tLayout->addRowSpacing(5, 5);
  tLayout->addRowSpacing(7, 10);

  config->setGroup("Personal Data");
  pilotName->setText(config->readEntry("PilotName", ""));

  config->setGroup(0);

  extern MapContents _globalMapContents;
  BaseFlightElement* task = _globalMapContents.getFlight();

  QListViewItem* item;
  QString idS;

  if(task && task->getTypeID() == BaseFlightElement::Task)
    {
      QList<wayPoint> wpList = ((FlightTask*)task)->getWPList();
      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          item = new QListViewItem(taskList);
          idS.sprintf("%.2d", loop + 1);
          item->setText(taskColID, idS);
          item->setText(taskColName, wpList.at(loop)->name);
          item->setText(taskColPosition, printPos(wpList.at(loop)->origP.x()) +
              " / " + printPos(wpList.at(loop)->origP.x()));
        }
    }
  else
    {
      warning("Keine Aufgaben geplant ...");
      writeTask->setEnabled(false);
    }

  connect(writeTask, SIGNAL(clicked()), SLOT(slotWriteTask()));
}

void RecorderDialog::slotConnectRecorder()
{
  // erstmal hard-codieren ...
  QString name = "libkfrgcs.so";

  // erstmal hard-codieren ...
  portName = "/dev/ttyS1";

  if(isOpen && libName != name)
    {
      // closing old lib
      dlclose(libHandle);
      isOpen = false;
    }

  if(portName == NULL)
    {
      warning(i18n("No port given!"));
      return;
    }

  if(__openLib(name) == -1)
    {
      warning(i18n("Could not open lib!"));
      return;
    }

  slotReadFlightList();

  void* funcH;
  char* error;

  apiID->setText(libName);

  funcH = dlsym(libHandle, "getRecorderName");

  CHECK_ERROR_EXIT

  serID->setText( ((QString (*)(char*))funcH)(qstrdup(portName)) );

  isOpen = true;

  // Fehlerbehandlung!
  // Meldung, wenn Logger nicht gefunden wurde ...
}

void RecorderDialog::slotReadFlightList()
{
  QStringList::Iterator it = libNameList.at(selectType->currentItem());
  libName = (*it).latin1();
  portName = "/dev/" + selectPort->currentText();

  warning(portName);
  warning(libName);

//  return;

  if(!isOpen)
    {
      // Should not happen ...
      if(__openLib(libName) == -1)
        {
          warning(i18n("Could not open lib!"));
          return;
        }
    }

   // Jetzt muss das Flugverzeichnis vom Logger gelesen werden!

  // Hier muss eine andere Reaktion hin
  if(portName == NULL)
    {
      warning(i18n("No port specified!"));
      return;
    }

  __fillDirList();

  flightList->clear();

  QListViewItem* item;
  FRDirEntry* e;
  QTime time;
  QString day;
  QString idS;

  for(unsigned int loop = 0; loop < dirList.count(); loop++)
    {
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

  if(item == NULL)  return;

  QStringList::Iterator it = libNameList.at(selectType->currentItem());

  libName = (*it).latin1();
  portName = "/dev/" + selectPort->currentText();

  QString fileName;
  config->setGroup("Path");
  fileName = config->readEntry("DefaultFlightDirectory") + "/";

  int flightID(item->text(colID).toInt() - 1);

  warning("Lade Flug %d (%d)", flightID, flightList->itemPos(item));
  warning(dirList.at(flightID)->longFileName);
  warning(dirList.at(flightID)->shortFileName);

  if(useLongNames->isChecked())
      fileName += dirList.at(flightID)->longFileName;
  else
      fileName += dirList.at(flightID)->shortFileName;

  warning(fileName);

  char* error;
  void* funcH;

  funcH = dlsym(libHandle, "downloadFlight");

  CHECK_ERROR

  int ret;
  ret = ((int (*)(char*, int, int, char*)) funcH)(qstrdup(portName),
      flightID, !useFastDownload->isChecked(), qstrdup(fileName));
}

void RecorderDialog::slotWriteTask()
{
  FRTaskDeclaration taskDecl;
  taskDecl.pilotA = pilotName->text();
  taskDecl.pilotB = copilotName->text();
  taskDecl.gliderID = gliderID->currentText();
  taskDecl.gliderType = gliderType->text();
  taskDecl.compID = compID->text();
  taskDecl.compClass = compClass->text();

  QList<FRTaskPoint> taskPointList;

  extern MapContents _globalMapContents;
  BaseFlightElement* task = _globalMapContents.getFlight();

  if(task && task->getTypeID() == BaseFlightElement::Task)
    {
      QList<wayPoint> wpList = ((FlightTask*)task)->getWPList();
      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          taskPointList.append(new FRTaskPoint);
          taskPointList.last()->name = wpList.at(loop)->name;
          taskPointList.last()->latPos = wpList.at(loop)->projP.x();
          taskPointList.last()->lonPos = wpList.at(loop)->projP.y();
        }
    }

  QStringList::Iterator it = libNameList.at(selectType->currentItem());
  libName = (*it).latin1();
  portName = "/dev/" + selectPort->currentText();

  char* error;
  void* funcH;

  funcH = dlsym(libHandle, "writeTask");

  CHECK_ERROR

  int ret;
  ret = ((int (*)(FRTaskDeclaration*, QList<FRTaskPoint>, char*)) funcH)(
      &taskDecl, taskPointList, qstrdup(portName));
}

int RecorderDialog::__fillDirList()
{
  char* error;
  void* funcH;

  funcH = dlsym(libHandle, "getFlightDir");

  CHECK_ERROR

  int ret;
  ret = ((int (*)(char*, QList<FRDirEntry>*)) funcH)(qstrdup(portName),
      &dirList);

  switch(ret)
    {
      case -2:
        warning(i18n("Method not implemented!"));
        break;
      case -1:
        warning(i18n("An error occured while reading the flight-directory!"));
        break;
      case 1:
        // Wenn keine Flüge gelesen wurden, muss die Funktion einen
        // entsprechenden Fehlercode liefern!
        // Ok!
        break;
      default:
        warning(i18n("Unknown return-code recieved!"));
        break;
    }
  return ret;
}

int RecorderDialog::__openLib(QString libN)
{
  char* error;

  libHandle = dlopen(KGlobal::dirs()->findResource("lib", libN), RTLD_NOW);

  CHECK_ERROR_RETURN

  void* funcH;

  funcH = dlsym(libHandle, "getLibName");

  CHECK_ERROR_RETURN

  libName = ((QString (*)())funcH)();

  isOpen = true;

  return 0;
}
