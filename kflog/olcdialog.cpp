/***********************************************************************
**
**   olcdialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "olcdialog.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>

#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

OLCDialog::OLCDialog(QWidget* parent, const char* name)
  : QDialog(parent, name)
{
  setCaption(i18n("OLC-Declaration"));

  QGridLayout* mainLayout = new QGridLayout(this, 7, 5, 8, 1);
  mainLayout->addMultiCellWidget(new QLabel(i18n("OLC-Declaration"), this),
      0, 0, 0, 6);
  mainLayout->addMultiCellWidget(new KSeparator(this), 1, 1, 0, 6);
  mainLayout->addMultiCellWidget(new KSeparator(this), 3, 3, 0, 6);

  olcName = new KComboBox(this);
  olcName->setEditable(false);
  olcName->insertItem("OLC-" + i18n("International"));
  olcName->insertItem("OLC-" + i18n("Australia"));
  olcName->insertItem("OLC-" + i18n("Austria"));
  olcName->insertItem("OLC-" + i18n("Africa"));
  olcName->insertItem("OLC-" + i18n("Brasil"));
  olcName->insertItem("OLC-" + i18n("Canada"));
  olcName->insertItem("OLC-" + i18n("CZ/SK"));
  olcName->insertItem("OLC-" + i18n("France"));
  olcName->insertItem("OLC-" + i18n("Germany"));
  olcName->insertItem("OLC-" + i18n("Italy"));
  // to be continued ...

  QPushButton* okButton = new QPushButton(i18n("Send"), this);
  QPushButton* saveButton = new QPushButton(i18n("Save as"), this);
  QPushButton* cancelButton = new QPushButton(i18n("Cancel"), this);

  QFrame* midFrame = new QFrame(this, "midFrame");

  mainLayout->addMultiCellWidget(midFrame, 2, 2, 0, 6);
  mainLayout->addWidget(olcName, 4, 0);
  mainLayout->addWidget(okButton, 4, 2);
  mainLayout->addWidget(saveButton, 4, 4);
  mainLayout->addWidget(cancelButton, 4, 6);

  mainLayout->setColStretch(1, 1);
  mainLayout->addColSpacing(3, 5);
  mainLayout->addColSpacing(5, 5);

  mainLayout->addRowSpacing(1, 10);
  mainLayout->setRowStretch(2, 1);
  mainLayout->addRowSpacing(3, 10);

  QGridLayout* dlgLayout = new QGridLayout(midFrame, 13, 14);

  dlgLayout->addMultiCellWidget(new QGroupBox(i18n("Pilot"), midFrame),
      0, 5, 0, 14);
  dlgLayout->addMultiCellWidget(new QGroupBox(i18n("Glider"), midFrame),
      7, 13, 0, 14);
  dlgLayout->addMultiCellWidget(new QGroupBox(i18n("Task"), midFrame),
      15, 21, 0, 14);

  preName = new KLineEdit(midFrame);
  surName = new KLineEdit(midFrame);
  birthday = new KRestrictedLine(midFrame, "birthday", "0123456789.-");

  QPushButton* pilotB = new QPushButton(midFrame);
  pilotB->setPixmap(BarIcon("find"));
  pilotB->setMaximumWidth(pilotB->sizeHint().width() + 10);
  pilotB->setMaximumHeight(pilotB->sizeHint().height() + 10);

  QPushButton* gliderB = new QPushButton(midFrame);
  gliderB->setPixmap(BarIcon("find"));
  gliderB->setMaximumWidth(pilotB->sizeHint().width() + 10);
  gliderB->setMaximumHeight(pilotB->sizeHint().height() + 10);

  gliderType = new KLineEdit(midFrame);
  gliderID = new KLineEdit(midFrame);
  daec = new KRestrictedLine(midFrame, "daec", "0123456789");

  pureGlider = new QCheckBox(i18n("pure glider"), midFrame);
  classSelect = new KComboBox(midFrame);
  classSelect->setEditable(false);
  classSelect->insertItem(i18n("not set"));
  classSelect->insertItem(i18n("Club"));
  classSelect->insertItem(i18n("Standard"));
  classSelect->insertItem(i18n("15 Meter"));
  classSelect->insertItem(i18n("18 Meter"));
  classSelect->insertItem(i18n("double sitter"));
  classSelect->insertItem(i18n("open"));

  startPoint = new KLineEdit(midFrame);
  startPos = new QLabel(midFrame);
  startPos->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  startPos->setBackgroundMode( PaletteLight );

  taskList = new KListView(midFrame);
  taskList->addColumn(i18n("Waypoint"));
  taskList->addColumn(i18n("Latitude"));
  taskList->addColumn(i18n("Longitude"));
  taskList->addColumn(i18n("Distance"));
  taskList->addColumn(i18n("Time"));

  routeLength = new QLabel(midFrame);
  routeLength->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  routeLength->setBackgroundMode( PaletteLight );

  routePoints = new QLabel(midFrame);
  routePoints->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  routePoints->setBackgroundMode( PaletteLight );

  dlgLayout->addWidget(new QLabel(i18n("Prename"), midFrame), 1, 1);
  dlgLayout->addWidget(new QLabel(i18n("Surname"), midFrame), 1, 5);
  dlgLayout->addWidget(new QLabel(i18n("Birthday"), midFrame), 1, 9);
  dlgLayout->addWidget(preName, 1, 3);
  dlgLayout->addWidget(surName, 1, 7);
  dlgLayout->addWidget(birthday, 1, 11);
  dlgLayout->addWidget(pilotB, 1, 13, Qt::AlignLeft);

  dlgLayout->addWidget(new QLabel(i18n("Glidertype"), midFrame), 8, 1);
  dlgLayout->addWidget(new QLabel(i18n("Glider-ID"), midFrame), 8, 5);
  // Was machen wir mit diesem Eintrag ???
  dlgLayout->addWidget(new QLabel(i18n("DAEC-Index"), midFrame), 8, 9);
  dlgLayout->addWidget(gliderType, 8, 3);
  dlgLayout->addWidget(gliderID, 8, 7);
  dlgLayout->addWidget(daec, 8, 11);
  dlgLayout->addWidget(gliderB, 8, 13, Qt::AlignLeft);
  dlgLayout->addWidget(new QLabel(i18n("Class"), midFrame), 10, 1);
  dlgLayout->addMultiCellWidget(classSelect, 10, 10, 3, 5);
  dlgLayout->addMultiCellWidget(pureGlider, 10, 10, 9, 11);

  dlgLayout->addWidget(new QLabel(i18n("Startpoint"), midFrame), 16, 1);
  dlgLayout->addMultiCellWidget(startPoint, 16, 16, 3, 5);
  dlgLayout->addMultiCellWidget(startPos, 16, 16, 7, 13);
  dlgLayout->addMultiCellWidget(taskList, 18, 18, 1, 13);
  dlgLayout->addMultiCellWidget(new QLabel(i18n("Task-Length"), midFrame),
      20, 20, 3, 5, Qt::AlignRight);
  dlgLayout->addWidget(routeLength, 20, 7);
  dlgLayout->addWidget(new QLabel(i18n("Points"), midFrame), 20, 9, Qt::AlignRight);
  dlgLayout->addMultiCellWidget(routePoints, 20, 20, 11, 13);

  dlgLayout->addColSpacing(0, 10);
  dlgLayout->addColSpacing(2, 5);
  dlgLayout->addColSpacing(4, 5);
  dlgLayout->addColSpacing(6, 5);
  dlgLayout->addColSpacing(8, 5);
  dlgLayout->addColSpacing(10, 5);
  dlgLayout->addColSpacing(12, 5);
  dlgLayout->addColSpacing(14, 10);

  dlgLayout->setColStretch(1, 1);
  dlgLayout->setColStretch(3, 1);
  dlgLayout->setColStretch(5, 0);

  dlgLayout->addRowSpacing(0, 20);
  dlgLayout->addRowSpacing(2, 5);
  dlgLayout->addRowSpacing(4, 5);
  dlgLayout->addRowSpacing(5, 8);

  dlgLayout->addRowSpacing(6, 5);

  dlgLayout->addRowSpacing(7, 20);
  dlgLayout->addRowSpacing(9, 5);
  dlgLayout->addRowSpacing(11, 5);
  dlgLayout->addRowSpacing(13, 8);

  dlgLayout->addRowSpacing(14, 5);

  dlgLayout->addRowSpacing(15, 20);
  dlgLayout->addRowSpacing(17, 5);
  dlgLayout->addRowSpacing(19, 5);
  dlgLayout->addRowSpacing(21, 8);

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

OLCDialog::~OLCDialog()
{

}
