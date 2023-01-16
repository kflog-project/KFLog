/***********************************************************************
**
**   waypointimpfilterdialog.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include "mapcontents.h"
#include "waypointimpfilterdialog.h"

#include <QtWidgets>
#include <QFileDialog>

extern MapContents *_globalMapContents;

WaypointImpFilterDialog::WaypointImpFilterDialog( QWidget *parent ) :
 QDialog(parent),
 centerRef(CENTER_HOMESITE)
{
  setObjectName( "WaypointImpFilterDialog" );
  setWindowTitle(tr("Waypoint Filter"));
  setModal( true );

  save.usedFilter = WaypointCatalog::Radius;
  save.radiusIdxPosition = 6;
  save.radiusIdxHome = 6;
  save.radiusIdxMap = 6;
  save.radiusIdxAirfield = 6;
  save.airfieldRefIdx = 0;

  // create checkboxes
  useAll       = new QCheckBox(tr("Check/Uncheck all"));
  airfields    = new QCheckBox(tr("&Airfields"));
  gliderfields = new QCheckBox(tr("&Gliderfields"));
  outlandings  = new QCheckBox(tr("Ou&tlandings"));
  navaids      = new QCheckBox(tr("&Navaids"));
  reportings   = new QCheckBox(tr("&Reportings"));
  obstacles    = new QCheckBox(tr("O&bstacles"));
  landmarks    = new QCheckBox(tr("&Landmarks"));
  hotspots     = new QCheckBox(tr("&Hotspots"));

  connect(useAll, SIGNAL(clicked()), this, SLOT(slotChangeUseAll()));

  QVBoxLayout *typeLayout = new QVBoxLayout;
  typeLayout->setSpacing( 10 );
  typeLayout->addWidget( useAll );
  typeLayout->addSpacing( 10 );
  typeLayout->addWidget( airfields );
  typeLayout->addWidget( gliderfields );
  typeLayout->addWidget( outlandings );
  typeLayout->addWidget( navaids );
  typeLayout->addWidget( reportings );
  typeLayout->addWidget( obstacles );
  typeLayout->addWidget( landmarks );
  typeLayout->addWidget( hotspots );

  QGroupBox* typeGroup = new QGroupBox( tr("Type") );
  typeGroup->setLayout( typeLayout );

  //---------------------------------------------------------------------------
  rbf0 = new QRadioButton(tr("Radius"));
  rbf1 = new QRadioButton(tr("Area"));

  QButtonGroup* filterButtonGroup = new QButtonGroup(this);
  filterButtonGroup->setExclusive(true);
  filterButtonGroup->addButton( rbf0, WaypointCatalog::Radius );
  filterButtonGroup->addButton( rbf1, WaypointCatalog::Area );

  connect( filterButtonGroup, SIGNAL( buttonClicked(int)),
           this, SLOT(slotFilterChanged(int)) );

  QHBoxLayout* filterBox = new QHBoxLayout;
  filterBox->addWidget( rbf0 );
  filterBox->addSpacing( 10 );
  filterBox->addWidget( rbf1 );
  filterBox->addStretch( 10 );

  filterGroup = new QGroupBox( tr("Filter") );
  filterGroup->setLayout( filterBox );

  //---------------------------------------------------------------------------
  fromLat  = new LatEdit;
  fromLong = new LongEdit;

  QGridLayout* fromGrid = new QGridLayout;
  fromGrid->setSpacing( 10 );
  fromGrid->addWidget( new QLabel(tr("Lat:")), 0 , 0 );
  fromGrid->addWidget( fromLat, 0, 1 );
  fromGrid->setColumnMinimumWidth( 2, 10 );
  fromGrid->addWidget( new QLabel(tr("Lon:")), 0 , 3 );
  fromGrid->addWidget( fromLong, 0, 4 );
  fromGrid->setColumnStretch( 5, 10 );

  fromGroup = new QGroupBox( tr("Area From") );
  fromGroup->setLayout( fromGrid );

  //---------------------------------------------------------------------------
  toLat  = new LatEdit;
  toLong = new LongEdit;

  QGridLayout* toGrid = new QGridLayout;
  toGrid->setSpacing( 10 );
  toGrid->addWidget( new QLabel(tr("Lat:")), 0 , 0 );
  toGrid->addWidget( toLat, 0, 1 );
  toGrid->setColumnMinimumWidth( 2, 10 );
  toGrid->addWidget( new QLabel(tr("Lon:")), 0 , 3 );
  toGrid->addWidget( toLong, 0, 4 );
  toGrid->setColumnStretch( 5, 10 );

  toGroup = new QGroupBox( tr("Area to") );
  toGroup->setLayout( toGrid );

  //---------------------------------------------------------------------------
  rb0 = new QRadioButton(tr("Position"));
  rb1 = new QRadioButton(tr("Homesite"));
  rb2 = new QRadioButton(tr("Center of Map"));
  rb3 = new QRadioButton(tr("Airfield"));

  QButtonGroup* radiusButtonGroup = new QButtonGroup(this);
  radiusButtonGroup->setExclusive(true);
  radiusButtonGroup->addButton( rb0, CENTER_POS );
  radiusButtonGroup->addButton( rb1, CENTER_HOMESITE );
  radiusButtonGroup->addButton( rb2, CENTER_MAP );
  radiusButtonGroup->addButton( rb3, CENTER_AIRFIELD );

  connect( radiusButtonGroup, SIGNAL( buttonClicked(int)),
           this, SLOT(selectRadius(int)) );

  centerLat  = new LatEdit;
  centerLong = new LongEdit;
  airfieldRefBox = new QComboBox;
  radiusUnit = new QLabel;

  connect( airfieldRefBox, SIGNAL(currentIndexChanged(const QString&)),
           this, SLOT(slotAirfieldRefChanged(const QString&)) );

  radius = new QComboBox;
  radius->setEditable( true );
  radius->setValidator( new QIntValidator(1, 10000, this) );
  QStringList itemList;
  itemList << tr("Off") << "10" << "25" << "50" << "100" << "250" << "500"
           << "750" << "1000" << "1500" << "2000";
  radius->addItems( itemList );
  radius->setCurrentIndex( 5 );

  connect( radius, SIGNAL(currentIndexChanged(int)), SLOT(slotRadiusChanged(int)) );

  QGridLayout *radiusGrid = new QGridLayout;
  radiusGrid->setSpacing(10);
  radiusGrid->addWidget( rb0, 0, 0 );
  radiusGrid->addWidget( new QLabel(tr("Lat:")), 0, 1 );
  radiusGrid->addWidget( centerLat, 0, 2);
  radiusGrid->addWidget( new QLabel(tr("Lon:")), 0, 3 );
  radiusGrid->addWidget( centerLong, 0, 4 );

  radiusGrid->addWidget( rb1, 1, 0 );
  radiusGrid->addWidget( rb2, 2, 0 );
  radiusGrid->addWidget( rb3, 3, 0 );
  radiusGrid->addWidget( airfieldRefBox, 3, 2 );

  radiusGrid->addWidget( radiusUnit, 4, 0 );
  radiusGrid->addWidget( radius, 4, 2 );
  radiusGrid->setColumnStretch( 5, 10 );

  radiusGroup = new QGroupBox( tr("Radius") );
  radiusGroup->setLayout( radiusGrid );

  //---------------------------------------------------------------------------
  QHBoxLayout *buttonBox = new QHBoxLayout;

  QPushButton *b = new QPushButton(tr("&Clear"), this);
  connect(b, SIGNAL(clicked()), this, SLOT(slotClear()));
  buttonBox->addWidget(b);
  buttonBox->addStretch( 10 );

  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), this, SLOT(slotOk()));
  buttonBox->addWidget(b);

  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), this, SLOT(slotCancel()));
  buttonBox->addWidget(b);

  //---------------------------------------------------------------------------
  QVBoxLayout *ftrBox = new QVBoxLayout;
  ftrBox->addWidget( filterGroup );
  ftrBox->addWidget( fromGroup );
  ftrBox->addWidget( toGroup );
  ftrBox->addWidget( radiusGroup );
  ftrBox->addStretch( 10 );

  QHBoxLayout *hBox = new QHBoxLayout;
  hBox->addWidget( typeGroup );
  hBox->addLayout( ftrBox );

  QVBoxLayout *top = new QVBoxLayout;
  top->addLayout( hBox );
  top->addLayout( buttonBox );

  setLayout( top );

  slotClear();
  slotChangeUseAll();
  selectRadius(CENTER_HOMESITE);
  loadRadiusValue();
}

WaypointImpFilterDialog::~WaypointImpFilterDialog()
{
}

void WaypointImpFilterDialog::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // The unit can be changed in the meantime. Therefore we do update it here.
  QString distUnit = Distance::getUnitText();
  radiusUnit->setText( tr("Radius") + " (" + distUnit + "):" );

  // Load airfield data into combo box. Can be changed in the meantime.
  loadAirfieldComboBox();

  // Save all values for restore in reject case.
  saveValues();
}

enum WaypointCatalog::FilterType WaypointImpFilterDialog::getFilter()
{
  if( rbf0->isChecked() )
    {
      return WaypointCatalog::Radius;
    }
  else
    {
      return WaypointCatalog::Area;
    }
}

void WaypointImpFilterDialog::slotOk()
{
  if( rbf0->isChecked() && rb0->isChecked() )
    {
      // Check the latitude coordinate values in the radius position box for
      // correctness, if it is selected.
      if( centerLat->KFLogDegree() == 0 )
	{
	  QMessageBox::warning( this, QObject::tr("Latitude error"),
	                        "<html>" + QObject::tr("Please enter a valid latitude for the Radius Position Filter!") +
	                        "</html>", QMessageBox::Ok);

	  return;
	}
    }

  if( rbf1->isChecked() )
    {
      int fLat = fromLat->KFLogDegree();
      int tLat = toLat->KFLogDegree();
      int fLon = fromLong->KFLogDegree();
      int tLon = toLong->KFLogDegree();

      // Check the area coordinates for correctness
      if( fLat == 00 || tLat == 00 ||
	        fLat == tLat || fLon == tLon )
        {
          QMessageBox::warning( this, QObject::tr("Error in coordinates"),
                                "<html>" + QObject::tr("Please enter valid coordinates for the Area filter!") +
                                "<br><br>" +
                                QObject::tr("Latitudes maybe not zero and from-to coordinates should be different.") +
                                "</html>", QMessageBox::Ok);

          return;
        }
    }

  QDialog::accept();
}

void WaypointImpFilterDialog::slotCancel()
{
  // The user has pressed the cancel button. All dialog values are restored
  // because the user could have modified them before canceling.
  restoreValues();
  QDialog::reject();
}

void WaypointImpFilterDialog::slotChangeUseAll()
{
  bool show = useAll->isChecked();

  airfields->setChecked(show);
  gliderfields->setChecked(show);
  navaids->setChecked(show);
  reportings->setChecked(show);
  outlandings->setChecked(show);
  obstacles->setChecked(show);
  landmarks->setChecked(show);
  hotspots->setChecked(show);
}

/** reset all dialog items to default values */
void WaypointImpFilterDialog::slotClear()
{
  useAll->setChecked( true );
  airfields->setChecked( false );
  gliderfields->setChecked( false );
  navaids->setChecked( false );
  reportings->setChecked( false );
  outlandings->setChecked( false );
  obstacles->setChecked( false );
  landmarks->setChecked( false );
  hotspots->setChecked( false );

  slotChangeUseAll();

  fromLat->setKFLogDegree( 0 );
  fromLong->setKFLogDegree( 0 );
  toLat->setKFLogDegree( 0 );
  toLong->setKFLogDegree( 0 );
  centerLat->setKFLogDegree( 0 );
  centerLong->setKFLogDegree( 0 );

  rb0->setChecked( false );
  rb1->setChecked( true );
  rb2->setChecked( false );
  rb3->setChecked( false );

  rbf0->setChecked( true );
  rbf1->setChecked( false );

  save.usedFilter = WaypointCatalog::Radius;
  save.radiusIdxPosition = 6;
  save.radiusIdxHome = 6;
  save.radiusIdxMap = 6;
  save.radiusIdxAirfield = 6;
  save.airfieldRefIdx = 0;

  radius->setCurrentIndex( 6 );
  airfieldRefBox->setCurrentIndex( 0 );

  selectRadius(CENTER_HOMESITE);
  slotFilterChanged( WaypointCatalog::Radius );
}

