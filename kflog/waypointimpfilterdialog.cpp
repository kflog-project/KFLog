/***********************************************************************
**
**   waypointimpfilterdialog.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "mapcontents.h"
#include "waypointimpfilterdialog.h"

#include <QtGui>

extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

WaypointImpFilterDialog::WaypointImpFilterDialog( QWidget *parent ) :
 QDialog(parent),
 center(0)
{
  qDebug() << "WaypointImpFilterDialog::WaypointImpFilterDialog";

  setObjectName( "WaypointImpFilterDialog" );
  setWindowTitle(tr("Waypoint Filter"));
  //setAttribute( Qt::WA_DeleteOnClose );
  setModal( true );

  // create checkboxes
  useAll       = new QCheckBox(tr("&Use all"));
  airfields    = new QCheckBox(tr("&Airfields"));
  gliderfields = new QCheckBox(tr("&Gliderfields"));
  outlandings  = new QCheckBox(tr("Ou&tlandings"));
  otherSites   = new QCheckBox(tr("&Other sites"));
  obstacles    = new QCheckBox(tr("O&bstacles"));
  landmarks    = new QCheckBox(tr("&Landmarks"));
  stations     = new QCheckBox(tr("&Stations"));

  useAll->setChecked(true);
  connect(useAll, SIGNAL(clicked()), this, SLOT(slotChangeUseAll()));

  airfields->setChecked(false);
  gliderfields->setChecked(false);
  outlandings->setChecked(false);
  otherSites->setChecked(false);
  obstacles->setChecked(false);
  landmarks->setChecked(false);
  stations->setChecked(false);

  QVBoxLayout *typeLayout = new QVBoxLayout;
  typeLayout->setSpacing( 10 );
  typeLayout->addWidget( useAll );
  typeLayout->addSpacing( 10 );
  typeLayout->addWidget( airfields );
  typeLayout->addWidget( gliderfields );
  typeLayout->addWidget( outlandings );
  typeLayout->addWidget( otherSites );
  typeLayout->addWidget( obstacles );
  typeLayout->addWidget( landmarks );
  typeLayout->addWidget( stations );

  QGroupBox* typeGroup = new QGroupBox( tr("Type") );
  typeGroup->setLayout( typeLayout );

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

  QGroupBox* fromGroup = new QGroupBox( tr("Area From") );
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

  QGroupBox* toGroup = new QGroupBox( tr("Area to") );
  toGroup->setLayout( toGrid );

  //---------------------------------------------------------------------------
  rb0 = new QRadioButton(tr("Position"));
  rb1 = new QRadioButton(tr("Homesite"));
  rb2 = new QRadioButton(tr("Center of Map"));
  rb3 = new QRadioButton(tr("Airfield"));

  rb0->setChecked( false );
  rb1->setChecked( false );
  rb2->setChecked( true );
  rb3->setChecked( false );

  QButtonGroup* radiusButtonGroup = new QButtonGroup(this);
  radiusButtonGroup->setExclusive(true);
  radiusButtonGroup->addButton( rb0, CENTER_POS );
  radiusButtonGroup->addButton( rb1, CENTER_HOMESITE );
  radiusButtonGroup->addButton( rb2, CENTER_MAP );
  radiusButtonGroup->addButton( rb3, CENTER_AIRFIELD );

  connect( radiusButtonGroup, SIGNAL( buttonClicked(int)),
           this, SLOT(selectRadius(int)));

  radiusLat  = new LatEdit;
  radiusLong = new LongEdit;
  refAirfieldBox = new QComboBox;
  radiusUnit = new QLabel;

  connect( refAirfieldBox, SIGNAL(currentIndexChanged(const QString&)),
           this, SLOT(slotAirfieldRefChanged(const QString&)) );

  radius = new QComboBox;
  radius->setEditable( true );
  radius->setValidator( new QIntValidator(1, 10000, this) );
  QStringList itemList;
  itemList << "10" << "50" << "100" << "300" << "500" << "1000" << "2000";
  radius->addItems( itemList );
  radius->setCurrentIndex( 4 );

  QGridLayout *radiusGrid = new QGridLayout;
  radiusGrid->setSpacing(10);
  radiusGrid->addWidget( rb0, 0, 0 );
  radiusGrid->addWidget( new QLabel(tr("Lat:")), 0, 1 );
  radiusGrid->addWidget( radiusLat, 0, 2);
  radiusGrid->addWidget( new QLabel(tr("Lon:")), 0, 3 );
  radiusGrid->addWidget( radiusLong, 0, 4 );

  radiusGrid->addWidget( rb1, 1, 0 );
  radiusGrid->addWidget( rb2, 2, 0 );
  radiusGrid->addWidget( rb3, 3, 0 );
  radiusGrid->addWidget( refAirfieldBox, 3, 2 );

  radiusGrid->addWidget( radiusUnit, 4, 0 );
  radiusGrid->addWidget( radius, 4, 2 );
  radiusGrid->setColumnStretch( 5, 10 );

  QGroupBox* radiusGroup = new QGroupBox( tr("Radius") );
  radiusGroup->setLayout( radiusGrid );

  //---------------------------------------------------------------------------
  QHBoxLayout *buttonBox = new QHBoxLayout;

  QPushButton *b = new QPushButton(tr("&Clear"), this);
  connect(b, SIGNAL(clicked()), this, SLOT(slotClear()));
  buttonBox->addWidget(b);
  buttonBox->addStretch( 10 );

  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), this, SLOT(accept()));
  buttonBox->addWidget(b);

  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), this, SLOT(reject()));
  buttonBox->addWidget(b);

  //---------------------------------------------------------------------------
  QVBoxLayout *ftrBox = new QVBoxLayout;
  ftrBox->addWidget( fromGroup );
  ftrBox->addWidget( toGroup );
  ftrBox->addWidget( radiusGroup );

  QHBoxLayout *hBox = new QHBoxLayout;
  hBox->addWidget( typeGroup );
  hBox->addLayout( ftrBox );

  QVBoxLayout *top = new QVBoxLayout;
  top->addLayout( hBox );
  top->addLayout( buttonBox );

  setLayout( top );

  slotChangeUseAll();
  selectRadius(CENTER_HOMESITE);
}

WaypointImpFilterDialog::~WaypointImpFilterDialog()
{
  //qDebug() << "~WaypointImpFilterDialog()";
}

void WaypointImpFilterDialog::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // The unit can be changed in the meantime. Therefore we do update it here.
  QString distUnit = Distance::getUnitText();
  radiusUnit->setText( tr("Radius") + " (" + distUnit + "):" );

  // Load airfield data into combo box. Can be changed in the meantime.
  loadAirfieldComboBox();
}

void WaypointImpFilterDialog::slotChangeUseAll()
{
  bool show = !useAll->isChecked();

  airfields->setEnabled(show);
  gliderfields->setEnabled(show);
  otherSites->setEnabled(show);
  outlandings->setEnabled(show);
  obstacles->setEnabled(show);
  landmarks->setEnabled(show);
  stations->setEnabled(show);
}
/** reset all dialog items to default values */
void WaypointImpFilterDialog::slotClear()
{
  useAll->setChecked( true );
  airfields->setChecked( false );
  gliderfields->setChecked( false );
  otherSites->setChecked( false );
  outlandings->setChecked( false );
  obstacles->setChecked( false );
  landmarks->setChecked( false );
  stations->setChecked( false );

  slotChangeUseAll();

  fromLat->setKFLogDegree( 0 );
  fromLong->setKFLogDegree( 0 );
  toLat->setKFLogDegree( 0 );
  toLong->setKFLogDegree( 0 );
  radiusLat->setKFLogDegree( 0 );
  radiusLong->setKFLogDegree( 0 );

  rb2->setChecked( true );
  refAirfieldBox->setCurrentIndex( 0 );
  radius->setCurrentIndex( 4 );

  selectRadius(CENTER_HOMESITE);
}

