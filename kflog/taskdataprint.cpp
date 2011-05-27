/***********************************************************************
**
**   taskdataprint.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "mapcalc.h"
#include "taskdataprint.h"
#include "wgspoint.h"

#define VERSION "4.0.0"

TaskDataPrint::TaskDataPrint(FlightTask* task)
{
  QPrinter printer;

  QPrintDialog dialog( &printer );

  if( dialog.exec() != QDialog::Accepted )
    {
      return;
    }

  printer.setDocName("kflog-map.ps");
  printer.setCreator(QString("KFLog ") + VERSION);

  printer.setFullPage(true);

  QString temp;
  Waypoint *cPoint;

  QPainter painter(&printer);

  painter.setFont(QFont("helvetica", 18, QFont::Bold));
  painter.drawText(50, 50, QObject::tr("Flight planning") + ":");
  painter.setPen(QPen(QColor(0, 0, 0), 2));
  painter.drawLine(50, 56, 545, 56);

  painter.setFont(QFont("helvetica", 10));
  painter.drawText(50, 100, QObject::tr("Task Type") + ":");
  painter.drawText(125, 100, task->getTaskTypeString());
  painter.drawText(50, 115, QObject::tr("Total Distance") + ":");
  painter.drawText(125, 115, task->getTotalDistanceString());
  painter.drawText(50, 130, QObject::tr("Task Distance") + ":");
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
      painter.drawText(125, yPos, WGSPoint::printPos(cPoint->origP.lat(), true));
      painter.drawText(190, yPos, "/");
      painter.drawText(200, yPos, WGSPoint::printPos(cPoint->origP.lon(), false));

      yPos += 13;
    }
}

TaskDataPrint::~TaskDataPrint()
{
}