void WaypointImpFilterDialog::selectRadius(int n)
{
  centerRef = n;

  switch (centerRef)
  {
    case CENTER_POS:
      rb0->setChecked(true);
      rb1->setChecked(false);
      rb2->setChecked(false);
      rb3->setChecked(false);
      centerLat->setEnabled(true);
      centerLong->setEnabled(true);
      airfieldRefBox->setEnabled(false);
      break;
    case CENTER_HOMESITE:
      rb0->setChecked(false);
      rb1->setChecked(true);
      rb2->setChecked(false);
      rb3->setChecked(false);
      centerLat->setEnabled(false);
      centerLong->setEnabled(false);
      airfieldRefBox->setEnabled(false);
      break;
    case CENTER_MAP:
      rb0->setChecked(false);
      rb1->setChecked(false);
      rb2->setChecked(true);
      rb3->setChecked(false);
      centerLat->setEnabled(false);
      centerLong->setEnabled(false);
      airfieldRefBox->setEnabled(false);
      break;
    case CENTER_AIRFIELD:
      rb0->setChecked(false);
      rb1->setChecked(false);
      rb2->setChecked(false);
      rb3->setChecked(true);
      centerLat->setEnabled(false);
      centerLong->setEnabled(false);
      airfieldRefBox->setEnabled(true);
      break;
    }

  loadRadiusValue();
}

