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
#include <qstringlist.h>

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

  // create group box for area filter
  QGroupBox *grp2 = new QGroupBox(2, QGroupBox::Horizontal, i18n("Area"), this);

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

  // create group box for radius filter
  QGroupBox *grp3 = new QGroupBox(2, QGroupBox::Horizontal, i18n("Radius"), this);
  new QLabel(i18n("Position"), grp3);
  grp3->addSpace(0);
  new QLabel(i18n("Lat"), grp3);
  new QLabel(i18n("Long"), grp3);
  posLat = new LatEdit(grp3);
  posLong = new LongEdit(grp3);
  new QLabel(i18n("Radius (km)"), grp3);
  radius = new QComboBox(true, grp3);
  QStringList l;
  l << "10" << "50" << "100" << "300" << "500" << "1000";
  radius->insertStringList(l);

  vbox1->addWidget(grp1);
  vbox1->addStretch();

  vbox2->addWidget(grp2);
  vbox2->addWidget(grp3);

  hbox->addLayout(vbox1);
  hbox->addLayout(vbox2);

  top->addLayout(hbox);

  QPushButton *b = new QPushButton(i18n("&Clear"), this);
  connect(b, SIGNAL(clicked()), this, SLOT(slotClear()));
  buttons->addWidget(b);
  buttons->addStretch();
  b = new QPushButton(i18n("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), this, SLOT(accept()));
  buttons->addWidget(b);
  b = new QPushButton(i18n("&Cancel"), this);
  connect(b, SIGNAL(clicked()), this, SLOT(reject()));
  buttons->addWidget(b);

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
/** reset all dialog items to default values */
void WaypointImpFilterDialog::slotClear()
{
  useAll->setChecked(true);
  airports->setChecked(false);
  gliderSites->setChecked(false);
  otherSites->setChecked(false);
  outlanding->setChecked(false);
  obstacle->setChecked(false);
  landmark->setChecked(false);
  station->setChecked(false);

  slotChangeUseAll();

  fromLat->clear();
  fromLong->clear();
  toLat->clear();
  toLong->clear();
  posLat->clear();
  posLong->clear();

  radius->setCurrentItem(0);

}
