/***********************************************************************
**
**   kflogconfig.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   kflogconfig.cpp
**
**   This file is part of KFLog4.
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

#include <cstdlib>
#include <pwd.h>
#include <unistd.h>

#include <QtGui>
#include <Qt3Support>

#include "configdrawelement.h"
#include "configprintelement.h"
#include "kflogconfig.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapdefaults.h"
#include "wgspoint.h"
#include "mainwindow.h"

extern MapContents  *_globalMapContents;
extern MainWindow   *_mainWindow;
extern QSettings    _settings;

KFLogConfig::KFLogConfig(QWidget* parent)
  : QDialog(parent, "KFLog setup (KFLogConfig)"),
    currentProjType(ProjectionBase::Unknown)
{
  configLayout = new QGridLayout(this, 2, 3, 1, -1, "main layout of KFLogConfig");

  setupTree = new Q3ListView(this, "setupTree");
  setupTree->addColumn("Menu");
  setupTree->hideColumn(1);

  configLayout->addWidget(setupTree, 0, 0);
  connect(setupTree, SIGNAL(currentChanged(Q3ListViewItem *)), this, SLOT(slotPageChanged(Q3ListViewItem *)));

  QPushButton *saveButton = new QPushButton(tr("&Save"), this);
  configLayout->addWidget(saveButton, 1, 1);
  QPushButton *cancelButton = new QPushButton(tr("&Cancel"), this);
  configLayout->addWidget(cancelButton, 1, 2);

  __addIDTab();
  __addPathTab();
  __addScaleTab();
  __addMapTab();
  __addFlightTab();
  __addProjectionTab();
  __addAirfieldTab();
  __addWaypointTab();

  setupTree->hideColumn(1);
  this->setFixedWidth(637);
  setupTree->setFixedWidth(137);
  setupTree->setColumnWidth(137, 1);
  setupTree->setResizeMode(Q3ListView::NoColumn);
  activePage = idPage;
  idPage->show();
  activePage->setFixedWidth(500);

  connect(saveButton, SIGNAL(clicked()), SLOT(slotOk()));
  connect(cancelButton, SIGNAL(clicked()), SLOT(close()));
}

KFLogConfig::~KFLogConfig()
{

}

void KFLogConfig::slotPageChanged(Q3ListViewItem *currentItem)
{
  if(currentItem->text(1)=="Airfields")
  {
    activePage->hide();
    airfieldPage->show();
    activePage = airfieldPage;
  }
  else if(currentItem->text(1)=="Flight Display")
  {
    activePage->hide();
    flightPage->show();
    activePage = flightPage;
  }
  else if(currentItem->text(1)=="Identity")
  {
    activePage->hide();
    idPage->show();
    activePage = idPage;
  }
  else if(currentItem->text(1)=="Map-Elements")
  {
    activePage->hide();
    mapPage->show();
    activePage = mapPage;
  }
  else if(currentItem->text(1)=="Paths")
  {
    activePage->hide();
    pathPage->show();
    activePage = pathPage;
  }
  else if(currentItem->text(1)=="Map-Projection")
  {
    activePage->hide();
    projPage->show();
    activePage = projPage;
  }
  else if(currentItem->text(1)=="Map-Scales")
  {
    activePage->hide();
    waypointPage->show();
    activePage = waypointPage;
  }
  else if(currentItem->text(1)=="Waypoints")
  {
    activePage->hide();
    airfieldPage->show();
    activePage = airfieldPage;
  }
  activePage->setFixedWidth(500);
  setupTree->hideColumn(1);
}

void KFLogConfig::slotOk()
{
  slotSelectProjection(ProjectionBase::Unknown);

  _settings.setValue("/GeneralOptions/Version", "3.0");

  _settings.setValue("/Path/DefaultFlightDirectory", igcPathE->text());
  _settings.setValue("/Path/DefaultTaskDirectory", taskPathE->text());
  _settings.setValue("/Path/DefaultWaypointDirectory", waypointPathE->text());
  _settings.setValue("/Path/DefaultMapDirectory", mapPathE->text());

  _settings.setValue("/Scale/LowerLimit", lLimitN->value());
  _settings.setValue("/Scale/UpperLimit", uLimitN->value());
  _settings.setValue("/Scale/SwitchScale", switchScaleN->value());
  _settings.setValue("/Scale/WaypointLabel", wpLabelN->value());
  _settings.setValue("/Scale/Border1", reduce1N->value());
  _settings.setValue("/Scale/Border2", reduce2N->value());
  _settings.setValue("/Scale/Border3", reduce3N->value());

  _settings.setValue("/MapData/Homesite", homeNameE->text());
  _settings.setValue("/MapData/HomesiteLatitude", WGSPoint::degreeToNum(homeLatE-> text()));
  _settings.setValue("/MapData/HomesiteLongitude", WGSPoint::degreeToNum(homeLonE-> text()));
  _settings.setValue("/MapData/ProjectionType", projectionSelect->currentItem());
  _settings.setValue("/Welt2000/CountryFilter", filterE->text());
  _settings.setValue("/Welt2000/HomeRadius", homeRadiusE->text());

  if(needUpdateDrawType)
  {
    _settings.setValue("/Flight/DrawType", drawTypeSelect->currentItem());
    //update menu Flight=>Show Flightdata
    emit newDrawType(drawTypeSelect->currentItem());
  }
  _settings.setValue("/Flight/ColorLeftTurn", __color2String(flightTypeLeftTurnColor));
  _settings.setValue("/Flight/ColorRightTurn", __color2String(flightTypeRightTurnColor));
  _settings.setValue("/Flight/ColorMixedTurn", __color2String(flightTypeMixedTurnColor));
  _settings.setValue("/Flight/ColorStraight", __color2String(flightTypeStraightColor));
  _settings.setValue("/Flight/ColorSolid", __color2String(flightTypeSolidColor));
  _settings.setValue("/Flight/ColorEngineNoise", __color2String(flightTypeEngineNoiseColor));
  _settings.setValue("/Flight/flightPathWidth", flightPathWidthE->text());

  _settings.setValue("/LambertProjection/Parallel1", lambertV1);
  _settings.setValue("/LambertProjection/Parallel2", lambertV2);
  _settings.setValue("/LambertProjection/Origin", lambertOrigin);

  _settings.setValue("/CylindricalProjection/Parallel", cylinPar);

  _settings.setValue("/PersonalData/PreName", preNameE->text());
  _settings.setValue("/PersonalData/SurName", surNameE->text());
  _settings.setValue("/PersonalData/Birthday", dateOfBirthE->text());

  _settings.setValue("/Waypoints/DefaultWaypointCatalog", waypointButtonGroup->id(waypointButtonGroup->selected()));
  _settings.setValue("/Waypoints/DefaultCatalogName", catalogPathE->text());

  emit scaleChanged((int)lLimitN->value(), (int)uLimitN->value());

  if (needUpdateWelt2000) {
    extern MapContents  *_globalMapContents;
    _globalMapContents->slotReloadMapData();
  }

  emit configOk();
  accept();
}

void KFLogConfig::slotSearchFlightPath()
{
  QString temp = Q3FileDialog::getExistingDirectory(igcPathE-> text(), this, 0,
      tr("Select Default-Directory for the IGC-Files"));

  if(temp != 0)  igcPathE-> setText(temp);
}

void KFLogConfig::slotSearchMapPath()
{
  QString temp = Q3FileDialog::getExistingDirectory(mapPathE-> text(), this, 0,
      tr("Select Directory for the Map-Files"));

  if(temp != 0)  mapPathE-> setText(temp);
}

void KFLogConfig::slotSearchTaskPath()
{
  QString temp = Q3FileDialog::getExistingDirectory(taskPathE-> text(), this, 0,
      tr("Select Default-Directory for the Task-Files"));

  if(temp != 0)  taskPathE-> setText(temp);
}

void KFLogConfig::slotSearchWaypointPath()
{
  QString temp = Q3FileDialog::getExistingDirectory(waypointPathE-> text(), this, 0,
      tr("Select Default-Directory for the Waypoint-Files"));

  if(temp != 0)  waypointPathE-> setText(temp);
}

void KFLogConfig::slotSelectProjection(int index)
{
  switch(currentProjType)
    {
      case ProjectionBase::Lambert:
        lambertV1 = WGSPoint::degreeToNum(firstParallel-> text());
        lambertV2 = WGSPoint::degreeToNum(secondParallel-> text());
        lambertOrigin = WGSPoint::degreeToNum(originLongitude-> text());
        break;
      case ProjectionBase::Cylindric:
        cylinPar = WGSPoint::degreeToNum(firstParallel-> text());
        break;
    }

  switch(index)
    {
      case ProjectionBase::Lambert:
        secondParallel-> setEnabled(true);
        originLongitude-> setEnabled(true);
        firstParallel-> setText(WGSPoint::printPos(lambertV1, true));
        secondParallel-> setText(WGSPoint::printPos(lambertV2, true));
        originLongitude-> setText(WGSPoint::printPos(lambertOrigin, false));
        break;
      case ProjectionBase::Cylindric:
        secondParallel-> setEnabled(false);
        originLongitude-> setEnabled(false);
        firstParallel-> setText(WGSPoint::printPos(cylinPar, true));
        break;
    }
  currentProjType = index;
}

void KFLogConfig::slotShowLowerLimit(int value)
{
  lLimitN-> display(__setScaleValue(value));
}

void KFLogConfig::slotShowUpperLimit(int value)
{
  uLimitN-> display(__setScaleValue(value));
}

void KFLogConfig::slotShowSwitchScale(int value)
{
  switchScaleN-> display(__setScaleValue(value));
}

void KFLogConfig::slotShowWpLabel(int value)
{
  wpLabelN-> display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleA(int value)
{
  reduce1N-> display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleB(int value)
{
  reduce2N-> display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleC(int value)
{
  reduce3N-> display(__setScaleValue(value));
}

void KFLogConfig::slotDefaultProjection()
{
  lambertV1 = 32400000;
  lambertV2 = 30000000;
  lambertOrigin = 0;

  cylinPar = 27000000;

  currentProjType = ProjectionBase::Unknown;

  projectionSelect-> setCurrentItem(ProjectionBase::Lambert);
  slotSelectProjection(ProjectionBase::Lambert);
}

void KFLogConfig::slotDefaultScale()
{
  lLimit-> setValue(__getScaleValue(L_LIMIT));
  lLimitN-> display(L_LIMIT);
  uLimit-> setValue(__getScaleValue(U_LIMIT));
  uLimitN-> display(U_LIMIT);
  switchScale-> setValue(__getScaleValue(SWITCH_S));
  switchScaleN-> display(SWITCH_S);
  wpLabel-> setValue(__getScaleValue(WPLABEL));
  wpLabelN-> display(WPLABEL);
  reduce1-> setValue(__getScaleValue(BORDER_1));
  reduce1N-> display(BORDER_1);
  reduce2-> setValue(__getScaleValue(BORDER_2));
  reduce2N-> display(BORDER_2);
  reduce3-> setValue(__getScaleValue(BORDER_3));
  reduce3N-> display(BORDER_3);
}

void KFLogConfig::slotDefaultPath()
{
  igcPathE-> setText( _mainWindow->getApplicationDataDirectory() );
  taskPathE-> setText( _mainWindow->getApplicationDataDirectory() );
  waypointPathE-> setText( _mainWindow->getApplicationDataDirectory() );
  mapPathE-> setText( _globalMapContents->getMapRootDirectory() );
}

void KFLogConfig::__addMapTab()
{
  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Map-Elements"), "Map-Elements");
  item->setPixmap(0, _mainWindow->getPixmap("kflog_32.png"));

  mapPage = new Q3Frame(this, "Map Configuration");
  mapPage->hide();
  configLayout->addMultiCellWidget(mapPage, 0, 0, 1, 2);

  Q3GroupBox* elementBox = new Q3GroupBox(mapPage, "elementBox");
  elementBox-> setTitle(tr("visible Map-Elements"));

  elementSelect = new QComboBox(mapPage, "elementSelect");
//  elementSelect-> setMaximumWidth(300);
  elementSelect-> insertItem(tr("Road"), KFLogConfig::Road);
  elementSelect-> insertItem(tr("Highway"), KFLogConfig::Highway);
  elementSelect-> insertItem(tr("Railway"), KFLogConfig::Railway);
  elementSelect-> insertItem(tr("River / Lake"), KFLogConfig::River);
  elementSelect-> insertItem(tr("Canal"), KFLogConfig::Canal);
  elementSelect-> insertItem(tr("City"), KFLogConfig::City);
  elementSelect-> insertItem(tr("Airspace A"), KFLogConfig::AirA);
  elementSelect-> insertItem(tr("Airspace B"), KFLogConfig::AirB);
  elementSelect-> insertItem(tr("Airspace C"), KFLogConfig::AirC);
  elementSelect-> insertItem(tr("Airspace D"), KFLogConfig::AirD);
  elementSelect-> insertItem(tr("Airspace E (low)"), KFLogConfig::AirElow);
  elementSelect-> insertItem(tr("Airspace E (high)"), KFLogConfig::AirEhigh);
  elementSelect-> insertItem(tr("Airspace F"), KFLogConfig::AirF);
  elementSelect-> insertItem(tr("Control C"), KFLogConfig::ControlC);
  elementSelect-> insertItem(tr("Control D"), KFLogConfig::ControlD);
  elementSelect-> insertItem(tr("Danger"), KFLogConfig::Danger);
  elementSelect-> insertItem(tr("Low flight area"), KFLogConfig::LowFlight);
  elementSelect-> insertItem(tr("Restricted"), KFLogConfig::Restricted);
  elementSelect-> insertItem(tr("TMZ"), KFLogConfig::Tmz);
  // Reihenfolge ???
  elementSelect-> insertItem(tr("Forest"), KFLogConfig::Forest);
  elementSelect-> insertItem(tr("Trail"), KFLogConfig::Trail);
  elementSelect-> insertItem(tr("double Railway"), KFLogConfig::Railway_D);
  elementSelect-> insertItem(tr("Aerial Cable"), KFLogConfig::Aerial_Cable);
  elementSelect-> insertItem(tr("temporarily River / Lake"), KFLogConfig::River_T);
  elementSelect-> insertItem(tr("Glacier"), KFLogConfig::Glacier);
  elementSelect-> insertItem(tr("Pack Ice"), KFLogConfig::PackIce);
  elementSelect-> insertItem(tr("FAI Area <500 km"), KFLogConfig::FAIAreaLow500);
  elementSelect-> insertItem(tr("FAI Area >500 km"), KFLogConfig::FAIAreaHigh500);

  QPushButton* defaultElements = new QPushButton(tr("Default"), mapPage,
      "defaultElements");
  defaultElements-> setMaximumWidth(defaultElements->sizeHint().width() + 10);
  defaultElements-> setMinimumHeight(defaultElements->sizeHint().height() + 2);

  QTabWidget* tabView = new QTabWidget(mapPage);
  Q3Frame* screenFrame = new Q3Frame(tabView, "ConfigDrawFrame");
  ConfigDrawElement* drawConfig = new ConfigDrawElement(screenFrame);

  Q3Frame* printFrame = new Q3Frame(tabView, "ConfigDrawFrame");
  ConfigPrintElement* printConfig = new ConfigPrintElement(printFrame);

  tabView-> addTab(screenFrame, tr("Display"));
  tabView-> addTab(printFrame, tr("Print"));

  QGridLayout* elLayout = new QGridLayout(mapPage, 7, 5, 8, 1);
  elLayout-> addMultiCellWidget(elementBox, 0, 4, 0, 4);
  elLayout-> addMultiCellWidget(elementSelect, 1, 1, 1, 2, Qt::AlignLeft);
  elLayout-> addMultiCellWidget(tabView, 3, 3, 1, 3);
  elLayout-> addMultiCellWidget(defaultElements, 6, 6, 0, 1, Qt::AlignLeft);

  elLayout-> addRowSpacing(0, 20);
  elLayout-> addRowSpacing(2, 5);
  elLayout-> setRowStretch(3, 1);
  elLayout-> addRowSpacing(4, 5);
  elLayout-> addRowSpacing(5, 15);

  elLayout-> addColSpacing(0, 10);
  elLayout-> setColStretch(0, 0);
  elLayout-> setColStretch(3, 1);
  elLayout-> addColSpacing(4, 10);
  elLayout-> setColStretch(4, 0);

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

  drawConfig-> slotSelectElement(0);
  printConfig-> slotSelectElement(0);
}

void KFLogConfig::__addFlightTab()
{
  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Flight Display"), "Flight Display");
  item->setPixmap(0, _mainWindow->getPixmap("flightpath_32.png"));

  flightPage = new Q3Frame(this, "Flight Display Configuration");
  flightPage->hide();
  configLayout->addMultiCellWidget(flightPage, 0, 0, 1, 2);

  QGridLayout* flightLayout = new QGridLayout(flightPage, 17, 40, 8, 1);

  Q3GroupBox* flightPathLineGroup = new Q3GroupBox(flightPage, "flightDisplayGroup");
  flightPathLineGroup-> setTitle(tr("Flight Path Line") + ":");
  flightLayout->addMultiCellWidget(flightPathLineGroup, 0, 4, 0, 39);


  drawTypeSelect = new QComboBox(flightPage, "drawTypeSelect");
  drawTypeSelect-> insertItem(tr("Altitude"), MapConfig::Altitude);
  drawTypeSelect-> insertItem(tr("Cycling"),  MapConfig::Cycling);
  drawTypeSelect-> insertItem(tr("Speed"),    MapConfig::Speed);
  drawTypeSelect-> insertItem(tr("Vario"),    MapConfig::Vario);
  drawTypeSelect-> insertItem(tr("Solid"),    MapConfig::Solid);
  drawTypeSelect-> setCurrentItem(_settings.readNumEntry("/Flight/DrawType"));

  flightLayout->addWidget(new QLabel(tr("type") + ":", flightPage), 1, 1);
  flightLayout->addWidget(drawTypeSelect, 1, 10);


  flightPathWidthE = new QSpinBox(flightPage, "flightPathWidthE");
  flightPathWidthE->setRange( 0, 9 );
  flightPathWidthE->setLineStep( 1 );
  flightPathWidthE->setValue(_settings.readNumEntry("/Flight/flightPathWidth", 4));

  flightLayout->addWidget(new QLabel(tr("width") + ":", flightPage), 3, 1);
  flightLayout->addWidget( flightPathWidthE, 3, 10);


  Q3GroupBox* flightPathColorGroup = new Q3GroupBox(flightPage, "flightDisplayGroup");
  flightPathColorGroup-> setTitle(tr("Flight Path Colors") + ":");
  flightLayout->addMultiCellWidget(flightPathColorGroup, 5, 14, 0, 39);

  flightLayout->addWidget(new QLabel(tr("left turn") + ":", flightPage), 6, 1);
  flightTypeLeftTurnColor = __string2Color(_settings.readEntry("/Flight/ColorLeftTurn", "255;50;0"));
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeLeftTurnColor);
  flightTypeLeftTurnColorButton = new QPushButton(flightPage);
  flightTypeLeftTurnColorButton->setPixmap(*buttonPixmap);
  flightTypeLeftTurnColorButton->setFixedHeight(24);
  flightTypeLeftTurnColorButton->setFixedWidth(92);
  flightLayout->addWidget( flightTypeLeftTurnColorButton, 6, 10);
  connect(flightTypeLeftTurnColorButton, SIGNAL(clicked()), this, SLOT(slotSelectFlightTypeLeftTurnColor()));

  flightLayout->addWidget(new QLabel(tr("right turn") + ":", flightPage), 7, 1);
  flightTypeRightTurnColor = __string2Color(_settings.readEntry("/Flight/ColorRightTurn", "50;255;0"));
  buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeRightTurnColor);
  flightTypeRightTurnColorButton = new QPushButton(flightPage);
  flightTypeRightTurnColorButton->setPixmap(*buttonPixmap);
  flightTypeRightTurnColorButton->setFixedHeight(24);
  flightTypeRightTurnColorButton->setFixedWidth(92);
  flightLayout->addWidget( flightTypeRightTurnColorButton, 7, 10);
  connect(flightTypeRightTurnColorButton, SIGNAL(clicked()), this, SLOT(slotSelectFlightTypeRightTurnColor()));

  flightLayout->addWidget(new QLabel(tr("mixed turn") + ":", flightPage), 8, 1);
  flightTypeMixedTurnColor = __string2Color(_settings.readEntry("/Flight/ColorMixedTurn", "200;0;200"));
  buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeMixedTurnColor);
  flightTypeMixedTurnColorButton = new QPushButton(flightPage);
  flightTypeMixedTurnColorButton->setPixmap(*buttonPixmap);
  flightTypeMixedTurnColorButton->setFixedHeight(24);
  flightTypeMixedTurnColorButton->setFixedWidth(92);
  flightLayout->addWidget( flightTypeMixedTurnColorButton, 8, 10);
  connect(flightTypeMixedTurnColorButton, SIGNAL(clicked()), this, SLOT(slotSelectFlightTypeMixedTurnColor()));

  flightLayout->addWidget(new QLabel(tr("straight") + ":", flightPage), 9, 1);
  flightTypeStraightColor = __string2Color(_settings.readEntry("/Flight/ColorStraight", "0;50;255"));
  buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeStraightColor);
  flightTypeStraightColorButton = new QPushButton(flightPage);
  flightTypeStraightColorButton->setPixmap(*buttonPixmap);
  flightTypeStraightColorButton->setFixedHeight(24);
  flightTypeStraightColorButton->setFixedWidth(92);
  flightLayout->addWidget( flightTypeStraightColorButton, 9, 10);
  connect(flightTypeStraightColorButton, SIGNAL(clicked()), this, SLOT(slotSelectFlightTypeStraightColor()));

  flightLayout->addWidget(new QLabel(tr("solid") + ":", flightPage), 11, 1);
  flightTypeSolidColor = __string2Color(_settings.readEntry("/Flight/ColorSolid", "0;100;200"));
  buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeSolidColor);
  flightTypeSolidColorButton = new QPushButton(flightPage);
  flightTypeSolidColorButton->setPixmap(*buttonPixmap);
  flightTypeSolidColorButton->setFixedHeight(24);
  flightTypeSolidColorButton->setFixedWidth(92);
  flightLayout->addWidget( flightTypeSolidColorButton, 11, 10);
  connect(flightTypeSolidColorButton, SIGNAL(clicked()), this, SLOT(slotSelectFlightTypeSolidColor()));

  flightLayout->addWidget(new QLabel(tr("engine noise") + ":", flightPage), 13, 1);
  flightTypeEngineNoiseColor = __string2Color(_settings.readEntry("/Flight/ColorEngineNoise", "255;255;255"));
  buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeEngineNoiseColor);
  flightTypeEngineNoiseColorButton = new QPushButton(flightPage);
  flightTypeEngineNoiseColorButton->setPixmap(*buttonPixmap);
  flightTypeEngineNoiseColorButton->setFixedHeight(24);
  flightTypeEngineNoiseColorButton->setFixedWidth(92);
  flightLayout->addWidget( flightTypeEngineNoiseColorButton, 13, 10);
  connect(flightTypeEngineNoiseColorButton, SIGNAL(clicked()), this, SLOT(slotSelectFlightTypeEngineNoiseColor()));


  needUpdateDrawType = false;
  connect(drawTypeSelect, SIGNAL(activated(int)), SLOT(slotDrawTypeSelect()));

}

void KFLogConfig::slotDrawTypeSelect()
{
  needUpdateDrawType = true;
}

void KFLogConfig::__addProjectionTab()
{
  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Map-Projection"), "Map-Projection");
  item->setPixmap(0, _mainWindow->getPixmap("projection_32.png"));

  projPage = new Q3Frame(this, "Configuration of Map-Projection");
  projPage->hide();
  configLayout->addMultiCellWidget(projPage, 0, 0, 1, 2);

  Q3GroupBox* projType = new Q3GroupBox(projPage, "projectionSelectBox");
  projType-> setTitle(tr("Type of Projection") + ":");

  projectionSelect = new QComboBox(projPage, "projectionSelect");
  projectionSelect-> insertItem(tr("Conical orthomorphic (Lambert)"));
  projectionSelect-> insertItem(tr("Cylindrical Equidistant (Plate Carre)"));

  Q3GroupBox* projConf = new Q3GroupBox(projPage, "projectionSelectBox");
  projConf-> setTitle(tr("Setup Projection") + ":");

  firstParallel = new LatEdit(projPage, "firstParallel");
  secondParallel = new LatEdit(projPage, "secondParallel");
  originLongitude = new LongEdit(projPage, "originLongitude");

  QPushButton* defaultProj = new QPushButton(tr("Default"), projPage,
      "defaultScale");
  defaultProj-> setMaximumWidth(defaultProj->sizeHint().width() + 10);
  defaultProj-> setMinimumHeight(defaultProj->sizeHint().height() + 2);

  QGridLayout* projLayout = new QGridLayout(projPage, 17, 7, 8, 1);
  projLayout-> addMultiCellWidget(projType, 0, 2, 0, 6);
  projLayout-> addMultiCellWidget(projectionSelect, 1, 1, 1, 5);
  projLayout-> addMultiCellWidget(projConf, 4, 10, 0, 6);
  projLayout-> addWidget(new QLabel(tr("1. Standard Parallel") + ":", projPage),
      5, 1);
  projLayout-> addWidget(firstParallel, 5, 3);
  projLayout-> addWidget(new QLabel(tr("2. Standard Parallel") + ":", projPage),
      7, 1);
  projLayout-> addWidget(secondParallel, 7, 3);
  projLayout-> addWidget(new QLabel(tr("Origin Longitude") + ":", projPage),
      9, 1);
  projLayout-> addWidget(originLongitude, 9, 3);

  projLayout-> addMultiCellWidget(defaultProj, 16, 16, 0, 1, Qt::AlignLeft);

  projLayout-> addColSpacing(0, 10);
  projLayout-> addColSpacing(2, 5);
  projLayout-> addColSpacing(4, 10);
  projLayout-> addColSpacing(6, 10);

  projLayout-> setColStretch(3, 1);

  projLayout-> addRowSpacing(0, 25);
  projLayout-> addRowSpacing(2, 5);
  projLayout-> addRowSpacing(4, 25);
  projLayout-> addRowSpacing(6, 5);
  projLayout-> addRowSpacing(8, 5);
  projLayout-> addRowSpacing(15, 10);

  connect(defaultProj, SIGNAL(clicked()), SLOT(slotDefaultProjection()));
  connect(projectionSelect, SIGNAL(activated(int)), SLOT(slotSelectProjection(int)));

  lambertV1 = _settings.readNumEntry("/LambertProjection/Parallel1", 32400000);
  lambertV2 = _settings.readNumEntry("/LambertProjection/Parallel2", 30000000);
  lambertOrigin = _settings.readNumEntry("/LambertProjection/Origin", 0);

  cylinPar = _settings.readNumEntry("/CylindricalProjection/Parallel", 27000000);

  int projIndex = _settings.readNumEntry("/MapData/Projection Type", ProjectionBase::Lambert);

  projectionSelect-> setCurrentItem(projIndex);
  slotSelectProjection(projIndex);
}

void KFLogConfig::__addScaleTab()
{
  int ll = _settings.readNumEntry("/Scale/LowerLimit", L_LIMIT);
  int ul = _settings.readNumEntry("/Scale/UpperLimit", U_LIMIT);
  int sw = _settings.readNumEntry("/Scale/SwitchScale", SWITCH_S);
  int wl = _settings.readNumEntry("/Scale/WaypointLabel", WPLABEL);
  int b1 = _settings.readNumEntry("/Scale/Border1", BORDER_1);
  int b2 = _settings.readNumEntry("/Scale/Border2", BORDER_2);
  int b3 = _settings.readNumEntry("/Scale/Border3", BORDER_3);

  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Map-Scales"), "Map-Scales");
  item->setPixmap(0, _mainWindow->getPixmap("kde_viewmag_32.png"));

  scalePage = new Q3Frame(this, "Map-Scale Configuration");
  scalePage->hide();
  configLayout->addMultiCellWidget(scalePage, 0, 0, 1, 2);

  Q3GroupBox* scaleLimits = new Q3GroupBox(scalePage, "scaleLimitBox");
  scaleLimits-> setTitle(tr("Scale-Range:"));

  QLabel* lLimitText = new QLabel(tr("lower limit"), scalePage);
  lLimit = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  lLimit-> setMinimumHeight(lLimit->sizeHint().height() + 5);
  lLimit-> setMaximumHeight(lLimit->sizeHint().height() + 20);
  lLimit-> setMinimumWidth(200);
  lLimitN = new QLCDNumber(5, scalePage);
  lLimitN-> setMinimumWidth(lLimitN->sizeHint().width() + 10);
  lLimit-> setValue(__getScaleValue(ll));
  lLimitN-> display(ll);

  QLabel* uLimitText = new QLabel(tr("upper limit"), scalePage);
  uLimit = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  uLimit-> setMinimumHeight(uLimit->sizeHint().height() + 5);
  uLimit-> setMaximumHeight(uLimit->sizeHint().height() + 20);
  uLimitN = new QLCDNumber(5, scalePage);
  uLimit-> setValue(__getScaleValue(ul));
  uLimitN-> display(ul);

  Q3GroupBox* borderBox = new Q3GroupBox(scalePage, "borderBox");
  borderBox-> setTitle("Scale-Thresholds:");

  QLabel* switchText = new QLabel(tr("use small icons"), scalePage);
  switchScale = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  switchScale-> setMinimumHeight(switchScale->sizeHint().height() + 5);
  switchScale-> setMaximumHeight(switchScale->sizeHint().height() + 20);
  switchScaleN = new QLCDNumber(5, scalePage);
  switchScale-> setValue(__getScaleValue(sw));
  switchScaleN-> display(sw);

  QLabel* wpLabelText = new QLabel(tr("draw waypoint labels"), scalePage);
  wpLabel = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  wpLabel-> setMinimumHeight(switchScale->sizeHint().height() + 5);
  wpLabel-> setMaximumHeight(switchScale->sizeHint().height() + 20);
  wpLabelN = new QLCDNumber(5, scalePage);
  wpLabel-> setValue(__getScaleValue(wl));
  wpLabelN-> display(wl);

  QLabel* reduce1Text = new QLabel(tr("threshold #1"), scalePage);
  reduce1 = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  reduce1-> setMinimumHeight(reduce1->sizeHint().height() + 5);
  reduce1-> setMaximumHeight(reduce1->sizeHint().height() + 20);
  reduce1N = new QLCDNumber(5, scalePage);
  reduce1-> setValue(__getScaleValue(b1));
  reduce1N-> display(b1);

  QLabel* reduce2Text = new QLabel(tr("threshold #2"), scalePage);
  reduce2 = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  reduce2-> setMinimumHeight(reduce2->sizeHint().height() + 5);
  reduce2-> setMaximumHeight(reduce2->sizeHint().height() + 20);
  reduce2N = new QLCDNumber(5, scalePage);
  reduce2-> setValue(__getScaleValue(b2));
  reduce2N-> display(b2);

  QLabel* reduce3Text = new QLabel(tr("threshold #3"), scalePage);
  reduce3 = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  reduce3-> setMinimumHeight(reduce3->sizeHint().height() + 5);
  reduce3-> setMaximumHeight(reduce3->sizeHint().height() + 20);
  reduce3N = new QLCDNumber(5, scalePage);
  reduce3-> setValue(__getScaleValue(b3));
  reduce3N-> display(b3);

  QPushButton* defaultScale = new QPushButton(tr("Default"), scalePage,
      "defaultScale");
  defaultScale-> setMaximumWidth(defaultScale->sizeHint().width() + 10);
  defaultScale-> setMinimumHeight(defaultScale->sizeHint().height() + 2);

  QGridLayout* scaleLayout = new QGridLayout(scalePage, 19, 7, 8, 1);
  scaleLayout-> addMultiCellWidget(scaleLimits, 0, 4, 0, 6);
  scaleLayout-> addWidget(lLimitText, 1, 1);
  scaleLayout-> addWidget(lLimit, 1, 3);
  scaleLayout-> addWidget(lLimitN, 1, 5);
  scaleLayout-> addWidget(uLimitText, 3, 1);
  scaleLayout-> addWidget(uLimit, 3, 3);
  scaleLayout-> addWidget(uLimitN, 3, 5);

  scaleLayout-> addMultiCellWidget(borderBox, 6, 16, 0, 6);
  scaleLayout-> addWidget(switchText, 7, 1);
  scaleLayout-> addWidget(switchScale, 7, 3);
  scaleLayout-> addWidget(switchScaleN, 7, 5);
  scaleLayout-> addWidget(wpLabelText, 9, 1);
  scaleLayout-> addWidget(wpLabel, 9, 3);
  scaleLayout-> addWidget(wpLabelN, 9, 5);
  scaleLayout-> addWidget(reduce1Text, 11, 1);
  scaleLayout-> addWidget(reduce1, 11, 3);
  scaleLayout-> addWidget(reduce1N, 11, 5);
  scaleLayout-> addWidget(reduce2Text, 13, 1);
  scaleLayout-> addWidget(reduce2, 13, 3);
  scaleLayout-> addWidget(reduce2N, 13, 5);
  scaleLayout-> addWidget(reduce3Text, 15, 1);
  scaleLayout-> addWidget(reduce3, 15, 3);
  scaleLayout-> addWidget(reduce3N, 15, 5);

  scaleLayout-> addMultiCellWidget(defaultScale, 18, 18, 0, 1, Qt::AlignLeft);

  scaleLayout-> addColSpacing(0, 10);
  scaleLayout-> addColSpacing(2, 5);
  scaleLayout-> addColSpacing(4, 10);
  scaleLayout-> addColSpacing(6, 10);

  scaleLayout-> setColStretch(3, 1);

  scaleLayout-> addRowSpacing(0, 25);
  scaleLayout-> addRowSpacing(4, 5);
  scaleLayout-> addRowSpacing(5, 10);
  scaleLayout-> addRowSpacing(6, 25);
  scaleLayout-> addRowSpacing(16, 5);
  scaleLayout-> addRowSpacing(17, 10);

  connect(defaultScale, SIGNAL(clicked()), SLOT(slotDefaultScale()));
  connect(lLimit, SIGNAL(valueChanged(int)), SLOT(slotShowLowerLimit(int)));
  connect(uLimit, SIGNAL(valueChanged(int)), SLOT(slotShowUpperLimit(int)));
  connect(switchScale, SIGNAL(valueChanged(int)), SLOT(slotShowSwitchScale(int)));
  connect(wpLabel, SIGNAL(valueChanged(int)), SLOT(slotShowWpLabel(int)));
  connect(reduce1, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleA(int)));
  connect(reduce2, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleB(int)));
  connect(reduce3, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleC(int)));
}

void KFLogConfig::__addPathTab()
{
  QString flightDir = _settings.value( "/Path/DefaultFlightDirectory",
                                       _mainWindow->getApplicationDataDirectory() ).toString();
  QString taskDir = _settings.value( "/Path/DefaultTaskDirectory",
                                     _mainWindow->getApplicationDataDirectory() ).toString();
  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

  QString mapDir = _globalMapContents->getMapRootDirectory();

  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Paths"), "Paths");
  item->setPixmap(0, _mainWindow->getPixmap("kde_fileopen_32.png"));

  pathPage = new Q3Frame(this, "Path Configuration");
  pathPage->hide();
  configLayout->addMultiCellWidget(pathPage, 0, 0, 1, 2);

  QGridLayout* pathLayout = new QGridLayout(pathPage, 17, 5, 8, 1);

  Q3GroupBox* igcGroup = new Q3GroupBox(pathPage, "igcGroup");
  igcGroup-> setTitle(tr("Flight-directory:"));

  igcPathE  = new QLineEdit(pathPage, "igcPathE");
  igcPathE-> setMinimumWidth(150);
  igcPathE-> setText(flightDir);

  QPushButton* igcPathSearch = new QPushButton(pathPage);
  igcPathSearch-> setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  igcPathSearch-> setMinimumWidth(igcPathSearch->sizeHint().width() + 5);
  igcPathSearch-> setMinimumHeight(igcPathSearch->sizeHint().height() + 5);

  pathLayout-> addMultiCellWidget(igcGroup, 0, 2, 0, 4);
  pathLayout-> addWidget(igcPathE, 1, 1);
  pathLayout-> addWidget(igcPathSearch, 1, 3);

  pathLayout-> addColSpacing(0, 10);
  pathLayout-> addColSpacing(2, 10);
  pathLayout-> addColSpacing(4, 10);

  pathLayout-> addRowSpacing(0, 25);
  pathLayout-> addRowSpacing(1, 10);
  pathLayout-> addRowSpacing(2, 10);

  pathLayout-> addRowSpacing(3, 25);
  pathLayout-> setRowStretch(3, 1);

  Q3GroupBox* taskGroup = new Q3GroupBox(pathPage, "taskGroup");
  taskGroup-> setTitle(tr("Task-directory:"));

  taskPathE = new QLineEdit(pathPage, "taskPathE");
  taskPathE-> setMinimumWidth(150);
  taskPathE-> setText(taskDir);

  QPushButton* taskPathSearch = new QPushButton(pathPage);
  taskPathSearch-> setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  taskPathSearch-> setMinimumWidth(taskPathSearch->sizeHint().width() + 5);
  taskPathSearch-> setMinimumHeight(taskPathSearch->sizeHint().height() + 5);

  pathLayout-> addMultiCellWidget(taskGroup, 4, 6, 0, 4);
  pathLayout-> addWidget(taskPathE, 5, 1);
  pathLayout-> addWidget(taskPathSearch, 5, 3);

  pathLayout-> addRowSpacing(4, 25);
  pathLayout-> addRowSpacing(5, 10);
  pathLayout-> addRowSpacing(6, 10);

  pathLayout-> addRowSpacing(7, 25);
  pathLayout-> setRowStretch(7, 1);

  Q3GroupBox* waypointGroup = new Q3GroupBox(pathPage, "waypointGroup");
  waypointGroup-> setTitle(tr("Waypoint-directory:"));

  waypointPathE = new QLineEdit(pathPage, "waypointPathE");
  waypointPathE-> setMinimumWidth(150);
  waypointPathE-> setText(wayPointDir);

  QPushButton* waypointPathSearch = new QPushButton(pathPage);
  waypointPathSearch-> setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  waypointPathSearch-> setMinimumWidth(waypointPathSearch->sizeHint().width() + 5);
  waypointPathSearch-> setMinimumHeight(waypointPathSearch->sizeHint().height() + 5);

  pathLayout-> addMultiCellWidget(waypointGroup, 8, 10, 0, 4);
  pathLayout-> addWidget(waypointPathE, 9, 1);
  pathLayout-> addWidget(waypointPathSearch, 9, 3);

  pathLayout-> addRowSpacing(8, 20);
  pathLayout-> addRowSpacing(9, 10);
  pathLayout-> addRowSpacing(10, 10);

  pathLayout-> addRowSpacing(11, 20);
  pathLayout-> setRowStretch(11, 1);

  Q3GroupBox* mapGroup = new Q3GroupBox(pathPage, "mapGroup");
  mapGroup-> setTitle(tr("Map-directory:"));

  mapPathE = new QLineEdit(pathPage, "mapPathE");
  mapPathE-> setMinimumWidth(150);
  mapPathE-> setText(mapDir);

  QPushButton* mapPathSearch = new QPushButton(pathPage);
  mapPathSearch-> setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  mapPathSearch-> setMinimumWidth(mapPathSearch->sizeHint().width() + 5);
  mapPathSearch-> setMinimumHeight(mapPathSearch->sizeHint().height() + 5);

  pathLayout-> addMultiCellWidget(mapGroup, 12, 14, 0, 4);
  pathLayout-> addWidget(mapPathE, 13, 1);
  pathLayout-> addWidget(mapPathSearch, 13, 3);

  pathLayout-> addRowSpacing(12, 20);
  pathLayout-> addRowSpacing(13, 10);
  pathLayout-> addRowSpacing(14, 10);

  QPushButton* defaultPath = new QPushButton(tr("Default"), pathPage,
      "defaultPath");
  defaultPath-> setMaximumWidth(defaultPath->sizeHint().width() + 10);
  defaultPath-> setMinimumHeight(defaultPath->sizeHint().height() + 2);

  pathLayout-> addMultiCellWidget(defaultPath, 16, 16, 0, 1, Qt::AlignLeft);

  pathLayout-> addRowSpacing(15, 10);
  pathLayout-> setRowStretch(15, 1);

  connect(igcPathSearch, SIGNAL(clicked()), SLOT(slotSearchFlightPath()));
  connect(mapPathSearch, SIGNAL(clicked()), SLOT(slotSearchMapPath()));
  connect(taskPathSearch, SIGNAL(clicked()), SLOT(slotSearchTaskPath()));
  connect(waypointPathSearch, SIGNAL(clicked()), SLOT(slotSearchWaypointPath()));
  connect(defaultPath, SIGNAL(clicked()), SLOT(slotDefaultPath()));
}

void KFLogConfig::__addIDTab()
{
  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Identity"), "Identity");
  item->setPixmap(0, _mainWindow->getPixmap("kde_identity_32.png"));

  idPage = new Q3Frame(this, "Personal Information");
  configLayout->addMultiCellWidget(idPage, 0, 0, 1, 2);
  idPage->hide();

  QGridLayout* idLayout = new QGridLayout(idPage, 18, 5, 8, 1);

  Q3GroupBox* pilotGroup = new Q3GroupBox(idPage, "pilotGroup");
  pilotGroup-> setTitle(tr("Pilot") + ":");

  preNameE = new QLineEdit(idPage, "preNameE");
  surNameE = new QLineEdit(idPage, "surNameE");
  dateOfBirthE = new QLineEdit(idPage, "dateOfBirthE");

  idLayout-> addMultiCellWidget(pilotGroup, 0, 7, 0, 4);
  idLayout-> addWidget(new QLabel(tr("Prename"), idPage), 2, 1);
  idLayout-> addWidget(preNameE, 2, 3);
  idLayout-> addWidget(new QLabel(tr("Surname"), idPage), 4, 1);
  idLayout-> addWidget(surNameE, 4, 3);
  idLayout-> addWidget(new QLabel(tr("Birthday"), idPage), 6, 1);
  idLayout-> addWidget(dateOfBirthE, 6, 3);

  Q3GroupBox* homeGroup = new Q3GroupBox(idPage, "homeGroup");
  homeGroup-> setTitle(tr("Homesite") + ":");

  homeNameE = new QLineEdit(idPage, "homeNameE");
  homeNameE-> setMinimumWidth(150);
  homeLatE = new LatEdit(idPage, "homeLatE");
  homeLatE-> setMinimumWidth(150);
  homeLonE = new LongEdit(idPage, "homeLonE");
  homeLonE-> setMinimumWidth(150);

  idLayout-> addMultiCellWidget(homeGroup, 9, 16, 0, 4);
  idLayout-> addWidget(new QLabel(tr("Homesite"), idPage), 11, 1);
  idLayout-> addWidget(homeNameE, 11, 3);
  idLayout-> addWidget(new QLabel(tr("Latitude"), idPage), 13, 1);
  idLayout-> addWidget(homeLatE, 13, 3);
  idLayout-> addWidget(new QLabel(tr("Longitude"), idPage), 15, 1);
  idLayout-> addWidget(homeLonE, 15, 3);

  idLayout-> addColSpacing(0, 10);
  idLayout-> addColSpacing(2, 10);
  idLayout-> addColSpacing(4, 10);

  idLayout-> addRowSpacing(0, 25);
  idLayout-> addRowSpacing(1, 5);
  idLayout-> addRowSpacing(3, 5);
  idLayout-> addRowSpacing(5, 5);
  idLayout-> addRowSpacing(7, 20);

  idLayout-> addRowSpacing(8, 25);

  idLayout-> addRowSpacing(9, 25);
  idLayout-> addRowSpacing(10, 5);
  idLayout-> addRowSpacing(11, 10);
  idLayout-> addRowSpacing(12, 10);
  idLayout-> addRowSpacing(13, 10);
  idLayout-> addRowSpacing(14, 10);
//  idLayout-> addRowSpacing(15, 10);
  idLayout-> addRowSpacing(16, 20);
  idLayout-> addRowSpacing(17, 20);
  idLayout-> addRowSpacing(18, 25);
  idLayout-> addRowSpacing(19, 10);
  idLayout-> addRowSpacing(20, 10);

//  idLayout-> setRowStretch(17, 1);

  homeLatE-> setText(WGSPoint::printPos(_settings.readNumEntry("/MapData/Homesite Latitude", HOME_DEFAULT_LAT), true));
  homeLonE-> setText(WGSPoint::printPos(_settings.readNumEntry("/MapData/Homesite Longitude", HOME_DEFAULT_LON), false));
  homeNameE-> setText(_settings.readEntry("/MapData/Homesite", ""));

  preNameE-> setText(_settings.readEntry("/PersonalData/PreName", ""));
  surNameE-> setText(_settings.readEntry("/PersonalData/SurName", ""));
  dateOfBirthE-> setText(_settings.readEntry("/PersonalData/Birthday", ""));

  // update airfield lists from Welt2000 if home site changes:
  connect(homeLatE, SIGNAL(textChanged(const QString&)), SLOT(slotFilterChanged(const QString&)) );
  connect(homeLonE, SIGNAL(textChanged(const QString&)), SLOT(slotFilterChanged(const QString&)) );
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

/** Add a tab for airfield (Welt2000) configuration.*/
void KFLogConfig::__addAirfieldTab()
{
  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Airfields"), "Airfields");
  item->setPixmap(0, _mainWindow->getPixmap("airfield_32.png"));

  airfieldPage = new Q3Frame(this, "Airfield Configuration");
  airfieldPage->hide();
  configLayout->addMultiCellWidget(airfieldPage, 0, 0, 1, 2);

  QGridLayout* airfieldLayout = new QGridLayout(airfieldPage, 15, 6, 1, 1);

  Q3GroupBox* welt2000Group = new Q3GroupBox(airfieldPage, "welt2000Group");
  welt2000Group-> setTitle(tr("Welt2000") + ":");

  filterE = new QLineEdit(airfieldPage, "filterE");

  airfieldLayout->addMultiCellWidget(welt2000Group, 0, 4, 0, 6);
  airfieldLayout->addWidget(new QLabel(tr("Country Filter"), airfieldPage), 1, 1);
  airfieldLayout->addWidget(filterE, 1, 3);

  airfieldLayout->addWidget(new QLabel(tr("Home Radius"), airfieldPage), 3, 1);
  homeRadiusE = new QSpinBox(airfieldPage, "homeRadiusE");
  homeRadiusE->setRange( 0, 10000 );
  homeRadiusE->setLineStep( 10 );
  homeRadiusE->setButtonSymbols(QSpinBox::PlusMinus);
  airfieldLayout->addWidget( homeRadiusE, 3, 3 );
  airfieldLayout->addWidget( new QLabel( "km", airfieldPage), 3, 4 );

  filterE-> setText(_settings.readEntry("/Welt2000/CountryFilter", ""));
  homeRadiusE-> setValue(_settings.readNumEntry("/Welt2000/HomeRadius", 0));

  if (filterE->text() != "")
    homeRadiusE->setEnabled(false);
  needUpdateWelt2000 = false;
  connect(filterE, SIGNAL(textChanged(const QString&)), SLOT(slotFilterChanged(const QString&)) );
  connect(homeRadiusE, SIGNAL(valueChanged(int)), SLOT(slotHomeRadiusChanged()));
}

