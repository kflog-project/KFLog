/***********************************************************************
**
**   printdialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "printdialog.h"

#include <cmath>

#include <flight.h>
#include <kflog.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapmatrix.h>

#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qprinter.h>

#define TOP_LEFT_X   ( ( 0 + leftMargin ) * 2 )
#define TOP_LEFT_Y   ( ( 0 + topMargin ) * 2 )
#define TOP_RIGHT_X  ( ( pS.width() - rightMargin ) * 2 )
#define TOP_RIGHT_Y  ( ( 0 + topMargin ) * 2 )
#define BOT_LEFT_X   ( ( 0 + leftMargin ) * 2 )
#define BOT_LEFT_Y   ( ( pS.height() - bottomMargin ) * 2 )
#define BOT_RIGHT_X  ( ( pS.width() - rightMargin ) * 2 )
#define BOT_RIGHT_Y  ( ( pS.height() - bottomMargin ) * 2 )

#define GRID_LEFT_X  ( TOP_LEFT_X + 30 )
#define GRID_RIGHT_X ( TOP_RIGHT_X - 30 )
#define GRID_TOP_Y   ( TOP_LEFT_Y + 30 )
#define GRID_BOT_Y   ( BOT_LEFT_Y - 95 )

PrintDialog::PrintDialog(KFLogApp* mApp, bool isMap)
: QDialog(0, "printdialog", true),
  mainApp(mApp)
{
  this->connect(&printProc, SIGNAL(receivedStdout(KProcess*, char*, int)),
                SLOT(slotReadPrintStderr(KProcess*, char*, int)));
  /* Meldungen über falsche Druckerwarteschlangen, ... landen auf stdout
   * und nicht stderr. Daher muss auch dieser Kanal abgehört werden.
   */
  this->connect(&printProc, SIGNAL(processExited(KProcess*)),
                SLOT(slotExitPrinting(KProcess*)));

  __createLayout(isMap);

  extern MapMatrix _globalMapMatrix;
  QPoint temp = _globalMapMatrix.getMapCenter();
  mapCenterLat = temp.x();
  mapCenterLon = temp.y();
}

PrintDialog::~PrintDialog()
{

}

