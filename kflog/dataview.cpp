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
#include <qlayout.h>
#include <qstrlist.h>

#include <flight.h>
#include <mapcalc.h>

DataView::DataView(QWidget* parent)
: QFrame(parent, "FlightData")
{
  flightDataText = new KTextBrowser(this, "flightDataBrowser", true);

  QHBoxLayout* flightLayout = new QHBoxLayout(this, 5);
  flightLayout->addWidget(flightDataText);

  connect(flightDataText, SIGNAL(urlClick(const QString &)), this,
      SLOT(slotWPSelected(const QString &)));
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

  QList<wayPoint> wpList = cF->getWPList();

  if(wpList.count())
    {
//      htmlText += "<B>" + i18n("Task") + ":</B>" +
//          "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>";
      htmlText += (QString)"<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>" +
          "<TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
          i18n("Task") + ":</B></TD></TR>";

      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          if(loop > 0)
            {
              QString tmp;
              tmp.sprintf("%.2f km",wpList.at(loop)->distance);

              htmlText += (QString)"<TR><TD ALIGN=center COLSPAN=3 BGCOLOR=#EEEEEE>" +
                    tmp + "</TD></TR>";
            }
          QString timeText;
          QString idString;
          idString.sprintf("%d", loop);
          if(wpList.at(loop)->sector1 != 0)
              timeText = printTime(wpList.at(loop)->sector1);
          else if(wpList.at(loop)->sector2 != 0)
              timeText = printTime(wpList.at(loop)->sector2);
          else if(wpList.at(loop)->sectorFAI != 0)
              timeText = printTime(wpList.at(loop)->sectorFAI);
          else
              timeText = (QString)"--";

          htmlText += (QString)"<TR><TD COLSPAN=2><A HREF=" + idString + ">" +
              wpList.at(loop)->name + "</A></TD>" +
              "<TD ALIGN=right>" + timeText + "</TD></TR>"+
              "<TR><TD WIDTH=15></TD>" +
              "<TD>" + printPos(wpList.at(loop)->origP.x()) +
              "</TD>" +
              "<TD ALIGN=right>" + printPos(wpList.at(loop)->origP.y(), false) +
              "</TD></TR>";
        }
      htmlText += (QString)"<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("total Distance") +
          ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" + cF->getDistance() + "</TD></TR>" +
          "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Points") +
          ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" + cF->getPoints() +
          "</TD></TR></TABLE></FONT>";
    }
  else
    {
      htmlText += (QString)"<EM>" + i18n("Flight contains no waypoints") +
          "</EM>";
    }
  flightDataText->setText(htmlText);
}

void DataView::slotWPSelected(const QString &url)
{
  emit wpSelected(url.toUInt());
}
