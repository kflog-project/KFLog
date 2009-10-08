/***********************************************************************
**
**   igc3ddialog.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by the KFLog-Team
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "igc3ddialog.h"

// Qt headers
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qvaluelist.h>

// Application headers
#include "mapcalc.h"

Igc3DDialog::Igc3DDialog(QWidget *parent)
: QDialog(parent, "Igc3D", false)
{
  setCaption( tr("KFLog - 3D View") );
  setWFlags(getWFlags() | WStyle_StaysOnTop);

  Igc3DView* igc3dView = new Igc3DView(this);
  connect(this, SIGNAL(flightChanged()), igc3dView,
      SLOT(slotShowFlight()));

  QBoxLayout * layout = new QHBoxLayout( this, 0, -1, "horizontal" );
//  QLabel* label = new QLabel( this, 0, 0 );

	igc3dView->setMinimumWidth( 500 );
	igc3dView->setMinimumHeight( 650 );

  // get the current ViewState from igc3dView and change its values
  Igc3DViewState* vs = new Igc3DViewState();
  vs = igc3dView->getState();
	vs->height = 650;
  vs->width = 500;

//  label->setText( tr("  Toolbar will\n  go here.") );
//  label->setFixedWidth( label->sizeHint().width() + 10 );

	layout->addWidget( igc3dView );
//	layout->addWidget( label );
	
  show();
}

Igc3DDialog::~Igc3DDialog()
{
  // Cursor löschen
//  emit(showCursor(QPoint(-100,-100), QPoint(-100,-100)));
}

void Igc3DDialog::resizeEvent(QResizeEvent* event)
{
//  warning("Igc3DDialog::resizeEvent");
  QDialog::resizeEvent(event);

//  if(flightList->count())
//      slotShowFlightData(combo_flight->currentItem());
}



void Igc3DDialog::slotShowFlightData()
{
//  warning("Igc3DDialog::slotShowFlightData");
//  this->setCaption(tr("FlightIgc3D:") + flightList->at(n)->getPilot()
//                    + "  " + flightList->at(n)->getDate().toString());

  emit flightChanged();
}



void Igc3DDialog::hide()
{
//warning("Igc3DDialog::hide()");
  this->Igc3DDialog::~Igc3DDialog();
}