void PrintDialog::__drawGrid(const double selectedScale, QPainter* gridP,
    const QSize pS, const QRect mapBorder,
    const double dX, const double dY, const double gridLeft,
    const double gridRight, const double gridTop, const double gridBot)
{
  extern const MapMatrix _globalMapMatrix;

  gridP->setBrush(NoBrush);

  QString text;
  if(mapCenterLon > 0)
    {
      const int lon1 = mapBorder.left() / 600000 - 1;
      const int lon2 = mapBorder.right() / 600000 + 1;
      const int lat1 = mapBorder.top() / 600000 + 1;
      const int lat2 = mapBorder.bottom() / 600000 - 1;

      /* Abstand zwischen zwei Linien in Minuten
       * Wenn __drawGrid() aufgerufen wird, ist der Maßstab bereits
       * skaliert worden.
       */
      int step = 1;
      if(selectedScale * 2.0 >= scaleRange[0]) step = 30;
      else if(selectedScale * 2.0 >= scaleRange[1]) step = 10;
      else if(selectedScale * 2.0 >= scaleRange[2]) step = 5;
      else if(selectedScale * 2.0 >= scaleRange[3]) step = 2;

      QPoint cP, cP2, cP3, cP4;

      /* Zunächst die Längengrade: */
      for(int loop = lon1; loop <= lon2; loop++)
        {
          cP = _globalMapMatrix.print(mapBorder.top(), (loop * 600000),
              dX, dY);
          cP2 = _globalMapMatrix.print(mapBorder.bottom(), (loop * 600000),
              dX, dY);

          /* Die Hauptlinien */
          gridP->setPen(QPen(QColor(0,0,0), 1));
          gridP->drawLine(cP.x(), cP.y(), cP2.x(), cP2.y());

          // Hier könnte mal 'ne Abfrage hin, ob wir zu nah am Rand sind ...
          gridP->setFont(QFont("helvetica", 14, QFont::Bold));
          text.sprintf("%d°", loop);
          if(cP.x() > gridLeft && cP.x() < gridRight)
              gridP->drawText(cP.x() - 101, gridTop - 28, 100, 50,
                  AlignTop | AlignRight, text);
          if(cP2.x() > gridLeft && cP2.x() < gridRight)
              gridP->drawText(cP2.x() - 101, gridBot + 2, 100, 50,
                  AlignTop | AlignRight, text);

          /* Die kleineren Linien */
          gridP->setFont(QFont("helvetica", 12));
          int number = (int) (60.0 / step);
          for(int loop2 = 1; loop2 < number; loop2++)
            {
              cP = _globalMapMatrix.print(mapBorder.top(),
                  ((loop + (loop2 * step / 60.0)) * 600000), dX, dY);
              cP2 = _globalMapMatrix.print(mapBorder.bottom(),
                  ((loop + (loop2 * step / 60.0)) * 600000), dX, dY);

              if(loop2 == (number / 2.0))
                  gridP->setPen(QPen(QColor(0,0,0), 1, DashLine));
              else
                  gridP->setPen(QPen(QColor(0,0,0), 1, DotLine));

              /* Die Linie wird nur gezeichnet, wenn das obere Ende innerhalb
               * der Breite des Gitternetzes liegt. Das klappt so allerdings
               * nur auf der Nordhalbkugel ...
               */
              if(cP.x() > gridLeft && cP.x() < gridRight)
                {
                  gridP->drawLine(cP.x(), cP.y(), cP2.x(), cP2.y());
                  text.sprintf("%d°", loop);
                  gridP->drawText(cP.x() - 101, gridTop - 27, 100, 50,
                      AlignTop | AlignRight, text);
                  text.sprintf("%d'", loop2 * step);
                  gridP->drawText(cP.x() + 3, gridTop - 27, 100, 50,
                      AlignTop | AlignLeft, text);
                  if(cP2.x() > gridLeft && cP2.x() < gridRight)
                    {
                      text.sprintf("%d°", loop);
                      gridP->drawText(cP2.x() - 101, gridBot + 3, 100, 50,
                          AlignTop | AlignRight, text);
                      text.sprintf("%d'", loop2 * step);
                      gridP->drawText(cP2.x() + 3, gridBot + 3, 100, 50,
                          AlignTop | AlignLeft, text);
                    }
                }
            }
        }

      /* Damit keine Längengrade in den Rand ragen, wird links und rechts je
       * ein weißer Strich gezogen. Damit wird eventuell auch der Text des
       * östlichsten Längengrades überdeckt.
       */
      gridP->setPen(QPen(QColor(255, 255, 255)));
      gridP->setBrush(QBrush(QColor(255, 255, 255), SolidPattern));
      gridP->drawRect(gridLeft - 30, gridTop, 30, pS.height() * 2);
      gridP->drawRect(gridRight, gridTop, 30, pS.height() * 2);

      cP2 = _globalMapMatrix.print(mapBorder.top(), mapBorder.left(),
          dX, dY);
      cP3 = _globalMapMatrix.print(mapBorder.bottom(), mapBorder.right(),
          dX, dY);
      /* Hier könnte es noch passieren, dass die Breitengrade in den Rand
       * hineinragen ...
       */
      for(int loop = 0; loop < (lat1 - lat2 + 1) ; loop++)
        {
          int size = (lon2 - lon1 + 1) * 10;
          QPointArray pointArray(size);

          for(int lonloop = 0; lonloop < size; lonloop++)
            {
              cP = _globalMapMatrix.print((lat2 + loop) * 600000,
                        (lon1 + (lonloop * 0.1)) * 600000, dX, dY);
              pointArray.setPoint(lonloop, cP.x(), cP.y());
              if(cP.x() < gridLeft)
                {
                  // Wir sind noch westlich der Westgrenze der Karte ...
                  cP4.setX(cP2.x());
                  cP4.setY(cP.y());
                }
            }

          if(cP.y() > gridTop && cP.y() < gridBot)
            {
              /* Die Hauptlinien */
              gridP->setPen(QPen(QColor(0,0,0), 1));
              gridP->drawPolyline(pointArray);
              gridP->setFont(QFont("helvetica", 14, QFont::Bold));
              text.sprintf("%d°", lat2 + loop);
              gridP->drawText(gridLeft - 27, cP4.y() - 52, 100, 50,
                  AlignBottom| AlignLeft, text);
              gridP->drawText(gridRight + 3, cP4.y() - 52, 100, 50,
                  AlignBottom| AlignLeft, text);
            }

          /* Die kleineren Linien */
          gridP->setFont(QFont("helvetica", 12));
          int number = (int) (60.0 / step);
          for(int loop2 = 1; loop2 < number; loop2++)
            {
              QPointArray pointArraySmall(size);

              for(int lonloop = 0; lonloop < size; lonloop++)
                {
                  cP = _globalMapMatrix.print((lat2 + loop +
                      (loop2 * (step / 60.0))) * 600000,
                      (lon1 + (lonloop * 0.1)) * 600000,
                      dX, dY);

                  pointArraySmall.setPoint(lonloop, cP.x(), cP.y());
                  if(cP.x() < gridLeft)
                    {
                      // Wir sind noch westlich der Westgrenze der Karte ...
                      cP4.setX(cP2.x());
                      cP4.setY(cP.y());
                    }
                }

              if(cP.y() > gridTop && cP.y() < gridBot)
                {
                  if(loop2 == (number / 2.0))
                    {
                      gridP->setPen(QPen(QColor(0,0,0), 1, DashLine));
                      gridP->drawPolyline(pointArraySmall);
                    }
                  else
                    {
                      gridP->setPen(QPen(QColor(0,0,0), 1, DotLine));
                      gridP->drawPolyline(pointArraySmall);
                    }
                  text.sprintf("%d°", lat2 + loop);
                  gridP->drawText(gridLeft - 27, cP4.y() - 52, 100, 50,
                      AlignBottom| AlignLeft, text);
                  gridP->drawText(gridRight + 3, cP4.y() - 52, 100, 50,
                      AlignBottom| AlignLeft, text);
                  text.sprintf("%d'", loop2 * step);
                  gridP->drawText(gridLeft - 27, cP4.y() + 2, 100, 50,
                      AlignTop| AlignLeft, text);
                  gridP->drawText(gridRight + 3, cP4.y() + 2, 100, 50,
                      AlignTop| AlignLeft, text);
                }
            }
        }
    }
}

