/***********************************************************************
**
**   dataview.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "dataview.h"

#include <klocale.h>
#include <qframe.h>
#include <qlayout.h>
#include <qstrlist.h>

#include <flight.h>
#include <mapcalc.h>

DataView::DataView(QWidget* parent)
: QTabWidget(parent)
{
  QFrame* flightDataFrame = new QFrame(this);
//  QFrame* taskDataFrame = new QFrame(this);
//  QFrame* mapElementFrame = new QFrame(this);
//  QFrame* searchDataFrame = new QFrame(this);

  addTab(flightDataFrame, i18n("&Flight"));
//  addTab(taskDataFrame, i18n("&Task"));
//  addTab(mapElementFrame, i18n("&Map"));
//  addTab(searchDataFrame, i18n("&Search"));

  flightDataText = new QTextView(flightDataFrame, "flightData");

  QHBoxLayout* flightLayout = new QHBoxLayout(flightDataFrame, 5);
  flightLayout->addWidget(flightDataText);
}

DataView::~DataView()
{

}

void DataView::setFlightData(Flight* cF)
{
  if(cF == 0)  return;

  QStrList h = cF->getHeader();
  QString htmlText;

  htmlText = (QString)"<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>" +
      "<TR><TD>" + i18n("Date") + ":</TD><TD>" + h.at(3) + "</TD></TR>" +
      "<TR><TD>" + i18n("Pilot") + ":</TD><TD> " + h.at(0) + "</TD></TR>" +
      "<TR><TD>" + i18n("Glider") + ":</TD><TD>" + h.at(2) +
          " / " + h.at(1) + "</TD></TR>" +
      "</TABLE>" + "<HR NOSHADE>";

  QList<struct wayPoint>* wpList = cF->getWPList();

  if(wpList->count())
    {
      htmlText += i18n("Task") + ":" +
          "<FONT><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>";
      for(unsigned int loop = 0; loop < wpList->count(); loop++)
        {
          QString timeText;
          if(wpList->at(loop)->sector1 != 0)
              timeText = printTime(wpList->at(loop)->sector1);
          else if(wpList->at(loop)->sector2 != 0)
              timeText = printTime(wpList->at(loop)->sector2);
          else if(wpList->at(loop)->sectorFAI != 0)
              timeText = printTime(wpList->at(loop)->sectorFAI);
          else
              timeText = (QString)"--";

          htmlText += (QString)"<TR><TD COLSPAN=2>" +
              wpList->at(loop)->name + "</TD>" +
              "<TD ALIGN=right>" + timeText + "</TD></TR>"+
              "<TR><TD WIDTH=15></TD>" +
              "<TD>" + printPos(wpList->at(loop)->origP.x()) +
              "</TD>" +
              "<TD ALIGN=right>" + printPos(wpList->at(loop)->origP.y(), true) +
              "</TD></TR>";
        }
      htmlText += (QString)"<TR><TD COLSPAN=2>" + i18n("total Distance") +
          ":</TD><TD ALIGN=right>" + cF->getDistance() +
          "</TD></TR></TABLE></FONT>";
    }
  else
    {
      htmlText += (QString)"<EM>" + i18n("Flight contains no waypoints") +
          "</EM>";
    }
  flightDataText->setText(htmlText);
}
