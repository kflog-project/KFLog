/***********************************************************************
**
**   evaluationview.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "evaluationdialog.h"
#include "evaluationview.h"

#include <kapp.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <flight.h>
#include <mapcalc.h>
#include <klocale.h>

EvaluationDialog::EvaluationDialog(QList<Flight>* fList)
: QDialog(0, "EvaluationsDialog", false),
  flightList(fList)
{
  glatt_va = 0;
  glatt_v = 0;
  glatt_h = 0;

  secWidth = 10;

  setCaption(i18n("Flightevaluation:"));
  setMinimumWidth(500);
  setMinimumHeight(500);

  // Diagrammfenster
  graphFrame = new QScrollView(this);
  graphFrame->setResizePolicy(QScrollView::AutoOne);
  graphFrame->setHScrollBarMode(QScrollView::AlwaysOn);
  graphFrame->setVScrollBarMode(QScrollView::AlwaysOff);
  graphFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  graphFrame->setBackgroundMode(PaletteLight);

  evalView = new EvaluationView(graphFrame,this);
  graphFrame->addChild(evalView);

  cursorLabel = new QTextView(this);
  textLabel = new QTextView(this);
  textLabel->sizeHint().setHeight(60);
  textLabel->setVScrollBarMode(QScrollView::AlwaysOff);

  // Auswahl
  QFrame* oben = new QFrame(this);
  QLabel* o1 = new QLabel("Flug:", oben);
  o1->setAlignment(AlignRight);
  combo_flight = new QComboBox(oben);
  o1->setMinimumHeight(o1->sizeHint().height() + 10);
  combo_flight->setMinimumWidth(120);

  check_vario = new QCheckBox("Variogramm",this);
//  check_vario->setMinimumWidth(check_vario->sizeHint().width());
  check_vario->setChecked(true);
  check_baro = new QCheckBox("Barogramm",this);
  check_baro->setChecked(true);
  check_speed = new QCheckBox("Geschw.",this);
  check_speed->setChecked(true);

  spinVario = new QSpinBox(0,999,1,this);
  spinBaro = new QSpinBox(0,999,1,this);
  spinSpeed = new QSpinBox(0,999,1,this);

  QLabel* scale_label = new QLabel("Zeitskalierung:", oben);
  scale_label->setAlignment(AlignRight);
  spinScale = new QSpinBox(1,60,1,oben);
  spinScale->setMaximumWidth(60);
  spinScale->setValue(secWidth);

  QPushButton* close = new QPushButton(i18n("Close"),this);
  close->setMinimumHeight(close->sizeHint().height() + 5);
  close->setMaximumWidth(close->sizeHint().width() + 5);

  QHBoxLayout* obenlayout = new QHBoxLayout(oben);
  QGridLayout* tabLayout = new QGridLayout(this,18,13,5,1);

  obenlayout->addWidget(o1);
  obenlayout->addWidget(combo_flight);
  obenlayout->addWidget(scale_label);
  obenlayout->addWidget(spinScale);

  tabLayout->addMultiCellWidget(oben,0,0,0,3);
  tabLayout->addMultiCellWidget(graphFrame,2,2,0,3);

  tabLayout->addMultiCellWidget(cursorLabel,4,4,0,3);
  tabLayout->addMultiCellWidget(textLabel,6,13,0,0);

  tabLayout->addMultiCellWidget(check_vario,6,6,2,3);
  tabLayout->addWidget(spinVario,7,3);
  tabLayout->addMultiCellWidget(check_baro,9,9,2,3);
  tabLayout->addWidget(spinBaro,10,3);
  tabLayout->addMultiCellWidget(check_speed,12,12,2,3);
  tabLayout->addWidget(spinSpeed,13,3);

  tabLayout->addMultiCellWidget(close,15,15,2,3);

  tabLayout->setColStretch(0,6);
  tabLayout->addColSpacing(0,620);
  tabLayout->setColStretch(1,0);
  tabLayout->addColSpacing(1,5);
  tabLayout->addColSpacing(2,30);
  tabLayout->setColStretch(2,0);
  tabLayout->setColStretch(3,1);

  tabLayout->addRowSpacing(0,o1->sizeHint().height() + 10);
  tabLayout->addRowSpacing(1,5);
  tabLayout->setRowStretch(2,4);
  tabLayout->addRowSpacing(2,graphFrame->sizeHint().height() + 5);
  tabLayout->addRowSpacing(3,5);
  tabLayout->addRowSpacing(4,cursorLabel->sizeHint().height() + 5);
  tabLayout->setRowStretch(4,0);
  tabLayout->addRowSpacing(5,0);
  tabLayout->setRowStretch(5,0);

  tabLayout->setRowStretch(6,2);
//  tabLayout->addRowSpacing(6,0);
  tabLayout->setRowStretch(7,2);
//  tabLayout->addRowSpacing(7,0);

  tabLayout->addRowSpacing(8,5);
  tabLayout->setRowStretch(9,2);
//  tabLayout->addRowSpacing(9,0);
  tabLayout->setRowStretch(10,2);
//  tabLayout->addRowSpacing(10,0);

  tabLayout->addRowSpacing(11,5);
  tabLayout->setRowStretch(12,2);
//  tabLayout->addRowSpacing(12,0);
  tabLayout->setRowStretch(13,2);
//  tabLayout->addRowSpacing(13,0);

  tabLayout->addRowSpacing(14,10);
  tabLayout->addRowSpacing(15,close->sizeHint().height() + 5);

  updateListBox();

  this->connect(combo_flight, SIGNAL(activated(int)),
        SLOT(slotShowFlightData(int)));
  this->connect(check_vario, SIGNAL(clicked()),
        SLOT(slotToggleView()));
  this->connect(check_baro, SIGNAL(clicked()),
        SLOT(slotToggleView()));
  this->connect(check_speed, SIGNAL(clicked()),
        SLOT(slotToggleView()));

  this->connect(spinVario, SIGNAL(valueChanged(int)),
        SLOT(slotVarioGlatt(int)));
  this->connect(spinSpeed, SIGNAL(valueChanged(int)),
        SLOT(slotSpeedGlatt(int)));
  this->connect(spinBaro, SIGNAL(valueChanged(int)),
        SLOT(slotBaroGlatt(int)));
  this->connect(close, SIGNAL(clicked()), SLOT(reject()));

  this->connect(spinScale, SIGNAL(valueChanged(int)),
        SLOT(slotScale(int)));

  this->show();
}

EvaluationDialog::~EvaluationDialog()
{
  // Cursor löschen
  emit(showCursor(QPoint(-100,-100), QPoint(-100,-100)));
}

void EvaluationDialog::updateText(int index1, int index2, bool updateAll)
{
  Flight* flight = flightList->at(combo_flight->currentItem());

  flightPoint p1 = flight->getPoint(index1);
  flightPoint p2 = flight->getPoint(index2);

  QString text;
  QString htmlText;

  htmlText = (QString)"<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>"+
           "<TR><TD WIDTH=100 ALIGN=center><FONT COLOR=green><B>" + printTime(p1.time, true) +
           "</B></TD><TD WIDTH=70 ALIGN=right>";
  text.sprintf("%5d m", p1.height);
  htmlText += text + "</TD><TD WIDTH=80 ALIGN=right>";
  text.sprintf("%.1f m/s", getVario(&p1));
  htmlText += text + "</TD><TD WIDTH=100 ALIGN=right>";
  text.sprintf("%.1f km/h", getSpeed(&p1));
  htmlText += text + "</TD>";

  htmlText += (QString) "<TD WIDTH=100 ALIGN=center><FONT COLOR=red><B>" + printTime(p2.time, true) +
           "</B></TD><TD WIDTH=70 ALIGN=right>";
  text.sprintf("%5d m", p2.height);
  htmlText += text + "</TD><TD WIDTH=80 ALIGN=right>";
  text.sprintf("%.1f m/s", getVario(&p2));
  htmlText += text + "</TD><TD WIDTH=100 ALIGN=right>";
  text.sprintf("%.1f km/h", getSpeed(&p2));
  htmlText += text + "</TD></TR>";

  cursorLabel->setText(htmlText);

  if(updateAll)
    {
      QStrList erg = flight->getFlightValues(index1, index2);

      htmlText = (QString) "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                            <TR><TD><B>" + i18n("Circling") + "</B></TR></TR> \
                            <TR><TD><B>" + i18n("Time") + "</B></TD> \
                                <TD><B>" + i18n("Vario") + "</B></TD> \
                                <TD><B>" + i18n("Alt. Gain") + "</B></TD> \
                                <TD><B>" + i18n("Alt. Loss") + "</B></TD></TR>";
      htmlText += (QString) "<TR><TD ALIGN=right WIDTH=130>" + erg.at(3) + \
                        "</TD><TD ALIGN=right>" + (QString)erg.at(7) + \
                        "</TD><TD ALIGN=right>" + erg.at(11) + \
                        "</TD><TD ALIGN=right>" + (QString)erg.at(15) + \
                        "</TD></TR>";

      htmlText += (QString) "<TR></TR><TR><TD><B>" + i18n("Straight") + "</B></TR> \
                                      <TR><TD><B>" + i18n("Time") + "</B></TD> \
                                      <TD><B>" + i18n("tot. Distance") + "</B></TD> \
                                      <TD><B>" + i18n("Alt. Gain") + "</B></TD> \
                                      <TD><B>" + i18n("Alt. Loss") + "</B></TD> \
                                      <TD><B>" + i18n("Speed") + "</B></TD> \
                                      <TD><B>" + i18n("L/D ratio") + "</B></TD></TR>";
      htmlText += (QString) "<TR><TD ALIGN=right>" + erg.at(21) + \
                        "</TD><TD ALIGN=right>" + erg.at(20) + \
                        "</TD><TD ALIGN=right>" + erg.at(18) + \
                        "</TD><TD ALIGN=right>" + erg.at(19) + \
                        "</TD><TD ALIGN=right>" + erg.at(17) + \
                        "</TD><TD ALIGN=right>" + erg.at(16) + \
                        "</TD></TR>";

      htmlText += (QString) "<TR></TR><TR><TD><B>" + i18n("Total:") + "</B></TD></TR>" + \
                        "<TR><TD ALIGN=right>" + erg.at(22) + \
                        "</TD><TD></TD>" + \
                        "</TD><TD ALIGN=right>" + erg.at(23) + \
                        "</TD><TD ALIGN=right>" + erg.at(24) + \
                        "</TD></TR></TABLE>";



      textLabel->setText(htmlText);
    }

  emit(showCursor(p1.projP,p2.projP));
}

void EvaluationDialog::resizeEvent(QResizeEvent* event)
{
  QDialog::resizeEvent(event);

  if(flightList->count())
      slotShowFlightData(combo_flight->currentItem());
}


void EvaluationDialog::updateListBox()
{
  combo_flight->clear();

  // Flüge eintragen
  for(unsigned int n = 0; n < flightList->count(); n++)
      combo_flight->insertItem(flightList->at(n)->getFileName());

  if(flightList->count())
      slotShowFlightData(combo_flight->currentItem());
}

void EvaluationDialog::slotShowFlightData(int n)
{
  this->setCaption(i18n("Flightevaluation: ") + flightList->at(n)->getPilot()
                    + "  " + flightList->at(n)->getDate());
  // GRUNDWERTE setzen
  updateText(0,flightList->at(n)->getRouteLength() - 1, true);

  // Kurve malen
  evalView->drawCurve(flightList->at(n), check_vario->isChecked(),
                check_speed->isChecked(), check_baro->isChecked(),
                glatt_va, glatt_v, glatt_h, secWidth);
}

void EvaluationDialog::slotToggleView()
{
  slotShowFlightData(combo_flight->currentItem());
}

void EvaluationDialog::slotVarioGlatt(int g)
{
  // gibt den Glästtungsfaktor zurück
  glatt_va = g;

  if(flightList->count())
      slotShowFlightData(combo_flight->currentItem());
}

void EvaluationDialog::slotBaroGlatt(int g)
{
  // gibt den Glättungsfaktor zurück
  glatt_h = g;

  if(flightList->count())
      slotShowFlightData(combo_flight->currentItem());
}

void EvaluationDialog::slotSpeedGlatt(int g)
{
  // gibt den Glättungsfaktor zurück
  glatt_v = g;

  if(flightList->count())
      slotShowFlightData(combo_flight->currentItem());
}

void EvaluationDialog::slotScale(int g)
{
  // gibt den Scalierungsfaktor zurück
  secWidth = g;

  if(flightList->count())
      slotShowFlightData(combo_flight->currentItem());
}

void EvaluationDialog::hide()
{
  this->EvaluationDialog::~EvaluationDialog();
}