QString PrintDialog::__createMapPrint()
{
  extern MapMatrix _globalMapMatrix;

  QPrinter printer;
  QString fileName;
  /* Es wird immer in eine Datei gedruckt ... */
  printer.setOutputToFile(true);
  printer.setDocName("kflog-map.ps");
  printer.setCreator((QString)"KFLog " + VERSION);
  printer.setFullPage(true);
  printer.setPageSize( QPrinter::A4 );

  /* Besser: Ins KDE-Tempdir schreiben !!! */
  if(printerB->isChecked())
      fileName = "/tmp/kflog-map.ps";
  else
      fileName = fileE->text();

  /* Hier wird bislang nicht geprüft, ob die Datei existiert bzw. erzeugt
   * werden darf ...
   */
  printer.setOutputFileName(fileName);

  if(scaleSelect->currentItem() > 5)
      fatal("KFLog: Wrong number of items in combobox \"scaleSelect\"");

  double selectedScale = scaleRange[scaleSelect->currentItem()];

  QPainter printPainter(&printer);
  const QWMatrix oldMatrix = printPainter.worldMatrix();
  QWMatrix newMatrix = printPainter.worldMatrix();

  const QSize pS = __getPaperSize(&printer);

  double dX, dY;

  _globalMapMatrix.createPrintMatrix(selectedScale, pS);

  __getMargin();
  QRect mapBorder;
  QPoint delta(_globalMapMatrix.print(mapCenterLat, mapCenterLon, 0, 0));

  if(pageOrient == QPrinter::Portrait)
    {
      dX = ( leftMargin + ( pS.width() - leftMargin - rightMargin ) / 2 )
            - delta.x();
      dY = ( topMargin + ( pS.height() - topMargin - bottomMargin ) / 2 )
            - delta.y();

      mapBorder = _globalMapMatrix.getPrintBorder(topMargin - dY, 0,
          BOT_LEFT_Y / 2 - dY, leftMargin - dX, topMargin - dY,
          pS.width() - rightMargin - dX, topMargin - dY, leftMargin - dX);
    }
  else
    {
      newMatrix.rotate(90);
      dY = ( -( leftMargin + ( pS.width() - leftMargin - rightMargin ) / 2 ) )
              - delta.y();
      dX = ( ( topMargin + ( pS.height() - topMargin - bottomMargin ) / 2 ) )
              + delta.x();

      mapBorder = _globalMapMatrix.getPrintBorder( -(BOT_RIGHT_X / 2 + dY), 0,
          -( TOP_LEFT_X / 2 + dY ), leftMargin - dX, leftMargin - dY,
          pS.height() - bottomMargin - dX, leftMargin - dY, topMargin - dX);
    }

  selectedScale /= 2.0;
  dX *= 2;
  dY *= 2;

  newMatrix.scale(0.5, 0.5);

  printPainter.setWorldMatrix(newMatrix);

  /* Der Rahmen für die Koordinaten ist 15 Punkte breit */
  if(pageOrient == QPrinter::Portrait)
    {
      printPainter.setClipRect(leftMargin, topMargin,
          pS.width() - leftMargin - rightMargin,
          pS.height() - topMargin - bottomMargin - 33);

      __drawGrid(selectedScale, &printPainter, pS, mapBorder, dX, dY,
          TOP_LEFT_X + 30, TOP_RIGHT_X - 30, TOP_LEFT_Y + 30, BOT_LEFT_Y - 95);

      printPainter.setClipRect(leftMargin + 15, topMargin + 15,
          pS.width() - leftMargin - rightMargin - 30,
          pS.height() - topMargin - bottomMargin - 63);

      QSize tempS(pS.width() - leftMargin - rightMargin - 30,
          pS.height() - topMargin - bottomMargin - 63);

      _globalMapMatrix.createPrintMatrix(selectedScale,
          tempS * 2, dX, dY);
    }
  else
    {
      printPainter.setClipRect(leftMargin + 33, topMargin,
          pS.width() - leftMargin - rightMargin - 33,
          pS.height() - topMargin - bottomMargin);

      __drawGrid(selectedScale, &printPainter, pS, mapBorder, dX, dY,
          TOP_LEFT_Y + 30, BOT_LEFT_Y - 30,
          -(TOP_RIGHT_X - 30), -(TOP_LEFT_X + 95));

      printPainter.setClipRect(leftMargin + 48, topMargin + 15,
          pS.width() - leftMargin - rightMargin - 63,
          pS.height() - topMargin - bottomMargin - 30);

      QSize tempS(pS.width() - topMargin - bottomMargin - 63,
          pS.height() - leftMargin - rightMargin - 30);

      _globalMapMatrix.createPrintMatrix(selectedScale,
          tempS * 2, dX, dY);
    }

  extern MapContents _globalMapContents;

  _globalMapContents.printContents(&printPainter);

  printPainter.setClipRect(0, 0, pS.width(), pS.height());

  QString scaleText;
  bool show1 = false;
  bool show5 = false;
  unsigned int stop10, stop1 = 10, stop_small10 = 0, stop_small1 = 10;

  switch(scaleSelect->currentItem())
    {
      case 0:
        scaleText = "1:1.000.000";
        stop10 = 10;
        stop_small10 = 5;
        stop1 = 0;
        break;
      case 1:
        scaleText = "1:500.000";
        stop10 = 5;
        stop_small10 = 3;
        break;
      case 2:
        scaleText = "1:200.000";
        stop10 = 2;
        stop_small10 = 1;
        show5 = true;
        break;
      case 3:
        scaleText = "1:100.000";
        stop10 = 1;
        stop_small10 = 0;
        show5 = true;
        break;
      case 4:
        scaleText = "1:50.000";
        stop10 = 0;
        stop1 = 5;
        stop_small1 = 3;
        show1 = true;
        break;
      case 5:
        scaleText = "1:25.000";
        stop10 = 0;
        stop1 = 2;
        stop_small1 = 1;
        show1 = true;
        break;
      default:
        /* Hier muß jetzt zunächst der verwendete Maßstab berechnet und dann
         * entschieden werden, welche Art der Balken-Darstellung verwendet wird.
         */
        ;
    }

  // 370 Punkte =^ ca. 13 cm
  if(pS.width() - leftMargin - rightMargin < 370)
    {
      stop10 = stop_small10;
      stop1 = stop_small1;
    }

  int mainLeft = TOP_LEFT_X;
  int mainTop = TOP_LEFT_Y;
  int mainWidth = BOT_RIGHT_X - TOP_LEFT_X;
  int mainHeight = BOT_RIGHT_Y - TOP_LEFT_Y;

  int lineLeftX = BOT_LEFT_X;
  int lineLeftY = BOT_LEFT_Y - 65;
  int lineRightX = BOT_RIGHT_X;
  int lineRightY = BOT_RIGHT_Y - 65;

  int gridLeft = GRID_LEFT_X;
  int gridTop = GRID_TOP_Y;
  int gridWidth = BOT_RIGHT_X - TOP_LEFT_X - 60;
  int gridHeight = BOT_RIGHT_Y - TOP_LEFT_Y - 125;

  int rightTextX = BOT_RIGHT_X - 310;
  int rightTextY = BOT_RIGHT_Y - 65;

  int scaleX = BOT_LEFT_X + 15;
  int scaleY = BOT_LEFT_Y - 25;

  if(pageOrient == QPrinter::Landscape)
    {
      // 370 Punkte =^ ca. 13 cm
      if(pS.height() - topMargin - bottomMargin < 370)
        {
          stop10 = stop_small10;
          stop1 = stop_small1;
        }

      mainLeft = TOP_LEFT_Y;
      mainTop = -TOP_LEFT_X;
      mainWidth = BOT_RIGHT_Y - TOP_LEFT_Y;
      mainHeight = -(BOT_RIGHT_X - TOP_LEFT_X);

      lineLeftX = TOP_LEFT_Y;
      lineLeftY = -TOP_LEFT_X - 65;
      lineRightX = BOT_LEFT_Y;
      lineRightY = -TOP_LEFT_X - 65;

      gridLeft = GRID_TOP_Y;
      gridTop = -GRID_LEFT_X - 65;
      gridWidth = BOT_RIGHT_Y - TOP_LEFT_Y - 60;
      gridHeight = -(BOT_RIGHT_X - TOP_LEFT_X - 125);

      rightTextX = BOT_RIGHT_Y - 300;
      rightTextY = -(BOT_LEFT_X + 65);

      scaleX = TOP_LEFT_Y + 15;
      scaleY = -(TOP_LEFT_X + 25);
    }

  int scale1Y = scaleY - 5;
  int scale5Y = scaleY - 8;
  int scale10Y = scaleY - 10;

  /*
   * Rahmen der Karte und der Legende.
   */
  printPainter.setPen(QPen(QColor(0, 0, 0), 2));
  printPainter.setBrush(NoBrush);
  printPainter.drawRect(mainLeft, mainTop, mainWidth, mainHeight);
  printPainter.drawLine(lineLeftX, lineLeftY, lineRightX, lineRightY);
  printPainter.setPen(QPen(QColor(0, 0, 0), 1));
  printPainter.drawRect(gridLeft, gridTop, gridWidth, gridHeight);

  /*
   * Wenn die Boxen hier zu klein sind, wird die gesamte Ausgabe zerstört.
   * Anscheinend ein Bug in QPainter :-(
   */
  printPainter.setFont(QFont("helvetica", 20, QFont::Bold));
  printPainter.drawText(rightTextX, rightTextY, 300, 50,
          AlignTop | AlignRight, scaleText);
  printPainter.setFont(QFont("helvetica", 12, QFont::Normal, true));
  printPainter.drawText(rightTextX, rightTextY + 30, 296, 30,
          AlignBottom | AlignRight,
          (QString)"printed by KFLog " + VERSION);

  /*
   * Wenn die druckbare Fläche zu klein ist, wird der Maßstabsbalken nicht
   * angepaßt. Der Balken ist bei den festen Maßstäben immer 8 bzw. 10cm breit!
   */
  printPainter.setPen(QPen(QColor(0, 0, 0), 2));
  printPainter.setFont(QFont("helvetica", 15));

  if(stop10 > 0)
      printPainter.drawLine(scaleX, scaleY,
            scaleX + (10000 * stop10 / selectedScale), scaleY);
  else
      printPainter.drawLine(scaleX, scaleY,
            scaleX + (1000 * stop1 / selectedScale), scaleY);

  printPainter.setPen(QPen(QColor(0, 0, 0), 1));
  printPainter.drawLine(scaleX, scaleY, scaleX, scale10Y);
  printPainter.drawText(scaleX - 4, scale10Y - 5, "0");
  printPainter.drawText(scaleX + 2, scaleY + 15, "[km]");

  /* 5- und 10-km Striche zeichnen */
  for(unsigned int loop = 1; loop <= stop10; loop++)
    {
      printPainter.drawLine(scaleX + (10000 * loop / selectedScale), scaleY,
          scaleX + (10000 * loop / selectedScale), scale10Y);
      scaleText.sprintf("%d", loop * 10);
      printPainter.drawText(scaleX - 9 + (10000 * loop / selectedScale),
              scale10Y - 5, scaleText);
      printPainter.drawLine(scaleX + ((10000 * loop - 5000) / selectedScale),
          scaleY, scaleX + ((10000 * loop - 5000) / selectedScale), scale5Y);
      if(show5)
        {
          int dx = 4;
          if(loop > 1) dx = 9;
          scaleText.sprintf("%d", (loop * 10) - 5);
          printPainter.drawText(scaleX - dx +
                ((10000 * loop - 5000) / selectedScale),
              scale10Y - 5, scaleText);
        }
    }

  /* 1-km Striche zeichnen */
  for(unsigned int loop = 1; loop <= stop1; loop++)
    {
      printPainter.drawLine(scaleX + (1000 * loop / selectedScale), scaleY,
            scaleX + (1000 * loop / selectedScale), scale1Y);
      if(show1)
        {
          scaleText.sprintf("%d", loop);
          printPainter.drawText(scaleX - 4 + (1000 * loop / selectedScale),
                scale10Y - 5, scaleText);
        }
    }

  /* Zurücksetzen der Darstellung   */
  printPainter.setWorldMatrix(oldMatrix);
  printPainter.end();

  /* Alles getan, wir beenden den Dialog ... */
  mainApp->slotStatusMsg("");

  return fileName;
}