void KFLogConfig::slotFilterChanged(const QString& filter) {
  needUpdateWelt2000 = true;
  if (filter != "")
    homeRadiusE->setEnabled(false);
  else
    homeRadiusE->setEnabled(true);
}

void KFLogConfig::slotHomeRadiusChanged() {
  needUpdateWelt2000 = true;
}

/** Add a tab for waypoint catalog configuration at sartup
Setting will be overwritten by commandline switch */
void KFLogConfig::__addWaypointTab()
{
  int catalogType = _settings.readNumEntry("/Waypoints/DefaultWaypointCatalog", LastUsed);
  QString catalogName = _settings.readEntry("/Waypoints/DefaultCatalogName", "");

  Q3ListViewItem *item = new Q3ListViewItem(setupTree, tr("Waypoints"), "Waypoints");
  item->setPixmap(0, _mainWindow->getPixmap("waypoint_32.png"));

  waypointPage = new Q3Frame(this, "Catalog Configuration");
  waypointPage->hide();
  configLayout->addMultiCellWidget(waypointPage, 0, 0, 1, 2);

  QVBoxLayout *top = new QVBoxLayout(waypointPage, 5);

  Q3GroupBox *group = new Q3GroupBox(tr("Default Catalog:"), waypointPage, "catalogConfiguration");
  QGridLayout *grid = new QGridLayout(group, 4, 2, 25, 5);

  waypointButtonGroup = new Q3ButtonGroup(group);
  waypointButtonGroup-> hide();
  waypointButtonGroup-> setExclusive(true);
  connect(waypointButtonGroup, SIGNAL(clicked(int)), SLOT(slotSelectDefaultCatalog(int)));

  QRadioButton *rb = new QRadioButton(tr("Empty"), group);
  waypointButtonGroup-> insert(rb, Empty);
  grid-> addWidget(rb, 0, 0);
  rb = new QRadioButton(tr("Last used"), group);
  waypointButtonGroup-> insert(rb, LastUsed);
  grid-> addWidget(rb, 1, 0);
  rb = new QRadioButton(tr("Specific"), group);
  waypointButtonGroup-> insert(rb, Specific);
  grid-> addWidget(rb, 2, 0);
  catalogPathE = new QLineEdit(group);
  grid-> addWidget(catalogPathE, 3, 0);
  catalogPathE-> setText(catalogName);

  catalogPathSearch = new QPushButton(group);
  catalogPathSearch-> setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  catalogPathSearch-> setMinimumWidth(catalogPathSearch->sizeHint().width() + 5);
  catalogPathSearch-> setMinimumHeight(catalogPathSearch->sizeHint().height() + 5);
  grid-> addWidget(catalogPathSearch, 3, 1);

  connect(catalogPathSearch, SIGNAL(clicked()), SLOT(slotSearchDefaultWaypoint()));

  QPushButton* defaultCatalog = new QPushButton(tr("Default"), waypointPage,
      "defaultWaypointPath");
  defaultCatalog-> setMaximumWidth(defaultCatalog->sizeHint().width() + 10);
  defaultCatalog-> setMinimumHeight(defaultCatalog->sizeHint().height() + 2);

  connect(defaultCatalog, SIGNAL(clicked()), SLOT(slotDefaultWaypoint()));

  top-> addWidget(group);
  top-> addStretch();
  top-> addWidget(defaultCatalog, Qt::AlignLeft);

  slotSelectDefaultCatalog(catalogType);
}

