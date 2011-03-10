/***********************************************************************
**
**   waypointdialog.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "basemapelement.h"
#include "runway.h"
#include "waypointdialog.h"
#include "wgspoint.h"

WaypointDialog::WaypointDialog( QWidget *parent ) :
  QDialog(parent),
  edit(false)
{
  setWindowModality( Qt::WindowModal );
  //setAttribute( Qt::WA_DeleteOnClose );
  setSizeGripEnabled( true );

  __initDialog();

  waypointType->addItems( BaseMapElement::getSortedTranslationList() );
  setWaypointType( BaseMapElement::Landmark );

  surface->addItems( Runway::getSortedTranslationList() );
  setSurface( Runway::Unknown );
}

WaypointDialog::~WaypointDialog()
{
}

/** No descriptions */
void WaypointDialog::__initDialog()
{
  QLabel *l1, *l2;

  QRegExp rxInt("[0-9]*");
  QValidator *intRxValidator = new QRegExpValidator(rxInt, this);

  //---------------------------------------------------------------------------
  QGridLayout *layout = new QGridLayout;
  layout->setSpacing( 7 );

  int row = 0;

  l1 = new QLabel( QString( "%1:" ).arg( tr( "&Name" ) ));
  layout->addWidget( l1, row, 0 );

  l2 = new QLabel( QString( "%1:" ).arg( tr( "&Description" ) ));
  layout->addWidget( l2, row, 2 );
  row++;

  name = new QLineEdit;
  name->setFocus();
  name->setMaxLength(8); // limit name to 8 characters

  connect( name, SIGNAL(textEdited( const QString& )),
           this, SLOT(slotTextEdited( const QString& )) );

  layout->addWidget(name, row, 0);

  description = new QLineEdit;
  layout->addWidget(description, row, 2);
  row++;

  l1->setBuddy( name );
  l2->setBuddy( description );

  //---
  l1 = new QLabel( QString( "%1:" ).arg( tr( "&Type" ) ));
  layout->addWidget( l1, row, 0 );

  l2 = new QLabel(tr("%1 (m):").arg(tr("&Elevation")));
  layout->addWidget(l2, row, 2);
  row++;

  waypointType = new QComboBox;
  waypointType->setAutoCompletion(true);
  layout->addWidget(waypointType, row, 0);

  elevation = new QLineEdit;
  elevation->setValidator( intRxValidator );
  layout->addWidget(elevation, row, 2);
  row++;

  l1->setBuddy( waypointType );
  l2->setBuddy( elevation );

  //----
  l1 = new QLabel(QString("%1:").arg(tr("&Latitude")));
  layout->addWidget(l1, row, 0);

  l2 = new QLabel(QString("%1:").arg(tr("L&ongitude")));
  layout->addWidget(l2, row, 2);
  row++;

  latitude = new LatEdit;
  layout->addWidget(latitude, row, 0);

  longitude = new LongEdit;
  layout->addWidget(longitude, row, 2);
  row++;

  l1->setBuddy( latitude );
  l2->setBuddy( longitude );

  //----
  l1 = new QLabel(tr("&ICAO:"));
  layout->addWidget(l1, row, 0);

  l2 = new QLabel(QString("%1:").arg(tr("&Frequency")));
  layout->addWidget(l2, row, 2);
  row++;

  icao = new QLineEdit;
  layout->addWidget(icao, row, 0);

  frequency = new QLineEdit;

  // Limit frequency range to VORs and speech bands.
  QDoubleValidator* doubleValidator = new QDoubleValidator ( 108.0, 140.0, 3, this );
  frequency->setValidator( doubleValidator );
  layout->addWidget(frequency, row, 2);
  row++;

  l1->setBuddy( icao );
  l2->setBuddy( frequency );

  //---
  l1 = new QLabel(QString("%1:").arg(tr("&Runway")));
  layout->addWidget(l1, row, 0);

  l2 = new QLabel(tr("%1 (m):").arg(tr("Len&gth")));
  layout->addWidget(l2, row, 2);
  row++;

  runway = new QComboBox;

  // init combo box with headings
  runway->addItem( "--" );

  for( int i = 1; i <= 36; i++ )
    {
      QString item;
      item = QString("%1").arg(i, 2, 10, QLatin1Char('0'));

      runway->addItem( item );
    }

  runway->setCurrentIndex(0);

  layout->addWidget( runway, row, 0 );

  length = new QLineEdit;
  length->setValidator( intRxValidator );
  layout->addWidget(length, row, 2);
  row++;

  l1->setBuddy( runway );
  l2->setBuddy( length );

  //---
  l1 = new QLabel(QString("%1:").arg(tr("&Surface")));
  layout->addWidget(l1, row, 0);
  row++;

  surface = new QComboBox;
  layout->addWidget(surface, row, 0);

  isLandable = new QCheckBox(tr("L&andable"));
  layout->addWidget(isLandable, row, 2);
  row++;

  l1->setBuddy( surface );

  //---
  l1 = new QLabel(QString("%1:").arg(tr("&Comment")));
  layout->addWidget(l1, row, 0);
  row++;

  comment = new QLineEdit;
  layout->addWidget(comment, row, 0, 1, 3);
  row++;

  l1->setBuddy( comment );

  layout->setColumnMinimumWidth( 1, 20 );

  //---------------------------------------------------------------------------
  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->setSpacing( 10 );
  buttonsLayout->addStretch( 10 );

  applyButton = new QPushButton(tr("&Apply"));
  connect(applyButton, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  buttonsLayout->addWidget(applyButton);

  QPushButton *pb = new QPushButton(tr("&Ok"));
  pb->setDefault(true);
  connect(pb, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  connect(pb, SIGNAL(clicked()), SLOT(accept()));
  buttonsLayout->addWidget(pb);

  pb = new QPushButton(tr("&Cancel"), this);
  connect(pb, SIGNAL(clicked()), SLOT(reject()));
  buttonsLayout->addWidget(pb);

  //---------------------------------------------------------------------------
  QVBoxLayout *topLayout = new QVBoxLayout;
  topLayout->setSpacing( 10 );
  topLayout->addLayout(layout);
  topLayout->addLayout(buttonsLayout);

  setLayout( topLayout );

  resize( minimumSizeHint().width() + 10, minimumSizeHint().height() + 10 );
}

/** clear all entries */
void WaypointDialog::clear()
{
  name->clear();
  description->clear();
  elevation->clear();
  icao->clear();
  frequency->clear();
  runway->setCurrentIndex(0);
  length->clear();
  surface->setCurrentIndex( surface->findText(Runway::item2Text(Runway::Unknown)) );
  comment->clear();
  latitude->setKFLogDegree(0);
  longitude->setKFLogDegree(0);
  isLandable->setChecked(false);
  setWaypointType(BaseMapElement::Landmark);
  edit = false;
}

/** No descriptions */
void WaypointDialog::slotAddWaypoint()
{
  if( edit )
    {
      // Waypoint was only edited.
      return;
    }

  QString text;

  // insert a new waypoint to current catalog
  Waypoint *w = new Waypoint;
  w->name = name->text().left(8).toUpper();
  w->description = description->text();
  w->type = getWaypointType();
  w->origP.setLat(latitude->KFLogDegree());
  w->origP.setLon(longitude->KFLogDegree());
  w->elevation = elevation->text().toInt();
  w->icao = icao->text().toUpper();
  w->frequency = frequency->text().toDouble();
  w->runway.first = runway->currentIndex();

  if( runway->currentIndex() > 0 )
    {
      int rw1 = w->runway.first;

      w->runway.second = ((rw1 > 18) ? rw1 - 18 : rw1 + 18 );
    }

  text = length->text();

  if( !text.isEmpty() )
    {
      w->length = text.toInt();
    }

  w->surface = getSurface();
  w->isLandable = isLandable->isChecked();
  w->comment = comment->text();

  emit addWaypoint(w);
  // clear should not be called when apply was pressed ...
  // and when ok is pressed, the dialog is closed anyway.
  // clear();
}

/**
 * Called to make all text to upper cases.
 */
void WaypointDialog::slotTextEdited( const QString& text )
{
  // Change edited text to upper cases
  name->setText( text.toUpper() );
}

/** return internal type of waypoint */
int WaypointDialog::getWaypointType()
{
  return BaseMapElement::text2Item( waypointType->currentText() );
}

/** return internal type of surface */
enum Runway::SurfaceType WaypointDialog::getSurface()
{
  return (enum Runway::SurfaceType) Runway::text2Item( surface->currentText() );
}

/** set waypoint type in combo box
translate internal id to index */
void WaypointDialog::setWaypointType(int type)
{
  waypointType->setCurrentIndex( waypointType->findText(BaseMapElement::item2Text(type)) );
}

/** set surface type in combo box
translate internal id to index */
void WaypointDialog::setSurface( enum Runway::SurfaceType st )
{
  surface->setCurrentIndex( surface->findText(Runway::item2Text(st)) );
}

/** No descriptions */
void WaypointDialog::enableApplyButton(bool enable)
{
  applyButton->setEnabled(enable);
}