void PrintDialog::__printPositionData(QPainter* painter,
        struct flightPoint* cPoint, int yPos, const char* text,
        bool printVario, bool printSpeed)
{
  QString temp;
  painter->drawText(50, yPos, text);
  painter->drawText(125, yPos, printPos(cPoint->origP.x(), true));
  painter->drawText(200, yPos, "/");
  painter->drawText(210, yPos, printPos(cPoint->origP.y(), false));

  painter->drawText(270, yPos - 18, 55, 20, AlignBottom | AlignRight,
            printTime(cPoint->time));
  temp.sprintf("%d m", cPoint->height);
  painter->drawText(335, yPos - 18, 45, 20, AlignBottom | AlignRight, temp);

  if(printVario)
    {
//      temp.sprintf("%.1f m/s", getVario(cPoint));
      painter->drawText(385, yPos - 18, 60, 20, AlignBottom | AlignRight, temp);
    }
  if(printSpeed)
    {
//      temp.sprintf("%.1f km/h", getSpeed(cPoint));
      painter->drawText(450, yPos - 18, 65, 20, AlignBottom | AlignRight, temp);
    }
}

void PrintDialog::__printPositionData(QPainter* painter,
        struct wayPoint* cPoint, int yPos)
{
  /*
   * Wenn Punkt nicht erreicht wurde, sollte alles in italic sein.
   */
  QString temp;
  painter->drawText(50, yPos, cPoint->name);
  painter->drawText(125, yPos, printPos(cPoint->origP.x(), true));
  painter->drawText(200, yPos, "/");
  painter->drawText(210, yPos, printPos(cPoint->origP.y(), false));

