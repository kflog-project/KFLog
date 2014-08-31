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
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "configmapelement.h"
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
  currentProjType( ProjectionBase::Unknown )
{
  setObjectName( "KFLogConfig" );
  setWindowTitle( tr("KFLog Configuration") );
  setAttribute( Qt::WA_DeleteOnClose );
  setModal( true );
  setSizeGripEnabled( true );

  configLayout = new QGridLayout(this);

  setupTree = new QTreeWidget( this );
  setupTree->setRootIsDecorated( false );
  setupTree->setItemsExpandable( false );
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
  configLayout->setColumnStretch(2, 10);

  connect( saveButton, SIGNAL(clicked()), this, SLOT(slotOk()) );
  connect( cancelButton, SIGNAL(clicked()), this, SLOT(close()) );

  __addPersonalTab();
  __addPathTab();
  __addScaleTab();
  __addMapTab();
  __addFlightTab();
  __addProjectionTab();
  __addAirfieldTab();
  __addAirspaceTab();
  __addWaypointTab();
  __addUnitTab();

  setupTree->sortByColumn ( 0, Qt::AscendingOrder );
  setupTree->resizeColumnToContents( 0 );

  personalPage->setVisible( true );
  activePage = personalPage;

  restoreGeometry( _settings.value("/KFLogConfig/Geometry").toByteArray() );
}

KFLogConfig::~KFLogConfig()
{
  _settings.setValue( "/KFLogConfig/Geometry", saveGeometry() );
}

