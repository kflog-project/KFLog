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
#include <evaluationframe.h>
#include <evaluationview.h>

#include <kapp.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qvaluelist.h>

#include <flight.h>
#include <mapcalc.h>
#include <klocale.h>

EvaluationDialog::EvaluationDialog(QList<Flight>* fList)
: QDialog(0, "EvaluationsDialog", false),
  flightList(fList)
{
  setCaption(i18n("Flightevaluation:"));
  setMinimumWidth(500);
  setMinimumHeight(500);

  // Auswahl - Kopfzeile
  QFrame* oben = new QFrame(this, "frame_oben");
  QLabel* o1 = new QLabel(i18n("Flight:"), oben);
  o1->setAlignment(AlignRight);
  combo_flight = new QComboBox(oben, "combo_oben");
  o1->setMinimumHeight(o1->sizeHint().height() + 10);
  combo_flight->setMinimumWidth(120);

  // variable Textanzeige
  QSplitter* textSplitter = new QSplitter(QSplitter::Vertical, this, "splitter");

  // Diagrammfenster - Mitte
  EvaluationFrame* evalFrame = new EvaluationFrame(textSplitter, this);

  connect(this, SIGNAL(flightChanged(Flight*)), evalFrame,
      SLOT(slotShowFlight(Flight*)));
  connect(this, SIGNAL(textChanged(QString)), evalFrame,
      SLOT(slotUpdateCursorText(QString)));

  // Textanzeige
  textLabel = new QTextView(textSplitter);
  textLabel->setMinimumHeight(1);

//  QPushButton* close = new QPushButton(i18n("Close"),this);
//  close->setMinimumHeight(close->sizeHint().height() + 5);
//  close->setMaximumWidth(close->sizeHint().width() + 5);

  QVBoxLayout* gesamtlayout = new QVBoxLayout(this,5,1);
  QHBoxLayout* obenlayout = new QHBoxLayout(oben);

  obenlayout->addWidget(o1);
  obenlayout->addWidget(combo_flight);

  gesamtlayout->addWidget(oben);
  gesamtlayout->addWidget(textSplitter);

  updateListBox();

  // Setting default-values for the splitter
  typedef QValueList<int> testList;
  testList list;
  list.append(100);
  list.append(60);
  textSplitter->setSizes(list);

  connect(combo_flight, SIGNAL(activated(int)),
        SLOT(slotShowFlightData(int)));
//  connect(close, SIGNAL(clicked()), SLOT(reject()));
  show();
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

  emit textChanged(htmlText);

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
//  warning("EvaluationDialog::resizeEvent");

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
//  warning("EvaluationDialog::slotShowFlightData");
  this->setCaption(i18n("Flightevaluation:") + flightList->at(n)->getPilot()
                    + "  " + flightList->at(n)->getDate());
  // GRUNDWERTE setzen
  updateText(0,flightList->at(n)->getRouteLength() - 1, true);


  emit flightChanged(flightList->at(n));
}



void EvaluationDialog::hide()
{
//warning("EvaluationDialog::hide()");
  this->EvaluationDialog::~EvaluationDialog();
}