  if(cPoint->sector1 != 0)
      painter->drawText(270, yPos - 18, 55, 20, AlignBottom | AlignRight,
              printTime(cPoint->sector1));
  else if(cPoint->sector2 != 0)
    {
      painter->setFont(QFont("helvetica", 11, QFont::Normal, true));
      painter->drawText(270, yPos - 18, 55, 20, AlignBottom | AlignRight,
              printTime(cPoint->sector2));
      painter->setFont(QFont("helvetica", 11));
    }
  else
      painter->drawText(270, yPos - 18, 55, 20, AlignBottom | AlignRight, "-");

  if(cPoint->sectorFAI != 0)
      painter->drawText(330, yPos - 18, 55, 20, AlignBottom | AlignRight,
              printTime(cPoint->sectorFAI));
  else
      painter->drawText(330, yPos - 18, 55, 20, AlignBottom | AlignRight, "-");

  if(cPoint->distance != 0)
    {
      temp.sprintf("%.1f km", cPoint->distance);
      painter->drawText(390, yPos - 18, 55, 20, AlignBottom | AlignRight,
            temp);
    }
}

QString PrintDialog::__createFlightPrint()
{
  QPrinter printer;
  printer.setOutputToFile(true);
  printer.setOutputFileName("/tmp/kflog-flightdata.ps");

  QString temp;

  struct flightPoint cPoint;

  QPainter painter(&printer);
//    painter.drawLine(0,0,10,10);
//    painter.drawLine(10,10, 585, 10);
//    painter.drawLine(585, 10, 595, 0);
//    painter.drawLine(585, 10, 585, 832);
//    painter.drawLine(585, 832, 595, 842);
  painter.setFont(QFont("helvetica", 18, QFont::Bold));
  painter.drawText(50, 50, "Flugauswertung:");
  painter.setPen(QPen(QColor(0, 0, 0), 2));
  painter.drawLine(50, 56, 545, 56);
  painter.setFont(QFont("helvetica", 10, QFont::Normal, true));
  painter.drawText(50, 58, 495, 20, AlignTop | AlignRight,
      (QString)i18n("file") + ": " + currentFlight->getFileName());
  painter.setFont(QFont("helvetica", 10));
  painter.drawText(50, 100, "Datum:");
  painter.drawText(125, 100, currentFlight->getDate());
  painter.drawText(50, 115, "Pilot:");
  painter.drawText(125, 115, currentFlight->getPilot());
  painter.drawText(50, 130, "Flugzeug:");
  painter.drawText(125, 130,
        currentFlight->getType() + " / " + currentFlight->getID());

  painter.setFont(QFont("helvetica", 12, QFont::Bold));
  painter.drawText(50, 170, "Flugweg:");

  painter.setFont(QFont("helvetica", 10));
  painter.drawText(50, 190, "Dauer:");
  painter.drawText(125, 190,
      printTime(currentFlight->getLandTime() - currentFlight->getStartTime()));

  cPoint = currentFlight->getPoint(0);
  __printPositionData(&painter, &cPoint, 210, "Start:");

  cPoint = currentFlight->getPoint(currentFlight->getRouteLength() - 1);
  __printPositionData(&painter, &cPoint, 223, "Landung:");

  cPoint = currentFlight->getPoint(Flight::H_MAX);
  __printPositionData(&painter, &cPoint, 248, "max. Höhe:", true, true);

  cPoint = currentFlight->getPoint(Flight::VA_MAX);
  __printPositionData(&painter, &cPoint, 261, "max. Vario:", true, true);

  cPoint = currentFlight->getPoint(Flight::VA_MIN);
  __printPositionData(&painter, &cPoint, 274, "min. Vario:", true, true);

  cPoint = currentFlight->getPoint(Flight::V_MAX);
  __printPositionData(&painter, &cPoint, 287, "max. Speed:", true, true);

  painter.setFont(QFont("helvetica", 12, QFont::Bold));
  painter.drawText(50, 335, "Aufgabe:");
  painter.setFont(QFont("helvetica", 10));

  painter.drawText(50, 355, "Typ:");
  temp.sprintf("%s  Strecke: %s  Punkte: %s",
            (const char*)currentFlight->getRouteType(true),
            (const char*)currentFlight->getDistance(true),
            (const char*)currentFlight->getPoints(true));
  painter.drawText(125, 355, temp);

  QList<struct wayPoint>* wpList = currentFlight->getWPList(true);
  int yPos = 375;
  for(unsigned int loop = 0; loop < wpList->count(); loop++)
    {
      __printPositionData(&painter, wpList->at(loop), yPos);
      yPos += 13;
    }

  if(currentFlight->isOptimized())
    {
      wpList = currentFlight->getWPList();
      yPos += 20;
      painter.setFont(QFont("helvetica", 12, QFont::Bold));
      painter.drawText(50, yPos, "optimierte Aufgabe:");
      painter.setFont(QFont("helvetica", 10));
      yPos += 20;

      painter.drawText(50, yPos, "Typ:");
      temp.sprintf("%s  Strecke: %s  Punkte: %s",
            (const char*)currentFlight->getRouteType(),
            (const char*)currentFlight->getDistance(),
            (const char*)currentFlight->getPoints());
      painter.drawText(125, yPos, temp);
      yPos += 20;

      for(unsigned int loop = 0; loop < wpList->count(); loop++)
        {
          __printPositionData(&painter, wpList->at(loop), yPos);
          yPos += 13;
        }
    }

  painter.end();

  return "/tmp/kflog-flightdata.ps";
}