void KFLogConfig::slotPageClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column );

  QString itemText = item->data( 0, Qt::UserRole ).toString();

  if( itemText == "Airfields" )
    {
      activePage->setVisible( false );
      airfieldPage->setVisible( true );
      activePage = airfieldPage;
    }
  else if( itemText == "Airspaces" )
    {
      // Because the airspace directory can be changed in the meantime, we
      // reload this page beore showing.
      __loadAirspaceFilesIntoTable();
      activePage->setVisible( false );
      airspacePage->setVisible( true );
      activePage = airspacePage;
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
      personalPage->setVisible( true );
      activePage = personalPage;
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
  else if( itemText == "Units" )
    {
      activePage->setVisible( false );
      unitPage->setVisible( true );
      activePage = unitPage;
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
  qDebug() << "KFLogConfig::slotOk()";

  // First check, if the Welt2000 and openAIP countries are valid
  QString input = countriesOpenAipAS->text().trimmed();

  if( input.isEmpty() == false && __checkOpenAipAirspaceInput( input ) == false )
    {
      return;
    }

  input = filterWelt2000->text().trimmed();

  if( input.isEmpty() == false && __checkWelt2000Input( input ) == false )
    {
      return;
    }

  setVisible( false );

  // Save current projection
  int usedMapProjection = currentProjType;

  slotSelectProjection( ProjectionBase::Unknown );

  // check for home latitude change
  bool homeLatitudeChanged =
      (homeLatE->KFLogDegree() != _settings.value("/Homesite/Latitude", HOME_DEFAULT_LAT).toInt());

  _settings.setValue( "/GeneralOptions/Version", "4.1" );

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

  _settings.setValue( "/Homesite/Name", homeNameE->text() );
  _settings.setValue( "/Homesite/Country", homeCountryE->text().toUpper() );
  _settings.setValue( "/Homesite/Latitude", homeLatE->KFLogDegree() );
  _settings.setValue( "/Homesite/Longitude", homeLonE->KFLogDegree() );
  _settings.setValue( "/MapData/ProjectionType", projectionSelect->currentIndex() );

  _settings.setValue( "/Welt2000/CountryFilter", filterWelt2000->text().trimmed().toUpper() );
  _settings.setValue( "/Welt2000/HomeRadius", homeRadiusWelt2000->value() );
  _settings.setValue( "/Welt2000/LoadOutlandings", readOlWelt2000->isChecked() );

  _settings.setValue( "/FlightColor/LeftTurn", flightTypeLeftTurnColor.name() );
  _settings.setValue( "/FlightColor/RightTurn", flightTypeRightTurnColor.name() );
  _settings.setValue( "/FlightColor/MixedTurn", flightTypeMixedTurnColor.name() );
  _settings.setValue( "/FlightColor/Straight", flightTypeStraightColor.name() );
  _settings.setValue( "/FlightColor/Solid", flightTypeSolidColor.name() );
  _settings.setValue( "/FlightColor/EngineNoise", flightTypeEngineNoiseColor.name() );

  _settings.setValue( "/FlightPathLine/Altitude", altitudePenWidth->value() );
  _settings.setValue( "/FlightPathLine/Cycling", cyclingPenWidth->value() );
  _settings.setValue( "/FlightPathLine/Speed", speedPenWidth->value() );
  _settings.setValue( "/FlightPathLine/Vario", varioPenWidth->value() );
  _settings.setValue( "/FlightPathLine/Solid", solidPenWidth->value() );
  _settings.setValue( "/FlightPathLine/Engine", enginePenWidth->value() );

  _settings.setValue( "/LambertProjection/Parallel1", lambertV1 );
  _settings.setValue( "/LambertProjection/Parallel2", lambertV2 );
  _settings.setValue( "/LambertProjection/Origin", lambertOrigin );

  _settings.setValue( "/CylindricalProjection/Parallel", cylinPar );

  _settings.setValue( "/PersonalData/PreName", preNameE->text() );
  _settings.setValue( "/PersonalData/SurName", surNameE->text() );
  _settings.setValue( "/PersonalData/Birthday", dateOfBirthE->text() );

  _settings.setValue( "/Waypoints/DefaultWaypointCatalog",
                      waypointButtonGroup->id(waypointButtonGroup->checkedButton()) );
  _settings.setValue( "/Waypoints/DefaultCatalogName", catalogPathE->text() );

  _settings.setValue( "/Airspace/Countries",
                      countriesOpenAipAS->text().trimmed().toLower() );

  // Save units
  int altUnit  = unitAltitude->itemData( unitAltitude->currentIndex() ).toInt();
  int distUnit = unitDistance->itemData( unitDistance->currentIndex() ).toInt();
  int posUnit  = unitPosition->itemData( unitPosition->currentIndex() ).toInt();

  _settings.setValue( "/Units/Altitude", altUnit );
  _settings.setValue( "/Units/Distance", distUnit );
  _settings.setValue( "/Units/Position", posUnit );

  // Set units to be used in the related classes.
  Altitude::setUnit( static_cast<enum Altitude::altitudeUnit>(altUnit) );
  Distance::setUnit( static_cast<enum Distance::distanceUnit>(distUnit) );
  WGSPoint::setFormat( static_cast<enum WGSPoint::Format>(posUnit) );

  // If Home latitude was changed and map projection is cylinder we take over
  // the new home latitude as parallel for the cylinder map projection.
  if( homeLatitudeChanged == true && usedMapProjection == ProjectionBase::Cylindric )
    {
      cylinPar = homeLatE->KFLogDegree();
      _settings.setValue( "/CylindricalProjection/Parallel", cylinPar );
    }

  QTableWidgetItem* asItem = asFileTable->item( 0, 0 );

  if( asItem != 0 )
    {
      // Save Airspace files to be loaded, if airspace tabulator was opened
      // by the user.
      QStringList files;

      if( asItem->checkState() == Qt::Checked )
        {
          // All files are selected.
          files << "All";
        }
      else
        {
          // Store only checked file items.
          for( int i = 1; i < asFileTable->rowCount(); i++ )
            {
              QTableWidgetItem* item = asFileTable->item( i, 0 );

              if( item->checkState() == Qt::Checked )
                {
                  files << item->text();
                }
            }
        }

      QStringList oldFiles = _settings.value( "/Airspace/FileList", QStringList(QString("All"))).toStringList();

      // save the new file list
      _settings.setValue( "/Airspace/FileList", files );

      // Check, if file list has been modified
      if( oldFiles.size() != files.size() )
        {
          // List size is different, emit signal.
          emit airspaceFileListChanged();
        }
      else
        {
          // The list size is equal, we have to check every single list element.
          // Note that the lists are always sorted.
          for( int i = 0; i < files.size(); i++ )
            {
              if( files.at(i) != oldFiles.at(i) )
                {
                  // File names are different, emit signal.
                  emit airspaceFileListChanged();
                  break;
                }
            }
        }
    }

  // Configuration subwidgets shall save their configuration.
  emit saveConfig();

  emit scaleChanged((int)lLimitN->value(), (int)uLimitN->value());

  // Check, if Welt2000 must be updated
  if( homeRadiusWelt2000Value !=  homeRadiusWelt2000->value() ||
      filterWelt2000Text != filterWelt2000->text() ||
      readOlWelt2000Value != readOlWelt2000->isChecked () )
    {
      emit reloadWelt2000Data();
    }

  emit configOk();
  accept();
}

void KFLogConfig::slotTextEditedCountry( const QString& text )
{
  // Change edited text to upper cases
  homeCountryE->setText( text.toUpper() );
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

void KFLogConfig::slotSelectProjection( int index )
{
  switch( currentProjType )
    {
      case ProjectionBase::Cylindric:

        cylinPar = firstParallel->KFLogDegree();
        break;

      case ProjectionBase::Lambert:

        lambertV1     = firstParallel->KFLogDegree();
        lambertV2     = secondParallel->KFLogDegree();
        lambertOrigin = originLongitude->KFLogDegree();
        break;
    }

  switch( index )
    {
      case ProjectionBase::Cylindric:

        secondParallel->setEnabled(false);
        originLongitude->setEnabled(false);
        firstParallel->setKFLogDegree(cylinPar);
        break;

      case ProjectionBase::Lambert:

        secondParallel->setEnabled(true);
        originLongitude->setEnabled(true);
        firstParallel->setKFLogDegree(lambertV1);
        secondParallel->setKFLogDegree(lambertV2);
        originLongitude->setKFLogDegree(lambertOrigin);
        break;
    }

  currentProjType = index;
}

void KFLogConfig::slotShowLowerLimit( int value )
{
  lLimitN->display( __setScaleValue( value ) );
}

void KFLogConfig::slotShowUpperLimit( int value )
{
  uLimitN->display( __setScaleValue( value ) );
}

void KFLogConfig::slotShowSwitchScale( int value )
{
  switchScaleN->display( __setScaleValue( value ) );
}

void KFLogConfig::slotShowWpLabel( int value )
{
  wpLabelN->display( __setScaleValue( value ) );
}

void KFLogConfig::slotShowReduceScaleA( int value )
{
  reduce1N->display( __setScaleValue( value ) );
}

void KFLogConfig::slotShowReduceScaleB( int value )
{
  reduce2N->display( __setScaleValue( value ) );
}

void KFLogConfig::slotShowReduceScaleC( int value )
{
  reduce3N->display( __setScaleValue( value ) );
}

void KFLogConfig::slotDefaultProjection()
{
  lambertV1 = 32400000;
  lambertV2 = 30000000;
  lambertOrigin = 0;

  cylinPar = 27000000;

  currentProjType = ProjectionBase::Unknown;

  projectionSelect->setCurrentIndex( ProjectionBase::Cylindric );
  slotSelectProjection( ProjectionBase::Cylindric );
}

void KFLogConfig::slotDefaultScale()
{
  lLimit->setValue( __getScaleValue( BORDER_L ) );
  lLimitN->display( BORDER_L );
  uLimit->setValue( __getScaleValue( BORDER_U ) );
  uLimitN->display( BORDER_U );
  switchScale->setValue( __getScaleValue( BORDER_S ) );
  switchScaleN->display( BORDER_S );
  wpLabel->setValue( __getScaleValue( WPLABEL ) );
  wpLabelN->display( WPLABEL );
  reduce1->setValue( __getScaleValue( BORDER_1 ) );
  reduce1N->display( BORDER_1 );
  reduce2->setValue( __getScaleValue( BORDER_2 ) );
  reduce2N->display( BORDER_2 );
  reduce3->setValue( __getScaleValue( BORDER_3 ) );
  reduce3N->display( BORDER_3 );
}

void KFLogConfig::slotDefaultPath()
{
  igcPathE->setText( _mainWindow->getApplicationDataDirectory() );
  taskPathE->setText( _mainWindow->getApplicationDataDirectory() );
  waypointPathE->setText( _mainWindow->getApplicationDataDirectory() );
  mapPathE->setText( _globalMapContents->getMapRootDirectory() );
}

void KFLogConfig::slotDefaultFlightPathLines()
{
  altitudePenWidth->setValue( FlightPathLineWidth );
  cyclingPenWidth->setValue( FlightPathLineWidth );
  speedPenWidth->setValue( FlightPathLineWidth );
  varioPenWidth->setValue( FlightPathLineWidth );
  solidPenWidth->setValue( FlightPathLineWidth );
  enginePenWidth->setValue( FlightPathLineWidth );
}

void KFLogConfig::slotDefaultFlightPathColors()
{
  for( int i=0; i < 6; i++ )
    {
      // Reset button color to default
      QPixmap buttonPixmap( 82, 14 );
      buttonPixmap.fill( ftcColorArrayDefault[i] );
      (*ftcButtonArray[i])->setIcon( buttonPixmap );
      (*ftcButtonArray[i])->setIconSize( buttonPixmap.size() );

      *ftcColorArray[i] = ftcColorArrayDefault[i];
    }
}

void KFLogConfig::slotSelectDrawElement( int index )
{
  int data = elementSelect->itemData( index ).toInt();

  if( data != KFLogConfig::Separator )
    {
      configDrawWidget->slotSelectElement( data );
    }
}

void KFLogConfig::slotSelectPrintElement( int index )
{
  int data = elementSelect->itemData( index ).toInt();

  if( data != KFLogConfig::Separator )
    {
      configPrintWidget->slotSelectElement( data );
    }
}

void KFLogConfig::__addMapTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Map Elements") );
  item->setData( 0, Qt::UserRole, "Map Elements" );
  item->setIcon( 0, _mainWindow->getPixmap("kflog_32.png") );
  setupTree->addTopLevelItem( item );

  mapPage = new QWidget(this);
  mapPage->setObjectName( "MapPage" );
  mapPage->setVisible( false );

  configLayout->addWidget( mapPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* elementGroupBox = new QGroupBox( tr("Visible Map Elements") );

  elementSelect = new QComboBox;
//  elementSelect->setMaximumWidth(300);
  elementSelect->addItem( tr( "Airspace A" ), KFLogConfig::AirA );
  elementSelect->addItem( tr( "Airspace B" ), KFLogConfig::AirB );
  elementSelect->addItem( tr( "Airspace C" ), KFLogConfig::AirC );
  elementSelect->addItem( tr( "Airspace D" ), KFLogConfig::AirD );
  elementSelect->addItem( tr( "Airspace E (low)" ), KFLogConfig::AirElow );
  elementSelect->addItem( tr( "Airspace E" ), KFLogConfig::AirE );
  elementSelect->addItem( tr( "Airspace F" ), KFLogConfig::AirF );
  elementSelect->addItem( tr( "Airspace FIR" ), KFLogConfig::AirFir );
  elementSelect->addItem( tr( "Control C" ), KFLogConfig::ControlC );
  elementSelect->addItem( tr( "Control D" ), KFLogConfig::ControlD );
  elementSelect->addItem( tr( "Danger/Prohibited" ), KFLogConfig::Danger );
  elementSelect->addItem( tr( "Glider Sector" ), KFLogConfig::GliderSector );
  elementSelect->addItem( tr( "Low Flight Area" ), KFLogConfig::LowFlight );
  elementSelect->addItem( tr( "Restricted" ), KFLogConfig::Restricted );
  elementSelect->addItem( tr( "TMZ" ), KFLogConfig::Tmz );
  elementSelect->addItem( tr( "WaveWindow" ), KFLogConfig::WaveWindow );
  elementSelect->addItem( "-------------", KFLogConfig::Separator );

  // sort order ?
  elementSelect->addItem( tr( "Aerial Cable" ), KFLogConfig::Aerial_Cable );
  elementSelect->addItem( tr( "Canal" ), KFLogConfig::Canal );
  elementSelect->addItem( tr( "City" ), KFLogConfig::City );
  elementSelect->addItem( tr( "Forest" ), KFLogConfig::Forest );
  elementSelect->addItem( tr( "Motorway" ), KFLogConfig::Motorway );
  elementSelect->addItem( tr( "Road" ), KFLogConfig::Road );
  elementSelect->addItem( tr( "Railway" ), KFLogConfig::Railway );
  elementSelect->addItem( tr( "Railway Double" ), KFLogConfig::Railway_D );
  elementSelect->addItem( tr( "River / Lake" ), KFLogConfig::River );
  elementSelect->addItem( tr( "Temporarily River / Lake" ), KFLogConfig::River_T );
  elementSelect->addItem( tr( "Glacier" ), KFLogConfig::Glacier );
  elementSelect->addItem( tr( "Pack Ice" ), KFLogConfig::PackIce );
  elementSelect->addItem( "-------------", KFLogConfig::Separator );
  elementSelect->addItem( tr( "FAI Area <500 km" ), KFLogConfig::FAIAreaLow500 );
  elementSelect->addItem( tr( "FAI Area >500 km" ), KFLogConfig::FAIAreaHigh500 );
  elementSelect->addItem( tr( "Trail" ), KFLogConfig::Trail );

  configDrawWidget  = new ConfigMapElement( this, true );
  configPrintWidget = new ConfigMapElement( this, false );

  configDrawWidget->setObjectName( "configDrawWidget" );
  configPrintWidget->setObjectName( "configPrintWidget" );

  QTabWidget* tabView = new QTabWidget;
  tabView->setObjectName( "tabView" );
  tabView->addTab( configDrawWidget, tr("Display") );
  tabView->addTab( configPrintWidget, tr("Print") );
  tabView->setTabEnabled( 0, true );

  QGridLayout* vbox = new QGridLayout;
  vbox->setMargin( 10 );
  vbox->addWidget( elementSelect, 0, 0 );

  vbox->addWidget( tabView, 1, 0, 1, 2 );

  vbox->setColumnStretch( 1, 10 );

  elementGroupBox->setLayout( vbox );

  QPushButton* defaultElements = new QPushButton( tr("Default") );
  defaultElements->setMaximumWidth(defaultElements->sizeHint().width() + 10);
  defaultElements->setMinimumHeight(defaultElements->sizeHint().height() + 2);

  QVBoxLayout* vboxAll = new QVBoxLayout();
  vboxAll->addWidget( elementGroupBox );
  vboxAll->addStretch( 10 );
  vboxAll->addWidget( defaultElements, Qt::AlignLeft );

  mapPage->setLayout( vboxAll );

  connect( defaultElements, SIGNAL(clicked()),
           configDrawWidget, SLOT(slotDefaultElements()) );

  connect( defaultElements, SIGNAL(clicked()),
           configPrintWidget, SLOT(slotDefaultElements()));

  connect( elementSelect, SIGNAL( currentIndexChanged(int)),
           this, SLOT(slotSelectDrawElement(int)));

  connect( elementSelect, SIGNAL( currentIndexChanged(int)),
           this, SLOT(slotSelectPrintElement(int)));

  connect( this, SIGNAL(saveConfig()), configDrawWidget, SLOT(slotOk()) );
  connect( this, SIGNAL(saveConfig()), configPrintWidget, SLOT(slotOk()) );

  configDrawWidget->slotSelectElement( KFLogConfig::AirA );
  configPrintWidget->slotSelectElement( KFLogConfig::AirA );
}

void KFLogConfig::__addFlightTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Flight Display") );
  item->setData( 0, Qt::UserRole, "Flight Display" );
  item->setIcon( 0, _mainWindow->getPixmap("flightpath_32.png") );
  setupTree->addTopLevelItem( item );

  flightPage = new QWidget(this);
  flightPage->setObjectName( "FlightPage" );
  flightPage->setVisible( false );

  configLayout->addWidget( flightPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  // initialize button array
  ftcButtonArray[0] = &flightTypeLeftTurnColorButton;
  ftcButtonArray[1] = &flightTypeRightTurnColorButton;
  ftcButtonArray[2] = &flightTypeMixedTurnColorButton;
  ftcButtonArray[3] = &flightTypeStraightColorButton;
  ftcButtonArray[4] = &flightTypeSolidColorButton;
  ftcButtonArray[5] = &flightTypeEngineNoiseColorButton;

  // initialize related button default color array
  ftcColorArrayDefault[0] = FlightTypeLeftTurnColor;
  ftcColorArrayDefault[1] = FlightTypeRightTurnColor;
  ftcColorArrayDefault[2] = FlightTypeMixedTurnColor;
  ftcColorArrayDefault[3] = FlightTypeStraightColor;
  ftcColorArrayDefault[4] = FlightTypeSolidColor;
  ftcColorArrayDefault[5] = FlightTypeEngineNoiseColor;

  // initialize related button color array
  ftcColorArray[0] = &flightTypeLeftTurnColor;
  ftcColorArray[1] = &flightTypeRightTurnColor;
  ftcColorArray[2] = &flightTypeMixedTurnColor;
  ftcColorArray[3] = &flightTypeStraightColor;
  ftcColorArray[4] = &flightTypeSolidColor;
  ftcColorArray[5] = &flightTypeEngineNoiseColor;

  // Load colors
  flightTypeLeftTurnColor    = _settings.value( "/FlightColor/LeftTurn", ftcColorArrayDefault[0].name() ).value<QColor>();
  flightTypeRightTurnColor   = _settings.value( "/FlightColor/RightTurn", ftcColorArrayDefault[1].name() ).value<QColor>();
  flightTypeMixedTurnColor   = _settings.value( "/FlightColor/MixedTurn", ftcColorArrayDefault[2].name() ).value<QColor>();
  flightTypeStraightColor    = _settings.value( "/FlightColor/Straight", ftcColorArrayDefault[3].name() ).value<QColor>();
  flightTypeSolidColor       = _settings.value( "/FlightColor/Solid", ftcColorArrayDefault[4].name() ).value<QColor>();
  flightTypeEngineNoiseColor = _settings.value( "/FlightColor/EngineNoise", ftcColorArrayDefault[5].name() ).value<QColor>();

  //----------------------------------------------------------------------------
  QGroupBox* flightPathLineGroup = new QGroupBox( tr("Flight Path Line Width") );
  flightPathLineGroup->setToolTip( tr("Set pen line width in pixel.") );

  altitudePenWidth = new QSpinBox();
  altitudePenWidth->setRange( 1, 9 );
  altitudePenWidth->setSingleStep( 1 );
  altitudePenWidth->setButtonSymbols( QSpinBox::PlusMinus );
  altitudePenWidth->setValue( _settings.value("/FlightPathLine/Altitude", FlightPathLineWidth).toInt() );

  cyclingPenWidth = new QSpinBox();
  cyclingPenWidth->setRange( 1, 9 );
  cyclingPenWidth->setSingleStep( 1 );
  cyclingPenWidth->setButtonSymbols( QSpinBox::PlusMinus );
  cyclingPenWidth->setValue( _settings.value("/FlightPathLine/Cycling", FlightPathLineWidth).toInt() );

  speedPenWidth = new QSpinBox();
  speedPenWidth->setRange( 1, 9 );
  speedPenWidth->setSingleStep( 1 );
  speedPenWidth->setButtonSymbols( QSpinBox::PlusMinus );
  speedPenWidth->setValue( _settings.value("/FlightPathLine/Speed", FlightPathLineWidth).toInt() );

  varioPenWidth = new QSpinBox();
  varioPenWidth->setRange( 1, 9 );
  varioPenWidth->setSingleStep( 1 );
  varioPenWidth->setButtonSymbols( QSpinBox::PlusMinus );
  varioPenWidth->setValue( _settings.value("/FlightPathLine/Vario", FlightPathLineWidth).toInt() );

  solidPenWidth = new QSpinBox();
  solidPenWidth->setRange( 1, 9 );
  solidPenWidth->setSingleStep( 1 );
  solidPenWidth->setButtonSymbols( QSpinBox::PlusMinus );
  solidPenWidth->setValue( _settings.value("/FlightPathLine/Solid", FlightPathLineWidth).toInt() );

  enginePenWidth = new QSpinBox();
  enginePenWidth->setRange( 1, 9 );
  enginePenWidth->setSingleStep( 1 );
  enginePenWidth->setButtonSymbols( QSpinBox::PlusMinus );
  enginePenWidth->setValue( _settings.value("/FlightPathLine/Engine", FlightPathLineWidth).toInt() );

  QPushButton* defaultFpl = new QPushButton( tr( "Default" ) );
  defaultFpl->setMaximumWidth( defaultFpl->sizeHint().width() + 10 );
  defaultFpl->setMinimumHeight( defaultFpl->sizeHint().height() + 2 );

  connect( defaultFpl, SIGNAL(clicked()), SLOT(slotDefaultFlightPathLines()) );

  QGridLayout* fplLayout = new QGridLayout();
  fplLayout->setSpacing(10);
  int row = 0;

  fplLayout->addWidget( new QLabel( tr("Altitude") + ":" ), row , 0, Qt::AlignRight );
  fplLayout->addWidget( altitudePenWidth, row, 1 );

  fplLayout->addWidget( new QLabel( tr("Cycling") + ":" ), row , 2, Qt::AlignRight );
  fplLayout->addWidget( cyclingPenWidth, row, 3 );
  row++;

  fplLayout->addWidget( new QLabel( tr("Speed") + ":" ), row , 0, Qt::AlignRight );
  fplLayout->addWidget( speedPenWidth, row, 1 );

  fplLayout->addWidget( new QLabel( tr("Vario") + ":" ), row , 2, Qt::AlignRight );
  fplLayout->addWidget( varioPenWidth, row, 3 );
  row++;

  fplLayout->addWidget( new QLabel( tr("Solid") + ":" ), row , 0, Qt::AlignRight );
  fplLayout->addWidget( solidPenWidth, row, 1 );

  fplLayout->addWidget( new QLabel( tr("Engine") + ":" ), row , 2, Qt::AlignRight );
  fplLayout->addWidget( enginePenWidth, row, 3 );
  row++;

  fplLayout->setRowMinimumHeight( row, 10 );
  row++;

  fplLayout->addWidget( defaultFpl, row, 0, Qt::AlignLeft );
  row++;

  fplLayout->setColumnStretch( 4, 10 );

  flightPathLineGroup->setLayout( fplLayout );

  //----------------------------------------------------------------------------
  QGroupBox* flightPathColorGroup = new QGroupBox( tr("Flight Path Colors") );

  QGridLayout *fpCLayout = new QGridLayout;

  fpCLayout->setSpacing(10);

  QPixmap buttonPixmap( 82, 14);

  //-----
  buttonPixmap.fill(flightTypeLeftTurnColor);
  flightTypeLeftTurnColorButton = new QPushButton();
  flightTypeLeftTurnColorButton->setIcon( buttonPixmap );
  flightTypeLeftTurnColorButton->setIconSize( buttonPixmap.size() );
  flightTypeLeftTurnColorButton->setFixedHeight(24);
  flightTypeLeftTurnColorButton->setFixedWidth(92);

  fpCLayout->addWidget( new QLabel(tr("Left turn") + ":"), 0, 0, Qt::AlignRight );
  fpCLayout->addWidget( flightTypeLeftTurnColorButton, 0, 1 );

  //-----
  buttonPixmap.fill(flightTypeRightTurnColor);
  flightTypeRightTurnColorButton = new QPushButton();
  flightTypeRightTurnColorButton->setIcon(buttonPixmap);
  flightTypeRightTurnColorButton->setIconSize( buttonPixmap.size() );
  flightTypeRightTurnColorButton->setFixedHeight(24);
  flightTypeRightTurnColorButton->setFixedWidth(92);

  fpCLayout->addWidget( new QLabel(tr("Right turn") + ":"), 1, 0, Qt::AlignRight );
  fpCLayout->addWidget( flightTypeRightTurnColorButton, 1, 1 );

  //-----
  buttonPixmap.fill(flightTypeMixedTurnColor);
  flightTypeMixedTurnColorButton = new QPushButton();
  flightTypeMixedTurnColorButton->setIcon(buttonPixmap);
  flightTypeMixedTurnColorButton->setIconSize( buttonPixmap.size() );
  flightTypeMixedTurnColorButton->setFixedHeight(24);
  flightTypeMixedTurnColorButton->setFixedWidth(92);

  fpCLayout->addWidget( new QLabel(tr("Mixed turn") + ":"), 2, 0, Qt::AlignRight );
  fpCLayout->addWidget( flightTypeMixedTurnColorButton, 2, 1 );

  //-----
  buttonPixmap.fill(flightTypeStraightColor);
  flightTypeStraightColorButton = new QPushButton();
  flightTypeStraightColorButton->setIcon(buttonPixmap);
  flightTypeStraightColorButton->setIconSize( buttonPixmap.size() );
  flightTypeStraightColorButton->setFixedHeight(24);
  flightTypeStraightColorButton->setFixedWidth(92);

  fpCLayout->addWidget( new QLabel(tr("Straight") + ":"), 0, 2, Qt::AlignRight );
  fpCLayout->addWidget( flightTypeStraightColorButton, 0, 3 );

  //-----
  buttonPixmap.fill(flightTypeSolidColor);
  flightTypeSolidColorButton = new QPushButton();
  flightTypeSolidColorButton->setIcon(buttonPixmap);
  flightTypeSolidColorButton->setIconSize( buttonPixmap.size() );
  flightTypeSolidColorButton->setFixedHeight(24);
  flightTypeSolidColorButton->setFixedWidth(92);

  fpCLayout->addWidget( new QLabel(tr("Solid") + ":"), 1, 2, Qt::AlignRight );
  fpCLayout->addWidget( flightTypeSolidColorButton, 1, 3 );

  //-----
  buttonPixmap.fill( flightTypeEngineNoiseColor );
  flightTypeEngineNoiseColorButton = new QPushButton();
  flightTypeEngineNoiseColorButton->setIcon( buttonPixmap );
  flightTypeEngineNoiseColorButton->setIconSize( buttonPixmap.size() );
  flightTypeEngineNoiseColorButton->setFixedHeight( 24 );
  flightTypeEngineNoiseColorButton->setFixedWidth( 92 );

  fpCLayout->addWidget( new QLabel(tr("Engine noise") + ":"), 2, 2, Qt::AlignRight );
  fpCLayout->addWidget( flightTypeEngineNoiseColorButton, 2, 3 );
  fpCLayout->setRowMinimumHeight( 3, 10 );

  QPushButton* defaultFpc = new QPushButton( tr( "Default" ) );
  defaultFpc->setMaximumWidth( defaultFpc->sizeHint().width() + 10 );
  defaultFpc->setMinimumHeight( defaultFpc->sizeHint().height() + 2 );

  connect( defaultFpc, SIGNAL(clicked()), SLOT(slotDefaultFlightPathColors()) );

  fpCLayout->addWidget( defaultFpc, 4, 0, Qt::AlignLeft );

  flightPathColorGroup->setLayout( fpCLayout );

  QButtonGroup* fpcButtonGroup = new QButtonGroup( flightPage );
  fpcButtonGroup->addButton( flightTypeLeftTurnColorButton, 0 );
  fpcButtonGroup->addButton( flightTypeRightTurnColorButton, 1 );
  fpcButtonGroup->addButton( flightTypeMixedTurnColorButton, 2 );
  fpcButtonGroup->addButton( flightTypeStraightColorButton, 3 );
  fpcButtonGroup->addButton( flightTypeSolidColorButton, 4 );
  fpcButtonGroup->addButton( flightTypeEngineNoiseColorButton, 5 );

  connect( fpcButtonGroup, SIGNAL(buttonClicked (int)),
           this, SLOT(slotSelectFlightTypeColor(int)) );

  //----------------------------------------------------------------------------
  QVBoxLayout* flightPagePageLayout = new QVBoxLayout;
  flightPagePageLayout->addWidget( flightPathLineGroup );
  flightPagePageLayout->addWidget( flightPathColorGroup );
  flightPagePageLayout->addStretch( 10 );

  flightPage->setLayout( flightPagePageLayout );
}

void KFLogConfig::__addProjectionTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Map Projection") );
  item->setData( 0, Qt::UserRole, "Map Projection" );
  item->setIcon( 0, _mainWindow->getPixmap("projection_32.png") );
  setupTree->addTopLevelItem( item );

  projPage = new QWidget(this);
  projPage->setObjectName( "MapProjectionPage" );
  projPage->setVisible( false );

  configLayout->addWidget( projPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* projTypeGroup = new QGroupBox( tr("Type of Projection") );

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
  QGroupBox* projConfGroup = new QGroupBox( tr( "Setup Projection" ) );

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
  int projIndex = _settings.value("/MapData/ProjectionType", ProjectionBase::Cylindric).toInt();

  projectionSelect->setCurrentIndex( projIndex );
  slotSelectProjection( projIndex );
}

void KFLogConfig::__addScaleTab()
{
  int ll = _settings.value( "/Scale/LowerLimit", BORDER_L ).toInt();
  int ul = _settings.value( "/Scale/UpperLimit", BORDER_U ).toInt();
  int sw = _settings.value( "/Scale/SwitchScale", BORDER_S ).toInt();
  int wl = _settings.value( "/Scale/WaypointLabel", WPLABEL ).toInt();
  int b1 = _settings.value( "/Scale/Border1", BORDER_1 ).toInt();
  int b2 = _settings.value( "/Scale/Border2", BORDER_2 ).toInt();
  int b3 = _settings.value( "/Scale/Border3", BORDER_3 ).toInt();

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr( "Map Scales" ) );
  item->setData( 0, Qt::UserRole, "Map Scales" );
  item->setIcon( 0, _mainWindow->getPixmap( "kde_viewmag_32.png" ) );
  setupTree->addTopLevelItem( item );

  scalePage = new QWidget( this );
  scalePage->setObjectName( "MapScalePage" );
  scalePage->setVisible( false );

  configLayout->addWidget( scalePage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* scaleRangeGroup = new QGroupBox( tr( "Scale Range" ) );

  QLabel* lLimitText = new QLabel( tr( "Lower limit" ) + ":" );

  lLimit = new QSlider();
  lLimit->setMinimum( 1 );
  lLimit->setMaximum( 105 );
  lLimit->setPageStep( 1 );
  lLimit->setOrientation( Qt::Horizontal );
  lLimit->setSingleStep( 1 );
  lLimit->setMinimumHeight( lLimit->sizeHint().height() + 5 );
  lLimit->setMaximumHeight( lLimit->sizeHint().height() + 20 );
  lLimit->setMinimumWidth( 200 );

  lLimitN = new QLCDNumber( 5 );
  lLimitN->setMinimumWidth( lLimitN->sizeHint().width() + 10 );
  lLimitN->setBackgroundRole( QPalette::Light );
  lLimitN->setAutoFillBackground( true );

  lLimit->setValue( __getScaleValue( ll ) );
  lLimitN->display( ll );

  QLabel* uLimitText = new QLabel( tr( "Upper limit" ) + ":" );

  uLimit = new QSlider();
  uLimit->setMinimum( 2 );
  uLimit->setMaximum( 105 );
  uLimit->setPageStep( 1 );
  uLimit->setOrientation( Qt::Horizontal );
  uLimit->setSingleStep( 1 );

  uLimitN = new QLCDNumber( 5 );
  uLimitN->setMinimumWidth( uLimitN->sizeHint().width() + 10 );
  uLimitN->setBackgroundRole( QPalette::Light );
  uLimitN->setAutoFillBackground( true );

  uLimit->setValue( __getScaleValue( ul ) );
  uLimitN->display( ul );

  QGridLayout* scaleRangeLayout = new QGridLayout();
  scaleRangeLayout->setSpacing( 10 );
  scaleRangeLayout->addWidget( lLimitText, 0, 0 );
  scaleRangeLayout->addWidget( lLimit, 0, 1 );
  scaleRangeLayout->addWidget( lLimitN, 0, 2 );
  scaleRangeLayout->addWidget( uLimitText, 1, 0 );
  scaleRangeLayout->addWidget( uLimit, 1, 1 );
  scaleRangeLayout->addWidget( uLimitN, 1, 2 );
  scaleRangeLayout->setColumnStretch( 1, 10 );
  scaleRangeGroup->setLayout( scaleRangeLayout );

  //----------------------------------------------------------------------------
  QGroupBox* scaleThresholdGroup = new QGroupBox( tr( "Scale Thresholds" ) );

  QLabel* switchText = new QLabel( tr( "Use small icons" ) + ":" );

  switchScale = new QSlider();
  switchScale->setMinimum( 2 );
  switchScale->setMaximum( 105 );
  switchScale->setPageStep( 1 );
  switchScale->setOrientation( Qt::Horizontal );
  switchScale->setSingleStep( 1 );

  switchScaleN = new QLCDNumber( 5 );
  switchScaleN->setMinimumWidth( switchScaleN->sizeHint().width() + 10 );
  switchScaleN->setBackgroundRole( QPalette::Light );
  switchScaleN->setAutoFillBackground( true );

  switchScale->setValue( __getScaleValue( sw ) );
  switchScaleN->display( sw );

  QLabel* wpLabelText = new QLabel( tr( "Draw waypoint labels" ) + ":" );

  wpLabel = new QSlider();
  wpLabel->setMinimum( 2 );
  wpLabel->setMaximum( 105 );
  wpLabel->setPageStep( 1 );
  wpLabel->setOrientation( Qt::Horizontal );
  wpLabel->setSingleStep( 1 );
  wpLabel->setMinimumHeight( wpLabel->sizeHint().height() + 5 );
  wpLabel->setMaximumHeight( wpLabel->sizeHint().height() + 20 );

  wpLabelN = new QLCDNumber( 5 );
  wpLabelN->setMinimumWidth( wpLabelN->sizeHint().width() + 10 );
  wpLabelN->setBackgroundRole( QPalette::Light );
  wpLabelN->setAutoFillBackground( true );

  wpLabel->setValue( __getScaleValue( wl ) );
  wpLabelN->display( wl );

  QLabel* reduce1Text = new QLabel( tr( "Threshold" ) + " #1:" );

  reduce1 = new QSlider();
  reduce1->setMinimum( 2 );
  reduce1->setMaximum( 105 );
  reduce1->setPageStep( 1 );
  reduce1->setOrientation( Qt::Horizontal );
  reduce1->setSingleStep( 1 );
  reduce1->setMinimumHeight( reduce1->sizeHint().height() + 5 );
  reduce1->setMaximumHeight( reduce1->sizeHint().height() + 20 );

  reduce1N = new QLCDNumber( 5 );
  reduce1N->setMinimumWidth( reduce1N->sizeHint().width() + 10 );
  reduce1N->setBackgroundRole( QPalette::Light );
  reduce1N->setAutoFillBackground( true );

  reduce1->setValue( __getScaleValue( b1 ) );
  reduce1N->display( b1 );

  QLabel* reduce2Text = new QLabel( tr( "Threshold" ) + " #2:" );

  reduce2 = new QSlider();
  reduce2->setMinimum( 2 );
  reduce2->setMaximum( 105 );
  reduce2->setPageStep( 1 );
  reduce2->setOrientation( Qt::Horizontal );
  reduce2->setSingleStep( 1 );
  reduce2->setMinimumHeight( reduce2->sizeHint().height() + 5 );
  reduce2->setMaximumHeight( reduce2->sizeHint().height() + 20 );

  reduce2N = new QLCDNumber( 5 );
  reduce2N->setMinimumWidth( reduce2N->sizeHint().width() + 10 );
  reduce2N->setBackgroundRole( QPalette::Light );
  reduce2N->setAutoFillBackground( true );

  reduce2->setValue( __getScaleValue( b2 ) );
  reduce2N->display( b2 );

  QLabel* reduce3Text = new QLabel( tr( "Threshold" ) + " #3:" );

  reduce3 = new QSlider();
  reduce3->setMinimum( 2 );
  reduce3->setMaximum( 105 );
  reduce3->setPageStep( 1 );
  reduce3->setOrientation( Qt::Horizontal );
  reduce3->setSingleStep( 1 );
  reduce3->setMinimumHeight( reduce3->sizeHint().height() + 5 );
  reduce3->setMaximumHeight( reduce3->sizeHint().height() + 20 );

  reduce3N = new QLCDNumber( 5 );
  reduce3N->setMinimumWidth( reduce3N->sizeHint().width() + 10 );
  reduce3N->setBackgroundRole( QPalette::Light );
  reduce3N->setAutoFillBackground( true );

  reduce3->setValue( __getScaleValue( b3 ) );
  reduce3N->display( b3 );

  QGridLayout* scaleThresholdLayout = new QGridLayout();
  scaleThresholdLayout->setSpacing( 10 );

  int row = 0;
  scaleThresholdLayout->addWidget( switchText, row, 0 );
  scaleThresholdLayout->addWidget( switchScale, row, 1 );
  scaleThresholdLayout->addWidget( switchScaleN, row, 2 );
  row++;

  scaleThresholdLayout->addWidget( wpLabelText, row, 0 );
  scaleThresholdLayout->addWidget( wpLabel, row, 1 );
  scaleThresholdLayout->addWidget( wpLabelN, row, 2 );
  row++;

  scaleThresholdLayout->addWidget( reduce1Text, row, 0 );
  scaleThresholdLayout->addWidget( reduce1, row, 1 );
  scaleThresholdLayout->addWidget( reduce1N, row, 2 );
  row++;

  scaleThresholdLayout->addWidget( reduce2Text, row, 0 );
  scaleThresholdLayout->addWidget( reduce2, row, 1 );
  scaleThresholdLayout->addWidget( reduce2N, row, 2 );
  row++;

  scaleThresholdLayout->addWidget( reduce3Text, row, 0 );
  scaleThresholdLayout->addWidget( reduce3, row, 1 );
  scaleThresholdLayout->addWidget( reduce3N, row, 2 );
  row++;

  scaleThresholdLayout->setColumnStretch( 1, 10 );
  scaleThresholdGroup->setLayout( scaleThresholdLayout );

  //----------------------------------------------------------------------------
  QPushButton* defaultScale = new QPushButton( tr( "Default" ) );
  defaultScale->setMaximumWidth( defaultScale->sizeHint().width() + 10 );
  defaultScale->setMinimumHeight( defaultScale->sizeHint().height() + 2 );

  //----------------------------------------------------------------------------
  QVBoxLayout* scalePageLayout = new QVBoxLayout;
  scalePageLayout->addWidget( scaleRangeGroup );
  scalePageLayout->addWidget( scaleThresholdGroup );
  scalePageLayout->addStretch( 10 );
  scalePageLayout->addWidget( defaultScale, 0, Qt::AlignLeft );

  scalePage->setLayout( scalePageLayout );

  connect( defaultScale, SIGNAL(clicked()), SLOT(slotDefaultScale()) );
  connect( lLimit, SIGNAL(valueChanged(int)), SLOT(slotShowLowerLimit(int)) );
  connect( uLimit, SIGNAL(valueChanged(int)), SLOT(slotShowUpperLimit(int)) );
  connect( switchScale, SIGNAL(valueChanged(int)), SLOT(slotShowSwitchScale(int)) );
  connect( wpLabel, SIGNAL(valueChanged(int)), SLOT(slotShowWpLabel(int)) );
  connect( reduce1, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleA(int)) );
  connect( reduce2, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleB(int)) );
  connect( reduce3, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleC(int)) );
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

  pathPage = new QWidget(this);
  pathPage->setObjectName( "PathPage" );
  pathPage->setVisible( false );

  configLayout->addWidget( pathPage, 0, 1, 1, 2 );

  // minimal length of path input fields
  const int minPath = 350;
  //----------------------------------------------------------------------------
  QGroupBox* igcGroup = new QGroupBox( tr("Flight Directory") );

  igcPathE  = new QLineEdit();
  igcPathE->setMinimumWidth(minPath);
  igcPathE->setText(flightDir);

  QPushButton* igcPathSearch = new QPushButton();
  igcPathSearch->setIcon(_mainWindow->getPixmap("kde_fileopen_16.png"));
  igcPathSearch->setMinimumWidth(igcPathSearch->sizeHint().width() + 5);
  igcPathSearch->setMinimumHeight(igcPathSearch->sizeHint().height() + 5);

  QGridLayout* igcLayout = new QGridLayout();
  igcLayout->setSpacing( 10 );
  igcLayout->addWidget( igcPathE, 0, 0 );
  igcLayout->addWidget( igcPathSearch, 0, 1 );
  igcLayout->setColumnStretch( 0, 10 );
  igcGroup->setLayout( igcLayout );

  //----------------------------------------------------------------------------
  QGroupBox* taskGroup = new QGroupBox( tr("Task Directory") );

  taskPathE = new QLineEdit();
  taskPathE->setMinimumWidth(minPath);
  taskPathE->setText(taskDir);

  QPushButton* taskPathSearch = new QPushButton();
  taskPathSearch->setIcon(_mainWindow->getPixmap("kde_fileopen_16.png"));
  taskPathSearch->setMinimumWidth(taskPathSearch->sizeHint().width() + 5);
  taskPathSearch->setMinimumHeight(taskPathSearch->sizeHint().height() + 5);

  QGridLayout* taskLayout = new QGridLayout();
  taskLayout->setSpacing( 10 );
  taskLayout->addWidget( taskPathE, 0, 0 );
  taskLayout->addWidget( taskPathSearch, 0, 1 );
  taskLayout->setColumnStretch( 0, 10 );
  taskGroup->setLayout( taskLayout );

  //----------------------------------------------------------------------------
  QGroupBox* waypointGroup = new QGroupBox( tr("Waypoint Directory") );

  waypointPathE = new QLineEdit();
  waypointPathE->setMinimumWidth(minPath);
  waypointPathE->setText(wayPointDir);

  QPushButton* waypointPathSearch = new QPushButton();
  waypointPathSearch->setIcon(_mainWindow->getPixmap("kde_fileopen_16.png"));
  waypointPathSearch->setMinimumWidth(waypointPathSearch->sizeHint().width() + 5);
  waypointPathSearch->setMinimumHeight(waypointPathSearch->sizeHint().height() + 5);

  QGridLayout* waypointLayout = new QGridLayout();
  waypointLayout->setSpacing( 10 );
  waypointLayout->addWidget( waypointPathE, 0, 0 );
  waypointLayout->addWidget( waypointPathSearch, 0, 1 );
  waypointLayout->setColumnStretch( 0, 10 );
  waypointGroup->setLayout( waypointLayout );

  //----------------------------------------------------------------------------
  QGroupBox* mapGroup = new QGroupBox( tr("Map Directory") );

  mapPathE = new QLineEdit();
  mapPathE->setMinimumWidth(minPath);
  mapPathE->setText(mapDir);

  QPushButton* mapPathSearch = new QPushButton();
  mapPathSearch->setIcon(_mainWindow->getPixmap("kde_fileopen_16.png"));
  mapPathSearch->setMinimumWidth(mapPathSearch->sizeHint().width() + 5);
  mapPathSearch->setMinimumHeight(mapPathSearch->sizeHint().height() + 5);

  QGridLayout* mapLayout = new QGridLayout();
  mapLayout->setSpacing( 10 );
  mapLayout->addWidget( mapPathE, 0, 0 );
  mapLayout->addWidget( mapPathSearch, 0, 1 );
  mapLayout->setColumnStretch( 0, 10 );
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

