/***********************************************************************
**
**   waypointimpfilterdialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "waypointimpfilterdialog.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>

WaypointImpFilterDialog::WaypointImpFilterDialog(QWidget *parent, const char *name)
 : KDialog(parent, name, true)
{
  QVBoxLayout *top = new QVBoxLayout(this, 5);
  QHBoxLayout *buttons = new QHBoxLayout();
  QHBoxLayout *hbox = new QHBoxLayout();
  QVBoxLayout *vbox1 = new QVBoxLayout();
  QVBoxLayout *vbox2 = new QVBoxLayout();

  setCaption(i18n("Filter waypoints"));

  // create non-exclusive buttongroup for type filter
  QButtonGroup *grp1 = new QButtonGroup(2, QGroupBox::Horizontal, i18n("Type"), this);
  // create group box for area filter
  QGroupBox *grp2 = new QGroupBox(2, QGroupBox::Horizontal, i18n("Area"), this);

  grp1->setExclusive(false);

  // insert checkboxes
  useAll = new QCheckBox(i18n("&Use all"), grp1);
  useAll->setChecked(true);
  grp1->addSpace(0);
  airports = new QCheckBox(i18n("&Airports"), grp1);
  gliderSites = new QCheckBox(i18n("&Glider sites"), grp1);
  otherSites = new QCheckBox(i18n("&Other sites"), grp1);
  outlanding = new QCheckBox(i18n("Ou&tlanding fields"), grp1);
  obstacle = new QCheckBox(i18n("O&bstacles"), grp1);
  landmark = new QCheckBox(i18n("&Landmarks"), grp1);
  station = new QCheckBox(i18n("&Stations"), grp1);

  new QLabel(i18n("From"), grp2);
  grp2->addSpace(0);
  new QLabel(i18n("Lat"), grp2);
  new QLabel(i18n("Long"), grp2);
  fromLat = new LatEdit(grp2);
  fromLong = new LongEdit(grp2);
  new QLabel(i18n("To"), grp2);
  grp2->addSpace(0);
  new QLabel(i18n("Lat"), grp2);
  new QLabel(i18n("Long"), grp2);
  toLat = new LatEdit(grp2);
  toLong = new LongEdit(grp2);

  vbox1->addWidget(grp1);
  vbox1->addStretch();

  vbox2->addWidget(grp2);
  vbox2->addStretch();

  hbox->addLayout(vbox1);
  hbox->addLayout(vbox2);

  top->addLayout(hbox);

  buttons->addStretch();
  QPushButton *ok = new QPushButton(i18n("&Ok"), this);
  ok->setDefault(true);
  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  buttons->addWidget(ok);
  ok = new QPushButton(i18n("&Cancel"), this);
  connect(ok, SIGNAL(clicked()), this, SLOT(reject()));
  buttons->addWidget(ok);

  top->addLayout(buttons);
  connect(useAll, SIGNAL(clicked()), this, SLOT(slotChangeUseAll()));
  slotChangeUseAll();
}

WaypointImpFilterDialog::~WaypointImpFilterDialog()
{
}

void WaypointImpFilterDialog::slotChangeUseAll()
{
  bool show = !useAll->isChecked();

  airports->setEnabled(show);
  gliderSites->setEnabled(show);
  otherSites->setEnabled(false);
  outlanding->setEnabled(false);
  obstacle->setEnabled(false);
  landmark->setEnabled(false);
  station->setEnabled(false);
}
