/***********************************************************************
**
**   flightdataprint.cpp
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


#ifdef QT_5
    #include <QtWidgets>
    #include <QPrinter>
    #include <QPrintDialog>
#else
    #include <QtGui>
#endif

#include "flightdataprint.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "target.h"
#include "wgspoint.h"

FlightDataPrint::FlightDataPrint(Flight* currentFlight)
{
  QPrinter printer( QPrinter::ScreenResolution );

  printer.setDocName( "kflog-flight" );
  printer.setCreator( QString( "KFLog " ) + KFLOG_VERSION );
  printer.setOutputFileName( MainWindow::instance()->getApplicationDataDirectory() +
                             "/kflog-flight.pdf" );

  QPrintDialog dialog( &printer, MainWindow::instance() );

  dialog.setWindowTitle( QObject::tr("Print Flight") );
  dialog.setSizeGripEnabled ( true );
  dialog.setOptions( QAbstractPrintDialog::PrintToFile |
                     QAbstractPrintDialog::PrintSelection |
                     QAbstractPrintDialog::PrintPageRange |
                     QAbstractPrintDialog::PrintShowPageSize );

  if( dialog.exec() != QDialog::Accepted )
    {
      return;
    }

  QPainter painter;
  painter.begin( &printer );

  lasttime=0;

  QString temp;
  FlightPoint cPoint;

  QFont font;
  font.setPixelSize( 8 );
  font.setStyle( QFont::StyleItalic );
  font.setStyleHint( QFont::SansSerif );

  painter.setFont( font );

  QString msg = QString("%1created by KFLog %2 (www.kflog.org)")
      .arg( QChar(Qt::Key_copyright) )
      .arg( KFLOG_VERSION );

  painter.drawText( 25, 12, msg );

  font.setPixelSize( 18 );
  font.setWeight( QFont::Bold );

  painter.setFont( font );
  painter.setPen(QPen(Qt::black, 2));
  painter.drawText(50, 50, QObject::tr("Flight Analysis") + ":");
  painter.drawLine(50, 56, 545, 56);

  font.setPixelSize( 9 );
  font.setWeight( QFont::Normal );
  font.setItalic( true );

  painter.setFont( font );
  painter.drawText(50, 58, 495, 20, Qt::AlignTop | Qt::AlignRight,
                   QString(QObject::tr("File")) + ": " + currentFlight->getFileName());

  font.setItalic( false );
  painter.setFont( font );
  painter.drawText(50, 100, QObject::tr("Date") + ":");
  painter.drawText(125, 100, currentFlight->getDate());
  painter.drawText(50, 115, QObject::tr("Pilot") + ":");
  painter.drawText(125, 115, currentFlight->getPilot());
  painter.drawText(50, 130, QObject::tr("Glider") + ":");
  painter.drawText(125, 130, currentFlight->getGliderType() + " / " + currentFlight->getGliderRegistration());

  painter.setPen(QPen(Qt::black, 2));
  painter.drawLine(50, 175, 545, 175);

  font.setPixelSize( 12 );
  font.setWeight( QFont::Bold );
  painter.setFont( font );
  painter.drawText(50, 170, QObject::tr("Flight track") + ":");

  font.setWeight( QFont::Normal );
  font.setPixelSize( 9 );
  painter.setFont( font );
  painter.drawText(50, 190, QObject::tr("Duration") + ":");
  painter.drawText(125, 190,
                   printTime(currentFlight->getLandTime() - currentFlight->getStartTime(), true, true));

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

  painter.setPen(QPen(Qt::black, 2));
  painter.drawLine(50, 340, 545, 340);

  font.setPixelSize( 12 );
  font.setWeight( QFont::Bold );
  painter.setFont( font );
  painter.drawText(50, 335, QObject::tr("Task") + ":");

  font.setPixelSize( 9 );
  font.setWeight( QFont::Normal );
  painter.setFont( font );

  painter.drawText(50, 355, QObject::tr("Type") + ":");

  temp = QObject::tr("%1  Track: %2  Points: %3")
                  .arg(currentFlight->getTaskTypeString(true))
                  .arg(currentFlight->getDistance(true))
                  .arg(currentFlight->getPoints(true));

  painter.drawText(125, 355, temp);

  QList<Waypoint*> wpList = currentFlight->getOriginalWPList(); // use original task
  int yPos = 375;

  for( int loop = 0; loop < wpList.count(); loop++ )
    {
      __printPositionData( &painter, wpList.at( loop ), yPos );
      yPos += 13;
    }

  if( currentFlight->isOptimized() )
    {
      wpList = currentFlight->getWPList();
      yPos += 20;

      font.setPixelSize( 12 );
      font.setWeight( QFont::Bold );
      painter.setFont( font );
      painter.drawText(50, yPos, QObject::tr("Optimized Task") + ":");
      yPos += 5;

      painter.setPen(QPen(Qt::black, 2));
      painter.drawLine(50, yPos, 545, yPos);

      font.setPixelSize( 9 );
      font.setWeight( QFont::Normal );
      painter.setFont( font );
      yPos += 15;

      painter.drawText(50, yPos, QObject::tr("Type") + ":");

      temp = QObject::tr("%1  Track: %2  Points: %3").arg(
            currentFlight->getTaskTypeString()).arg(
            currentFlight->getDistance()).arg(
            currentFlight->getPoints());
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

void FlightDataPrint::__printPositionData( QPainter* painter,
                                           FlightPoint* cPoint,
                                           int yPos,
                                           QString text,
                                           bool printVario,
                                           bool printSpeed )
{
  QString temp;

  painter->drawText(50, yPos, text);
  painter->drawText(145, yPos, WGSPoint::printPos(cPoint->origP.lat(), true));
  painter->drawText(220, yPos, "/");
  painter->drawText(230, yPos, WGSPoint::printPos(cPoint->origP.lon(), false));

  painter->drawText(300, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
                    printTime(cPoint->time, true, true));
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
  painter->drawText(145, yPos, WGSPoint::printPos(cPoint->origP.lat(), true));
  painter->drawText(220, yPos, "/");
  painter->drawText(230, yPos, WGSPoint::printPos(cPoint->origP.lon(), false));

  if(cPoint->fixTime != 0){
      time=cPoint->fixTime;
      painter->drawText(300, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
                        printTime(cPoint->fixTime, true, true));
      nospeed=false;
  }
  else if(cPoint->sector1 != 0){
      time=cPoint->sector1;
      painter->drawText(290, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
                        printTime(cPoint->sector1, true, true));
      nospeed=false;
  }
  else if(cPoint->sector2 != 0)
    {
      time=cPoint->sector2;
      QFont font = painter->font();
      font.setItalic( true );
      painter->setFont(font);
      painter->drawText(290, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
                        printTime(cPoint->sector2, true, true));
      font.setItalic( false );
      painter->setFont(font);
      nospeed=false;
    }
  else
      painter->drawText(290, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight, "-");

  if(cPoint->sectorFAI != 0){
      painter->drawText(360, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight,
                        printTime(cPoint->sectorFAI, true, true));
      nospeed=false;
  }
  else{
      painter->drawText(360, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight, "-");
      nospeed=true;
  }

  if(cPoint->distance != 0)
    {
      temp.sprintf("%.1f km", cPoint->distance);
      painter->drawText(410, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight, temp);

    if(time && lasttime && !nospeed)
      {
        float speed = cPoint->distance/(time-lasttime)*3600.0;

        if( speed > 0.0 && speed < 500.0 )
          { // suppress nonsense values
            temp.sprintf("%.1f km/h", speed);
            painter->drawText(470, yPos - 18, 55, 20, Qt::AlignBottom | Qt::AlignRight, temp);
        }
      }
    }

  lasttime=time;
}
