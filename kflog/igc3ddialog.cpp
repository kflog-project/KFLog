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

#include <QtGui>
#include <Qt3Support>

#include "igc3ddialog.h"
#include "mapcalc.h"

Igc3DDialog::Igc3DDialog(QWidget *parent) :
  QDialog(parent)
{
  setWindowTitle( tr("KFLog - 3D View") );
  setWindowFlags(windowFlags() | Qt::WStyle_StaysOnTop);
  setAttribute( Qt::WA_DeleteOnClose );

  Igc3DView* igc3dView = new Igc3DView(this);

  connect( this, SIGNAL(flightChanged()), igc3dView, SLOT(slotShowFlight()) );

  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin(0);
//  QLabel* label = new QLabel( this, 0, 0 );

  igc3dView->setMinimumWidth( 500 );
  igc3dView->setMinimumHeight( 650 );

  // get the current ViewState from igc3dView and change its values
  Igc3DViewState* vs = igc3dView->getState();
  vs->height = 650;
  vs->width = 500;

//  label->setText( tr("  Toolbar will\n  go here.") );
//  label->setFixedWidth( label->sizeHint().width() + 10 );

  layout->addWidget( igc3dView );
//layout->addWidget( label );
	
  setVisible(true);
}

Igc3DDialog::~Igc3DDialog()
{
  // Cursor l�schen
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
  emit flightChanged();
}
