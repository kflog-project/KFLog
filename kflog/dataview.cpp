/***********************************************************************
**
**   dataview.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "dataview.h"
#include <flight.h>
#include <flightgroup.h>
#include <flighttask.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <wgspoint.h>

extern MapContents *_globalMapContents;

DataView::DataView(QWidget* parent) : QWidget(parent)
{
  setObjectName("DataView");

  flightDataText = new QTextBrowser(this);
  flightDataText->setObjectName( "FlightDataBrowser" );

  QHBoxLayout* flightLayout = new QHBoxLayout(this);
  flightLayout->setMargin(5);
  flightLayout->addWidget(flightDataText);

  connect( flightDataText, SIGNAL(anchorClicked(const QUrl &)),
           this, SLOT(slotWPSelected(const QUrl &)) );
}

DataView::~DataView()
{
}

QString DataView::__writeTaskInfo(FlightTask* task)
{
  QString htmlText;
  QString txt, tmp,speed;
  QString idString, timeString;
  Waypoint *wp1, *wp2 = 0;
  int t1, t2, loop = 0;

  htmlText = "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\
      <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
      tr("Task") + ":</B></TD></TR>";

  QList<Waypoint*> wpList = task->getWPList();

  foreach(wp1, wpList) {//int loop = 0; loop < wpList.count(); loop++) {
//    *wp1 = wpList.value(loop);

    if(wp1->fixTime != 0) {
      timeString = printTime(wp1->fixTime, true);
      t1 = wp1->fixTime;
    }
    else if(wp1->sector1 != 0) {
      timeString = printTime(wp1->sector1, true);
      t1 = wp1->sector1;
    }
    else if(wp1->sector2 != 0) {
      timeString = printTime(wp1->sector2, true);
      t1 = wp1->sector2;
    }
    else if(wp1->sectorFAI != 0) {
      timeString = printTime(wp1->sectorFAI, true);
      t1 = wp1->sectorFAI;
    }
    else {
      timeString = "--";
      t1 = 0;
    }

    if(wp2!=0) {
//      *wp2 = wpList.value(loop - 1);
      if(wp2->fixTime != 0) {
        t2 = wp2->fixTime;
      }
      else if(wp2->sector1 != 0) {
        t2 = wp2->sector1;
      }
      else if(wp2->sector2 != 0) {
        t2 = wp2->sector2;
      }
      else if(wp2->sectorFAI != 0) {
        t2 = wp2->sectorFAI;
      }
      else {
        t2 = 0;
      }

      tmp.sprintf("t1 : %d, t2 : %d", t1, t2);
      //warning(tmp);

      tmp.sprintf("%.2f km / %03.0f\260 / %.1f km/h",
                  wp1->distance,
                  getTrueCourse(wp1->origP, wp2->origP),
                  (t1 != 0 && t2 != 0) ? wp1->distance / (t1 - t2) * 3600.0 : 0.0);
      htmlText += "<TR><TD ALIGN=center COLSPAN=3 BGCOLOR=#EEEEEE>" +
        tmp + "</TD></TR>";
    }

    idString.sprintf("%d", loop);

    QString wpLat = WGSPoint::printPos(wp1->origP.lat()).replace(QRegExp(" "), "&nbsp;");
    QString wpLon = WGSPoint::printPos(wp1->origP.lon()).replace(QRegExp(" "), "&nbsp;");

    htmlText += "<TR><TD COLSPAN=2><A HREF=" + idString + ">" + wp1->name +
          "</A></TD><TD ALIGN=right>" + timeString + "</TD></TR>" +
          "<TR><TD NOWRAP COLSPAN=2>&nbsp;" + wpLat + "&nbsp;</TD>" +
          "<TD ALIGN=right>&nbsp;" + wpLon + "&nbsp;</TD></TR>";
    wp2 = wp1;
    loop++;
  }

  if (task->getTaskType() == FlightTask::OLC2003){
    txt.sprintf("%.2f", task->getOlcPoints());
    speed.sprintf("%.2f",task->getAverageSpeed());
    htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Total Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      task->getTotalDistanceString() + "</TD></TR>\
      <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Task Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      task->getTaskDistanceString() + "</TD></TR>\
      <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Average Speed") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      speed + "</TD>" +
      "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Points") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      txt + "</TD>" +
      "</TR></TABLE>";
  }
  else {
    if (task->getPlanningType() == FlightTask::Route) {
      txt.sprintf("%d", task->getPlannedPoints());
      htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("total Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        task->getTotalDistanceString() + "</TD></TR>\
        <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Task Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        task->getTaskDistanceString() + "</TD></TR>\
        <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Points") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        txt + "</TD></TR></TABLE>";
    }
    else {  // area based
      if (wpList.count() < 3) {
        tmp = "--";
      }
      else {
        tmp.sprintf("%.2f km", wpList.at(2)->distance);
      }
      htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Leg Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        tmp + "</TD></TR>\
        <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("FAI Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        task->getFAIDistanceString() + "</TD></TR></TABLE>";
    }
  }

  return htmlText;
}

void DataView::slotShowTaskText(FlightTask* task)
{
  QList<Waypoint*> taskPointList = task->getWPList();
  QString htmlText = "";
  QString tmp;

  htmlText = "<HTML><B>" + tr("Task-Type: ") + "</B>" +
      task->getTaskTypeString() + "<BR>";

  if(task->getWPList().count() > 0)  htmlText += __writeTaskInfo(task);

  if(taskPointList.count() == 0)
    {
      htmlText += "Bitte wählen Sie den <b>Startort</b> der Aufgabe in der Karte<br>";
    }
  else if(taskPointList.count() == 1)
    {
      htmlText += "<b>" + tr("Begin of task") + "?</b>";
    }
  else
    {
      htmlText += "<b>" + tr("Next waypoint") + "?</b>";
    }

  htmlText += "</HTML>\n";

  flightDataText->setHtml(htmlText);
}

void DataView::setFlightData()
{
  BaseFlightElement* e = _globalMapContents->getFlight();
  QString htmlText;
  QString idString;
  QList<Flight*> fl;
  QStringList h;
  FlightTask* fTask;
  QFileInfo fi;

  slotClearView();

  if(e)
    {
      switch (e->getObjectType())
        {
          case BaseMapElement::Flight:
            h = ((Flight*)e)->getHeader();
            fTask = ((Flight*) e)->getTask();

            //
            // For some strange reason, the widget adds a large vertical space
            // between the the first table and the following rule. Therfore I
            // have removed the rule
            //                                                     Heiner, 2003-01-02
            //
            htmlText = QString("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>") +
                "<TR><TD>" + tr("Date") + ":</TD><TD>" + h.at(3) + "</TD></TR>" +
                "<TR><TD>" + tr("Pilot") + ":</TD><TD> " + h.at(0) + "</TD></TR>" +
                "<TR><TD>" + tr("Glider") + ":</TD><TD>" + h.at(2) +
                " / " + h.at(1) + "</TD></TR>" +
                "</TABLE><BR>";

            if(fTask->getWPList().count())
                htmlText += __writeTaskInfo(fTask);
            else
                htmlText += "<EM>" + tr("Flight contains no waypoints") + "</EM>";
            break;

          case BaseMapElement::Task:
            htmlText = __writeTaskInfo((FlightTask*)e);
            break;

          case BaseMapElement::FlightGroup:
            htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD>" + tr("Flight group") + ":</TD><TD><A HREF=EDITGROUP>" +
                e->getFileName() +  + "</A></TD></TR>\
                </TABLE><HR NOSHADE>";
            fl = ((FlightGroup*)e)->getFlightList();
            if(fl.count()) {
              htmlText += "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
                 tr("Flights") + ":</B></TD></TR>";

              for (int loop = 0; loop < fl.count(); loop++) {
                Flight *flight = fl.at(loop);
                // store pointer of flight instead of index
                // flight list of mapcontents will change
                idString.sprintf("%lu", (unsigned long)flight);
                fi.setFile(flight->getFileName());

                htmlText += "<TR><TD><A HREF=" + idString + ">" +
                  fi.fileName() + "</A></TD><TD ALIGN=right>" + flight->getDate().toString(Qt::LocalDate) + "</TD></TR>\
                  <TR><TD>" + flight->getDistance() + "</TD><TD ALIGN=right>" +
                  printTime(flight->getLandTime() - flight->getStartTime()) + "</TD></TR>";
              }
            }
            else {
              htmlText += tr("Click on the group name to start editing");
            }
            break;

          default:
            break;
        }

      flightDataText->setHtml(htmlText);
      flightDataText->document()->adjustSize();
    }
}

void DataView::slotWPSelected(const QUrl &link)
{
  extern MapContents *_globalMapContents;
  BaseFlightElement* e = _globalMapContents->getFlight();

#warning "Check, if URL context is right ported here!"

  QString url = link.toString();

  // this seems to happen sometimes, prevent crash
  if (!e)
    return;

  switch (e->getObjectType())
    {
      case BaseMapElement::Flight:

        emit wpSelected(url.toInt());
        break;

      case BaseMapElement::Task:

        if (url == "EDITTASK")
          {
            QMessageBox::information(0, tr("Edit task"), tr(
                "This will bring up the task editing dialog"), QMessageBox::Ok);
          }
        else
          {
            emit wpSelected(url.toInt());
          }

        break;

      case BaseMapElement::FlightGroup:

        if (url == "EDITGROUP")
          {
            emit editFlightGroup();
          }
        else
          {
            emit flightSelected((BaseFlightElement *) url.toUInt());
          }

        break;

      default:

        break;
    }

  setFlightData();
}

void DataView::slotClearView()
{
  flightDataText->clear();
}