void KFLogConfig::__addPersonalTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Identity") );
  item->setData( 0, Qt::UserRole, "Identity" );
  item->setIcon( 0, _mainWindow->getPixmap("kde_identity_32.png") );
  setupTree->addTopLevelItem( item );
  setupTree->setCurrentItem( item );

  personalPage = new QWidget(this);
  personalPage->setObjectName( "IdentityPage" );
  personalPage->setVisible( false );

  configLayout->addWidget( personalPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  const int minLen = 150;

  QGroupBox* pilotGroup = new QGroupBox( tr("Pilot") );

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
  QGroupBox* homeGroup = new QGroupBox( tr("Homesite") );

  QFormLayout* homeLayout = new QFormLayout();
  homeLayout->setSpacing(10);

  homeCountryE = new QLineEdit(personalPage);
  homeCountryE->setMaxLength(2);
  QRegExp rx("[A-Za-z]{2}");
  homeCountryE->setValidator( new QRegExpValidator(rx, this) );
  homeCountryE->setToolTip(tr("Add country as two letter code according to ISO 3166-1-alpha-2"));

  connect( homeCountryE, SIGNAL(textEdited( const QString& )),
           this, SLOT(slotTextEditedCountry( const QString& )) );

  homeNameE = new QLineEdit(personalPage);
  homeLatE  = new LatEdit(personalPage);
  homeLonE  = new LongEdit(personalPage);

  homeCountryE->setMinimumWidth( minLen );
  homeNameE->setMinimumWidth( minLen );
  homeLatE->setMinimumWidth( minLen );
  homeLonE->setMinimumWidth( minLen );

  homeLayout->addRow( tr("Homesite") + ":", homeNameE );
  homeLayout->addRow( tr("Country") + ":", homeCountryE );
  homeLayout->addRow( tr("Latitude") + ":", homeLatE );
  homeLayout->addRow( tr("Longitude") + ":", homeLonE );

  homeGroup->setLayout( homeLayout );

  //----------------------------------------------------------------------------
  QVBoxLayout* idLayout = new QVBoxLayout;

  idLayout->addWidget( pilotGroup );
  idLayout->addSpacing( 20 );
  idLayout->addWidget( homeGroup );
  idLayout->addStretch( 10 );

  personalPage->setLayout( idLayout );

  homeNameE->setText(_settings.value("/Homesite/Name", "").toString());
  homeCountryE->setText(_settings.value("/Homesite/Country", "").toString());

  homeLatE->setKFLogDegree(_settings.value("/Homesite/Latitude", HOME_DEFAULT_LAT).toInt());
  homeLonE->setKFLogDegree(_settings.value("/Homesite/Longitude", HOME_DEFAULT_LON).toInt());

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

  airfieldPage = new QWidget(this);
  airfieldPage->setObjectName( "AirfieldPage" );
  airfieldPage->setVisible( false );

  configLayout->addWidget( airfieldPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* welt2000Group = new QGroupBox( tr("Welt2000") );

  // Check input with a validator. Country codes are consist of 2 letters and
  // are separated by space, comma or semicolon.
  QRegExp rx("[A-Za-z]{2}([ ,;][A-Za-z]{2})*");
  QValidator *validator = new QRegExpValidator(rx, this);

  filterWelt2000 = new QLineEdit();
  filterWelt2000->setMinimumWidth( 150 );
  filterWelt2000->setValidator( validator );
  filterWelt2000->setToolTip( tr("Add countries to be read in as 2 letter code according to ISO 3166-1-alpha-2.") );

  homeRadiusWelt2000 = new QSpinBox();
  homeRadiusWelt2000->setRange( 0, 10000 );
  homeRadiusWelt2000->setSingleStep( 10 );
  homeRadiusWelt2000->setButtonSymbols( QSpinBox::PlusMinus );
  homeRadiusWelt2000->setSuffix( " Km" );
  homeRadiusWelt2000->setSpecialValueText(tr("Off"));
  homeRadiusWelt2000->setToolTip( tr("Read in all objects within home site radius.") );

  readOlWelt2000 = new QCheckBox( tr("Read Outlandings:") );
  readOlWelt2000->setToolTip( tr("Activate checkbox, if outlandings should be read in.") );

  QPushButton* downloadWelt2000 = new QPushButton( tr("Download") );
  downloadWelt2000->setToolTip( tr("Press button to download the Welt2000 file.") );
  downloadWelt2000->setMaximumWidth(downloadWelt2000->sizeHint().width() + 10);
  downloadWelt2000->setMinimumHeight(downloadWelt2000->sizeHint().height() + 2);
  connect( downloadWelt2000, SIGNAL(clicked()), this, SLOT(slotDownloadWelt2000()) );

  QFormLayout* weltLayout = new QFormLayout();
  weltLayout->setSpacing( 10 );
  weltLayout->addRow( tr( "Country Filter" ) + ":", filterWelt2000 );
  weltLayout->addRow( tr( "Home Radius" )  + ":", homeRadiusWelt2000 );

  QVBoxLayout* weltGroupLayout = new QVBoxLayout;
  weltGroupLayout->addLayout( weltLayout );
  weltGroupLayout->addWidget( readOlWelt2000 );
  weltGroupLayout->addSpacing( 10 );
  weltGroupLayout->addWidget( downloadWelt2000, Qt::AlignLeft );

  welt2000Group->setLayout( weltGroupLayout );

  //----------------------------------------------------------------------------
  QVBoxLayout* afLayout = new QVBoxLayout;

  afLayout->addWidget( welt2000Group );
  afLayout->addStretch( 10 );

  airfieldPage->setLayout( afLayout );

  homeRadiusWelt2000Value = _settings.value( "/Welt2000/HomeRadius", 0 ).toInt();
  filterWelt2000Text      = _settings.value( "/Welt2000/CountryFilter", "" ).toString();
  readOlWelt2000Value     = _settings.value( "/Welt2000/LoadOutlandings", true ).toBool();

  homeRadiusWelt2000->setValue( homeRadiusWelt2000Value );
  filterWelt2000->setText( filterWelt2000Text );
  readOlWelt2000->setChecked( readOlWelt2000Value );
}


/** Add a tab for airspace file management.*/
void KFLogConfig::__addAirspaceTab()
{
 _settings.setValue("/Airspace/OpenAipLink",
                    "9EEAi^^HHH]@A6?2:A]?6E^<7=@806IA@CE097uwab`987");

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Airspace Management") );
  item->setData( 0, Qt::UserRole, "Airspaces" );
  item->setIcon( 0, _mainWindow->getPixmap("kde_move_16.png") );
  setupTree->addTopLevelItem( item );

  airspacePage = new QWidget(this);
  airspacePage->setObjectName( "AirspacePage" );
  airspacePage->setVisible( false );

  configLayout->addWidget( airspacePage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QVBoxLayout *topLayout = new QVBoxLayout;

  QGroupBox* openAipGroup = new QGroupBox( tr("openAIP Airspaces") );

  // Check input with a validator. Country codes are consist of 2 letters and
  // are separated by space, comma or semicolon.
  QRegExp rx("[A-Za-z]{2}([ ,;][A-Za-z]{2})*");
  QValidator *validator = new QRegExpValidator(rx, this);

  countriesOpenAipAS = new QLineEdit;
  countriesOpenAipAS->setMinimumWidth( 150 );
  countriesOpenAipAS->setValidator( validator );
  countriesOpenAipAS->setToolTip( tr("Add countries to be downloaded as 2 letter code according to ISO 3166-1-alpha-2.") );

  QString countries = _settings.value("/Airspace/Countries", "").toString();

  if( countries.isEmpty() == false )
    {
      countriesOpenAipAS->setText(countries);
    }

  QPushButton* downloadAs = new QPushButton( tr("Download") );
  downloadAs->setToolTip( tr("Press button to download the desired openAIP airspace files.") );
  downloadAs->setMaximumWidth(downloadAs->sizeHint().width() + 10);
  downloadAs->setMinimumHeight(downloadAs->sizeHint().height() + 2);
  connect( downloadAs, SIGNAL(clicked()), this, SLOT(slotDownloadOpenAipAS()) );

  QFormLayout* openAipFormLayout = new QFormLayout();
  openAipFormLayout->setSpacing( 10 );
  openAipFormLayout->addRow( tr( "Countries" ) + ":", countriesOpenAipAS );

  QVBoxLayout* openAipLayout = new QVBoxLayout;
  openAipLayout->addLayout( openAipFormLayout );
  openAipLayout->addWidget( downloadAs, Qt::AlignLeft );
  openAipGroup->setLayout( openAipLayout );

  topLayout->addWidget( openAipGroup );

  asFileTable = new QTableWidget( 0, 1, this );
  asFileTable->setToolTip( tr("Use check boxes to activate or deactivate airspace file loading.") );
  asFileTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  asFileTable->setShowGrid( true );

  connect( asFileTable, SIGNAL(cellClicked ( int, int )),
           SLOT(slotToggleAsCheckBox( int, int )) );

  QHeaderView* hHeader = asFileTable->horizontalHeader();
  hHeader->setStretchLastSection( true );

  topLayout->addWidget( asFileTable, 10 );
  airspacePage->setLayout( topLayout );
}

/** Add a tab for waypoint catalog configuration at sartup
Setting will be overwritten by commandline switch */
void KFLogConfig::__addWaypointTab()
{
  int catalogType     = _settings.value("/Waypoints/DefaultWaypointCatalog", LastUsed).toInt();
  QString catalogName = _settings.value("/Waypoints/DefaultCatalogName", "").toString();

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Waypoint Catalog") );
  item->setData( 0, Qt::UserRole, "Waypoints" );
  item->setIcon( 0, _mainWindow->getPixmap("waypoint_32.png") );
  setupTree->addTopLevelItem( item );

  waypointPage = new QWidget(this);
  waypointPage->setObjectName( "WaypointPage" );
  waypointPage->setVisible( false );

  configLayout->addWidget( waypointPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* catGroup = new QGroupBox( tr("Default Catalog") );

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
  catalogPathSearch->setIcon( _mainWindow->getPixmap( "kde_fileopen_16.png" ) );
  catalogPathSearch->setMinimumWidth( catalogPathSearch->sizeHint().width() + 5 );
  catalogPathSearch->setMinimumHeight( catalogPathSearch->sizeHint().height() + 5 );

  connect( catalogPathSearch, SIGNAL(clicked()),
           SLOT(slotSearchDefaultWaypoint()) );

  QGridLayout* catPathLayout = new QGridLayout();
  catPathLayout->setSpacing( 10 );
  catPathLayout->addWidget( catalogPathE, 0, 0 );
  catPathLayout->addWidget( catalogPathSearch, 0, 1 );
  catPathLayout->setColumnStretch( 0, 10 );

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

/** Add a tab for the unit configuration.*/
void KFLogConfig::__addUnitTab()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Units") );
  item->setData( 0, Qt::UserRole, "Units" );
  item->setIcon( 0, _mainWindow->getPixmap("edit_unit_32x32.png") );
  setupTree->addTopLevelItem( item );

  unitPage = new QWidget(this);
  unitPage->setObjectName( "UnitPage" );
  unitPage->setVisible( false );

  configLayout->addWidget( unitPage, 0, 1, 1, 2 );

  //----------------------------------------------------------------------------
  QGroupBox* unitGroup = new QGroupBox( tr("Units") );

  unitAltitude = new QComboBox;
  unitAltitude->setObjectName("UnitAltitude");
  unitAltitude->setEditable(false);
  unitAltitude->addItem(tr("meters"), Altitude::meters);
  unitAltitude->addItem(tr("feet"),   Altitude::feet);

  unitDistance = new QComboBox;
  unitDistance->setObjectName("UnitDistance");
  unitDistance->setEditable(false);
  unitDistance->addItem(tr("kilometers"),     Distance::kilometers);
  unitDistance->addItem(tr("statute miles"),  Distance::miles);
  unitDistance->addItem(tr("nautical miles"), Distance::nautmiles);

  unitPosition = new QComboBox;
  unitPosition->setObjectName("UnitPosition");
  unitPosition->setEditable(false);
  unitPosition->addItem("ddd\260mm'ss\"",  WGSPoint::DMS);
  unitPosition->addItem("ddd\260mm.mmm'",  WGSPoint::DDM);
  unitPosition->addItem("ddd.ddddd\260",   WGSPoint::DDD);

  QFormLayout* unitLayout = new QFormLayout();
  unitLayout->setSpacing( 10 );
  unitLayout->addRow( tr( "Altitude" ) + ":", unitAltitude );
  unitLayout->addRow( tr( "Distance" ) + ":", unitDistance );
  unitLayout->addRow( tr( "Position" ) + ":", unitPosition );

  unitGroup->setLayout( unitLayout );

  //----------------------------------------------------------------------------
  QVBoxLayout* unitPageLayout = new QVBoxLayout;

  unitPageLayout->addWidget( unitGroup );
  unitPageLayout->addStretch( 10 );

  unitPage->setLayout( unitPageLayout );

  int altUnit  = _settings.value( "/Units/Altitude", Altitude::meters ).toInt();
  int distUnit = _settings.value( "/Units/Distance", Distance::kilometers ).toInt();
  int posUnit  = _settings.value( "/Units/Position", WGSPoint::DMS ).toInt();

  unitAltitude->setCurrentIndex( unitAltitude->findData( altUnit ) );
  unitDistance->setCurrentIndex( unitDistance->findData( distUnit ) );
  unitPosition->setCurrentIndex( unitPosition->findData( posUnit ) );
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
                         waypointPathE->text(),
                         tr("KFLog Catalogs (*.kflogwp *.KFLOGWP)"));

  if( ! fileName.isEmpty() && fileName.endsWith(".kflogwp", Qt::CaseInsensitive) )
    {
      catalogPathE->setText( fileName );
    }
}

void KFLogConfig::slotSelectFlightTypeColor( int buttonIdentifier )
{
  QPushButton* pressedButton;
  QColor*      relatedColor;

  pressedButton = *ftcButtonArray[buttonIdentifier];
  relatedColor  = ftcColorArray[buttonIdentifier];

  QColor newColor = QColorDialog::getColor( *relatedColor, this );

  if( newColor.isValid() )
    {
      *relatedColor = newColor;
      QPixmap buttonPixmap( 82, 14 );
      buttonPixmap.fill( newColor );
      pressedButton->setIcon( buttonPixmap );
      pressedButton->setIconSize( buttonPixmap.size() );
    }
}

void KFLogConfig::slotDownloadWelt2000()
{
  QString input = filterWelt2000->text().trimmed();

  if( __checkWelt2000Input( input ) == false )
    {
      return;
    }

  _settings.setValue( "/Welt2000/CountryFilter", filterWelt2000->text().trimmed().toUpper() );
  _settings.setValue( "/Welt2000/HomeRadius", homeRadiusWelt2000->value() );
  _settings.setValue( "/Welt2000/LoadOutlandings", readOlWelt2000->isChecked() );

  emit downloadWelt2000();
}

void KFLogConfig::slotDownloadOpenAipAS()
{
  QString input = countriesOpenAipAS->text().trimmed();

  if( __checkOpenAipAirspaceInput( input ) == false )
    {
      return;
    }

  _settings.setValue( "/Airspace/Countries",
                      countriesOpenAipAS->text().trimmed().toLower() );

  emit downloadOpenAipAirspaces();
}

/**
 * Called to toggle the check box of the clicked table cell in the airspace
 * file table.
 */
void KFLogConfig::slotToggleAsCheckBox( int row, int column )
{
  QTableWidgetItem* item = asFileTable->item( row, column );

  if( row > 0 && asFileTable->item( 0, 0 )->checkState() == Qt::Checked )
    {
      // All is checked, do not changed other items
      return;
    }

  item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );

  if( row == 0 && column == 0 )
    {
      // First entry was clicked. Change related check items.
      if( item->checkState() == Qt::Checked )
        {
          // All other items are checked too
          for( int i = asFileTable->rowCount() - 1; i > 0; i-- )
            {
              asFileTable->item( i, 0 )->setCheckState( Qt::Checked );
            }
        }
    }
}

/**
 * Loads the content of the current airspace file directory into the
 * file table.
 */
void KFLogConfig::__loadAirspaceFilesIntoTable()
{
  asFileTable->clear();

  QString mapDir = _globalMapContents->getMapRootDirectory() + "/airspaces";

  QTableWidgetItem *hrItem = new QTableWidgetItem( tr("Airspace Files in ") + mapDir );
  asFileTable->setHorizontalHeaderItem( 0, hrItem );

  QDir dir( mapDir );
  QStringList filters; filters << "*.txt" << "*.TXT" << "*.aip" << "*.AIP";
  dir.setNameFilters(filters);
  dir.setFilter( QDir::Files|QDir::Readable);
  dir.setSorting( QDir::Name );

  QStringList preselect = dir.entryList();

  int row = 0;
  asFileTable->setRowCount( row + 1 );

  QTableWidgetItem* item = new QTableWidgetItem( tr("Select all"), 0 );
  item->setFlags( Qt::ItemIsEnabled );
  item->setCheckState( Qt::Unchecked );
  asFileTable->setItem( row, 0, item );
  row++;

  for( int i = 0; i < preselect.size(); i++ )
    {
      asFileTable->setRowCount( row + 1 );

      QString file = QFileInfo( preselect.at(i) ).fileName();
      item = new QTableWidgetItem( file, row );
      item->setFlags( Qt::ItemIsEnabled );
      item->setCheckState( Qt::Unchecked );
      asFileTable->setItem( row, 0, item );
      row++;
    }

  QStringList files = _settings.value( "/Airspace/FileList", QStringList(QString("All"))).toStringList();

  if( files.isEmpty() )
    {
      return;
    }

  if( files.at(0) == "All" )
    {
      // Set all items to checked, if All is contained in the list at the first
      // position.
      for( int i = 0; i < asFileTable->rowCount(); i++ )
        {
          asFileTable->item( i, 0 )->setCheckState( Qt::Checked );
        }
    }
  else
    {
      // Set the All item to unchecked.
      asFileTable->item( 0, 0 )->setCheckState( Qt::Unchecked );

      for( int i = 1; i < asFileTable->rowCount(); i++ )
        {
          QTableWidgetItem* item = asFileTable->item( i, 0 );

          if( files.contains( item->text()) )
            {
              asFileTable->item( i, 0 )->setCheckState( Qt::Checked );
            }
          else
            {
              asFileTable->item( i, 0 )->setCheckState( Qt::Unchecked );
            }
        }
    }

  asFileTable->resizeColumnsToContents();
}

bool KFLogConfig::__checkOpenAipAirspaceInput( QString& input )
{
  QRegExp rx("[A-Za-z]{2}([ ,;][A-Za-z]{2})*");
  QRegExpValidator v(rx, 0);

  int pos = 0;

  QValidator::State state = v.validate( input, pos );

  if( state != QValidator::Acceptable )
    {
      // Popup a warning message box
      QMessageBox::warning( this,
                            tr("Wrong country entries"),
                            "<html>" +
                            tr("Please check the openAIP airspace country entries!") +
                            "<br><br>" +
                            tr("The expected format is a two letter country code separated by spaces") +
                            "</html>",
                            QMessageBox::Ok );
      return false;
    }

  return true;
}

bool KFLogConfig::__checkWelt2000Input( QString& input )
{
  QRegExp rx("[A-Za-z]{2}([ ,;][A-Za-z]{2})*");
  QRegExpValidator v(rx, 0);

  int pos = 0;

  QValidator::State state = v.validate( input, pos );

  if( state != QValidator::Acceptable )
    {
      // Popup a warning message box
      QMessageBox::warning( this,
                            tr("Wrong country entries"),
                            "<html>" +
                            tr("Please check the Welt2000 country entries!") +
                            "<br><br>" +
                            tr("The expected format is a two letter country code separated by spaces") +
                            "</html>",
                            QMessageBox::Ok );
      return false;
    }

  return true;
}

QByteArray KFLogConfig::rot47( const QByteArray& input )
{
  // const char* a0 = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  const char* rotA  = "PQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNO";

  QByteArray out;

  if( input.isEmpty() )
    {
      return out;
    }

  for( int i = 0; i < input.size(); i++ )
    {
      unsigned char c = input.at(i);

      if( c < '!' || c > '~' )
        {
          // let it as it is
          out.append(c);
        }
      else
        {
          // translate character
          out.append( rotA[c - 0x21] );
        }
    }

  return out;
}