void PrintDialog::__createLayout(bool isMap)
{
  mainApp->slotStatusMsg(i18n("Printing..."));

  this->setMaximumWidth(400);

  QFrame* bot = new QFrame(this);

  QGroupBox* printBox = new QGroupBox(i18n("Printer Output"), this);
  QGroupBox* paperBox = new QGroupBox(i18n("Paper Format"), this);
  QGroupBox* scaleBox;
  if(isMap)
    {
      scaleBox = new QGroupBox(i18n("Map scale"), this);
      scaleSelect = new KComboBox(this);
      scaleSelect->insertItem("1:1.000.000");
      scaleSelect->insertItem("1:500.000");
      scaleSelect->insertItem("1:200.000");
      scaleSelect->insertItem("1:100.000");
      scaleSelect->insertItem("1:50.000");
      scaleSelect->insertItem("1:25.000");
    }

  printerB = new QRadioButton(i18n("Print to Printer:"), this);
  QLabel* printerCommandL = new QLabel(i18n("Command:"), this);
  printerCommandL->setMinimumWidth(printerCommandL->sizeHint().width() + 5);
  printerCommandE = new QLineEdit(this);
  printerCommandE->setMinimumHeight(printerCommandE->sizeHint().height() + 2);
  printerCommandE->setMaximumHeight(printerCommandE->sizeHint().height() + 5);
  QLabel* printerQueueL = new QLabel(i18n("Printer:"), this);
  printerQueueL->setAlignment(AlignRight | AlignVCenter);
  printerQueueL->setMinimumWidth(printerQueueL->sizeHint().width() + 5);
  printerQueueE = new QLineEdit(this);
  printerQueueE->setMinimumHeight(printerQueueE->sizeHint().height() + 2);
  printerQueueE->setMaximumHeight(printerQueueE->sizeHint().height() + 5);

  fileB = new QRadioButton(i18n("Print to File:"), this);
  fileB->setMinimumWidth(fileB->sizeHint().width());
  fileE = new QLineEdit(this);
  fileE->setMinimumWidth(200);
  fileE->setText("");

  fileSearch = new QPushButton(this);
  fileSearch->setPixmap(BarIcon("fileopen"));
  fileSearch->setMinimumWidth(fileSearch->sizeHint().width() + 5);
  fileSearch->setMinimumHeight(fileSearch->sizeHint().height() + 5);

  QLabel* rand = new QLabel("Margins [mm]:",this);
  rand->setAlignment(AlignCenter);
  portraitB = new QRadioButton(i18n("Portrait"), this);
  landscB = new QRadioButton(i18n("Landscape"), this);
  landscB->setMinimumWidth(landscB->sizeHint().width() + 2);
  leftM = new QSpinBox(this);
  leftM->setMinimumWidth(65);
  rightM = new QSpinBox(this);
  topM = new QSpinBox(this);
  bottomM = new QSpinBox(this);

  format = new QComboBox(this);
  format->insertItem("A2");
  format->insertItem("A3");
  format->insertItem("A4");
  format->insertItem("A5");
  format->insertItem("B2");
  format->insertItem("B3");
  format->insertItem("B4");
  format->insertItem("B5");
  format->insertItem("Executive");
  format->insertItem("Legal");
  format->insertItem("Letter");
  format->insertItem("Tabloid");
  greyB = new QRadioButton(i18n("Grey"),this);
  colorB = new QRadioButton(i18n("Color"),this);

  QGridLayout* pageLayout;

  pageLayout = new QGridLayout(this, 32, 15, 10, 1);
  pageLayout->addMultiCellWidget(printBox,0,9,0,14);
  pageLayout->addMultiCellWidget(paperBox,11,23,0,14);
  if(isMap)
    {
      pageLayout->addMultiCellWidget(scaleBox, 25,29,0,14);
      pageLayout->addMultiCellWidget(scaleSelect,27,27,1,12);
    }

  pageLayout->addMultiCellWidget(printerB,2,2,1,6);
  pageLayout->addWidget(printerCommandL,4,2);
  pageLayout->addMultiCellWidget(printerCommandE,4,4,4,6);
  pageLayout->addWidget(printerQueueL,4,8);
  pageLayout->addMultiCellWidget(printerQueueE,4,4,10,12);

  pageLayout->addMultiCellWidget(fileB,6,6,1,6);
  pageLayout->addMultiCellWidget(fileE,8,8,2,10);
  pageLayout->addWidget(fileSearch,8,12);

  pageLayout->addWidget(portraitB,15,2);
  pageLayout->addMultiCellWidget(rand,13,13,6,12);
  pageLayout->addWidget(landscB,17,2);
  pageLayout->addWidget(topM,15,8);
  pageLayout->addWidget(leftM,17,6);
  pageLayout->addMultiCellWidget(rightM,17,17,10,12);
  pageLayout->addWidget(bottomM,19,8);
  pageLayout->addWidget(new QLabel("Format:",this),19,2);
  pageLayout->addMultiCellWidget(format,21,21,2,4);
  pageLayout->addWidget(greyB,21,8);
  pageLayout->addMultiCellWidget(colorB,21,21,10,12);

  pageLayout->addColSpacing(0,8);
  pageLayout->addColSpacing(1,15);
  pageLayout->addColSpacing(3,3);
  pageLayout->setColStretch(4,40);
  pageLayout->addColSpacing(5,5);
  pageLayout->addColSpacing(6,35);
  pageLayout->addColSpacing(7,3);
  pageLayout->addColSpacing(8,40);
  pageLayout->addColSpacing(9,5);
  pageLayout->addColSpacing(10,25);
  pageLayout->addColSpacing(11,8);
  pageLayout->addColSpacing(13,8);

  pageLayout->addRowSpacing(0,15);
  pageLayout->addRowSpacing(1,5);
  pageLayout->setRowStretch(1,0);
  pageLayout->addRowSpacing(2, printerB->sizeHint().height() + 2);
  pageLayout->addRowSpacing(3,5);
  pageLayout->setRowStretch(3,0);
  pageLayout->addRowSpacing(5,5);
  pageLayout->setRowStretch(5,0);
  pageLayout->addRowSpacing(6, fileB->sizeHint().height() + 2);
  pageLayout->addRowSpacing(9,8);
  pageLayout->setRowStretch(9,0);
  pageLayout->addRowSpacing(10,5);
  pageLayout->addRowSpacing(11,15);
  pageLayout->addRowSpacing(12,5);
  pageLayout->setRowStretch(12,0);
  pageLayout->addRowSpacing(13,20);
  pageLayout->setRowStretch(13,0);
  pageLayout->addRowSpacing(14,5);
  pageLayout->setRowStretch(14,0);
  pageLayout->addRowSpacing(15, portraitB->sizeHint().height() + 2);
  pageLayout->addRowSpacing(16,3);
  pageLayout->setRowStretch(16,0);
  pageLayout->addRowSpacing(17, portraitB->sizeHint().height() + 2);
  pageLayout->addRowSpacing(18,3);
  pageLayout->setRowStretch(18,0);
  pageLayout->addRowSpacing(19, bottomM->sizeHint().height() + 2);
  pageLayout->addRowSpacing(20,3);
  pageLayout->setRowStretch(20,0);
  pageLayout->addRowSpacing(21, format->sizeHint().height() + 2);
  pageLayout->addRowSpacing(23,8);
  pageLayout->addRowSpacing(24,5);

  if(isMap)
    {
      pageLayout->addRowSpacing(25,15);
      pageLayout->addRowSpacing(26,5);
      pageLayout->addRowSpacing(27, scaleSelect->sizeHint().height() + 2);
      pageLayout->addRowSpacing(28,3);
      pageLayout->addRowSpacing(29,8);
      pageLayout->addRowSpacing(30,10);
    }

  QPushButton* printB = new QPushButton(i18n("&Print"), bot);
  printB->setMinimumWidth(printB->sizeHint().width() + 3);
  printB->setMinimumHeight(printB->sizeHint().height());
  QPushButton* previewB = new QPushButton(i18n("Pre&view"), bot);
  previewB->setMinimumWidth(previewB->sizeHint().width() + 3);
  QPushButton* cancelB = new QPushButton(i18n("Cancel"), bot);
  cancelB->setMinimumWidth(cancelB->sizeHint().width() + 3);

  QGridLayout* botLayout = new QGridLayout(bot,1,5,0,1);
  botLayout->addWidget(printB,0,0);
  botLayout->addWidget(previewB,0,2);
  botLayout->addWidget(cancelB,0,4);

  botLayout->addColSpacing(1,5);
  botLayout->setColStretch(3,1);
  botLayout->addColSpacing(3,5);

  pageLayout->addMultiCellWidget(bot, 31, 31, 0, 12);

  slotToggleFileOut(fileB->isChecked());

  this->connect(landscB, SIGNAL(toggled(bool)),
      SLOT(slotToggleLandscape(bool)));
  this->connect(portraitB, SIGNAL(toggled(bool)),
      SLOT(slotTogglePortrait(bool)));

  this->connect(colorB, SIGNAL(toggled(bool)), SLOT(slotToggleColor(bool)));
  this->connect(greyB, SIGNAL(toggled(bool)), SLOT(slotToggleGrayscale(bool)));

  this->connect(fileB, SIGNAL(toggled(bool)), SLOT(slotToggleFileOut(bool)));
  this->connect(printerB, SIGNAL(toggled(bool)),
      SLOT(slotTogglePrintOut(bool)));
  this->connect(fileSearch, SIGNAL(clicked()), SLOT(slotSearchFileOut()));

  this->connect(cancelB, SIGNAL(clicked()), SLOT(reject()));

  this->connect(previewB, SIGNAL(clicked()), SLOT(slotPreview()));
  this->connect(printB, SIGNAL(clicked()), SLOT(slotPrint()));

  pageLayout->activate();
}