void KFLogConfig::slotDefaultWaypoint()
{
  catalogPathE-> setText(QString::null);
  slotSelectDefaultCatalog(LastUsed);
}

void KFLogConfig::slotSelectDefaultCatalog(int item)
{
  QRadioButton *b = (QRadioButton *)waypointButtonGroup-> find(item);
  if (b != 0) {
    b-> setChecked(true);
  }

  catalogPathE-> setEnabled(item == Specific);
  catalogPathSearch-> setEnabled(item == Specific);
}

void KFLogConfig::slotSearchDefaultWaypoint()
{
  QString temp = Q3FileDialog::getOpenFileName(catalogPathE-> text(), "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)",
    this, 0, tr("Select default waypoint catalog"));

    if(temp != 0) {
      catalogPathE-> setText(temp);
    }
}

void KFLogConfig::slotSelectFlightTypeLeftTurnColor()
{
  flightTypeLeftTurnColor = QColorDialog::getColor(flightTypeLeftTurnColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeLeftTurnColor);
  flightTypeLeftTurnColorButton->setPixmap(*buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeRightTurnColor()
{
  flightTypeRightTurnColor = QColorDialog::getColor(flightTypeRightTurnColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeRightTurnColor);
  flightTypeRightTurnColorButton->setPixmap(*buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeMixedTurnColor()
{
  flightTypeMixedTurnColor = QColorDialog::getColor(flightTypeMixedTurnColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeMixedTurnColor);
  flightTypeMixedTurnColorButton->setPixmap(*buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeStraightColor()
{
  flightTypeStraightColor = QColorDialog::getColor(flightTypeStraightColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeStraightColor);
  flightTypeStraightColorButton->setPixmap(*buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeSolidColor()
{
  flightTypeSolidColor = QColorDialog::getColor(flightTypeSolidColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeSolidColor);
  flightTypeSolidColorButton->setPixmap(*buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeEngineNoiseColor()
{
  flightTypeEngineNoiseColor = QColorDialog::getColor(flightTypeEngineNoiseColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(82,14);
  buttonPixmap->fill(flightTypeEngineNoiseColor);
  flightTypeEngineNoiseColorButton->setPixmap(*buttonPixmap);
}

/** this is a temporary function and it is not needed in Qt 4 */
QString KFLogConfig::__color2String(QColor color)
{
  QString colstr;
  colstr.sprintf("%d;%d;%d", color.red(), color.green(), color.blue());
  return colstr;
}

/** this is a temporary function and it is not needed in Qt 4 */
QColor KFLogConfig::__string2Color(QString colstr)
{
  QColor color(colstr.section(";", 0, 0).toInt(), colstr.section(";", 1, 1).toInt(), colstr.section(";", 2, 2).toInt());
  return color;
}
