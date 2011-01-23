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
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
#include "rowdelegate.h"

extern MapContents  *_globalMapContents;
extern MainWindow   *_mainWindow;
extern QSettings    _settings;

KFLogConfig::KFLogConfig(QWidget* parent) :
  QDialog( parent ),
  needUpdateDrawType( false ),
  currentProjType( ProjectionBase::Unknown )
{
  setObjectName( "KFLogConfig" );
  setWindowTitle( tr("KFLog Configuration") );
  setModal( true );
  setSizeGripEnabled( true );

  configLayout = new QGridLayout(this);

  setupTree = new QTreeWidget( this );
  setupTree->setRootIsDecorated( false );
  setupTree->setItemsExpandable( false );
  //setupTree->setUniformRowHeights( true );
  setupTree->setSortingEnabled( true );
  setupTree->setSelectionMode( QAbstractItemView::SingleSelection );
  setupTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  setupTree->setColumnCount( 1 );
  setupTree->setFocusPolicy( Qt::StrongFocus );
  setupTree->setHeaderLabel( tr( "Menu" ) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( setupTree, 10 );
  setupTree->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = setupTree->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  configLayout->addWidget(setupTree, 0, 0);

  connect( setupTree, SIGNAL(itemClicked( QTreeWidgetItem*, int )),
           this, SLOT( slotPageClicked( QTreeWidgetItem*, int )) );

  QPushButton *saveButton = new QPushButton( tr( "&Save" ) );

  QPushButton *cancelButton = new QPushButton( tr( "&Cancel" ) );

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget( saveButton );
  hbox->addSpacing( 40 );
  hbox->addWidget( cancelButton );

  configLayout->addLayout( hbox, 1, 1, 1, 2 );
  configLayout->setColStretch(2, 10);

  connect( saveButton, SIGNAL(clicked()), SLOT(slotOk()) );
  connect( cancelButton, SIGNAL(clicked()), SLOT(close()) );

  __addIDTab();
  __addPathTab();
  __addScaleTab();
  __addMapTab();
  __addFlightTab();
  __addProjectionTab();
  __addAirfieldTab();
  __addWaypointTab();

  setupTree->sortByColumn ( 0, Qt::AscendingOrder );
  setupTree->resizeColumnToContents( 0 );

  idPage->setVisible( true );
  activePage = idPage;

  qDebug() << "setupTreeSizeHint=" << setupTree->sizeHint();
  qDebug() << "setupTreeSize=" << setupTree->size();

  setupTree->setFixedWidth( 160 );

  // activePage->setFixedWidth(500);
}

KFLogConfig::~KFLogConfig()
{
  qDebug() << "~KFLogConfig()";
}

void KFLogConfig::slotPageClicked( QTreeWidgetItem * item, int column )
{
  Q_UNUSED( column );

  QString itemText = item->data( 0, Qt::UserRole ).toString();

  if( itemText == "Airfields" )
    {
      activePage->setVisible( false );
      airfieldPage->setVisible( true );
      activePage = airfieldPage;
    }
  else if( itemText == "Flight Display" )
    {
      activePage->setVisible( false );
      flightPage->setVisible( true );
      activePage = flightPage;
    }
  else if( itemText == "Identity" )
    {
      activePage->setVisible( false );
      idPage->setVisible( true );
      activePage = idPage;
    }
  else if( itemText == "Map Elements" )
    {
      activePage->setVisible( false );
      mapPage->setVisible( true );
      activePage = mapPage;
    }
  else if( itemText == "Paths" )
    {
      activePage->setVisible( false );
      pathPage->setVisible( true );
      activePage = pathPage;
    }
  else if( itemText == "Map Projection" )
    {
      activePage->setVisible( false );
      projPage->setVisible( true );
      activePage = projPage;
    }
  else if( itemText == "Map Scales" )
    {
      activePage->setVisible( false );
      scalePage->setVisible( true );
      activePage = scalePage;
    }
  else if( itemText == "Waypoints" )
    {
      activePage->setVisible( false );
      waypointPage->setVisible( true );
      activePage = waypointPage;
    }
  else
    {
      qWarning() << "KFLogConfig::slotPageClicked: Unknown item"
                 << itemText
                 << "received!";
    }

  //activePage->setFixedWidth( 500 );
}

void KFLogConfig::slotOk()
{
  slotSelectProjection( ProjectionBase::Unknown );

  _settings.setValue( "/GeneralOptions/Version", "4.0" );

  _settings.setValue( "/Path/DefaultFlightDirectory", igcPathE->text() );
  _settings.setValue( "/Path/DefaultTaskDirectory", taskPathE->text() );
  _settings.setValue( "/Path/DefaultWaypointDirectory", waypointPathE->text() );
  _settings.setValue( "/Path/DefaultMapDirectory", mapPathE->text() );

  _settings.setValue( "/Scale/LowerLimit", lLimitN->value() );
  _settings.setValue( "/Scale/UpperLimit", uLimitN->value() );
  _settings.setValue( "/Scale/SwitchScale", switchScaleN->value() );
  _settings.setValue( "/Scale/WaypointLabel", wpLabelN->value() );
  _settings.setValue( "/Scale/Border1", reduce1N->value() );
  _settings.setValue( "/Scale/Border2", reduce2N->value() );
  _settings.setValue( "/Scale/Border3", reduce3N->value() );

  _settings.setValue( "/MapData/Homesite", homeNameE->text() );
  _settings.setValue( "/MapData/HomesiteLatitude", WGSPoint::degreeToNum( homeLatE->text() ) );
  _settings.setValue( "/MapData/HomesiteLongitude", WGSPoint::degreeToNum( homeLonE->text() ) );
  _settings.setValue( "/MapData/ProjectionType", projectionSelect->currentItem() );

  _settings.setValue( "/Welt2000/CountryFilter", filterWelt2000->text() );
  _settings.setValue( "/Welt2000/HomeRadius", homeRadiusWelt2000->value() );

  if( needUpdateDrawType )
    {
      _settings.setValue( "/Flight/DrawType", drawTypeSelect->currentItem() );
      //update menu Flight=>Show Flightdata
      emit newDrawType( drawTypeSelect->currentItem() );
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

  _settings.setValue( "/Waypoints/DefaultWaypointCatalog",
                      waypointButtonGroup->id(waypointButtonGroup->checkedButton()) );
  _settings.setValue("/Waypoints/DefaultCatalogName", catalogPathE->text());

  emit scaleChanged((int)lLimitN->value(), (int)uLimitN->value());

  // Check, if Welt2000 must be updated
  if( homeRadiusWelt2000Value !=  homeRadiusWelt2000->value() ||
      filterWelt2000Text != filterWelt2000->text() )
    {
      extern MapContents *_globalMapContents;
      _globalMapContents->slotReloadWelt2000Data();
    }

  emit configOk();
  accept();
}

void KFLogConfig::slotSearchFlightPath()
{
  QString dir = QFileDialog::getExistingDirectory( this,
                 tr("Select a directory for the IGC-Files"),
                 igcPathE->text(),
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  if( ! dir.isEmpty() )
    {
      igcPathE->setText( dir );
    }
}

void KFLogConfig::slotSearchMapPath()
{
  QString dir = QFileDialog::getExistingDirectory( this,
                 tr("Select a directory for the Map-Files"),
                 mapPathE->text(),
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  if( ! dir.isEmpty() )
    {
      mapPathE->setText( dir );
    }
}

void KFLogConfig::slotSearchTaskPath()
{
  QString dir = QFileDialog::getExistingDirectory( this,
                 tr("Select a directory for the Task-Files"),
                 taskPathE->text(),
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  if( ! dir.isEmpty() )
    {
      taskPathE->setText( dir );
    }
}

void KFLogConfig::slotSearchWaypointPath()
{
  QString dir = QFileDialog::getExistingDirectory( this,
                 tr("Select a directory for the Waypoint-Files"),
                 waypointPathE->text(),
                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

  if( ! dir.isEmpty() )
    {
      waypointPathE->setText( dir );
    }
}

void KFLogConfig::slotSelectProjection(int index)
{
  switch(currentProjType)
    {
      case ProjectionBase::Lambert:

        lambertV1     = WGSPoint::degreeToNum(firstParallel->text());
        lambertV2     = WGSPoint::degreeToNum(secondParallel->text());
        lambertOrigin = WGSPoint::degreeToNum(originLongitude->text());
        break;

      case ProjectionBase::Cylindric:

        cylinPar = WGSPoint::degreeToNum(firstParallel->text());
        break;
    }

  switch(index)
    {
      case ProjectionBase::Lambert:

        secondParallel->setEnabled(true);
        originLongitude->setEnabled(true);
        firstParallel->setText(WGSPoint::printPos(lambertV1, true));
        secondParallel->setText(WGSPoint::printPos(lambertV2, true));
        originLongitude->setText(WGSPoint::printPos(lambertOrigin, false));
        break;

      case ProjectionBase::Cylindric:

        secondParallel->setEnabled(false);
        originLongitude->setEnabled(false);
        firstParallel->setText(WGSPoint::printPos(cylinPar, true));
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

void KFLogConfig::slotShowWpLabel(int value)
{
  wpLabelN->display(__setScaleValue(value));
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

  currentProjType = ProjectionBase::Unknown;

  projectionSelect->setCurrentItem(ProjectionBase::Lambert);
  slotSelectProjection(ProjectionBase::Lambert);
}

void KFLogConfig::slotDefaultScale()
{
  lLimit->setValue(__getScaleValue(L_LIMIT));
  lLimitN->display(L_LIMIT);
  uLimit->setValue(__getScaleValue(U_LIMIT));
  uLimitN->display(U_LIMIT);
  switchScale->setValue(__getScaleValue(SWITCH_S));
  switchScaleN->display(SWITCH_S);
  wpLabel->setValue(__getScaleValue(WPLABEL));
  wpLabelN->display(WPLABEL);
  reduce1->setValue(__getScaleValue(BORDER_1));
  reduce1N->display(BORDER_1);
  reduce2->setValue(__getScaleValue(BORDER_2));
  reduce2N->display(BORDER_2);
  reduce3->setValue(__getScaleValue(BORDER_3));
  reduce3N->display(BORDER_3);
}

void KFLogConfig::slotDefaultPath()
{
  igcPathE->setText( _mainWindow->getApplicationDataDirectory() );
  taskPathE->setText( _mainWindow->getApplicationDataDirectory() );
  waypointPathE->setText( _mainWindow->getApplicationDataDirectory() );
  mapPathE->setText( _globalMapContents->getMapRootDirectory() );
}

void KFLogConfig::__addMapTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Map Elements") );
  item->setData( 0, Qt::UserRole, "Map Elements" );
  item->setIcon( 0, _mainWindow->getPixmap("kflog_32.png") );
  setupTree->addTopLevelItem( item );

  mapPage = new QFrame(this);
  mapPage->setObjectName( "MapPage" );
  mapPage->setVisible( false );

  configLayout->addWidget( mapPage, 0, 1, 1, 2 );

  Q3GroupBox* elementBox = new Q3GroupBox(mapPage, "elementBox");
  elementBox->setTitle(tr("visible Map-Elements"));

  elementSelect = new QComboBox(mapPage, "elementSelect");
//  elementSelect->setMaximumWidth(300);
  elementSelect->insertItem(tr("Road"), KFLogConfig::Road);
  elementSelect->insertItem(tr("Highway"), KFLogConfig::Highway);
  elementSelect->insertItem(tr("Railway"), KFLogConfig::Railway);
  elementSelect->insertItem(tr("River / Lake"), KFLogConfig::River);
  elementSelect->insertItem(tr("Canal"), KFLogConfig::Canal);
  elementSelect->insertItem(tr("City"), KFLogConfig::City);
  elementSelect->insertItem(tr("Airspace A"), KFLogConfig::AirA);
  elementSelect->insertItem(tr("Airspace B"), KFLogConfig::AirB);
  elementSelect->insertItem(tr("Airspace C"), KFLogConfig::AirC);
  elementSelect->insertItem(tr("Airspace D"), KFLogConfig::AirD);
  elementSelect->insertItem(tr("Airspace E (low)"), KFLogConfig::AirElow);
  elementSelect->insertItem(tr("Airspace E (high)"), KFLogConfig::AirEhigh);
  elementSelect->insertItem(tr("Airspace F"), KFLogConfig::AirF);
  elementSelect->insertItem(tr("Control C"), KFLogConfig::ControlC);
  elementSelect->insertItem(tr("Control D"), KFLogConfig::ControlD);
  elementSelect->insertItem(tr("Danger"), KFLogConfig::Danger);
  elementSelect->insertItem(tr("Low flight area"), KFLogConfig::LowFlight);
  elementSelect->insertItem(tr("Restricted"), KFLogConfig::Restricted);
  elementSelect->insertItem(tr("TMZ"), KFLogConfig::Tmz);
  // Reihenfolge ???
  elementSelect->insertItem(tr("Forest"), KFLogConfig::Forest);
  elementSelect->insertItem(tr("Trail"), KFLogConfig::Trail);
  elementSelect->insertItem(tr("double Railway"), KFLogConfig::Railway_D);
  elementSelect->insertItem(tr("Aerial Cable"), KFLogConfig::Aerial_Cable);
  elementSelect->insertItem(tr("temporarily River / Lake"), KFLogConfig::River_T);
  elementSelect->insertItem(tr("Glacier"), KFLogConfig::Glacier);
  elementSelect->insertItem(tr("Pack Ice"), KFLogConfig::PackIce);
  elementSelect->insertItem(tr("FAI Area <500 km"), KFLogConfig::FAIAreaLow500);
  elementSelect->insertItem(tr("FAI Area >500 km"), KFLogConfig::FAIAreaHigh500);

  QPushButton* defaultElements = new QPushButton(tr("Default"), mapPage,
      "defaultElements");
  defaultElements->setMaximumWidth(defaultElements->sizeHint().width() + 10);
  defaultElements->setMinimumHeight(defaultElements->sizeHint().height() + 2);

  QTabWidget* tabView = new QTabWidget(mapPage);
  QFrame* screenFrame = new QFrame(tabView);
  ConfigDrawElement* drawConfig = new ConfigDrawElement(screenFrame);

  QFrame* printFrame = new QFrame(tabView);
  ConfigPrintElement* printConfig = new ConfigPrintElement(printFrame);

  tabView->addTab(screenFrame, tr("Display"));
  tabView->addTab(printFrame, tr("Print"));

  QGridLayout* elLayout = new QGridLayout(mapPage, 7, 5, 8, 1);
  elLayout->addMultiCellWidget(elementBox, 0, 4, 0, 4);
  elLayout->addMultiCellWidget(elementSelect, 1, 1, 1, 2, Qt::AlignLeft);
  elLayout->addMultiCellWidget(tabView, 3, 3, 1, 3);
  elLayout->addMultiCellWidget(defaultElements, 6, 6, 0, 1, Qt::AlignLeft);

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

void KFLogConfig::__addFlightTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Flight Display") );
  item->setData( 0, Qt::UserRole, "Flight Display" );
  item->setIcon( 0, _mainWindow->getPixmap("flightpath_32.png") );
  setupTree->addTopLevelItem( item );

  flightPage = new QFrame(this);
  flightPage->setObjectName( "FlightPage" );
  flightPage->setVisible( false );

  configLayout->addWidget( flightPage, 0, 1, 1, 2 );

  QGridLayout* flightLayout = new QGridLayout(flightPage, 17, 40, 8, 1);

  Q3GroupBox* flightPathLineGroup = new Q3GroupBox(flightPage, "flightDisplayGroup");
  flightPathLineGroup->setTitle(tr("Flight Path Line") + ":");
  flightLayout->addMultiCellWidget(flightPathLineGroup, 0, 4, 0, 39);


  drawTypeSelect = new QComboBox(flightPage, "drawTypeSelect");
  drawTypeSelect->insertItem(tr("Altitude"), MapConfig::Altitude);
  drawTypeSelect->insertItem(tr("Cycling"),  MapConfig::Cycling);
  drawTypeSelect->insertItem(tr("Speed"),    MapConfig::Speed);
  drawTypeSelect->insertItem(tr("Vario"),    MapConfig::Vario);
  drawTypeSelect->insertItem(tr("Solid"),    MapConfig::Solid);
  drawTypeSelect->setCurrentItem(_settings.readNumEntry("/Flight/DrawType"));

  flightLayout->addWidget(new QLabel(tr("type") + ":", flightPage), 1, 1);
  flightLayout->addWidget(drawTypeSelect, 1, 10);


  flightPathWidthE = new QSpinBox(flightPage, "flightPathWidthE");
  flightPathWidthE->setRange( 0, 9 );
  flightPathWidthE->setLineStep( 1 );
  flightPathWidthE->setValue(_settings.readNumEntry("/Flight/flightPathWidth", 4));

  flightLayout->addWidget(new QLabel(tr("width") + ":", flightPage), 3, 1);
  flightLayout->addWidget( flightPathWidthE, 3, 10);


  Q3GroupBox* flightPathColorGroup = new Q3GroupBox(flightPage, "flightDisplayGroup");
  flightPathColorGroup->setTitle(tr("Flight Path Colors") + ":");
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
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Map Projection") );
  item->setData( 0, Qt::UserRole, "Map Projection" );
  item->setIcon( 0, _mainWindow->getPixmap("projection_32.png") );
  setupTree->addTopLevelItem( item );

  projPage = new QFrame(this);
  projPage->setObjectName( "MapProjectionPage" );
  projPage->setVisible( false );

  configLayout->addWidget( projPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* projTypeGroup = new QGroupBox( tr("Type of Projection") + ":" );

  QGridLayout* projTypeLayout = new QGridLayout();
  projTypeLayout->setSpacing(10);

  projectionSelect = new QComboBox( projPage );
  projectionSelect->addItem( tr( "Conical orthomorphic (Lambert)" ) );
  projectionSelect->addItem( tr( "Cylindrical Equidistant (Plate Carré)" ) );
  connect( projectionSelect, SIGNAL(activated(int)), SLOT(slotSelectProjection(int)) );

  projTypeLayout->addWidget( projectionSelect, 0, 0 );
  projTypeLayout->setColumnStretch( 1, 10);

  projTypeGroup->setLayout( projTypeLayout );

  //----------------------------------------------------------------------------
  QGroupBox* projConfGroup = new QGroupBox( tr( "Setup Projection" ) + ":" );

  QFormLayout* projConfLayout = new QFormLayout();
  projConfLayout->setSpacing( 10 );

  firstParallel   = new LatEdit();
  secondParallel  = new LatEdit();
  originLongitude = new LongEdit();

  const int minLen = 150;

  firstParallel->setMinimumWidth( minLen );
  secondParallel->setMinimumWidth( minLen );
  originLongitude->setMinimumWidth( minLen );

  projConfLayout->addRow( tr( "1. Standard Parallel" ) + ":", firstParallel );
  projConfLayout->addRow( tr( "2. Standard Parallel" ) + ":", secondParallel );
  projConfLayout->addRow( tr( "Origin Longitude" ) + ":", originLongitude );

  projConfGroup->setLayout( projConfLayout );

  //----------------------------------------------------------------------------
  QPushButton* defaultProjButton = new QPushButton( tr( "Default" ), projPage );
  defaultProjButton->setMaximumWidth( defaultProjButton->sizeHint().width() + 10 );
  defaultProjButton->setMinimumHeight( defaultProjButton->sizeHint().height() + 2 );
  connect( defaultProjButton, SIGNAL(clicked()), SLOT(slotDefaultProjection()) );

  //----------------------------------------------------------------------------
  QVBoxLayout* projPageLayout = new QVBoxLayout;

  projPageLayout->addWidget( projTypeGroup );
  projPageLayout->addSpacing( 20 );
  projPageLayout->addWidget( projConfGroup );
  projPageLayout->addStretch( 10 );
  projPageLayout->addWidget( defaultProjButton, 0, Qt::AlignLeft );

  projPage->setLayout( projPageLayout );

  lambertV1     = _settings.value("/LambertProjection/Parallel1", 32400000).toInt();
  lambertV2     = _settings.value("/LambertProjection/Parallel2", 30000000).toInt();
  lambertOrigin = _settings.value("/LambertProjection/Origin", 0).toInt();
  cylinPar      = _settings.value("/CylindricalProjection/Parallel", 27000000).toInt();
  int projIndex = _settings.value("/MapData/ProjectionType", ProjectionBase::Lambert).toInt();

  projectionSelect->setCurrentItem( projIndex );
  slotSelectProjection( projIndex );
}

void KFLogConfig::__addScaleTab()
{
  int ll = _settings.value("/Scale/LowerLimit", L_LIMIT).toInt();
  int ul = _settings.value("/Scale/UpperLimit", U_LIMIT).toInt();
  int sw = _settings.value("/Scale/SwitchScale", SWITCH_S).toInt();
  int wl = _settings.value("/Scale/WaypointLabel", WPLABEL).toInt();
  int b1 = _settings.value("/Scale/Border1", BORDER_1).toInt();
  int b2 = _settings.value("/Scale/Border2", BORDER_2).toInt();
  int b3 = _settings.value("/Scale/Border3", BORDER_3).toInt();

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Map Scales") );
  item->setData( 0, Qt::UserRole, "Map Scales" );
  item->setIcon( 0, _mainWindow->getPixmap("kde_viewmag_32.png") );
  setupTree->addTopLevelItem( item );

  scalePage = new QFrame(this);
  scalePage->setObjectName( "MapScalePage" );
  scalePage->setVisible( false );

  configLayout->addWidget( scalePage, 0, 1, 1, 2 );

  Q3GroupBox* scaleLimits = new Q3GroupBox(scalePage, "scaleLimitBox");
  scaleLimits->setTitle(tr("Scale-Range:"));

  QLabel* lLimitText = new QLabel(tr("lower limit"), scalePage);
  lLimit = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  lLimit->setMinimumHeight(lLimit->sizeHint().height() + 5);
  lLimit->setMaximumHeight(lLimit->sizeHint().height() + 20);
  lLimit->setMinimumWidth(200);
  lLimitN = new QLCDNumber(5, scalePage);
  lLimitN->setMinimumWidth(lLimitN->sizeHint().width() + 10);
  lLimit->setValue(__getScaleValue(ll));
  lLimitN->display(ll);

  QLabel* uLimitText = new QLabel(tr("upper limit"), scalePage);
  uLimit = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  uLimit->setMinimumHeight(uLimit->sizeHint().height() + 5);
  uLimit->setMaximumHeight(uLimit->sizeHint().height() + 20);
  uLimitN = new QLCDNumber(5, scalePage);
  uLimit->setValue(__getScaleValue(ul));
  uLimitN->display(ul);

  Q3GroupBox* borderBox = new Q3GroupBox(scalePage, "borderBox");
  borderBox->setTitle("Scale-Thresholds:");

  QLabel* switchText = new QLabel(tr("use small icons"), scalePage);
  switchScale = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  switchScale->setMinimumHeight(switchScale->sizeHint().height() + 5);
  switchScale->setMaximumHeight(switchScale->sizeHint().height() + 20);
  switchScaleN = new QLCDNumber(5, scalePage);
  switchScale->setValue(__getScaleValue(sw));
  switchScaleN->display(sw);

  QLabel* wpLabelText = new QLabel(tr("draw waypoint labels"), scalePage);
  wpLabel = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  wpLabel->setMinimumHeight(switchScale->sizeHint().height() + 5);
  wpLabel->setMaximumHeight(switchScale->sizeHint().height() + 20);
  wpLabelN = new QLCDNumber(5, scalePage);
  wpLabel->setValue(__getScaleValue(wl));
  wpLabelN->display(wl);

  QLabel* reduce1Text = new QLabel(tr("threshold #1"), scalePage);
  reduce1 = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  reduce1->setMinimumHeight(reduce1->sizeHint().height() + 5);
  reduce1->setMaximumHeight(reduce1->sizeHint().height() + 20);
  reduce1N = new QLCDNumber(5, scalePage);
  reduce1->setValue(__getScaleValue(b1));
  reduce1N->display(b1);

  QLabel* reduce2Text = new QLabel(tr("threshold #2"), scalePage);
  reduce2 = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  reduce2->setMinimumHeight(reduce2->sizeHint().height() + 5);
  reduce2->setMaximumHeight(reduce2->sizeHint().height() + 20);
  reduce2N = new QLCDNumber(5, scalePage);
  reduce2->setValue(__getScaleValue(b2));
  reduce2N->display(b2);

  QLabel* reduce3Text = new QLabel(tr("threshold #3"), scalePage);
  reduce3 = new QSlider(2,105,1,0, Qt::Horizontal, scalePage);
  reduce3->setMinimumHeight(reduce3->sizeHint().height() + 5);
  reduce3->setMaximumHeight(reduce3->sizeHint().height() + 20);
  reduce3N = new QLCDNumber(5, scalePage);
  reduce3->setValue(__getScaleValue(b3));
  reduce3N->display(b3);

  QPushButton* defaultScale = new QPushButton(tr("Default"), scalePage,
      "defaultScale");
  defaultScale->setMaximumWidth(defaultScale->sizeHint().width() + 10);
  defaultScale->setMinimumHeight(defaultScale->sizeHint().height() + 2);

  QGridLayout* scaleLayout = new QGridLayout(scalePage, 19, 7, 8, 1);
  scaleLayout->addMultiCellWidget(scaleLimits, 0, 4, 0, 6);
  scaleLayout->addWidget(lLimitText, 1, 1);
  scaleLayout->addWidget(lLimit, 1, 3);
  scaleLayout->addWidget(lLimitN, 1, 5);
  scaleLayout->addWidget(uLimitText, 3, 1);
  scaleLayout->addWidget(uLimit, 3, 3);
  scaleLayout->addWidget(uLimitN, 3, 5);

  scaleLayout->addMultiCellWidget(borderBox, 6, 16, 0, 6);
  scaleLayout->addWidget(switchText, 7, 1);
  scaleLayout->addWidget(switchScale, 7, 3);
  scaleLayout->addWidget(switchScaleN, 7, 5);
  scaleLayout->addWidget(wpLabelText, 9, 1);
  scaleLayout->addWidget(wpLabel, 9, 3);
  scaleLayout->addWidget(wpLabelN, 9, 5);
  scaleLayout->addWidget(reduce1Text, 11, 1);
  scaleLayout->addWidget(reduce1, 11, 3);
  scaleLayout->addWidget(reduce1N, 11, 5);
  scaleLayout->addWidget(reduce2Text, 13, 1);
  scaleLayout->addWidget(reduce2, 13, 3);
  scaleLayout->addWidget(reduce2N, 13, 5);
  scaleLayout->addWidget(reduce3Text, 15, 1);
  scaleLayout->addWidget(reduce3, 15, 3);
  scaleLayout->addWidget(reduce3N, 15, 5);

  scaleLayout->addMultiCellWidget(defaultScale, 18, 18, 0, 1, Qt::AlignLeft);

  scaleLayout->addColSpacing(0, 10);
  scaleLayout->addColSpacing(2, 5);
  scaleLayout->addColSpacing(4, 10);
  scaleLayout->addColSpacing(6, 10);

  scaleLayout->setColStretch(3, 1);

  scaleLayout->addRowSpacing(0, 25);
  scaleLayout->addRowSpacing(4, 5);
  scaleLayout->addRowSpacing(5, 10);
  scaleLayout->addRowSpacing(6, 25);
  scaleLayout->addRowSpacing(16, 5);
  scaleLayout->addRowSpacing(17, 10);

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

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Paths") );
  item->setData( 0, Qt::UserRole, "Paths" );
  item->setIcon( 0, _mainWindow->getPixmap("kde_fileopen_32.png") );
  setupTree->addTopLevelItem( item );

  pathPage = new QFrame(this);
  pathPage->setObjectName( "PathPage" );
  pathPage->setVisible( false );

  configLayout->addWidget( pathPage, 0, 1, 1, 2 );

  // minimal length of path input fields
  const int minPath = 350;
  //----------------------------------------------------------------------------
  QGroupBox* igcGroup = new QGroupBox( tr("Flight Directory") + ":" );

  igcPathE  = new QLineEdit();
  igcPathE->setMinimumWidth(minPath);
  igcPathE->setText(flightDir);

  QPushButton* igcPathSearch = new QPushButton();
  igcPathSearch->setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  igcPathSearch->setMinimumWidth(igcPathSearch->sizeHint().width() + 5);
  igcPathSearch->setMinimumHeight(igcPathSearch->sizeHint().height() + 5);

  QGridLayout* igcLayout = new QGridLayout();
  igcLayout->setSpacing( 10 );
  igcLayout->addWidget( igcPathE, 0, 0 );
  igcLayout->addWidget( igcPathSearch, 0, 1 );
  igcLayout->setColStretch( 0, 10 );
  igcGroup->setLayout( igcLayout );

  //----------------------------------------------------------------------------
  QGroupBox* taskGroup = new QGroupBox( tr("Task Directory") + ":" );

  taskPathE = new QLineEdit();
  taskPathE->setMinimumWidth(minPath);
  taskPathE->setText(taskDir);

  QPushButton* taskPathSearch = new QPushButton();
  taskPathSearch->setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  taskPathSearch->setMinimumWidth(taskPathSearch->sizeHint().width() + 5);
  taskPathSearch->setMinimumHeight(taskPathSearch->sizeHint().height() + 5);

  QGridLayout* taskLayout = new QGridLayout();
  taskLayout->setSpacing( 10 );
  taskLayout->addWidget( taskPathE, 0, 0 );
  taskLayout->addWidget( taskPathSearch, 0, 1 );
  taskLayout->setColStretch( 0, 10 );
  taskGroup->setLayout( taskLayout );

  //----------------------------------------------------------------------------
  QGroupBox* waypointGroup = new QGroupBox( tr("Waypoint Directory") + ":" );

  waypointPathE = new QLineEdit();
  waypointPathE->setMinimumWidth(minPath);
  waypointPathE->setText(wayPointDir);

  QPushButton* waypointPathSearch = new QPushButton();
  waypointPathSearch->setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  waypointPathSearch->setMinimumWidth(waypointPathSearch->sizeHint().width() + 5);
  waypointPathSearch->setMinimumHeight(waypointPathSearch->sizeHint().height() + 5);

  QGridLayout* waypointLayout = new QGridLayout();
  waypointLayout->setSpacing( 10 );
  waypointLayout->addWidget( waypointPathE, 0, 0 );
  waypointLayout->addWidget( waypointPathSearch, 0, 1 );
  waypointLayout->setColStretch( 0, 10 );
  waypointGroup->setLayout( waypointLayout );

  //----------------------------------------------------------------------------
  QGroupBox* mapGroup = new QGroupBox( tr("Map Directory") + ":" );

  mapPathE = new QLineEdit();
  mapPathE->setMinimumWidth(minPath);
  mapPathE->setText(mapDir);

  QPushButton* mapPathSearch = new QPushButton();
  mapPathSearch->setPixmap(_mainWindow->getPixmap("kde_fileopen_16.png"));
  mapPathSearch->setMinimumWidth(mapPathSearch->sizeHint().width() + 5);
  mapPathSearch->setMinimumHeight(mapPathSearch->sizeHint().height() + 5);

  QGridLayout* mapLayout = new QGridLayout();
  mapLayout->setSpacing( 10 );
  mapLayout->addWidget( mapPathE, 0, 0 );
  mapLayout->addWidget( mapPathSearch, 0, 1 );
  mapLayout->setColStretch( 0, 10 );
  mapGroup->setLayout( mapLayout );

  //----------------------------------------------------------------------------
  QPushButton* defaultPath = new QPushButton( tr("Default") );
  defaultPath->setMaximumWidth(defaultPath->sizeHint().width() + 10);
  defaultPath->setMinimumHeight(defaultPath->sizeHint().height() + 2);

  //----------------------------------------------------------------------------
  QVBoxLayout*  pathPageLayout = new QVBoxLayout;
  pathPageLayout->addWidget( igcGroup );
  pathPageLayout->addSpacing( 10 );
  pathPageLayout->addWidget( taskGroup );
  pathPageLayout->addSpacing( 10 );
  pathPageLayout->addWidget( waypointGroup );
  pathPageLayout->addSpacing( 10 );
  pathPageLayout->addWidget( mapGroup );
  pathPageLayout->addSpacing( 10 );
  pathPageLayout->addStretch( 10 );
  pathPageLayout->addWidget( defaultPath, 0, Qt::AlignLeft );

  pathPage->setLayout( pathPageLayout );

  connect(igcPathSearch, SIGNAL(clicked()), SLOT(slotSearchFlightPath()));
  connect(mapPathSearch, SIGNAL(clicked()), SLOT(slotSearchMapPath()));
  connect(taskPathSearch, SIGNAL(clicked()), SLOT(slotSearchTaskPath()));
  connect(waypointPathSearch, SIGNAL(clicked()), SLOT(slotSearchWaypointPath()));
  connect(defaultPath, SIGNAL(clicked()), SLOT(slotDefaultPath()));
}

void KFLogConfig::__addIDTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Identity") );
  item->setData( 0, Qt::UserRole, "Identity" );
  item->setIcon( 0, _mainWindow->getPixmap("kde_identity_32.png") );
  setupTree->addTopLevelItem( item );

  idPage = new QFrame(this);
  idPage->setObjectName( "IdentityPage" );
  idPage->setVisible( false );

  configLayout->addWidget( idPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  const int minLen = 150;

  QGroupBox* pilotGroup = new QGroupBox( tr("Pilot") + ":" );

  QFormLayout* pilotLayout = new QFormLayout();
  pilotLayout->setSpacing(10);

  preNameE     = new QLineEdit();
  surNameE     = new QLineEdit();
  dateOfBirthE = new QLineEdit();

  preNameE->setMinimumWidth( minLen );
  surNameE->setMinimumWidth( minLen );
  dateOfBirthE->setMinimumWidth( minLen );

  pilotLayout->addRow( tr("Forename") + ":", preNameE );
  pilotLayout->addRow( tr("Surname") + ":", surNameE );
  pilotLayout->addRow( tr("Birthday") + ":", dateOfBirthE );

  pilotGroup->setLayout( pilotLayout );

  //----------------------------------------------------------------------------
  QGroupBox* homeGroup = new QGroupBox( tr("Homesite") + ":" );

  QFormLayout* homeLayout = new QFormLayout();
  homeLayout->setSpacing(10);

  homeNameE = new QLineEdit(idPage, "homeNameE");
  homeLatE  = new LatEdit(idPage, "homeLatE");
  homeLonE  = new LongEdit(idPage, "homeLonE");

  homeNameE->setMinimumWidth( minLen );
  homeLatE->setMinimumWidth( minLen );
  homeLonE->setMinimumWidth( minLen );

  homeLayout->addRow( tr("Homesite") + ":", homeNameE );
  homeLayout->addRow( tr("Latitude") + ":", homeLatE );
  homeLayout->addRow( tr("Longitude") + ":", homeLonE );

  homeGroup->setLayout( homeLayout );

  //----------------------------------------------------------------------------
  QVBoxLayout* idLayout = new QVBoxLayout;

  idLayout->addWidget( pilotGroup );
  idLayout->addSpacing( 20 );
  idLayout->addWidget( homeGroup );
  idLayout->addStretch( 10 );

  idPage->setLayout( idLayout );

  homeLatE->setText(WGSPoint::printPos(_settings.value("/MapData/Homesite Latitude", HOME_DEFAULT_LAT).toInt(), true));
  homeLonE->setText(WGSPoint::printPos(_settings.value("/MapData/Homesite Longitude", HOME_DEFAULT_LON).toInt(), false));
  homeNameE->setText(_settings.value("/MapData/Homesite", "").toString());

  preNameE->setText(_settings.value("/PersonalData/PreName", "").toString());
  surNameE->setText(_settings.value("/PersonalData/SurName", "").toString());
  dateOfBirthE->setText(_settings.value("/PersonalData/Birthday", "").toString());
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
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Airfields") );
  item->setData( 0, Qt::UserRole, "Airfields" );
  item->setIcon( 0, _mainWindow->getPixmap("airfield_32.png") );
  setupTree->addTopLevelItem( item );

  airfieldPage = new QFrame(this);
  airfieldPage->setObjectName( "AirfieldPage" );
  airfieldPage->setVisible( false );

  configLayout->addWidget( airfieldPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* welt2000Group = new QGroupBox( tr("Welt2000") + ":" );

  // Check input with a validator. Country codes are consist of 2 letters and
  // are separated by space, comma or semicolon.
  QRegExp rx("[A-Za-z]{2}([ ,;][A-Za-z]{2})*");
  QValidator *validator = new QRegExpValidator(rx, this);

  filterWelt2000 = new QLineEdit();
  filterWelt2000->setMinimumWidth( 150 );
  filterWelt2000->setValidator( validator );

  QFormLayout* weltLayout = new QFormLayout();
  weltLayout->setSpacing( 10 );
  weltLayout->addRow( tr( "Country Filter" ) + ":", filterWelt2000 );

  homeRadiusWelt2000 = new QSpinBox();
  homeRadiusWelt2000->setRange( 0, 10000 );
  homeRadiusWelt2000->setLineStep( 10 );
  homeRadiusWelt2000->setButtonSymbols( QSpinBox::PlusMinus );
  homeRadiusWelt2000->setSuffix( " Km" );
  homeRadiusWelt2000->setSpecialValueText(tr("Off"));

  weltLayout->addRow( tr( "Home Radius" )  + ":", homeRadiusWelt2000 );

  welt2000Group->setLayout( weltLayout );

  //----------------------------------------------------------------------------
  QVBoxLayout* afLayout = new QVBoxLayout;

  afLayout->addWidget( welt2000Group );
  afLayout->addStretch( 10 );

  airfieldPage->setLayout( afLayout );

  homeRadiusWelt2000Value = _settings.value( "/Welt2000/HomeRadius", 0 ).toInt();
  filterWelt2000Text      = _settings.value( "/Welt2000/CountryFilter", "" ).toString();

  homeRadiusWelt2000->setValue( homeRadiusWelt2000Value );
  filterWelt2000->setText( filterWelt2000Text );
}

/** Add a tab for waypoint catalog configuration at sartup
Setting will be overwritten by commandline switch */
void KFLogConfig::__addWaypointTab()
{
  int catalogType     = _settings.value("/Waypoints/DefaultWaypointCatalog", LastUsed).toInt();
  QString catalogName = _settings.value("/Waypoints/DefaultCatalogName", "").toString();

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Waypoints") );
  item->setData( 0, Qt::UserRole, "Waypoints" );
  item->setIcon( 0, _mainWindow->getPixmap("waypoint_32.png") );
  setupTree->addTopLevelItem( item );

  waypointPage = new QFrame(this);
  waypointPage->setObjectName( "WaypointPage" );
  waypointPage->setVisible( false );

  configLayout->addWidget( waypointPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* catGroup = new QGroupBox( tr("Default Catalog") + ":" );

  QVBoxLayout* catGroupLayout = new QVBoxLayout();

  waypointButtonGroup = new QButtonGroup(waypointPage);
  waypointButtonGroup->setExclusive(true);
  connect( waypointButtonGroup, SIGNAL( buttonClicked(int)),
           SLOT(slotSelectDefaultCatalog(int)) );

  QRadioButton *rb = new QRadioButton( tr( "Empty" ) );
  waypointButtonGroup->addButton( rb, Empty );
  catGroupLayout->addWidget( rb );
  rb = new QRadioButton( tr( "Last used" ) );
  waypointButtonGroup->addButton( rb, LastUsed );
  catGroupLayout->addWidget( rb );
  rb = new QRadioButton( tr( "Specific" ) );
  waypointButtonGroup->addButton( rb, Specific );
  catGroupLayout->addWidget( rb );

  catalogPathE = new QLineEdit();
  catalogPathE->setMinimumWidth( 350 );
  catalogPathE->setText( catalogName );

  catalogPathSearch = new QPushButton();
  catalogPathSearch->setPixmap( _mainWindow->getPixmap( "kde_fileopen_16.png" ) );
  catalogPathSearch->setMinimumWidth( catalogPathSearch->sizeHint().width() + 5 );
  catalogPathSearch->setMinimumHeight( catalogPathSearch->sizeHint().height() + 5 );

  connect( catalogPathSearch, SIGNAL(clicked()),
           SLOT(slotSearchDefaultWaypoint()) );

  QGridLayout* catPathLayout = new QGridLayout();
  catPathLayout->setSpacing( 10 );
  catPathLayout->addWidget( catalogPathE, 0, 0 );
  catPathLayout->addWidget( catalogPathSearch, 0, 1 );
  catPathLayout->setColStretch( 0, 10 );

  catGroupLayout->addLayout( catPathLayout );
  catGroup->setLayout( catGroupLayout );

  QPushButton* defaultCatalog = new QPushButton( tr("Default") );
  defaultCatalog->setMaximumWidth(defaultCatalog->sizeHint().width() + 10);
  defaultCatalog->setMinimumHeight(defaultCatalog->sizeHint().height() + 2);

  connect( defaultCatalog, SIGNAL(clicked()), SLOT(slotDefaultWaypoint()) );

  //----------------------------------------------------------------------------
  QVBoxLayout* waypointPageLayout = new QVBoxLayout;
  waypointPageLayout->addWidget( catGroup );
  waypointPageLayout->addStretch( 10 );
  waypointPageLayout->addWidget( defaultCatalog, 0, Qt::AlignLeft );

  waypointPage->setLayout( waypointPageLayout );

  slotSelectDefaultCatalog( catalogType );
}

void KFLogConfig::slotDefaultWaypoint()
{
  catalogPathE->setText(QString::null);
  slotSelectDefaultCatalog(LastUsed);
}

void KFLogConfig::slotSelectDefaultCatalog( int item )
{
  QRadioButton *button = static_cast<QRadioButton *> (waypointButtonGroup->button( item ));

  if( button != static_cast<QRadioButton *>(0) )
    {
      button->setChecked( true );
    }

  catalogPathE->setEnabled( item == Specific );
  catalogPathSearch->setEnabled( item == Specific );
}

void KFLogConfig::slotSearchDefaultWaypoint()
{
  QString fileName = QFileDialog::getOpenFileName(
                         this,
                         tr("Select a waypoint catalog"),
                         catalogPathE->text(),
                         tr("KFLog Catalogs (*.kflogwp *.KFLOGWP)"));

  if( ! fileName.isEmpty() )
    {
      catalogPathE->setText(fileName);
    }
}

void KFLogConfig::slotSelectFlightTypeLeftTurnColor()
{
  flightTypeLeftTurnColor = QColorDialog::getColor(flightTypeLeftTurnColor, this);
  QPixmap buttonPixmap(82,14);
  buttonPixmap.fill(flightTypeLeftTurnColor);
  flightTypeLeftTurnColorButton->setIcon(buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeRightTurnColor()
{
  flightTypeRightTurnColor = QColorDialog::getColor(flightTypeRightTurnColor, this);
  QPixmap buttonPixmap(82,14);
  buttonPixmap.fill(flightTypeRightTurnColor);
  flightTypeRightTurnColorButton->setIcon(buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeMixedTurnColor()
{
  flightTypeMixedTurnColor = QColorDialog::getColor(flightTypeMixedTurnColor, this);
  QPixmap buttonPixmap(82,14);
  buttonPixmap.fill(flightTypeMixedTurnColor);
  flightTypeMixedTurnColorButton->setIcon(buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeStraightColor()
{
  flightTypeStraightColor = QColorDialog::getColor(flightTypeStraightColor, this);
  QPixmap buttonPixmap(82,14);
  buttonPixmap.fill(flightTypeStraightColor);
  flightTypeStraightColorButton->setIcon(buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeSolidColor()
{
  flightTypeSolidColor = QColorDialog::getColor(flightTypeSolidColor, this);
  QPixmap buttonPixmap(82,14);
  buttonPixmap.fill(flightTypeSolidColor);
  flightTypeSolidColorButton->setIcon(buttonPixmap);
}

void KFLogConfig::slotSelectFlightTypeEngineNoiseColor()
{
  flightTypeEngineNoiseColor = QColorDialog::getColor(flightTypeEngineNoiseColor, this);
  QPixmap buttonPixmap(82,14);
  buttonPixmap.fill(flightTypeEngineNoiseColor);
  flightTypeEngineNoiseColorButton->setIcon(buttonPixmap);
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
