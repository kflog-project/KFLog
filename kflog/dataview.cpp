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

#include <kglobal.h>
#include <klocale.h>

#include <qlayout.h>
#include <qstrlist.h>
#include <qfileinfo.h>

#include <flight.h>
#include <mapcalc.h>
#include <flighttask.h>
#include <flightgroup.h>
#include <mapcontents.h>

#include <kmessagebox.h>
#include "iostream.h"

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

QString DataView::__writeTaskInfo(FlightTask* task)
{
  QString htmlText;

  htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
      <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
      i18n("Task") + ":</B></TD></TR>";

  QList<wayPoint> wpList = task->getWPList();

  for(unsigned int loop = 0; loop < wpList.count(); loop++)
    {
      if(loop > 0)
        {
          QString tmp;
          tmp.sprintf("%.2f km",wpList.at(loop)->distance);

          htmlText += "<TR><TD ALIGN=center COLSPAN=3 BGCOLOR=#EEEEEE>" +
              tmp + "</TD></TR>";
        }
      QString idString, timeString;
      idString.sprintf("%d", loop);

      if(wpList.at(loop)->sector1 != 0)
          timeString = printTime(wpList.at(loop)->sector1);
      else if(wpList.at(loop)->sector2 != 0)
          timeString = printTime(wpList.at(loop)->sector2);
      else if(wpList.at(loop)->sectorFAI != 0)
          timeString = printTime(wpList.at(loop)->sectorFAI);
      else
          timeString = "--";

      htmlText += "<TR><TD COLSPAN=2><A HREF=" + idString + ">" +
          wpList.at(loop)->name + "</A></TD>\
          <TD ALIGN=right>" + timeString + "</TD></TR>\
          <TR><TD WIDTH=15></TD>\
          <TD>" + printPos(wpList.at(loop)->origP.lat()) + "</TD>\
          <TD ALIGN=right>" + printPos(wpList.at(loop)->origP.lon(), false) +
          "</TD></TR>";
    }

  QString pointString;
  pointString.sprintf("%d", task->getPlannedPoints());

  htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("total Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      task->getTotalDistanceString() + "</TD></TR>\
      <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Task Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      task->getTaskDistanceString() + "</TD></TR>\
      <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Points") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      pointString + "</TD></TR></TABLE>";

  return htmlText;
}

void DataView::slotShowTaskText(FlightTask* task)
{
  QList<wayPoint> taskPointList = task->getWPList();
  QString htmlText = "";
  QString tmp;

  htmlText = "<B>" + i18n("Task-Type: ") + "</B>" +
      task->getTaskTypeString() + "<BR>";

  if(task->getWPList().count() > 0)  htmlText += __writeTaskInfo(task);

  // Frage
  if(taskPointList.count() == 0)
    {
      htmlText += "Bitte wählen Sie den <b>Startort</b> der Aufgabe in der Karte<br>";
    }
  else if(taskPointList.count() == 1)
    {
      htmlText += "<b>" + i18n("Begin of task") + "?</b>";
    }
  else
    {
      htmlText += "<b>" + i18n("Next waypoint") + "?</b>";
    }

  flightDataText->setText(htmlText);
}

void DataView::setFlightData()
{
  extern MapContents _globalMapContents;
  BaseFlightElement* e = _globalMapContents.getFlight();
  QString htmlText;
  QString idString;
  QList<Flight> fl;
  QStrList h;
  FlightTask fTask("");
  QFileInfo fi;

  slotClearView();

  if(e)
    {
      switch (e->getTypeID())
        {
          case BaseMapElement::Flight:
            h = ((Flight*)e)->getHeader();
            fTask = ((Flight*)e)->getTask();

            htmlText = (QString)"<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD>" + i18n("Date") + ":</TD><TD>" + h.at(3) + "</TD></TR>\
                <TR><TD>" + i18n("Pilot") + ":</TD><TD> " + h.at(0) + "</TD></TR>\
                <TR><TD>" + i18n("Glider") + ":</TD><TD>" + h.at(2) +
                " / " + h.at(1) + "</TD></TR>" +
                "</TABLE><HR NOSHADE>";

            if(fTask.getWPList().count())
                htmlText += __writeTaskInfo(&fTask);
            else
                htmlText += "<EM>" + i18n("Flight contains no waypoints") + "</EM>";
            break;
          case BaseMapElement::Task:
            if(e->getWPList().count())
                htmlText = __writeTaskInfo((FlightTask*)e);
            else
              {
                htmlText = i18n(
                  "You can select waypoints with the left mouse button."
                  "You can also select free waypoints by clicking anywhere in the map."
                  "<br><br>"
                  "When you press &lt;CTRL&gt; and click with the left mouse button on a taskpoint, "
                  "it will be deleted.<br>"
                  "You can compute the task up to your current mouse position by pressing &lt;SHIFT&gt;."
                  "<br>"
                  "To finish the task, press &lt;CTRL&gt; and click the right mouse button.<br>"
                  "It's possible to move and delete taskpoints from the finished task."
                  );
              }
            break;
          case BaseMapElement::FlightGroup:
            htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD>" + i18n("Flight group") + ":</TD><TD><A HREF=EDITGROUP>" +
                e->getFileName() +  + "</A></TD></TR>\
                </TABLE><HR NOSHADE>";
            fl = ((FlightGroup*)e)->getFlightList();
            if(fl.count()) {
              htmlText += "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
                 i18n("Flights") + ":</B></TD></TR>";

              for (unsigned int loop = 0; loop < fl.count(); loop++) {
                Flight *flight = fl.at(loop);
                // store pointer of flight instead of index
                // flight list of mapcontents will change
                idString.sprintf("%d", (int)flight);
                fi.setFile(flight->getFileName());

                htmlText += "<TR><TD><A HREF=" + idString + ">" +
                  fi.fileName() + "</A></TD><TD ALIGN=right>" +
                  KGlobal::locale()->formatDate(flight->getDate(), true) + "</TD></TR>\
                  <TR><TD>" + flight->getDistance() + "</TD><TD ALIGN=right>" +
                  printTime(flight->getLandTime() - flight->getStartTime()) + "</TD></TR>";
              }
            }
            else {
              htmlText += i18n("Click on the group name to start editing");
            }
            break;
        }
      flightDataText->setText(htmlText);
    }
}

void DataView::slotWPSelected(const QString &url)
{
  extern MapContents _globalMapContents;
  BaseFlightElement* e = _globalMapContents.getFlight();

  switch(e->getTypeID()) {
    case BaseMapElement::Flight:
      emit wpSelected(url.toUInt());
      break;
    case BaseMapElement::Task:
      if (url == "EDITTASK") {
        KMessageBox::information(0, "This will bring up the task editing dialog");
      }
      else {
        emit wpSelected(url.toUInt());
      }
      break;
    case BaseMapElement::FlightGroup:
      if (url == "EDITGROUP") {
        emit editFlightGroup();
      }
      else {
        emit flightSelected((BaseFlightElement *)url.toUInt());
      }
      break;
  }
}

void DataView::slotClearView()
{
  QString htmlText = "";
  flightDataText->setText(htmlText);
}

