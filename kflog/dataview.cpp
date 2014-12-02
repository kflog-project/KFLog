/***********************************************************************
**
**   dataview.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "dataview.h"
#include "distance.h"
#include "flight.h"
#include "flightgroup.h"
#include "flighttask.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "Speed.h"
#include "wgspoint.h"

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
  if( task == static_cast<FlightTask *>(0) )
    {
      return "";
    }

  QString txt, tmp, speed;
  QString idString, timeString;
  Waypoint *wp1, *wp2 = 0;
  int t1, t2, loop = 0;

  QString htmlText = "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\
                      <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
                      tr("Task") + ":</B></TD></TR>";

  QList<Waypoint*> wpList = task->getWPList();

  foreach(wp1, wpList)
    {
      if (wp1->fixTime != 0)
        {
          timeString = printTime(wp1->fixTime, true);
          t1 = wp1->fixTime;
        }
      else if (wp1->sector1 != 0)
        {
          timeString = printTime(wp1->sector1, true);
          t1 = wp1->sector1;
        }
      else if (wp1->sector2 != 0)
        {
          timeString = printTime(wp1->sector2, true);
          t1 = wp1->sector2;
        }
      else if (wp1->sectorFAI != 0)
        {
          timeString = printTime(wp1->sectorFAI, true);
          t1 = wp1->sectorFAI;
        }
      else
        {
          timeString = "--";
          t1 = 0;
        }

      if (wp2 != 0)
        {
          if (wp2->fixTime != 0)
            {
              t2 = wp2->fixTime;
            }
          else if (wp2->sector1 != 0)
            {
              t2 = wp2->sector1;
            }
          else if (wp2->sector2 != 0)
            {
              t2 = wp2->sector2;
            }
          else if (wp2->sectorFAI != 0)
            {
              t2 = wp2->sectorFAI;
            }
          else
            {
              t2 = 0;
            }

          Distance dist(0);
          dist.setKilometers( wp1->distance );

          Speed speed1(0);

          if( (t1 - t2) != 0 )
            {
              speed1.setKph( wp1->distance / (t1 - t2) * 3600.0 );
            }

          tmp = QString("%1 / %2%3 / %4")
                .arg( dist.getText( true, 2 ) )
                .arg( getTrueCourse(wp1->origP, wp2->origP), 0, 'f', 0 )
                .arg( QChar(Qt::Key_degree) )
                .arg( speed1.getHorizontalText( true, 0 ) );

          tmp = tmp.replace(QRegExp(" "), "&nbsp;");

          htmlText += "<TR><TD NOWRAP ALIGN=center COLSPAN=3 BGCOLOR=#EEEEEE>" + tmp
                   + "</TD></TR>";
        }

    idString = QString("%1").arg(loop);

    QString wpLat = WGSPoint::printPos(wp1->origP.lat()).replace(QRegExp(" "), "&nbsp;");
    QString wpLon = WGSPoint::printPos(wp1->origP.lon()).replace(QRegExp(" "), "&nbsp;");

    htmlText +=
        "<TR><TD COLSPAN=2><A HREF=" + idString + ">" + wp1->name +
        "</A></TD><TD ALIGN=right>" + timeString + "</TD></TR>" +
        "<TR><TD NOWRAP COLSPAN=2>&nbsp;" + wpLat + "&nbsp;</TD>" +
        "<TD ALIGN=right>&nbsp;" + wpLon + "&nbsp;</TD></TR>";

    wp2 = wp1;
    loop++;
  }

  if (task->getTaskType() == FlightTask::OLC2003)
    {
      txt = QString("%1").arg(task->getOlcPoints(), 0, 'f', 2);

      Speed speed2(0);
      speed2.setKph( task->getAverageSpeed() );

      speed = QString("%1").arg(speed2.getHorizontalText( true, 2 ) );

      htmlText +=
          "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Total Distance") +
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
  else
    {
      if (task->getPlanningType() == FlightTask::Route)
        {
          txt = QString("%1").arg(task->getPlannedPoints());

          htmlText +=
              "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("total Distance") +
              ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
              task->getTotalDistanceString() + "</TD></TR>\
              <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Task Distance") +
              ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
              task->getTaskDistanceString() + "</TD></TR>\
              <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Points") +
              ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
              txt + "</TD></TR></TABLE>";
        }
      else
        {  // area based
          if (wpList.count() < 3)
            {
              tmp = "--";
            }
          else
            {
              Distance dist(0);
              dist.setKilometers( wpList.at(2)->distance );
              tmp = QString("%1 %2")
        	           .arg( dist.getText(true, 2) )
        	           .arg( Distance::getUnitText() );

              tmp = tmp.replace(QRegExp(" "), "&nbsp;");
            }

          htmlText +=
              "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + tr("Leg Distance") +
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
  if( static_cast<FlightTask *>(0) )
    {
      return;
    }

  QList<Waypoint*> taskPointList = task->getWPList();

  QString htmlText = "<HTML><B>" + tr("Task-Type: ") + "</B>" +
                     task->getTaskTypeString() + "<BR>";

  if(task->getWPList().count() > 0)
    {
      htmlText += __writeTaskInfo(task);
    }

  if(taskPointList.count() == 0)
    {
      htmlText += tr("Please select the <b>start point</b> of the task at the map!") + "<br>";
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

void DataView::slotSetFlightData()
{
  QString htmlText;
  QString idString;
  QList<Flight*> fl;
  QStringList h;
  QFileInfo fi;

  slotClearView();

  BaseFlightElement* e = _globalMapContents->getFlight();

  if( e == static_cast<BaseFlightElement *>(0) )
    {
      return;
    }

  switch (e->getTypeID())
    {
      case BaseMapElement::Flight:
        {
          Flight* flight = dynamic_cast<Flight *>(e);

          if( flight == static_cast<Flight *>(0) )
            {
              return;
            }

          h = flight->getHeader();
          //
          // For some strange reason, the widget adds a large vertical space
          // between the the first table and the following rule. Therefore I
          // have removed the rule
          //                                                     Heiner, 2003-01-02
          //
          htmlText = QString("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>") +
              "<TR><TD>" + tr("File") + ":</TD><TD>" + QFileInfo(flight->getFileName()).fileName() + "</TD></TR>" +
              "<TR><TD>" + tr("Date") + ":</TD><TD>" + h.at(3) + "</TD></TR>" +
              "<TR><TD>" + tr("Pilot") + ":</TD><TD> " + h.at(0) + "</TD></TR>" +
              "<TR><TD>" + tr("Glider") + ":</TD><TD>" + h.at(2) +
              " / " + h.at(1) + "</TD></TR>" +
              "</TABLE><BR>";

          if( flight->getWPList().count() )
            {
              htmlText += __writeTaskInfo(flight->getTask());
            }
          else
            {
              htmlText += "<EM>" + tr("Flight contains no waypoints") + "</EM>";
            }

          break;
        }

      case BaseMapElement::Task:
        {
          FlightTask* ft = dynamic_cast<FlightTask *>(e);

          if( ft == static_cast<FlightTask *>(0) )
            {
              return;
            }

          htmlText = __writeTaskInfo( ft );
          break;
        }

      case BaseMapElement::FlightGroup:
        {
          FlightGroup* fg = dynamic_cast<FlightGroup *>(e);

          if( fg == static_cast<FlightGroup *>(0) )
            {
              return;
            }

          htmlText =
              "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
              <TR><TD>" + tr("Flight group") + ":</TD><TD><A HREF=EDITGROUP>" +
              e->getFileName() +  + "</A></TD></TR>\
              </TABLE><HR NOSHADE>";

          fl = fg->getFlightList();

          if(fl.count())
            {
              htmlText +=
                  "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                  <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
                  tr("Flights") + ":</B></TD></TR>";

              for (int loop = 0; loop < fl.count(); loop++)
                {
                  Flight *flight = fl.at(loop);
                  // store pointer of flight instead of index
                  // flight list of mapcontents will change
                  idString.sprintf("%lu", (unsigned long)flight);
                  fi.setFile(flight->getFileName());

                  htmlText +=
                      "<TR><TD><A HREF=" + idString + ">" +
                      fi.fileName() + "</A></TD><TD ALIGN=right>" +
                      flight->getDate() + "</TD></TR>\
                      <TR><TD>" + flight->getDistance() + "</TD><TD ALIGN=right>" +
                      printTime(flight->getLandTime() - flight->getStartTime()) +
                      "</TD></TR>";
              }
            }
          else
            {
              htmlText += tr("Click on the group name to start editing");
            }

          break;
        }

      default:
        break;
    }

  flightDataText->setHtml(htmlText);
  flightDataText->document()->adjustSize();
}

void DataView::slotWPSelected(const QUrl &link)
{
  BaseFlightElement* e = _globalMapContents->getFlight();

  if ( e == static_cast<BaseFlightElement *>(0) )
    {
      return;
    }

  QString url = link.toString();

  switch (e->getTypeID())
    {
      case BaseMapElement::Flight:

        emit wpSelected(url.toInt());
        break;

      case BaseMapElement::Task:

        if (url == "EDITTASK")
          {
            QMessageBox::information( this,
				      tr("Edit task"),
				      tr("This will bring up the task editing dialog"),
				      QMessageBox::Ok );
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
            emit flightSelected((BaseFlightElement *) e);
          }

        break;

      default:
        break;
    }

  slotSetFlightData();
}

void DataView::slotClearView()
{
  flightDataText->clear();
}
