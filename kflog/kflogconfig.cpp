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
#include <qradiobutton.h>

#include <guicontrols/coordedit.h>

KFLogConfig::KFLogConfig(QWidget* parent, KConfig* cnf, const char* name)
  : KDialogBase(IconList, i18n("KFlog Setup"), Ok|Cancel, Ok,
      parent, name, true, true),
    config(cnf),
    currentProjType(-1)
{
  __addIDTab();
  __addPathTab();
  __addScaleTab();
  __addMapTab();
  __addProjectionTab();
//  __addTopographyTab();
  __addWaypointTab();

  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}

KFLogConfig::~KFLogConfig()
{

}

void KFLogConfig::slotOk()
{
  slotSelectProjection(-1);

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
  config->writeEntry("Projection Type", projectionSelect->currentItem());

  config->setGroup("Lambert Projection");
  config->writeEntry("Parallel1", lambertV1);
  config->writeEntry("Parallel2", lambertV2);
  config->writeEntry("Origin", lambertOrigin);

  config->setGroup("Cylindrical Projection");
  config->writeEntry("Parallel", cylinPar);

  config->setGroup("Personal Data");
  config->writeEntry("PreName", preNameE->text());
  config->writeEntry("SurName", surNameE->text());
  config->writeEntry("Birthday", dateOfBirthE->text());

  config->setGroup("Waypoints");
  config->writeEntry("DefaultWaypointCatalog", waypointButtonGroup->id(waypointButtonGroup->selected()));
  config->writeEntry("DefaultCatalogName", catalogPathE->text());

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

void KFLogConfig::slotSelectProjection(int index)
{
  switch(currentProjType)
    {
      case 0:
        lambertV1 = MapContents::degreeToNum(firstParallel->text());
        lambertV2 = MapContents::degreeToNum(secondParallel->text());
        lambertOrigin = MapContents::degreeToNum(originLongitude->text());
        break;
      case 1:
        cylinPar = MapContents::degreeToNum(firstParallel->text());
        break;
    }

  switch(index)
    {
      case 0:    // Lambert
        secondParallel->setEnabled(true);
        originLongitude->setEnabled(true);
        firstParallel->setText(printPos(lambertV1, true));
        secondParallel->setText(printPos(lambertV2, true));
        originLongitude->setText(printPos(lambertOrigin, false));
        break;
      case 1:    // plate carre
        secondParallel->setEnabled(false);
        originLongitude->setEnabled(false);
        firstParallel->setText(printPos(cylinPar, true));
        break;
    }
  currentProjType = index;
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

void KFLogConfig::slotDefaultProjection()
{
  lambertV1 = 32400000;
  lambertV2 = 30000000;
  lambertOrigin = 0;

  cylinPar = 27000000;

  currentProjType = -1;

  projectionSelect->setCurrentItem(0);
  slotSelectProjection(0);
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

  elementSelect = new KComboBox(mapPage, "elementSelect");
//  elementSelect->setMaximumWidth(300);
  elementSelect->insertItem(i18n("Road"), KFLogConfig::Road);
  elementSelect->insertItem(i18n("Highway"), KFLogConfig::Highway);
  elementSelect->insertItem(i18n("Railway"), KFLogConfig::Railway);
  elementSelect->insertItem(i18n("River / Lake"), KFLogConfig::River);
  elementSelect->insertItem(i18n("Canal"), KFLogConfig::Canal);
  elementSelect->insertItem(i18n("City"), KFLogConfig::City);
  elementSelect->insertItem(i18n("Airspace A"), KFLogConfig::AirA);
  elementSelect->insertItem(i18n("Airspace B"), KFLogConfig::AirB);
  elementSelect->insertItem(i18n("Airspace C"), KFLogConfig::AirC);
  elementSelect->insertItem(i18n("Airspace D"), KFLogConfig::AirD);
  elementSelect->insertItem(i18n("Airspace E (low)"), KFLogConfig::AirElow);
  elementSelect->insertItem(i18n("Airspace E (high)"), KFLogConfig::AirEhigh);
  elementSelect->insertItem(i18n("Airspace F"), KFLogConfig::AirF);
  elementSelect->insertItem(i18n("Control C"), KFLogConfig::ControlC);
  elementSelect->insertItem(i18n("Control D"), KFLogConfig::ControlD);
  elementSelect->insertItem(i18n("Danger"), KFLogConfig::Danger);
  elementSelect->insertItem(i18n("Low flight area"), KFLogConfig::LowFlight);
  elementSelect->insertItem(i18n("Restricted"), KFLogConfig::Restricted);
  elementSelect->insertItem(i18n("TMZ"), KFLogConfig::Tmz);
  // Reihenfolge ???
  elementSelect->insertItem(i18n("Forest"), KFLogConfig::Forest);
  elementSelect->insertItem(i18n("Trail"), KFLogConfig::Trail);
  elementSelect->insertItem(i18n("double Railway"), KFLogConfig::Railway_D);
  elementSelect->insertItem(i18n("Aerial Cable"), KFLogConfig::Aerial_Cable);
  elementSelect->insertItem(i18n("temporarily River / Lake"), KFLogConfig::River_T);
  elementSelect->insertItem(i18n("Glacier"), KFLogConfig::Glacier);
  elementSelect->insertItem(i18n("Pack Ice"), KFLogConfig::Pack_Ice);

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

void KFLogConfig::__addProjectionTab()
{
  projPage = addPage(i18n("Map-Projection"),i18n("Configuration of Map-Projection"),
      KGlobal::instance()->iconLoader()->loadIcon("projection", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGroupBox* projType = new QGroupBox(projPage, "projectionSelectBox");
  projType->setTitle(i18n("Type of Projection") + ":");

  projectionSelect = new KComboBox(projPage, "projectionSelect");
  projectionSelect->insertItem(i18n("Conical orthomorphic (Lambert)"));
  projectionSelect->insertItem(i18n("Cylindrical Equidistant (Plate Carre)"));

  QGroupBox* projConf = new QGroupBox(projPage, "projectionSelectBox");
  projConf->setTitle(i18n("Setup Projection") + ":");

  firstParallel = new LatEdit(projPage, "firstParallel");
  secondParallel = new LatEdit(projPage, "secondParallel");
  originLongitude = new LongEdit(projPage, "originLongitude");

  QPushButton* defaultProj = new QPushButton(i18n("Default"), projPage,
      "defaultScale");
  defaultProj->setMaximumWidth(defaultProj->sizeHint().width() + 10);
  defaultProj->setMinimumHeight(defaultProj->sizeHint().height() + 2);

  QGridLayout* projLayout = new QGridLayout(projPage, 17, 7, 8, 1);
  projLayout->addMultiCellWidget(projType, 0, 2, 0, 6);
  projLayout->addMultiCellWidget(projectionSelect, 1, 1, 1, 5);
  projLayout->addMultiCellWidget(projConf, 4, 10, 0, 6);
  projLayout->addWidget(new QLabel(i18n("1. Standard Parallel") + ":", projPage),
      5, 1);
  projLayout->addWidget(firstParallel, 5, 3);
  projLayout->addWidget(new QLabel(i18n("2. Standard Parallel") + ":", projPage),
      7, 1);
  projLayout->addWidget(secondParallel, 7, 3);
  projLayout->addWidget(new QLabel(i18n("Origin Longitude") + ":", projPage),
      9, 1);
  projLayout->addWidget(originLongitude, 9, 3);

  projLayout->addMultiCellWidget(defaultProj, 16, 16, 0, 1, AlignLeft);

  projLayout->addColSpacing(0, 10);
  projLayout->addColSpacing(2, 5);
  projLayout->addColSpacing(4, 10);
  projLayout->addColSpacing(6, 10);

  projLayout->setColStretch(3, 1);

  projLayout->addRowSpacing(0, 25);
  projLayout->addRowSpacing(2, 5);
  projLayout->addRowSpacing(4, 25);
  projLayout->addRowSpacing(6, 5);
  projLayout->addRowSpacing(8, 5);
  projLayout->addRowSpacing(15, 10);

  connect(defaultProj, SIGNAL(clicked()), SLOT(slotDefaultProjection()));
  connect(projectionSelect, SIGNAL(activated(int)),
      SLOT(slotSelectProjection(int)));

  config->setGroup("Lambert Projection");
  lambertV1 = config->readNumEntry("Parallel1", 32400000);
  lambertV2 = config->readNumEntry("Parallel2", 30000000);
  lambertOrigin = config->readNumEntry("Origin", 0);

  config->setGroup("Cylindrical Projection");
  cylinPar = config->readNumEntry("Parallel", 27000000);

  config->setGroup("Map Data");
  int projIndex = config->readNumEntry("Projection Type", 0);

  projectionSelect->setCurrentItem(projIndex);
  slotSelectProjection(projIndex);

  config->setGroup(0);
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
  borderBox->setTitle("Scale-Thresholds:");

  QLabel* switchText = new QLabel(i18n("use small icons"), scalePage);

  switchScale = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  switchScale->setMinimumHeight(switchScale->sizeHint().height() + 5);
  switchScale->setMaximumHeight(switchScale->sizeHint().height() + 20);
  switchScaleN = new QLCDNumber(5, scalePage);
  switchScale->setValue(__getScaleValue(sw));
  switchScaleN->display(sw);

  QLabel* reduce1Text = new QLabel(i18n("threshold #1"), scalePage);
  reduce1 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce1->setMinimumHeight(reduce1->sizeHint().height() + 5);
  reduce1->setMaximumHeight(reduce1->sizeHint().height() + 20);
  reduce1N = new QLCDNumber(5, scalePage);
  reduce1->setValue(__getScaleValue(b1));
  reduce1N->display(b1);

  QLabel* reduce2Text = new QLabel(i18n("threshold #2"), scalePage);
  reduce2 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce2->setMinimumHeight(reduce2->sizeHint().height() + 5);
  reduce2->setMaximumHeight(reduce2->sizeHint().height() + 20);
  reduce2N = new QLCDNumber(5, scalePage);
  reduce2->setValue(__getScaleValue(b2));
  reduce2N->display(b2);

  QLabel* reduce3Text = new QLabel(i18n("threshold #3"), scalePage);
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

  scaleLayout->addRowSpacing(0, 25);
  scaleLayout->addRowSpacing(4, 5);
  scaleLayout->addRowSpacing(5, 10);
  scaleLayout->addRowSpacing(6, 25);
  scaleLayout->addRowSpacing(14, 5);
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

  pathLayout->addRowSpacing(0, 25);
  pathLayout->addRowSpacing(1, 10);
  pathLayout->addRowSpacing(2, 10);

  pathLayout->addRowSpacing(3, 25);
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

  pathLayout->addRowSpacing(4, 25);
  pathLayout->addRowSpacing(5, 10);
  pathLayout->addRowSpacing(6, 10);

  pathLayout->addRowSpacing(7, 25);
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
  idPage = addPage(i18n("Identity"),i18n("Personal Information"),
      KGlobal::instance()->iconLoader()->loadIcon("identity", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* idLayout = new QGridLayout(idPage, 18, 5, 8, 1);

  QGroupBox* pilotGroup = new QGroupBox(idPage, "pilotGroup");
  pilotGroup->setTitle(i18n("Pilot") + ":");

  preNameE = new QLineEdit(idPage, "preNameE");
  surNameE = new QLineEdit(idPage, "surNameE");
  dateOfBirthE = new QLineEdit(idPage, "dateOfBirthE");

  idLayout->addMultiCellWidget(pilotGroup, 0, 7, 0, 4);
  idLayout->addWidget(new QLabel(i18n("Prename"), idPage), 2, 1);
  idLayout->addWidget(preNameE, 2, 3);
  idLayout->addWidget(new QLabel(i18n("Surname"), idPage), 4, 1);
  idLayout->addWidget(surNameE, 4, 3);
  idLayout->addWidget(new QLabel(i18n("Birthday"), idPage), 6, 1);
  idLayout->addWidget(dateOfBirthE, 6, 3);

  QGroupBox* homeGroup = new QGroupBox(idPage, "homeGroup");
  homeGroup->setTitle(i18n("Homesite") + ":");

  homeNameE = new QLineEdit(idPage, "homeNameE");
  homeNameE->setMinimumWidth(150);
  homeLatE = new LatEdit(idPage, "homeLatE");
  homeLatE->setMinimumWidth(150);
  homeLonE = new LongEdit(idPage, "homeLonE");
  homeLonE->setMinimumWidth(150);

  idLayout->addMultiCellWidget(homeGroup, 9, 16, 0, 4);
  idLayout->addWidget(new QLabel(i18n("Homesite"), idPage), 11, 1);
  idLayout->addWidget(homeNameE, 11, 3);
  idLayout->addWidget(new QLabel(i18n("Latitude"), idPage), 13, 1);
  idLayout->addWidget(homeLatE, 13, 3);
  idLayout->addWidget(new QLabel(i18n("Longitude"), idPage), 15, 1);
  idLayout->addWidget(homeLonE, 15, 3);

  idLayout->addColSpacing(0, 10);
  idLayout->addColSpacing(2, 10);
  idLayout->addColSpacing(4, 10);

  idLayout->addRowSpacing(0, 25);
  idLayout->addRowSpacing(1, 5);
  idLayout->addRowSpacing(3, 5);
  idLayout->addRowSpacing(5, 5);
  idLayout->addRowSpacing(7, 20);

  idLayout->addRowSpacing(8, 25);

  idLayout->addRowSpacing(9, 25);
  idLayout->addRowSpacing(10, 5);
  idLayout->addRowSpacing(11, 10);
  idLayout->addRowSpacing(12, 10);
  idLayout->addRowSpacing(13, 10);
  idLayout->addRowSpacing(14, 10);
//  idLayout->addRowSpacing(15, 10);
  idLayout->addRowSpacing(16, 20);

  idLayout->setRowStretch(17, 1);

  config->setGroup("Map Data");

  homeLatE->setText(printPos(config->readNumEntry("Homesite Latitude",
      HOME_DEFAULT_LAT), true));
  homeLonE->setText(printPos(config->readNumEntry("Homesite Longitude",
      HOME_DEFAULT_LON), false));
  homeNameE->setText(config->readEntry("Homesite", ""));

  config->setGroup("Personal Data");
  preNameE->setText(config->readEntry("PreName", ""));
  surNameE->setText(config->readEntry("SurName", ""));
  dateOfBirthE->setText(config->readEntry("Birthday", ""));

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

/** Add a tab for waypoint catalog configuration at sartup
Setting will be overwritten by commandline switch */
void KFLogConfig::__addWaypointTab()
{
  config->setGroup("Waypoints");
  int catalogType = config->readNumEntry("DefaultWaypointCatalog", LastUsed);
  QString catalogName = config->readEntry("DefaultCatalogName", "");

  waypointPage = addPage(i18n("Waypoint"), i18n("Catalog Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("waypoint", KIcon::NoGroup,
          KIcon::SizeLarge));

  QVBoxLayout *top = new QVBoxLayout(waypointPage, 5);

  QGroupBox *group = new QGroupBox(i18n("Default Catalog:"), waypointPage, "catalogConfiguration");
  QGridLayout *grid = new QGridLayout(group, 4, 2, 25, 5);

  waypointButtonGroup = new QButtonGroup(group);
  waypointButtonGroup->hide();
  waypointButtonGroup->setExclusive(true);
  connect(waypointButtonGroup, SIGNAL(clicked(int)), SLOT(slotSelectDefaultCatalog(int)));

  QRadioButton *rb = new QRadioButton(i18n("Empty"), group);
  waypointButtonGroup->insert(rb, Empty);
  grid->addWidget(rb, 0, 0);
  rb = new QRadioButton(i18n("Last used"), group);
  waypointButtonGroup->insert(rb, LastUsed);
  grid->addWidget(rb, 1, 0);
  rb = new QRadioButton(i18n("Specific"), group);
  waypointButtonGroup->insert(rb, Specific);
  grid->addWidget(rb, 2, 0);
  catalogPathE = new QLineEdit(group);
  grid->addWidget(catalogPathE, 3, 0);
  catalogPathE->setText(catalogName);

  catalogPathSearch = new QPushButton(group);
  catalogPathSearch->setPixmap(BarIcon("fileopen"));
  catalogPathSearch->setMinimumWidth(catalogPathSearch->sizeHint().width() + 5);
  catalogPathSearch->setMinimumHeight(catalogPathSearch->sizeHint().height() + 5);
  grid->addWidget(catalogPathSearch, 3, 1);

  connect(catalogPathSearch, SIGNAL(clicked()), SLOT(slotSearchDefaultWaypoint()));

  QPushButton* defaultCatalog = new QPushButton(i18n("Default"), waypointPage,
      "defaultWaypointPath");
  defaultCatalog->setMaximumWidth(defaultCatalog->sizeHint().width() + 10);
  defaultCatalog->setMinimumHeight(defaultCatalog->sizeHint().height() + 2);

  connect(defaultCatalog, SIGNAL(clicked()), SLOT(slotDefaultWaypoint()));

  top->addWidget(group);
  top->addStretch();
  top->addWidget(defaultCatalog, AlignLeft);

  slotSelectDefaultCatalog(catalogType);
  
  config->setGroup(0);
}

void KFLogConfig::slotDefaultWaypoint()
{
  catalogPathE->setText(QString::null);
  slotSelectDefaultCatalog(LastUsed);
}

void KFLogConfig::slotSelectDefaultCatalog(int item)
{
  QRadioButton *b = (QRadioButton *)waypointButtonGroup->find(item);
  if (b != 0) {
    b->setChecked(true);
  }

  catalogPathE->setEnabled(item == Specific);
  catalogPathSearch->setEnabled(item == Specific);
}

void KFLogConfig::slotSearchDefaultWaypoint()
{
  QString temp = KFileDialog::getOpenFileName(catalogPathE->text(), "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)",
    this, i18n("Select default waypoint catalog"));

    if(temp != 0) {
      catalogPathE->setText(temp);
    }
}
