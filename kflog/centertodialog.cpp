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

#include <klocale.h>

#include <qgrid.h>
#include <qlabel.h>
#include <qapplication.h>

CenterToDialog::CenterToDialog(QWidget *parent, const char *name )
  : KDialogBase(parent, name)
{
  setCaption(i18n("Center to ..."));

  QGrid* page = this->makeGridMainWidget(2, QGrid::Horizontal);

  new QLabel( i18n("Latitude"), page, "captionLat");
  latE = new LatEdit(page, "latitude");
  latE->setMinimumWidth(150);

  new QLabel( i18n("Longitude"), page, "captionLong");
  longE = new LongEdit(page, "longitude");
  longE->setMinimumWidth(150);

  connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
  connect(latE, SIGNAL(returnPressed()), this, SLOT(slotOk()));
  connect(longE, SIGNAL(returnPressed()), this, SLOT(slotOk()));
  
  showButtonApply(false);
  latE->setFocus();
  
  disableResize();
}

CenterToDialog::~CenterToDialog()
{

}

void CenterToDialog::slotOk()
{
  hide();
  qApp->processEvents();
  emit centerTo(MapContents::degreeToNum(latE->text()),
    MapContents::degreeToNum(longE->text()));
  close();
}
