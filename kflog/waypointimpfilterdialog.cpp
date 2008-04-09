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
#include "mapcontents.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qradiobutton.h>
#include <qlistbox.h>

#include <klocale.h>

extern MapContents _globalMapContents;
extern MapMatrix _globalMapMatrix;

WaypointImpFilterDialog::WaypointImpFilterDialog(QWidget *parent, const char *name)
 : KDialog(parent, name, true),
 center(0)
{
  QVBoxLayout *top = new QVBoxLayout(this, 5);
  QHBoxLayout *buttons = new QHBoxLayout();
  QHBoxLayout *hbox = new QHBoxLayout();
  QVBoxLayout *vbox1 = new QVBoxLayout();
  QVBoxLayout *vbox2 = new QVBoxLayout();

  setCaption(i18n("Filter waypoints"));

  // create non-exclusive buttongroup for type filter
  QButtonGroup *grp1 = new QButtonGroup(1, QGroupBox::Horizontal, i18n("Type"), this);

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
  QGroupBox *grp3 = new QGroupBox(i18n("Radius"), this);
  QVBoxLayout *vbox3 = new QVBoxLayout(grp3, 10);

  // create an invisible bouttengroup
  QButtonGroup *bg = new QButtonGroup(this);
  bg->hide();
  connect(bg, SIGNAL(clicked(int)), SLOT(selectRadius(int)));

  QGridLayout *grid = new QGridLayout(4, 5);
  QRadioButton *rb = new QRadioButton(i18n("Position"), grp3);
  rb->setChecked(true);
  bg->insert(rb, CENTER_POS);
  grid->addWidget(rb, 0, 0);
  rb = new QRadioButton(i18n("Homesite"), grp3);
  bg->insert(rb, CENTER_HOMESITE);
  grid->addWidget(rb, 1, 0);
  rb = new QRadioButton(i18n("Center of map"), grp3);
  bg->insert(rb, CENTER_MAP);
  grid->addWidget(rb, 2, 0);
  rb = new QRadioButton(i18n("Airfield"), grp3);
  bg->insert(rb, CENTER_AIRPORT);
  grid->addWidget(rb, 3, 0);

  // create a grid layout for input fields
  grid->addWidget(new QLabel(i18n("Lat"), grp3), 0, 1);
  posLat = new LatEdit(grp3);
  grid->addWidget(posLat, 0, 2);
  grid->addWidget(new QLabel(i18n("Long"), grp3), 0, 3);
  posLong = new LongEdit(grp3);
  grid->addWidget(posLong, 0, 4);
  refAirport = new QComboBox(false, grp3);
  grid->addMultiCellWidget(refAirport, 3, 3, 1, 4);

  radius = new QComboBox(true, grp3);
  QStringList l;
  l << "10" << "50" << "100" << "300" << "500" << "1000";
  radius->insertStringList(l);

  vbox3->addSpacing(10);
  vbox3->addLayout(grid);
  vbox3->addWidget(new QLabel(i18n("Radius (km)"), grp3));
  vbox3->addWidget(radius);

  vbox1->addWidget(grp1);
  vbox1->addStretch();

  vbox2->addWidget(grp2);
  vbox2->addWidget(grp3);
  vbox2->addStretch();

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
  selectRadius(CENTER_POS);
}

WaypointImpFilterDialog::~WaypointImpFilterDialog()
{
}

void WaypointImpFilterDialog::slotChangeUseAll()
{
  bool show = !useAll->isChecked();

  airports->setEnabled(show);
  gliderSites->setEnabled(show);
  otherSites->setEnabled(show);
  outlanding->setEnabled(show);
  obstacle->setEnabled(show);
  landmark->setEnabled(show);
  station->setEnabled(show);
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
/** No descriptions */
void WaypointImpFilterDialog::selectRadius(int n)
{
  center = n;

  switch (center) {
  case CENTER_POS:
    posLat->setEnabled(true);
    posLong->setEnabled(true);
    refAirport->setEnabled(false);
    break;
  case CENTER_HOMESITE:
    // fall through
  case CENTER_MAP:
    posLat->setEnabled(false);
    posLong->setEnabled(false);
    refAirport->setEnabled(false);
    break;
  case CENTER_AIRPORT:
    posLat->setEnabled(false);
    posLong->setEnabled(false);
    refAirport->setEnabled(true);
    break;
  }
}
/** No descriptions */
int WaypointImpFilterDialog::getCenterRef()
{
  return center;
}

void WaypointImpFilterDialog::polish()
{
  int searchList[] = {MapContents::GliderSiteList, MapContents::AirportList};

  KDialog::polish();
  for (int l = 0; l < 2; l++) {
    for(unsigned int loop = 0; loop < _globalMapContents.getListLength(searchList[l]);
      loop++) {
        SinglePoint *hitElement = (SinglePoint *)_globalMapContents.getElement(searchList[l], loop);
        refAirport->insertItem(hitElement->getName());
        airportDict.insert(hitElement->getName(), hitElement);
      }
  }
  refAirport->listBox()->sort();
}

WGSPoint WaypointImpFilterDialog::getAirportRef()
{
  QString s = refAirport->currentText();
  SinglePoint *sp = airportDict.find(s);
  WGSPoint p = sp->getWGSPosition();
  return p;
}
