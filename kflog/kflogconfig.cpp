/***********************************************************************
**
**   kflogconfig.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   kflogconfig.cpp
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

#include "kflogconfig.h"
#include <configdrawelement.h>
#include <configprintelement.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapdefaults.h>

#include <unistd.h>
#include <pwd.h>
//#include <sys/types.h>
#include <stdlib.h>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstddirs.h>
#include <ktabctl.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

KFLogConfig::KFLogConfig(QWidget* parent, KConfig* cnf, const char* name)
  : KDialogBase(IconList, i18n("KFlog Setup"), Ok|Cancel, Ok,
      parent, name, true, true),
    config(cnf)
{
  __addIDTab();
  __addPathTab();
  __addScaleTab();
  __addMapTab();
//  __addTopographyTab();

  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}

KFLogConfig::~KFLogConfig()
{

}

void KFLogConfig::slotOk()
{
  config->setGroup("General Options");
  config->writeEntry("Version", "2.0.2");

  config->setGroup("Path");
  config->writeEntry("DefaultFlightDirectory", igcPathE->text());
  config->writeEntry("DefaultTaskDirectory", taskPathE->text());
  config->writeEntry("DefaultWaypointDirectory", waypointPathE->text());
  config->writeEntry("DefaultMapDirectory", mapPathE->text());

  config->setGroup("Scale");
  config->writeEntry("Lower Limit", lLimitN->value());
  config->writeEntry("Upper Limit", uLimitN->value());
  config->writeEntry("Switch Scale", switchScaleN->value());
  config->writeEntry("Border 1", reduce1N->value());
  config->writeEntry("Border 2", reduce2N->value());
  config->writeEntry("Border 3", reduce3N->value());

  config->setGroup("Map Data");
  config->writeEntry("Homesite", homeNameE->text());
  config->writeEntry("Homesite Latitude",
      MapContents::degreeToNum(homeLatE->text()));
  config->writeEntry("Homesite Longitude",
      MapContents::degreeToNum(homeLonE->text()));

  config->sync();
  config->setGroup(0);

  emit scaleChanged(lLimitN->value(), uLimitN->value());
  emit configOk();
  accept();
}

void KFLogConfig::slotSearchFlightPath()
{
  QString temp = KFileDialog::getExistingDirectory(igcPathE->text(), this,
      i18n("Select Default-Directory for the IGC-Files"));

  if(temp != 0)  igcPathE->setText(temp);
}

void KFLogConfig::slotSearchMapPath()
{
  QString temp = KFileDialog::getExistingDirectory(mapPathE->text(), this,
      i18n("Select Directory for the Map-Files"));

  if(temp != 0)  mapPathE->setText(temp);
}

void KFLogConfig::slotSearchTaskPath()
{
  QString temp = KFileDialog::getExistingDirectory(taskPathE->text(), this,
      i18n("Select Default-Directory for the Task-Files"));

  if(temp != 0)  taskPathE->setText(temp);
}

void KFLogConfig::slotSearchWaypointPath()
{
  QString temp = KFileDialog::getExistingDirectory(waypointPathE->text(), this,
      i18n("Select Default-Directory for the Waypoint-Files"));

  if(temp != 0)  waypointPathE->setText(temp);
}

void KFLogConfig::slotShowLowerLimit(int value)
{
  lLimitN->display(__setScaleValue(value));
}

void KFLogConfig::slotShowUpperLimit(int value)
{
  uLimitN->display(__setScaleValue(value));
}

void KFLogConfig::slotShowSwitchScale(int value)
{
  switchScaleN->display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleA(int value)
{
  reduce1N->display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleB(int value)
{
  reduce2N->display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleC(int value)
{
  reduce3N->display(__setScaleValue(value));
}

void KFLogConfig::slotDefaultScale()
{
  lLimit->setValue(__getScaleValue(L_LIMIT));
  lLimitN->display(L_LIMIT);
  uLimit->setValue(__getScaleValue(U_LIMIT));
  uLimitN->display(U_LIMIT);
  switchScale->setValue(__getScaleValue(SWITCH_S));
  switchScaleN->display(SWITCH_S);
  reduce1->setValue(__getScaleValue(BORDER_1));
  reduce1N->display(BORDER_1);
  reduce2->setValue(__getScaleValue(BORDER_2));
  reduce2N->display(BORDER_2);
  reduce3->setValue(__getScaleValue(BORDER_3));
  reduce3N->display(BORDER_3);
}

void KFLogConfig::slotDefaultPath()
{
  igcPathE->setText(getpwuid(getuid())->pw_dir);
  taskPathE->setText(getpwuid(getuid())->pw_dir);
  waypointPathE->setText(getpwuid(getuid())->pw_dir);
  mapPathE->setText(KGlobal::dirs()->findResource("appdata", "mapdata/"));
}

void KFLogConfig::__addMapTab()
{
  mapPage = addPage(i18n("Map-Elements"),i18n("Map Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("kflog", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGroupBox* elementBox = new QGroupBox(mapPage, "elementBox");
  elementBox->setTitle(i18n("visible Map-Elements"));

  elementSelect = new KComboBox(mapPage, "elementBox");
//  elementSelect->setMaximumWidth(300);
  elementSelect->insertItem(i18n("Road"), Road);
  elementSelect->insertItem(i18n("Highway"), Highway);
  elementSelect->insertItem(i18n("Railway"), Railway);
  elementSelect->insertItem(i18n("River / Lake"), River);
  elementSelect->insertItem(i18n("Canal"), Canal);
  elementSelect->insertItem(i18n("City"), City);
  elementSelect->insertItem(i18n("Airspace A"), AirA);
  elementSelect->insertItem(i18n("Airspace B"), AirB);
  elementSelect->insertItem(i18n("Airspace C"), AirC);
  elementSelect->insertItem(i18n("Airspace D"), AirD);
  elementSelect->insertItem(i18n("Airspace E (low)"), AirElow);
  elementSelect->insertItem(i18n("Airspace E (high)"), AirEhigh);
  elementSelect->insertItem(i18n("Airspace F"), AirF);
  elementSelect->insertItem(i18n("Control C"), ControlC);
  elementSelect->insertItem(i18n("Control D"), ControlD);
  elementSelect->insertItem(i18n("Danger"), Danger);
  elementSelect->insertItem(i18n("Low flight area"), LowFlight);
  elementSelect->insertItem(i18n("Restricted"), Restricted);
  elementSelect->insertItem(i18n("TMZ"), TMZ);

  QPushButton* defaultElements = new QPushButton(i18n("Default"), mapPage,
      "defaultElements");
  defaultElements->setMaximumWidth(defaultElements->sizeHint().width() + 10);
  defaultElements->setMinimumHeight(defaultElements->sizeHint().height() + 2);

  KTabCtl* tabView = new KTabCtl(mapPage);
  QFrame* screenFrame = new QFrame(tabView, "ConfigDrawFrame");
  ConfigDrawElement* drawConfig = new ConfigDrawElement(screenFrame, config);

  QFrame* printFrame = new QFrame(tabView, "ConfigDrawFrame");
  ConfigPrintElement* printConfig = new ConfigPrintElement(printFrame, config);

  tabView->addTab(screenFrame, i18n("Display"));
  tabView->addTab(printFrame, i18n("Print"));

  QGridLayout* elLayout = new QGridLayout(mapPage, 7, 5, 8, 1);
  elLayout->addMultiCellWidget(elementBox, 0, 4, 0, 4);
  elLayout->addMultiCellWidget(elementSelect, 1, 1, 1, 2, AlignLeft);
  elLayout->addMultiCellWidget(tabView, 3, 3, 1, 3);
  elLayout->addMultiCellWidget(defaultElements, 6, 6, 0, 1, AlignLeft);

  elLayout->addRowSpacing(0, 20);
  elLayout->addRowSpacing(2, 5);
  elLayout->setRowStretch(3, 1);
  elLayout->addRowSpacing(4, 5);
  elLayout->addRowSpacing(5, 15);

  elLayout->addColSpacing(0, 10);
  elLayout->setColStretch(0, 0);
  elLayout->setColStretch(3, 1);
  elLayout->addColSpacing(4, 10);
  elLayout->setColStretch(4, 0);

  connect(defaultElements, SIGNAL(clicked()), drawConfig,
      SLOT(slotDefaultElements()));
  connect(defaultElements, SIGNAL(clicked()), printConfig,
      SLOT(slotDefaultElements()));
  connect(elementSelect, SIGNAL(activated(int)), drawConfig,
      SLOT(slotSelectElement(int)));
  connect(this, SIGNAL(configOk()), drawConfig, SLOT(slotOk()));
  connect(elementSelect, SIGNAL(activated(int)), printConfig,
      SLOT(slotSelectElement(int)));
  connect(this, SIGNAL(configOk()), printConfig, SLOT(slotOk()));

  drawConfig->slotSelectElement(0);
  printConfig->slotSelectElement(0);
}

void KFLogConfig::__addScaleTab()
{
  config->setGroup("Scale");
  int ll = config->readNumEntry("Lower Limit", L_LIMIT);
  int ul = config->readNumEntry("Upper Limit", U_LIMIT);
  int sw = config->readNumEntry("Switch Scale", SWITCH_S);
  int b1 = config->readNumEntry("Border 1", BORDER_1);
  int b2 = config->readNumEntry("Border 2", BORDER_2);
  int b3 = config->readNumEntry("Border 3", BORDER_3);

  scalePage = addPage(i18n("Map-Scales"),i18n("Map-Scale Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("viewmag", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGroupBox* scaleLimits = new QGroupBox(scalePage, "scaleLimitBox");
  scaleLimits->setTitle(i18n("Scale-Range:"));

  QLabel* lLimitText = new QLabel(i18n("lower limit"), scalePage);
  lLimit = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  lLimit->setMinimumHeight(lLimit->sizeHint().height() + 5);
  lLimit->setMaximumHeight(lLimit->sizeHint().height() + 20);
  lLimit->setMinimumWidth(200);
  lLimitN = new QLCDNumber(5, scalePage);
  lLimitN->setMinimumWidth(lLimitN->sizeHint().width() + 10);
  lLimit->setValue(__getScaleValue(ll));
  lLimitN->display(ll);

  QLabel* uLimitText = new QLabel(i18n("upper limit"), scalePage);
  uLimit = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  uLimit->setMinimumHeight(uLimit->sizeHint().height() + 5);
  uLimit->setMaximumHeight(uLimit->sizeHint().height() + 20);
  uLimitN = new QLCDNumber(5, scalePage);
  uLimit->setValue(__getScaleValue(ul));
  uLimitN->display(ul);

  QGroupBox* borderBox = new QGroupBox(scalePage, "borderBox");
  borderBox->setTitle("Scale-Borders:");

  QLabel* switchText = new QLabel(i18n("use small icons"), scalePage);

  switchScale = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  switchScale->setMinimumHeight(switchScale->sizeHint().height() + 5);
  switchScale->setMaximumHeight(switchScale->sizeHint().height() + 20);
  switchScaleN = new QLCDNumber(5, scalePage);
  switchScale->setValue(__getScaleValue(sw));
  switchScaleN->display(sw);

  QLabel* reduce1Text = new QLabel(i18n("border #1"), scalePage);
  reduce1 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce1->setMinimumHeight(reduce1->sizeHint().height() + 5);
  reduce1->setMaximumHeight(reduce1->sizeHint().height() + 20);
  reduce1N = new QLCDNumber(5, scalePage);
  reduce1->setValue(__getScaleValue(b1));
  reduce1N->display(b1);

  QLabel* reduce2Text = new QLabel(i18n("border #2"), scalePage);
  reduce2 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce2->setMinimumHeight(reduce2->sizeHint().height() + 5);
  reduce2->setMaximumHeight(reduce2->sizeHint().height() + 20);
  reduce2N = new QLCDNumber(5, scalePage);
  reduce2->setValue(__getScaleValue(b2));
  reduce2N->display(b2);

  QLabel* reduce3Text = new QLabel(i18n("border #3"), scalePage);
  reduce3 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce3->setMinimumHeight(reduce3->sizeHint().height() + 5);
  reduce3->setMaximumHeight(reduce3->sizeHint().height() + 20);
  reduce3N = new QLCDNumber(5, scalePage);
  reduce3->setValue(__getScaleValue(b3));
  reduce3N->display(b3);

  QPushButton* defaultScale = new QPushButton(i18n("Default"), scalePage,
      "defaultScale");
  defaultScale->setMaximumWidth(defaultScale->sizeHint().width() + 10);
  defaultScale->setMinimumHeight(defaultScale->sizeHint().height() + 2);

  QGridLayout* scaleLayout = new QGridLayout(scalePage, 17, 7, 8, 1);
  scaleLayout->addMultiCellWidget(scaleLimits, 0, 4, 0, 6);
  scaleLayout->addWidget(lLimitText, 1, 1);
  scaleLayout->addWidget(lLimit, 1, 3);
  scaleLayout->addWidget(lLimitN, 1, 5);
  scaleLayout->addWidget(uLimitText, 3, 1);
  scaleLayout->addWidget(uLimit, 3, 3);
  scaleLayout->addWidget(uLimitN, 3, 5);

  scaleLayout->addMultiCellWidget(borderBox, 6, 14, 0, 6);
  scaleLayout->addWidget(switchText, 7, 1);
  scaleLayout->addWidget(switchScale, 7, 3);
  scaleLayout->addWidget(switchScaleN, 7, 5);
  scaleLayout->addWidget(reduce1Text, 9, 1);
  scaleLayout->addWidget(reduce1, 9, 3);
  scaleLayout->addWidget(reduce1N, 9, 5);
  scaleLayout->addWidget(reduce2Text, 11, 1);
  scaleLayout->addWidget(reduce2, 11, 3);
  scaleLayout->addWidget(reduce2N, 11, 5);
  scaleLayout->addWidget(reduce3Text, 13, 1);
  scaleLayout->addWidget(reduce3, 13, 3);
  scaleLayout->addWidget(reduce3N, 13, 5);

  scaleLayout->addMultiCellWidget(defaultScale, 16, 16, 0, 1, AlignLeft);

  scaleLayout->addColSpacing(0, 10);
  scaleLayout->addColSpacing(2, 5);
  scaleLayout->addColSpacing(4, 10);
  scaleLayout->addColSpacing(6, 10);

  scaleLayout->setColStretch(3, 1);

  scaleLayout->addRowSpacing(0, 20);
  scaleLayout->addRowSpacing(4, 10);
  scaleLayout->addRowSpacing(5, 10);
  scaleLayout->addRowSpacing(6, 20);
  scaleLayout->addRowSpacing(14, 10);
  scaleLayout->addRowSpacing(15, 10);

  connect(defaultScale, SIGNAL(clicked()), SLOT(slotDefaultScale()));
  connect(lLimit, SIGNAL(valueChanged(int)), SLOT(slotShowLowerLimit(int)));
  connect(uLimit, SIGNAL(valueChanged(int)), SLOT(slotShowUpperLimit(int)));
  connect(switchScale, SIGNAL(valueChanged(int)),
      SLOT(slotShowSwitchScale(int)));
  connect(reduce1, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleA(int)));
  connect(reduce2, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleB(int)));
  connect(reduce3, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleC(int)));

  config->setGroup(0);
}

void KFLogConfig::__addPathTab()
{
  config->setGroup("Path");
  QString flightDir = config->readEntry("DefaultFlightDirectory",
      getpwuid(getuid())->pw_dir);
  QString taskDir = config->readEntry("DefaultTaskDirectory",
      getpwuid(getuid())->pw_dir);
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);
  QString mapDir = config->readEntry("DefaultMapDirectory",
      KGlobal::dirs()->findResource("appdata", "mapdata/"));

  pathPage = addPage(i18n("Path"),i18n("Path Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("fileopen", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* pathLayout = new QGridLayout(pathPage, 17, 5, 8, 1);

  QGroupBox* igcGroup = new QGroupBox(pathPage, "igcGroup");
  igcGroup->setTitle(i18n("Flight-directory:"));

  igcPathE  = new QLineEdit(pathPage, "igcPathE");
  igcPathE->setMinimumWidth(150);
  igcPathE->setText(flightDir);

  QPushButton* igcPathSearch = new QPushButton(pathPage);
  igcPathSearch->setPixmap(BarIcon("fileopen"));
  igcPathSearch->setMinimumWidth(igcPathSearch->sizeHint().width() + 5);
  igcPathSearch->setMinimumHeight(igcPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(igcGroup, 0, 2, 0, 4);
  pathLayout->addWidget(igcPathE, 1, 1);
  pathLayout->addWidget(igcPathSearch, 1, 3);

  pathLayout->addColSpacing(0, 10);
  pathLayout->addColSpacing(2, 10);
  pathLayout->addColSpacing(4, 10);

  pathLayout->addRowSpacing(0, 20);
  pathLayout->addRowSpacing(1, 10);
  pathLayout->addRowSpacing(2, 10);

  pathLayout->addRowSpacing(3, 20);
  pathLayout->setRowStretch(3, 1);

  QGroupBox* taskGroup = new QGroupBox(pathPage, "taskGroup");
  taskGroup->setTitle(i18n("Task-directory:"));

  taskPathE = new QLineEdit(pathPage, "taskPathE");
  taskPathE->setMinimumWidth(150);
  taskPathE->setText(taskDir);

  QPushButton* taskPathSearch = new QPushButton(pathPage);
  taskPathSearch->setPixmap(BarIcon("fileopen"));
  taskPathSearch->setMinimumWidth(taskPathSearch->sizeHint().width() + 5);
  taskPathSearch->setMinimumHeight(taskPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(taskGroup, 4, 6, 0, 4);
  pathLayout->addWidget(taskPathE, 5, 1);
  pathLayout->addWidget(taskPathSearch, 5, 3);

  pathLayout->addRowSpacing(4, 20);
  pathLayout->addRowSpacing(5, 10);
  pathLayout->addRowSpacing(6, 10);

  pathLayout->addRowSpacing(7, 20);
  pathLayout->setRowStretch(7, 1);

  QGroupBox* waypointGroup = new QGroupBox(pathPage, "waypointGroup");
  waypointGroup->setTitle(i18n("Waypoint-directory:"));

  waypointPathE = new QLineEdit(pathPage, "waypointPathE");
  waypointPathE->setMinimumWidth(150);
  waypointPathE->setText(wayPointDir);

  QPushButton* waypointPathSearch = new QPushButton(pathPage);
  waypointPathSearch->setPixmap(BarIcon("fileopen"));
  waypointPathSearch->setMinimumWidth(waypointPathSearch->sizeHint().width() + 5);
  waypointPathSearch->setMinimumHeight(waypointPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(waypointGroup, 8, 10, 0, 4);
  pathLayout->addWidget(waypointPathE, 9, 1);
  pathLayout->addWidget(waypointPathSearch, 9, 3);

  pathLayout->addRowSpacing(8, 20);
  pathLayout->addRowSpacing(9, 10);
  pathLayout->addRowSpacing(10, 10);

  pathLayout->addRowSpacing(11, 20);
  pathLayout->setRowStretch(11, 1);

  QGroupBox* mapGroup = new QGroupBox(pathPage, "mapGroup");
  mapGroup->setTitle(i18n("Map-directory:"));

  mapPathE = new QLineEdit(pathPage, "mapPathE");
  mapPathE->setMinimumWidth(150);
  mapPathE->setText(mapDir);

  QPushButton* mapPathSearch = new QPushButton(pathPage);
  mapPathSearch->setPixmap(BarIcon("fileopen"));
  mapPathSearch->setMinimumWidth(mapPathSearch->sizeHint().width() + 5);
  mapPathSearch->setMinimumHeight(mapPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(mapGroup, 12, 14, 0, 4);
  pathLayout->addWidget(mapPathE, 13, 1);
  pathLayout->addWidget(mapPathSearch, 13, 3);

  pathLayout->addRowSpacing(12, 20);
  pathLayout->addRowSpacing(13, 10);
  pathLayout->addRowSpacing(14, 10);

  QPushButton* defaultPath = new QPushButton(i18n("Default"), pathPage,
      "defaultPath");
  defaultPath->setMaximumWidth(defaultPath->sizeHint().width() + 10);
  defaultPath->setMinimumHeight(defaultPath->sizeHint().height() + 2);

  pathLayout->addMultiCellWidget(defaultPath, 16, 16, 0, 1, AlignLeft);

  pathLayout->addRowSpacing(15, 10);
  pathLayout->setRowStretch(15, 1);

  connect(igcPathSearch, SIGNAL(clicked()), SLOT(slotSearchFlightPath()));
  connect(mapPathSearch, SIGNAL(clicked()), SLOT(slotSearchMapPath()));
  connect(taskPathSearch, SIGNAL(clicked()), SLOT(slotSearchTaskPath()));
  connect(waypointPathSearch, SIGNAL(clicked()), SLOT(slotSearchWaypointPath()));
  connect(defaultPath, SIGNAL(clicked()), SLOT(slotDefaultPath()));

  config->setGroup(0);
}

void KFLogConfig::__addTopographyTab()
{
  topoPage = addPage(i18n("Topography"),i18n("Topography Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("kflog", KIcon::NoGroup,
          KIcon::SizeLarge));
}

void KFLogConfig::__addIDTab()
{
  config->setGroup("Map Data");
  int homeLat = config->readNumEntry("Homesite Latitude", HOME_DEFAULT_LAT);
  int homeLon = config->readNumEntry("Homesite Longitude", HOME_DEFAULT_LON);

  idPage = addPage(i18n("Identity"),i18n("Personal Information"),
      KGlobal::instance()->iconLoader()->loadIcon("identity", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* idLayout = new QGridLayout(idPage, 13, 5, 8, 1);

  QGroupBox* homeGroup = new QGroupBox(idPage, "homeGroup");
  homeGroup->setTitle(i18n("Homesite:"));

  homeNameE = new QLineEdit(idPage, "homeNameE");
  homeNameE->setMinimumWidth(150);
  homeNameE->setText(config->readEntry("Homesite", ""));
  homeLatE  = new QLineEdit(idPage, "homeLatE");
  homeLatE->setMinimumWidth(150);
  homeLatE->setText(printPos(homeLat, true));
  homeLonE  = new QLineEdit(idPage, "homeLonE");
  homeLonE->setMinimumWidth(150);
  homeLonE->setText(printPos(homeLon, false));

  idLayout->addMultiCellWidget(homeGroup, 0, 6, 0, 4);
  idLayout->addWidget(new QLabel(i18n("Homesite"), idPage), 1, 1);
  idLayout->addWidget(homeNameE, 1, 3);
  idLayout->addWidget(new QLabel(i18n("Latitude"), idPage), 3, 1);
  idLayout->addWidget(homeLatE, 3, 3);
  idLayout->addWidget(new QLabel(i18n("Longitude"), idPage), 5, 1);
  idLayout->addWidget(homeLonE, 5, 3);

  idLayout->addColSpacing(0, 10);
  idLayout->addColSpacing(2, 10);
  idLayout->addColSpacing(4, 10);

  idLayout->addRowSpacing(0, 20);
  idLayout->addRowSpacing(1, 10);
  idLayout->addRowSpacing(2, 10);

  idLayout->addRowSpacing(3, 20);

  config->setGroup(0);
}

int KFLogConfig::__setScaleValue(int value)
{
  if(value <= 40) return (value * 5);
  else if(value <= 70) return (200 + (value - 40) * 10);
  else if(value <= 95) return (500 + (value - 70) * 20);
  else if(value <= 105) return (1000 + (value - 95) * 50);
  else return (2000 + (value - 105) * 100);
}

int KFLogConfig::__getScaleValue(double scale)
{
  if(scale <= 200) return ((int) scale / 5);
  else if(scale <= 500) return (((int) scale - 200) / 10 + 40);
  else if(scale <= 1000) return (((int) scale - 500) / 20 + 70);
  else if(scale <= 2000) return (((int) scale - 1000) / 50 + 95);
  else return (((int) scale - 2000) / 100 + 125);
}