int WaypointImpFilterDialog::getCenterRef() const
{
  return centerRef;
}

void WaypointImpFilterDialog::loadAirfieldComboBox()
{
  int searchList[] = { MapContents::GliderfieldList, MapContents::AirfieldList };

  airfieldDict.clear();
  airfieldRefBox->clear();

  QStringList airfieldList;

  for( int l = 0; l < 2; l++ )
    {
      for( int loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++ )
      {
        SinglePoint *hitElement = (SinglePoint *) _globalMapContents->getElement(searchList[l], loop );
        airfieldList.append( hitElement->getName() );
        airfieldDict.insert( hitElement->getName(), hitElement );
      }
  }

  airfieldList.sort();
  airfieldRefBox->addItems( airfieldList );

  // try to find the last selection in the new content.
  int newIndex = airfieldRefBox->findText( airfieldRefTxt );

  if( newIndex != -1 )
    {
      // Try to find the last selection.
      airfieldRefBox->setCurrentIndex( newIndex );
    }
  else
    {
      airfieldRefBox->setCurrentIndex( 0 );
    }
}

WGSPoint WaypointImpFilterDialog::getAirfieldRef()
{
  QString s = airfieldRefBox->currentText();

  WGSPoint p;

  if( airfieldDict.contains(s) )
    {
      SinglePoint *sp = airfieldDict.value(s);
      p = sp->getWGSPosition();
    }

  return p;
}

