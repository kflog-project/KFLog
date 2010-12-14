/***********************************************************************
**
**   flightselectiondialog.cpp
**
**   This file is part of KFLog4.
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

#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

FlightSelectionDialog::FlightSelectionDialog(QWidget *parent, const char *name )
 : QDialog(parent, name, true)
{
  setCaption(tr("Flight selection"));
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

  Q3VBoxLayout *topLayout = new Q3VBoxLayout(this, 10);
  Q3VBoxLayout *leftLayout = new Q3VBoxLayout(5);
  Q3VBoxLayout *middleLayout = new Q3VBoxLayout(5);
  Q3VBoxLayout *rightLayout = new Q3VBoxLayout(5);
  Q3HBoxLayout *topGroup = new Q3HBoxLayout(10);
  Q3HBoxLayout *buttons = new Q3HBoxLayout(10);
  Q3HBoxLayout *smallButtons = new Q3HBoxLayout(5);

  buttons->addStretch();

  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(slotAccept()));
  buttons->addWidget(b);
  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  smallButtons->addStretch();
  b = new QPushButton(this);
  b->setPixmap(QDir::homePath() + "/.kflog/pics/kde_up_16.png");
  connect(b, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(QDir::homePath() + "/.kflog/pics/kde_down_16.png");
  connect(b, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(b);
  smallButtons->addStretch();

  aFlights = new Q3ListBox(this, "availableFlights");
  l = new QLabel(aFlights, tr("&available Flights"), this);
  leftLayout->addWidget(l);
  leftLayout->addWidget(aFlights);

  middleLayout->addStretch();
  b = new QPushButton(this);
  b->setPixmap(QDir::homePath() + "/.kflog/pics/kde_forward_16.png");
  connect(b, SIGNAL(clicked()), SLOT(slotAddOne()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(QDir::homePath() + "/.kflog/pics/kde_2rightarrow_16.png");
  connect(b, SIGNAL(clicked()), SLOT(slotAddAll()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(QDir::homePath() + "/.kflog/pics/kde_back_16.png");
  connect(b, SIGNAL(clicked()), SLOT(slotRemoveOne()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(QDir::homePath() + "/.kflog/pics/kde_2leftarrow_16.png");
  connect(b, SIGNAL(clicked()), SLOT(slotRemoveAll()));
  middleLayout->addWidget(b);
  middleLayout->addStretch();

  sFlights = new Q3ListBox(this, "selectedFlights");
  l = new QLabel(sFlights, tr("&selected Flights"), this);
  rightLayout->addWidget(l);
  rightLayout->addWidget(sFlights);
  rightLayout->addLayout(smallButtons);

  topGroup->addLayout(leftLayout);
  topGroup->addLayout(middleLayout);
  topGroup->addLayout(rightLayout);
  topLayout->addLayout(topGroup);
  topLayout->addLayout(buttons);

}

/** No descriptions */
void FlightSelectionDialog::slotAccept()
{
  if(selectedFlights.count()>0)
    accept();
  else
    if(QMessageBox::Yes == QMessageBox::warning(0, tr("No flights selected"), "<qt>" + tr("No flights are selected, so no flight group can be made. Do you want to close this screen?") + "</qt>", QMessageBox::Yes, QMessageBox::No))
      reject();
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
    newItem = std::max(0, i - 1);
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
    newItem = std::min(sFlights->count(), (unsigned int)i + 1);
    sFlights->insertItem(s, newItem);
    sFlights->setCurrentItem(newItem);
    BaseFlightElement *e = selectedFlights.take(i);
    selectedFlights.insert(newItem, e);
  }
}

void FlightSelectionDialog::polish()
{
  QDialog::polish();
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