/** No descriptions */
void WaypointImpFilterDialog::selectRadius(int n)
{
  center = n;

  switch (center)
  {
    case CENTER_POS:
      radiusLat->setEnabled(true);
      radiusLong->setEnabled(true);
      refAirfieldBox->setEnabled(false);
      break;
    case CENTER_HOMESITE:
      // fall through
    case CENTER_MAP:
      radiusLat->setEnabled(false);
      radiusLong->setEnabled(false);
      refAirfieldBox->setEnabled(false);
      break;
    case CENTER_AIRFIELD:
      radiusLat->setEnabled(false);
      radiusLong->setEnabled(false);
      refAirfieldBox->setEnabled(true);
      break;
    }
}
/** No descriptions */
int WaypointImpFilterDialog::getCenterRef()
{
  return center;
}

void WaypointImpFilterDialog::loadAirfieldComboBox()
{
  int searchList[] = { MapContents::GliderfieldList, MapContents::AirfieldList };

  airfieldDict.clear();
  refAirfieldBox->clear();

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
  refAirfieldBox->addItems( airfieldList );

  // try to find the last selection in the new content.
  int newIndex = refAirfieldBox->findText( airfieldRefTxt );

  if( newIndex != -1 )
    {
      // Try to find the last selection.
      refAirfieldBox->setCurrentIndex( newIndex );
    }
  else
    {
      refAirfieldBox->setCurrentIndex( 0 );
    }
}

WGSPoint WaypointImpFilterDialog::getAirfieldRef()
{
  QString s = refAirfieldBox->currentText();

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
