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
#include <flighttask.h>
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

  slotClearView();

  if(e && e->getTypeID() == BaseMapElement::Flight)
    {
      QString htmlText;

      QStrList h = ((Flight*)e)->getHeader();
      FlightTask fTask = ((Flight*)e)->getTask();

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

      flightDataText->setText(htmlText);
    }
  else if(e && e->getTypeID() == BaseMapElement::Task)
    {
      if(e->getWPList().count())
          flightDataText->setText(__writeTaskInfo((FlightTask*)e));
      else
        {
          flightDataText->setText(i18n(
            "You can select waypoints with the left mouse button."
            "You can also select free waypoints by clicking anywhere in the map."
            "<br><br>"
            "When you press &lt;CTRL&gt; and click with the left mouse button on a taskpoint, "
            "it will be deleted.<br>"
            "You can compute the task up to your current mouse position by pressing &lt;SHIFT&gt;."
            "<br>"
            "To finish the task, press &lt;CTRL&gt; and click the right mouse button.<br>"
            "It's possible to move and delete taskpoints from the finished task."
            ));
        }
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
        KMessageBox::information(0, "This will bring up the flight group editing dialog");
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