void WaypointImpFilterDialog::slotAirfieldRefChanged( const QString& text )
{
  airfieldRefTxt = text;
}

void WaypointImpFilterDialog::saveValues()
{
  save.useAll = useAll->isChecked();
  save.airfields = airfields->isChecked();
  save.gliderfields = gliderfields->isChecked();
  save.navaids = navaids->isChecked();
  save.reportings = reportings->isChecked();
  save.outlandings = outlandings->isChecked();
  save.obstacles = obstacles->isChecked();
  save.landmarks = landmarks->isChecked();
  save.hotspots = hotspots->isChecked();

  save.rb0 = rb0->isChecked();
  save.rb1 = rb1->isChecked();
  save.rb2 = rb2->isChecked();
  save.rb3 = rb3->isChecked();

  save.rbf0 = rbf0->isChecked();
  save.rbf1 = rbf1->isChecked();

  save.fromLat = fromLat->KFLogDegree();
  save.fromLong = fromLong->KFLogDegree();
  save.toLat = toLat->KFLogDegree();
  save.toLong = toLong->KFLogDegree();
  save.centerLat = centerLat->KFLogDegree();
  save.centerLong = centerLong->KFLogDegree();
  save.centerRef = centerRef;
  save.airfieldRefIdx = airfieldRefBox->currentIndex();
  saveRadiusValue();
}

void WaypointImpFilterDialog::slotRadiusChanged( int newIndex )
{
  Q_UNUSED(newIndex)

  saveRadiusValue();
}

