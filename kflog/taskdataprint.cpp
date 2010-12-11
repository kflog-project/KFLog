/***********************************************************************
**
**   taskdataprint.cpp
**
**   This file is part of KFLog4.
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

#include <QPrinter>

#include "mapcalc.h"
#include "taskdataprint.h"

#define VERSION "3.0"

TaskDataPrint::TaskDataPrint(FlightTask* task)
{
  QPrinter printer;

  if(!printer.setup())  return;

  printer.setDocName("kflog-map.ps");
  printer.setCreator((QString)"KFLog " + VERSION);

  printer.setFullPage(true);

  QString temp;
  Waypoint *cPoint;

  QPainter painter(&printer);

  painter.setFont(QFont("helvetica", 18, QFont::Bold));
  painter.drawText(50, 50, QObject::tr("Flightplanning") + ":");
  painter.setPen(QPen(QColor(0, 0, 0), 2));
  painter.drawLine(50, 56, 545, 56);

  painter.setFont(QFont("helvetica", 10));
  painter.drawText(50, 100, QObject::tr("Task-Type") + ":");
  painter.drawText(125, 100, task->getTaskTypeString());
  painter.drawText(50, 115, QObject::tr("total Distance") + ":");
  painter.drawText(125, 115, task->getTotalDistanceString());
  painter.drawText(50, 130, QObject::tr("Task-Distance") + ":");
  painter.drawText(125, 130, task->getTaskDistanceString());

  painter.setPen(QPen(QColor(0,0,0), 1));
  painter.drawLine(50, 175, 545, 175);

  painter.setFont(QFont("helvetica", 12, QFont::Bold));
  painter.drawText(50, 170, QObject::tr("Task") + ":");

  painter.setFont(QFont("helvetica", 10));

  int yPos = 210;
  for(int loop = 0; loop < task->getWPList().count(); loop++)
    {
      cPoint = task->getWPList().at(loop);
      painter.drawText(50, yPos, cPoint->name);
      painter.drawText(125, yPos, printPos(cPoint->origP.lat(), true));
      painter.drawText(190, yPos, "/");
      painter.drawText(200, yPos, printPos(cPoint->origP.lon(), false));

      yPos += 13;
    }
}

TaskDataPrint::~TaskDataPrint()
{

}
