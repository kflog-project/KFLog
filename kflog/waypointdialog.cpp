/***********************************************************************
**
**   waypointdialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "waypointdialog.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kseparator.h>

WaypointDialog::WaypointDialog(QWidget *parent, const char *name)
 : KDialog(parent, name, true)
{
  setCaption(i18n("Waypoint definition"));
  __initDialog();
}

WaypointDialog::~WaypointDialog()
{
}

/** No descriptions */
void WaypointDialog::__initDialog()
{
  QLabel *l;
  QPushButton *b;

  QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
  QGridLayout *layout = new QGridLayout(11, 3, 5);
  QHBoxLayout *buttons = new QHBoxLayout(10);

  buttons->addStretch();

  b = new QPushButton(i18n("&Add"), this);
  connect(b, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  buttons->addWidget(b);
  b = new QPushButton(i18n("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(accept()));
  buttons->addWidget(b);
  b = new QPushButton(i18n("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  name = new QLineEdit(this);
  name->setFocus();
  layout->addWidget(name, 1, 0);
  l = new QLabel(name, i18n("%1:").arg("&Name"), this);
  layout->addWidget(l, 0, 0);

  description = new QLineEdit(this);
  layout->addWidget(description, 1, 1);
  l = new QLabel(description, i18n("%1:").arg("&Description"), this);
  layout->addWidget(l, 0, 1);

  waypointType = new KComboBox(false, this);
  waypointType->setCompletionMode(KGlobalSettings::CompletionAuto);
  layout->addWidget(waypointType, 1, 2);
  l = new QLabel(waypointType, i18n("%1:").arg("&Type"), this);
  layout->addWidget(l, 0, 2);

  latitude = new LatEdit(this);
  layout->addWidget(latitude, 3, 0);
  l = new QLabel(latitude, i18n("%1:").arg("&Latitude"), this);
  layout->addWidget(l, 2, 0);

  longitude = new LongEdit(this);
  layout->addWidget(longitude, 3, 1);
  l = new QLabel(longitude, i18n("%1:").arg("L&ongitude"), this);
  layout->addWidget(l, 2, 1);

  elevation = new QLineEdit(this);
  layout->addWidget(elevation, 3, 2);
  l = new QLabel(elevation, i18n("%1 (m):").arg("&Elevation"), this);
  layout->addWidget(l, 2, 2);

  icao = new QLineEdit(this);
  layout->addWidget(icao, 5, 0);
  l = new QLabel(icao, i18n("&ICAO:"), this);
  layout->addWidget(l, 4, 0);

  frequency = new QLineEdit(this);
  layout->addWidget(frequency, 5, 1);
  l = new QLabel(frequency, i18n("%1:").arg("&Frequency"), this);
  layout->addWidget(l, 4, 1);

  runway = new QLineEdit(this);
  layout->addWidget(runway, 7, 0);
  l = new QLabel(runway, i18n("%1:").arg("&Runnway"), this);
  layout->addWidget(l, 6, 0);

  length = new QLineEdit(this);
  layout->addWidget(length, 7, 1);
  l = new QLabel(length, i18n("%1 (m):").arg("Len&gth"), this);
  layout->addWidget(l, 6, 1);

  surface = new KComboBox(false, this);
  layout->addWidget(surface, 7, 2);
  l = new QLabel(surface, i18n("%1:").arg("&Surface"), this);
  layout->addWidget(l, 6, 2);

  comment = new QLineEdit(this);
  layout->addMultiCellWidget(comment, 9, 9, 0, 2);
  l = new QLabel(comment, i18n("%1:").arg("&Comment"), this);
  layout->addWidget(l, 8, 0);

  topLayout->addLayout(layout);
  topLayout->addWidget(new KSeparator(this));
  topLayout->addLayout(buttons);
}

/** clear all entries */
void WaypointDialog::clear()
{
  name->clear();
  description->clear();
  waypointType->setCurrentItem(-1);
  elevation->clear();
  icao->clear();
  frequency->clear();
  runway->clear();
  length->clear();
  surface->setCurrentItem(-1);
  comment->clear();
  latitude->clear();
  longitude->clear();
}
/** No descriptions */
void WaypointDialog::slotAddWaypoint()
{
  emit addWaypoint();
  clear();
}