void PrintDialog::__readOptions()
{
  int paperSize;
  KConfig* config = kapp->config();
  config->setGroup("Printing");

  QString printer = config->readEntry("DefaultPrinter", "lp");
  printCommand = config->readEntry("PrintingCommand", "lpr");
  previewCommand = config->readEntry("PreviewCommand", "kghostview");
  int marginL = config->readNumEntry("MarginLeft", 15);
  int marginR = config->readNumEntry("MarginRight", 20);
  int marginT = config->readNumEntry("MarginTop", 15);
  int marginB = config->readNumEntry("MarginBottom", 20);

  if(printMode == Map)
    {
      paperSize = config->readNumEntry("MapPaperSize", A4);
      pageOrient = config->readNumEntry("MapPaperOrientation",
          QPrinter::Portrait);
      colorMode = config->readNumEntry("MapColorMode", QPrinter::GrayScale);
      scaleSelect->setCurrentItem(config->readNumEntry("MapPrintScale", 1));
    }
  else
    {
      paperSize = config->readNumEntry("PaperSize", A4);
      pageOrient = config->readNumEntry("PaperOrientation",
          QPrinter::Portrait);
      colorMode = config->readNumEntry("ColorMode", QPrinter::GrayScale);
    }

  format->setCurrentItem(paperSize);

  if(pageOrient == QPrinter::Portrait)
    {
      portraitB->setChecked(true);
      landscB->setChecked(false);
    }
  else
    {
      portraitB->setChecked(false);
      landscB->setChecked(true);
    }

  if(colorMode == QPrinter::GrayScale)
    {
      greyB->setChecked(true);
      colorB->setChecked(false);
    }
  else
    {
      greyB->setChecked(false);
      colorB->setChecked(true);
    }

  leftM->setValue(marginL);
  rightM->setValue(marginR);
  topM->setValue(marginT);
  bottomM->setValue(marginB);
  printerQueueE->setText(printer);
  printerCommandE->setText(printCommand);

  config->setGroup(0);
}