void WaypointImpFilterDialog::saveRadiusValue()
{
  if( rb0->isChecked() )
    {
      save.radiusIdxPosition = radius->currentIndex();
    }
  else if( rb1->isChecked() )
    {
      save.radiusIdxHome = radius->currentIndex();
    }
  else if( rb2->isChecked() )
    {
      save.radiusIdxMap = radius->currentIndex();
    }
  else if( rb3->isChecked() )
    {
      save.radiusIdxAirfield = radius->currentIndex();
    }
}

void WaypointImpFilterDialog::loadRadiusValue()
{
  if( rb0->isChecked() )
    {
      radius->setCurrentIndex( save.radiusIdxPosition );
    }
  else if( rb1->isChecked() )
    {
      radius->setCurrentIndex( save.radiusIdxHome );
    }
  else if( rb2->isChecked() )
    {
      radius->setCurrentIndex( save.radiusIdxMap );
    }
  else if( rb3->isChecked() )
    {
      radius->setCurrentIndex( save.radiusIdxAirfield );
    }
  else
    {
      // Set default to 5
      radius->setCurrentIndex( 5 );
    }
}

double WaypointImpFilterDialog::getCenterRadius()
{
  int index = 0;

  if( rb0->isChecked() )
    {
      index = save.radiusIdxPosition;
    }
  else if( rb1->isChecked() )
    {
      index = save.radiusIdxHome;
    }
  else if( rb2->isChecked() )
    {
      index = save.radiusIdxMap;
    }
  else if( rb3->isChecked() )
    {
      index = save.radiusIdxAirfield;
    }

  if( index == 0 )
    {
      return 0.0;
    }

  return radius->itemText(index).toDouble();
}

void WaypointImpFilterDialog::setCenterRadius( QString& value )
{
  int idx = radius->findText( value );

  if( idx != -1 )
    {
      radius->setCurrentIndex( idx );
    }
  else
    {
      radius->setCurrentIndex( 0 );
    }
}

/** restore all dialog items to the saved values */
void WaypointImpFilterDialog::restoreValues()
{
  useAll->setChecked( save.useAll );
  airfields->setChecked( save.airfields );
  gliderfields->setChecked( save.gliderfields );
  navaids->setChecked( save.navaids );
  reportings->setChecked( save.reportings );
  outlandings->setChecked( save.outlandings );
  obstacles->setChecked( save.obstacles );
  landmarks->setChecked( save.landmarks );
  hotspots->setChecked( save.hotspots );

  fromLat->setKFLogDegree( save.fromLat );
  fromLong->setKFLogDegree( save.fromLong );
  toLat->setKFLogDegree( save.toLat );
  toLong->setKFLogDegree( save.toLong );
  centerLat->setKFLogDegree( save.centerLat );
  centerLong->setKFLogDegree( save.centerLong );

  rb0->setChecked( save.rb0 );
  rb1->setChecked( save.rb1 );
  rb2->setChecked( save.rb2 );
  rb3->setChecked( save.rb3 );

  rbf0->setChecked( save.rbf0 );
  rbf1->setChecked( save.rbf1 );

  airfieldRefBox->setCurrentIndex( save.airfieldRefIdx );
  loadRadiusValue();
  selectRadius(save.centerRef);

  if( save.rbf1 )
    {
      slotFilterChanged( WaypointCatalog::Area );
    }
  else
    {
      slotFilterChanged( WaypointCatalog::Radius );
    }
}

void WaypointImpFilterDialog::slotFilterChanged( int newFilter )
{
  switch( newFilter )
  {
    case WaypointCatalog::Area:
      save.usedFilter = WaypointCatalog::Area;
      radiusGroup->hide();
      fromGroup->show();
      toGroup->show();
      break;

    case WaypointCatalog::Radius:
    default:
      save.usedFilter = WaypointCatalog::Radius;
      radiusGroup->show();
      fromGroup->hide();
      toGroup->hide();
      break;
  }
}

void WaypointImpFilterDialog::setFilter( enum WaypointCatalog::FilterType newFilter )
{
  switch( newFilter )
  {
    case WaypointCatalog::Area:
      rbf0->setChecked( false );
      rbf1->setChecked( true );
      break;

    case WaypointCatalog::Radius:
    default:
      rbf0->setChecked( true );
      rbf1->setChecked( false );
      break;
  }

  slotFilterChanged( newFilter );
}
