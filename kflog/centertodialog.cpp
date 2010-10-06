/***********************************************************************
**
**   centertodialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Andree Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "centertodialog.h"
#include "mapcontents.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwidget.h>

CenterToDialog::CenterToDialog(QWidget *parent, const char *name )
  : QDialog(parent, name)
{
  setCaption(tr("Center to ..."));

  QGridLayout* grid = new QGridLayout(this, 3, 3, 3);

  grid->addWidget(new QLabel( tr("Latitude"), this, "captionLat"), 0, 0);
  latE = new LatEdit(this, "latitude");
  latE->setMinimumWidth(150);
  grid->addMultiCellWidget(latE, 0, 0, 1, 2);

  grid->addWidget(new QLabel( tr("Longitude"), this, "captionLong"), 1, 0);
  longE = new LongEdit(this, "longitude");
  longE->setMinimumWidth(150);
  grid->addMultiCellWidget(longE, 1, 1, 1, 2);

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
  hide();
  qApp->processEvents();
  emit centerTo(MapContents::degreeToNum(latE->text()), MapContents::degreeToNum(longE->text()));
  close();
}
