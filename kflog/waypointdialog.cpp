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
#include "translationlist.h"
#include "kflog.h"
#include "mapcontents.h"
#include "airport.h"

#include <qlayout.h>
#include <qlabel.h>

#include <kseparator.h>
#include <kapp.h>

extern TranslationList surfaces;
extern TranslationList waypointTypes;

WaypointDialog::WaypointDialog(QWidget *parent, const char *name)
 : QDialog(parent, name, true)
{
  setCaption(tr("Waypoint definition"));
  __initDialog();

  TranslationElement *te;
  // init comboboxes
  for (te = waypointTypes.first(); te != 0; te = waypointTypes.next()) {
    waypointType->insertItem(te->text);
  }

  for (te = surfaces.first(); te != 0; te = surfaces.next()) {
    surface->insertItem(te->text);
  }
  setSurface(Airport::NotSet);
  setWaypointType(BaseMapElement::Landmark);
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

  applyButton = new QPushButton(tr("&Apply"), this);
  connect(applyButton, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  buttons->addWidget(applyButton);
  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  connect(b, SIGNAL(clicked()), SLOT(accept()));
  buttons->addWidget(b);
  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  name = new QLineEdit(this);
  name->setFocus();
  layout->addWidget(name, 1, 0);
  l = new QLabel(name, QString("%1:").arg(tr("&Name")), this);
  layout->addWidget(l, 0, 0);

  description = new QLineEdit(this);
  layout->addWidget(description, 1, 1);
  l = new QLabel(description, QString("%1:").arg(tr("&Description")), this);
  layout->addWidget(l, 0, 1);

  waypointType = new KComboBox(false, this);
  waypointType->setCompletionMode(KGlobalSettings::CompletionAuto);
  layout->addWidget(waypointType, 1, 2);
  l = new QLabel(waypointType, QString("%1:").arg(tr("&Type")), this);
  layout->addWidget(l, 0, 2);

  latitude = new LatEdit(this);
  layout->addWidget(latitude, 3, 0);
  l = new QLabel(latitude, QString("%1:").arg(tr("&Latitude")), this);
  layout->addWidget(l, 2, 0);

  longitude = new LongEdit(this);
  layout->addWidget(longitude, 3, 1);
  l = new QLabel(longitude, QString("%1:").arg(tr("L&ongitude")), this);
  layout->addWidget(l, 2, 1);

  elevation = new QLineEdit(this);
  layout->addWidget(elevation, 3, 2);
  l = new QLabel(elevation, tr("%1 (m):").arg(tr("&Elevation")), this);
  layout->addWidget(l, 2, 2);

  icao = new QLineEdit(this);
  layout->addWidget(icao, 5, 0);
  l = new QLabel(icao, tr("&ICAO:"), this);
  layout->addWidget(l, 4, 0);

  frequency = new QLineEdit(this);
  layout->addWidget(frequency, 5, 1);
  l = new QLabel(frequency, QString("%1:").arg(tr("&Frequency")), this);
  layout->addWidget(l, 4, 1);

  isLandable = new QCheckBox(tr("L&andable"), this);
  layout->addWidget(isLandable, 5, 2);

  runway = new QLineEdit(this);
  layout->addWidget(runway, 7, 0);
  l = new QLabel(runway, QString("%1:").arg(tr("&Runway")), this);
  layout->addWidget(l, 6, 0);

  length = new QLineEdit(this);
  layout->addWidget(length, 7, 1);
  l = new QLabel(length, tr("%1 (m):").arg(tr("Len&gth")), this);
  layout->addWidget(l, 6, 1);

  surface = new KComboBox(false, this);
  layout->addWidget(surface, 7, 2);
  l = new QLabel(surface, QString("%1:").arg(tr("&Surface")), this);
  layout->addWidget(l, 6, 2);

  comment = new QLineEdit(this);
  layout->addMultiCellWidget(comment, 9, 9, 0, 2);
  l = new QLabel(comment, QString("%1:").arg(tr("&Comment")), this);
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
  elevation->clear();
  icao->clear();
  frequency->clear();
  runway->clear();
  length->clear();
  surface->setCurrentItem(-1);
  comment->clear();
  latitude->clear();
  longitude->clear();
  isLandable->setChecked(false);
  setWaypointType(BaseMapElement::Landmark);
}
/** No descriptions */
void WaypointDialog::slotAddWaypoint()
{
  QString text;
  extern MapContents _globalMapContents;

  //if (!name->text().isEmpty()) { we accept an empty name. A syntetic one will be created.
    // insert a new waypoint to current catalog
    Waypoint *w = new Waypoint;
    w->name = name->text().upper();
    w->description = description->text();
    w->type = getWaypointType();
    w->origP.setLat(_globalMapContents.degreeToNum(latitude->text()));
    w->origP.setLon(_globalMapContents.degreeToNum(longitude->text()));
    w->elevation = elevation->text().toInt();
    w->icao = icao->text().upper();
    w->frequency = frequency->text().toDouble();
    text = runway->text();
    if (!text.isEmpty()) {
      w->runway = text.toInt();
    }

    text = length->text();
    if (!text.isEmpty()) {
      w->length = text.toInt();
    }
    w->surface = getSurface();
    w->isLandable = isLandable->isChecked();
    w->comment = comment->text();

    emit addWaypoint(w);
    // clear should not be called when apply was pressed ...
    // and when ok is pressed, the dialog is closed anyway.
    // clear();
  //}
}

/** return internal type of waypoint */
int WaypointDialog::getWaypointType()
{
  int type = waypointType->currentItem();

  if (type != -1) {
    type = waypointTypes.at(type)->id;
  }

  return type;
}

/** return interna type of surface */
int WaypointDialog::getSurface()
{
  int s = surface->currentItem();

  if (s != -1) {
    s = surfaces.at(s)->id;
  }

  return s;
}

/** set waypoint type in combo box
translate internal id to index */
void WaypointDialog::setWaypointType(int type)
{
  if (type != -1) {
    type = waypointTypes.idxById(type);
  }
  waypointType->setCurrentItem(type);
}

/** set surface type in combo box
translate internal id to index */
void WaypointDialog::setSurface(int s)
{
  if (s != -1) {
    s = surfaces.idxById(s);
  }
  surface->setCurrentItem(s);
}
/** No descriptions */
void WaypointDialog::enableApplyButton(bool enable)
{
  applyButton->setEnabled(enable);
}