void PrintDialog::__writeOptions()
{
  KConfig* config = kapp->config();
  config->setGroup("Printing");
  config->writeEntry("DefaultPrinter", printerQueueE->text());
  config->writeEntry("PrintingCommand", printCommand);

  config->writeEntry("MarginLeft", leftM->value());
  config->writeEntry("MarginRight", rightM->value());
  config->writeEntry("MarginTop", topM->value());
  config->writeEntry("MarginBottom", bottomM->value());

  if(printMode == Map)
    {
      config->writeEntry("MapPaperSize", format->currentItem());
      config->writeEntry("MapPaperOrientation", pageOrient);
      config->writeEntry("MapColorMode", colorMode);
      config->writeEntry("MapPrintScale", scaleSelect->currentItem());
    }
  else
    {
      config->writeEntry("PaperSize", format->currentItem());
      config->writeEntry("PaperOrientation", pageOrient);
      config->writeEntry("ColorMode", colorMode);
    }

  config->setGroup(0);
}

void PrintDialog::openMapPrintDialog()
{
  mainApp->slotStatusMsg(i18n("Printing..."));

  this->setCaption(i18n("Printing Map"));
  printMode = Map;

  /* Wenn auch Flüge und Aufgaben zur Auswahl stehen, muss das
   * entsprechend länger werden ... */
  scaleRange = new double[6];
  scaleRange[0] = 1000.0 / 72 * 25.4;          /* 1:1.000.000 */
  scaleRange[1] =  500.0 / 72 * 25.4;          /* 1:500.000   */
  scaleRange[2] =  200.0 / 72 * 25.4;          /* 1:200.000   */
  scaleRange[3] =  100.0 / 72 * 25.4;          /* 1:100.000   */
  scaleRange[4] =   50.0 / 72 * 25.4;          /* 1:50.000    */
  scaleRange[5] =   25.0 / 72 * 25.4;          /* 1:25.000    */

  __readOptions();

  if(this->exec() == QDialog::Accepted) __writeOptions();

  mainApp->slotStatusMsg("");
}

void PrintDialog::openFlightPrintDialog(Flight* cFlight)
{
  currentFlight = cFlight;
  mainApp->slotStatusMsg(i18n("Printing..."));

  this->setCaption(i18n("Printing Flight-data"));
  printMode = FlightData;

  __readOptions();

  if(this->exec() == QDialog::Accepted)  __writeOptions();

  mainApp->slotStatusMsg("");
}

void PrintDialog::slotToggleColor(bool isColor)
{
  greyB->setChecked(!isColor);
  colorMode = Color;
}

void PrintDialog::slotToggleGrayscale(bool isGray)
{
  colorB->setChecked(!isGray);
  colorMode = QPrinter::GrayScale;
}

void PrintDialog::slotToggleFileOut(bool isFile)
{
  fileE->setEnabled(isFile);
  fileSearch->setEnabled(isFile);
  printerQueueE->setEnabled(!isFile);
  printerCommandE->setEnabled(!isFile);
  printerB->setChecked(!isFile);
}

void PrintDialog::slotTogglePrintOut(bool isPrinter)
{
  fileB->setChecked(!isPrinter);
  slotToggleFileOut(!isPrinter);
}

void PrintDialog::slotToggleLandscape(bool isLandscape)
{
  portraitB->setChecked(!isLandscape);
  pageOrient = QPrinter::Landscape;
}

void PrintDialog::slotTogglePortrait(bool isPortrait)
{
  landscB->setChecked(!isPortrait);
  pageOrient = QPrinter::Portrait;
}

void PrintDialog::slotPreview()
{
  QString fileName;

  switch(printMode)
    {
      case Map:
        fileName = __createMapPrint();
        break;
      case FlightData:
        fileName = __createFlightPrint();
        break;
    }

  // Jetzt darf angezeigt werden ...
  previewProc.clearArguments();
//  previewProc << "ghostview" << "-magstep" << "-2" <<  fileName;
  previewProc << "kghostview" << fileName;
//  previewProc << previewCommand <<  fileName;

  // Hier wird nicht gewartet, ob der Prozess irgendwann beendet wird.
  // Ein erneutes Aufrufen erzeugt ein weiteres Fenster ...
  previewProc.start();
}

void PrintDialog::slotPrint()
{
  QString fileName;

  switch(printMode)
    {
      case Map:
        fileName = __createMapPrint();
        break;
      case FlightData:
        fileName = __createFlightPrint();
        break;
    }

  if(fileB->isChecked())
    {
      KMessageBox::information(this, i18n("\nThe file has been created!\n"),
          i18n("File created!"));
      this->accept();
    }
  else
    {
      // Jetzt darf ausgedruckt werden ...
      printProc.clearArguments();
      printProc << printCommand << "-P" << printerQueueE->text() << fileName;
      printProc << printCommand << fileName;
      printProc.start(KProcess::NotifyOnExit,
                      KProcess::Communication(KProcess::All));
    }
}

void PrintDialog::slotReadPrintStderr(KProcess* proc, char* str, int str_len)
{
  KMessageBox::error(this, i18n("Could not send the map to the printer!\n"
      "The spooler send the following message:") +
      "\n\n" + ((QString)str).left(str_len), i18n("Error occurred!"));
}

void PrintDialog::slotExitPrinting(KProcess*)
{
  printProc.normalExit();

  if(printProc.exitStatus() == 0)
    {
      KMessageBox::information(this,
          i18n("\nThe file was sent to the printer!\n"), i18n("File spooled!"));
      this->accept();
    }
}

void PrintDialog::slotSearchFileOut()
{
  fileE->setText(KFileDialog::getOpenFileName("", "*.ps", this));
}
