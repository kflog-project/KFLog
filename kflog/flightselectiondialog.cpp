/***********************************************************************
**
**   flightselectiondialog.cpp
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

#include "flightselectiondialog.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kseparator.h>
#include <kglobal.h>
#include <kiconloader.h>

FlightSelectionDialog::FlightSelectionDialog(QWidget *parent, const char *name )
 : KDialog(parent, name, true)
{
  setCaption(i18n("Flight selection"));
  __initDialog();

  availableFlights.setAutoDelete(false);
  selectedFlights.setAutoDelete(false);

  setMinimumWidth(400);
  setMinimumHeight(300);
}

FlightSelectionDialog::~FlightSelectionDialog()
{
}
/** No descriptions */
void FlightSelectionDialog::__initDialog()
{
  QLabel *l;
  QPushButton *b;

  QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
  QVBoxLayout *leftLayout = new QVBoxLayout(5);
  QVBoxLayout *middleLayout = new QVBoxLayout(5);
  QVBoxLayout *rightLayout = new QVBoxLayout(5);
  QHBoxLayout *topGroup = new QHBoxLayout(10);
  QHBoxLayout *buttons = new QHBoxLayout(10);
  QHBoxLayout *smallButtons = new QHBoxLayout(5);

  buttons->addStretch();

  b = new QPushButton(i18n("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(accept()));
  buttons->addWidget(b);
  b = new QPushButton(i18n("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  smallButtons->addStretch();
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("up", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("down", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(b);
  smallButtons->addStretch();

  aFlights = new KListBox(this, "availableFlights");
  l = new QLabel(aFlights, i18n("&available Flights"), this);
  leftLayout->addWidget(l);
  leftLayout->addWidget(aFlights);

  middleLayout->addStretch();
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("forward", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotAddOne()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("stepforward", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotAddAll()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("back", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotRemoveOne()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("stepback", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotRemoveAll()));
  middleLayout->addWidget(b);
  middleLayout->addStretch();

  sFlights = new KListBox(this, "selectedFlights");
  l = new QLabel(sFlights, i18n("&selected Flights"), this);
  rightLayout->addWidget(l);
  rightLayout->addWidget(sFlights);
  rightLayout->addLayout(smallButtons);

  topGroup->addLayout(leftLayout);
  topGroup->addLayout(middleLayout);
  topGroup->addLayout(rightLayout);
  topLayout->addLayout(topGroup);
  topLayout->addWidget(new KSeparator(this));
  topLayout->addLayout(buttons);

}
/** No descriptions */
void FlightSelectionDialog::slotAddOne()
{
  unsigned int i = aFlights->currentItem();

  if ((int)i != -1) {
    sFlights->insertItem(aFlights->text(i));
    aFlights->removeItem(i);
    selectedFlights.append(availableFlights.take(i));
  }
}

/** No descriptions */
void FlightSelectionDialog::slotAddAll()
{
  unsigned int i;
  BaseFlightElement *f;

  for (i = 0; i < availableFlights.count(); i++) {
    f = availableFlights.at(i);
    selectedFlights.append(f);
    sFlights->insertItem(f->getFileName(), i);
  }

  aFlights->clear();
  availableFlights.clear();
}

/** No descriptions */
void FlightSelectionDialog::slotRemoveOne()
{
  unsigned int i = sFlights->currentItem();

  if ((int)i != -1) {
    aFlights->insertItem(sFlights->text(i));
    sFlights->removeItem(i);
    availableFlights.append(selectedFlights.take(i));
  }
}

/** No descriptions */
void FlightSelectionDialog::slotRemoveAll()
{
  unsigned int i;
  BaseFlightElement *f;

  for (i = 0; i < selectedFlights.count(); i++) {
    f = selectedFlights.at(i);
    availableFlights.append(f);
    aFlights->insertItem(f->getFileName(), i);
  }

  sFlights->clear();
  selectedFlights.clear();
}

/** No descriptions */
void FlightSelectionDialog::slotMoveUp()
{
  int i = sFlights->currentItem();
  int newItem;

  if (i != -1) {
    QString s = sFlights->text(i);
    sFlights->removeItem(i);
    newItem = MAX(0, i - 1);
    sFlights->insertItem(s, newItem);
    sFlights->setCurrentItem(newItem);
    BaseFlightElement *e = selectedFlights.take(i);
    selectedFlights.insert(newItem, e);
  }
}

/** No descriptions */
void FlightSelectionDialog::slotMoveDown()
{
  int i = sFlights->currentItem();
  int newItem;

  if (i != -1) {
    QString s = sFlights->text(i);
    sFlights->removeItem(i);
    newItem = MIN(sFlights->count(), (unsigned int)i + 1);
    sFlights->insertItem(s, newItem);
    sFlights->setCurrentItem(newItem);
    BaseFlightElement *e = selectedFlights.take(i);
    selectedFlights.insert(newItem, e);
  }
}

void FlightSelectionDialog::polish()
{
  KDialog::polish();
  unsigned int i;
  BaseFlightElement *f;

  for (i = 0; i < availableFlights.count(); i++) {
    f = availableFlights.at(i);
    aFlights->insertItem(f->getFileName(), i);
  }

  for (i = 0; i < selectedFlights.count(); i++) {
    f = selectedFlights.at(i);
    sFlights->insertItem(f->getFileName(), i);
  }
}