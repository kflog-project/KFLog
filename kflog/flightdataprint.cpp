/***********************************************************************
**
**   flightdataprint.cpp
**
**   This file is part of KFLog2.
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

#include "flightdataprint.h"
#include "mapcalc.h"

#define VERSION "3.0"

FlightDataPrint::FlightDataPrint(Flight* currentFlight)
{
  QPrinter printer(QPrinter::PrinterResolution);

  if(!printer.setup())  return;

  printer.setDocName("kflog-map.ps");
  printer.setCreator((QString)"KFLog " + VERSION);

  lasttime=0;

  printer.setFullPage(true);

  QString temp;

  flightPoint cPoint;

  QPainter painter(&printer);

  QFont font;
  font.setPointSize(18);
  font.setWeight(QFont::Bold);

  painter.setFont( font );
  painter.drawText(50, 50, QObject::tr("Flightanalysis") + ":");
  painter.setPen(QPen(QColor(0, 0, 0), 2));
  painter.drawLine(50, 56, 545, 56);
//  painter.setFont(QFont("helvetica", 10, QFont::Normal, true));

  font.setPointSize( 9 );
  font.setWeight( QFont::Normal );
  font.setItalic( true );
  
  painter.setFont( font );
  painter.drawText(50, 58, 495, 20, Qt::AlignTop | Qt::AlignRight,
      (QString)QObject::tr("File") + ": " + currentFlight->getFileName());

  font.setItalic( false );
  painter.setFont( font );
  painter.drawText(50, 100, QObject::tr("Date") + ":");
  painter.drawText(125, 100, currentFlight->getDate().toString());
  painter.drawText(50, 115, QObject::tr("Pilot") + ":");
  painter.drawText(125, 115, currentFlight->getPilot());
  painter.drawText(50, 130, QObject::tr("Glider") + ":");
  painter.drawText(125, 130,
        currentFlight->getType() + " / " + currentFlight->getID());

  painter.setPen(QPen(QColor(0,0,0), 1));
  painter.drawLine(50, 175, 545, 175);

  font.setPointSize( 12 );
  font.setWeight( QFont::Bold );
  painter.setFont( font );
  painter.drawText(50, 170, QObject::tr("Flighttrack") + ":");

  font.setWeight( QFont::Normal );
  font.setPointSize( 9 );
  painter.setFont( font );
  painter.drawText(50, 190, QObject::tr("Duration") + ":");
  painter.drawText(125, 190,
      printTime(currentFlight->getLandTime() - currentFlight->getStartTime()));

  cPoint = currentFlight->getPoint(currentFlight->getStartIndex());
  __printPositionData(&painter, &cPoint, 210, QObject::tr("Takeoff") + ":");

  cPoint = currentFlight->getPoint(currentFlight->getLandIndex());
  __printPositionData(&painter, &cPoint, 223, QObject::tr("Landing") + ":");

  cPoint = currentFlight->getPoint(Flight::H_MAX);
  __printPositionData(&painter, &cPoint, 248, QObject::tr("max. Altitude") + ":",
      true, true);

  cPoint = currentFlight->getPoint(Flight::VA_MAX);
  __printPositionData(&painter, &cPoint, 261, QObject::tr("max. Vario") + ":",
      true, true);

  cPoint = currentFlight->getPoint(Flight::VA_MIN);
  __printPositionData(&painter, &cPoint, 274, QObject::tr("min. Vario") + ":",
      true, true);

  cPoint = currentFlight->getPoint(Flight::V_MAX);
  __printPositionData(&painter, &cPoint, 287, QObject::tr("max. Speed") + ":",
      true, true);

  painter.setPen(QPen(QColor(0,0,0), 1));
  painter.drawLine(50, 340, 545, 340);

  font.setPointSize( 12 );
  font.setWeight( QFont::Bold );
  painter.setFont( font );
  painter.drawText(50, 335, QObject::tr("Task") + ":");

  font.setPointSize( 9 );
  font.setWeight( QFont::Normal );
  painter.setFont( font );

  painter.drawText(50, 355, QObject::tr("Typ") + ":");
  temp = QObject::tr("%1  Track: %2  Points: %3").arg(
            (const char*)currentFlight->getTaskTypeString(true)).arg(
            (const char*)currentFlight->getDistance(true)).arg(
            (const char*)currentFlight->getPoints(true));
  painter.drawText(125, 355, temp);

  QList<Waypoint*> wpList = currentFlight->getOriginalWPList(); // use original task
  int yPos = 375;
  for(int loop = 0; loop < wpList.count(); loop++)
    {
      __printPositionData(&painter, wpList.at(loop), yPos);
      yPos += 13;
    }

  if(currentFlight->isOptimized())
    {
      wpList = currentFlight->getWPList();
      yPos += 20;

      font.setPointSize( 12 );
      font.setWeight( QFont::Bold );
      painter.setFont( font );
      painter.drawText(50, yPos, QObject::tr("Optimized Task") + ":");
      yPos += 5;

      painter.setPen(QPen(QColor(0,0,0), 1));
      painter.drawLine(50, yPos, 545, yPos);

      font.setPointSize( 9 );
      font.setWeight( QFont::Normal );
      painter.setFont( font );
      yPos += 15;

      painter.drawText(50, yPos, QObject::tr("Typ") + ":");
      temp = QObject::tr("%1  Track: %2  Points: %3").arg(
            (const char*)currentFlight->getTaskTypeString()).arg(
            (const char*)currentFlight->getDistance()).arg(
            (const char*)currentFlight->getPoints());
      painter.drawText(125, yPos, temp);
      yPos += 20;

      for(int loop = 0; loop < wpList.count(); loop++)
        {
          __printPositionData(&painter, wpList.at(loop), yPos);
          yPos += 13;
        }
    }

  painter.end();

}

FlightDataPrint::~FlightDataPrint()
{

}

void FlightDataPrint::__printPositionData(QPainter* painter,
        flightPoint* cPoint, int yPos, const char* text,
        bool printVario, bool printSpeed)
{
  QString temp;
  painter->drawText(50, yPos, text);
  painter->drawText(145, yPos, printPos(cPoint->origP.lat(), true));
  painter->drawText(220, yPos, "/");
  painter->drawText(230, yPos, printPos(cPoint->origP.lon(), false));

  painter->drawText(300, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
            printTime(cPoint->time));
  temp.sprintf("%d m", cPoint->height);
  painter->drawText(360, yPos - 18, 45, 20, Qt::AlignBottom | Qt::AlignRight, temp);

  if(printVario)
    {
      temp.sprintf("%.1f m/s", getVario(*cPoint));
      painter->drawText(405, yPos - 18, 60, 20, Qt::AlignBottom | Qt::AlignRight, temp);
    }
  if(printSpeed)
    {
      temp.sprintf("%.1f km/h", getSpeed(*cPoint));
      painter->drawText(470, yPos - 18, 65, 20, Qt::AlignBottom | Qt::AlignRight, temp);
    }
}

void FlightDataPrint::__printPositionData(QPainter *painter, Waypoint *cPoint, int yPos)
{
  /*
   * Use italic font if waypoint was not reached
   */
  QString temp;
  bool nospeed;
  painter->drawText(50, yPos, cPoint->name);
  painter->drawText(145, yPos, printPos(cPoint->origP.lat(), true));
  painter->drawText(220, yPos, "/");
  painter->drawText(230, yPos, printPos(cPoint->origP.lon(), false));

  if(cPoint->fixTime != 0){
      time=cPoint->fixTime;
      painter->drawText(300, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
              printTime(cPoint->fixTime));
      nospeed=false;
  }
  else if(cPoint->sector1 != 0){
      time=cPoint->sector1;
      painter->drawText(290, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
              printTime(cPoint->sector1));
      nospeed=false;
  }
  else if(cPoint->sector2 != 0)
    {
      time=cPoint->sector2;
      painter->setFont(QFont("helvetica", 9, QFont::Normal, true));
      painter->drawText(290, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
              printTime(cPoint->sector2));
      painter->setFont(QFont("helvetica", 9));
      nospeed=false;
    }
  else
      painter->drawText(290, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight, "-");

  if(cPoint->sectorFAI != 0){
      painter->drawText(360, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
              printTime(cPoint->sectorFAI));
      nospeed=false;
  }
  else{
      painter->drawText(360, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight, "-");
      nospeed=true;
  }

  if(cPoint->distance != 0)
    {
      temp.sprintf("%.1f km", cPoint->distance);
      painter->drawText(410, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
            temp);
    if(time && lasttime && !nospeed)
      {
        float speed = cPoint->distance/(time-lasttime)*3600.0;
        if ( (speed>0.0) && (speed<500.0)){ // suppress nonsense values
          temp.sprintf("%.1f km/h", speed);
          painter->drawText(470, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
                temp);
        }
      }
    }
  lasttime=time;
}
