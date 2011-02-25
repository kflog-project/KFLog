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
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "igc3ddialog.h"
#include "mapcalc.h"

Igc3DDialog::Igc3DDialog(QWidget *parent) :
  QDialog(parent)
{
  setObjectName( "Igc3DDialog" );
  setWindowTitle( tr("IGC - 3D View") );
  setModal( false );
  setSizeGripEnabled( true );
  setAttribute( Qt::WA_DeleteOnClose );

  Igc3DView* igc3dView = new Igc3DView(this);
  igc3dView->setMinimumWidth( 500 );
  igc3dView->setMinimumHeight( 650 );

  connect( this, SIGNAL(flightChanged()), igc3dView, SLOT(slotShowFlight()) );

  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin(0);
  layout->addWidget( igc3dView );

  // get the current ViewState from igc3dView and change its values
  Igc3DViewState* vs = igc3dView->getState();
  vs->height = 650;
  vs->width = 500;
}

Igc3DDialog::~Igc3DDialog()
{
}

void Igc3DDialog::showEvent( QShowEvent* event )
{
  QDialog::showEvent( event );
  QString help = igc3dView->getHelp();
  emit igc3dHelp( help );
}

void Igc3DDialog::resizeEvent(QResizeEvent* event)
{
  QDialog::resizeEvent(event);
}

void Igc3DDialog::slotShowFlightData()
{
  emit flightChanged();
}
