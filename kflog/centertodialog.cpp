/***********************************************************************
**
**   centertodialog.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Andreé Somers, 2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "centertodialog.h"
#include "mapcontents.h"

CenterToDialog::CenterToDialog( QWidget *parent ) : QDialog(parent)
{
  setObjectName( "CenterToDialog" );
  setWindowTitle(tr("Center to ..."));
  setAttribute( Qt::WA_DeleteOnClose );

  QGridLayout* grid = new QGridLayout( this );
  grid->setMargin( 3 );

  grid->addWidget(new QLabel( tr("Latitude"), this ), 0, 0);

  latE = new LatEdit(this);
  latE->setMinimumWidth(150);
  grid->addWidget( latE, 0, 1, 1, 2 );

  grid->addWidget(new QLabel( tr("Longitude"), this ), 1, 0);

  longE = new LongEdit(this);
  longE->setMinimumWidth(150);
  grid->addWidget( longE, 1, 1, 1, 2);

  QPushButton *okButton = new QPushButton( "&Ok", this );
  grid->addWidget(okButton, 2, 1);
  QPushButton *cancelButton = new QPushButton( "&Cancel", this );
  grid->addWidget(cancelButton, 2, 2);

  connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(latE, SIGNAL(returnPressed()), this, SLOT(slotOk()));
  connect(longE, SIGNAL(returnPressed()), this, SLOT(slotOk()));

  latE->setFocus();
}

CenterToDialog::~CenterToDialog()
{
}

void CenterToDialog::slotOk()
{
  setVisible( false );
  emit centerTo( latE->KFLogDegree(), longE->KFLogDegree() );
  close();
}
