/***********************************************************************
**
**   flight.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**                :  1999/10/03 Some fixes by Jan Max Walter Krueger
**                :  ( jkrueger@physics.otago.ac.nz )
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <unistd.h>

#include "flight.h"

#include <kflog.h>
#include <mapcalc.h>
#include <mapmatrix.h>
#include <wp.h>

#include <kapp.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlist.h>
#include <qpointarray.h>
#include <qregion.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <qfileinfo.h>

#define PRE_ID loop - 1
#define CUR_ID loop
#define NEXT_ID loop + 1

/* Anzahl der zu merkenden besten Flüge */
#define NUM_TASKS 3
#define NUM_TASKS_POINTS ( NUM_TASKS * 3 )
#define MAX_TASK_ID NUM_TASKS - 1

/* Die Einstellungen können mal in die Voreinstellungsdatei wandern ... */
#define FAI_POINT 2.0
#define NORMAL_POINT 1.75
#define R1 (3000.0 / _currentScale)
#define R2 (500.0 / _currentScale)

#define GET_SPEED(a) ( (float)a->dS / (float)a->dT )
#define GET_VARIO(a) ( (float)a->dH / (float)a->dT )

/* Maximale Vergrößerung beim Prüfen! */
#define SCALE 10.0

Flight::Flight(QString fName, QList<flightPoint> r, QString pName,
    QString gType, QString gID, QList<struct wayPoint> wpL, QString d)
  : BaseMapElement("flight", BaseMapElement::Flight),
    pilotName(pName),
    gliderType(gType),
    gliderID(gID),
    date(d),
    tBegin(0),
    tEnd(0),
    distance_tot(0),
    distance_wp(0),
    flightType(0),
    origType(0),
    origDistanceWP(0),
    origDistanceTot(0),
    origPoints(0),
    taskPoints(0),
    sourceFileName(fName),
    v_max(0),
    h_max(0),
    va_min(0),
    va_max(0),
    route(r),
    landTime(route.last()->time),
    startTime(route.at(0)->time)
{
  wpList.setAutoDelete(true);
  origList.setAutoDelete(true);


  // Die Wegpunkte müssen einzeln übergeben werden, da sie gleichzeitig
  // geprüft werden ...
  for(unsigned int loop = 0; loop < wpL.count(); loop++)
      wpList.append(wpL.at(loop));


  __setWaypointType();

  __checkType();
  __checkWaypoints();
  __checkMaxMin();
  __flightState();

int links = 0;
int rechts = 0;
int vermischt = 0;

  for(unsigned int n = 0; n < route.count(); n++)
    {
      if(route.at(n)->f_state == Links) links++;
      else if(route.at(n)->f_state == Rechts) rechts++;
      else if(route.at(n)->f_state == Vermischt) vermischt++;
    }

  warning("Rechts:    %d \nLinks:     %d \nVermischt: %d", rechts, links,vermischt);
  warning("Gesamtanzahl: %d", route.count());

}

Flight::~Flight()
{
  wpList.~QList();
  origList.~QList();
}

bool Flight::__isFAI(double d_wp, double d1, double d2, double d3)
{
  if( ( d_wp < 500.0 ) &&
      ( d1 >= 0.28 * d_wp && d2 >= 0.28 * d_wp && d3 >= 0.28 * d_wp ) )
      // small FAI
      return true;
  else if( ( d1 > 0.25 * d_wp && d2 > 0.25 * d_wp && d3 > 0.25 * d_wp ) &&
           ( d1 <= 0.45 * d_wp && d2 <= 0.45 * d_wp && d3 <= 0.45 * d_wp ) )
      // large FAI
      return true;

  return false;
}

double Flight::__polar(double x, double y)
{
  double angle = 0.0;

  /*
   *          Fallunterscheidung, falls dX = 0
   */
  if(x >= -0.001 && x <= 0.001)
    {
      if(y < 0.0) return ( 1.5 * PI );
      else  return ( 0.5 * PI );
    }

  // Punkt liegt auf der neg. X-Achse
  if(x < 0.0)  angle = atan( y / x ) + PI;
  else  angle = atan( y / x );

  // Neg. value not allowed.
  if(angle < 0.0)  angle = 2 * PI + angle;

  if(angle > (2 * PI))  angle = angle - (2 * PI);

  return angle;
}

void Flight::__moveOptimizePoint(unsigned int idList[], double taskValue[],
    unsigned int id)
{
  if(id > MAX_TASK_ID - 1) return;

  taskValue[id + 1] = taskValue[id];
  idList[3 * id + 3] = idList[3 * id];
  idList[3 * id + 4] = idList[3 * id + 1];
  idList[3 * id + 5] = idList[3 * id + 2];
}


void Flight::__setOptimizeRange(unsigned int start[], unsigned int stop[],
    unsigned int idList[], unsigned int id, unsigned int step)
{
  /*
   * Die benutzten Abschnitte müssen komplett innerhalb des Fluges liegen.
   * Daher werden sie hier ggf. verschoben.
   */
  start[0] = MAX(idList[id], step) - step;
  start[1] = idList[id + 1] - step;
  start[2] = MIN(idList[id + 2] + step, route.count()) - ( 2 * step );

  stop[0] = start[0] + ( 2 * step );
  stop[1] = start[1] + ( 2 * step );
  stop[2] = start[2] + ( 2 * step );
}

double Flight::__calculateOptimizePoints(struct flightPoint* fp1,
    struct flightPoint* fp2, struct flightPoint* fp3)
{
  double dist1 = dist(fp1, fp2);
  double dist2 = dist(fp2, fp3);
  double dist3 = dist(fp1, fp3);
  double tDist = dist1 + dist2 + dist3;

  if(__isFAI(tDist, dist1, dist2, dist3)) return tDist * FAI_POINT;

  return tDist * NORMAL_POINT;
}

void Flight::__flightState()
{
  int s_point = -1;
  int e_point = -1;

  int weiter = 0;

  int dreh = 0;

  for(unsigned int n = 0; n < route.count(); n++)
    {

      // Überprüfen ob man rausgefallen ist !!!

      warning("Bearing %d", route.at(n)->bearing);

      // Bedingungen für Kreisflug müssen noch geprüft werden !!!
      // Bei kleinen Zeitabständen extra Abfrage
      // Noch mehr Zeitabschnitte einfügen ab ca 25° - 35° Kursänderung
      if(( abs(route.at(n)->bearing) > route.at(n)->dT * PI / 20.0))
//         getSpeed(route.at(n)) < 107.0 && route.at(n)->dT > 5 ))
//         ( abs(route.at(n)->bearing) > route.at(n)->dT * PI / 25.0 &&
//         getSpeed(route.at(n)) < 107.0 && route.at(n)->dT <= 5 ))
        {

          weiter = 0;
warning("##################################################################");
          warning("Kreisflug %s",(const char*)printTime(route.at(n)->time));
          // Drehrichtung
          if(route.at(n)->bearing > 0)
            {
              dreh++;
            }
          else
            {
              dreh--;
            }

          // Kreisflug eingeleitet
          if(s_point < 0)
            {
              s_point = n;
            }
        }
      else if(s_point > -1 &&
              route.at(n)->time - route.at(n - weiter)->time  >= 20 &&
              route.at(n - weiter)->time - route.at(s_point)->time > 20)
          // Zeit eines Kreisfluges mindestens 20s
          // Zeit zwischen zwei Kreisflügen höchstens 20s
        {

          warning("Wir fliegen im Kreis");
          // Endpunkt des Kreisfluges

          e_point = n - weiter - 1;

          // Punkte zwischen s_point und e_point setzen
          for(int n = s_point; n <=  e_point; n++)
            {
              if((e_point - s_point) * 0.8 <= dreh)
                {
                  route.at(n)->f_state = Flight::Rechts;
                }
               else if((e_point - s_point) * 0.8 >= - dreh)
                 {
                   route.at(n)->f_state = Flight::Links;
                 }
               else
                 {
                   // vermischt
                   route.at(n)->f_state = Flight::Vermischt;
                 }
            }
          s_point = - 1;
          e_point = - 1;
          dreh = 0;
          weiter = 0;
        }
      else
        {
          if(route.at(n)->time - route.at(n - weiter)->time  >= 20)
            {
              // Kreisflug war unter 20s und wird daher nicht gewertet
              s_point = - 1;
              e_point = - 1;
              dreh = 0;
              weiter = 0;
            }
          // 4 Punkte warten bis wir endgültig rausgehen ;-)
          weiter++;

        }
    }

}

unsigned int Flight::__calculateBestTask(unsigned int start[],
    unsigned int stop[], unsigned int step, unsigned int idList[],
    double taskValue[], bool isTotal)
{
  unsigned int numSteps = 0;
  double temp = 0;
  struct flightPoint *pointA, *pointB, *pointC;

  for(unsigned int loopA = start[0]; loopA < stop[0]; loopA += step)
    {
      pointA = route.at(loopA);

      if(isTotal) start[1] = loopA + step;

      for(unsigned int loopB = start[1]; loopB < stop[1]; loopB += step)
        {
          pointB = route.at(loopB);

          if(isTotal) start[2] = loopB + step;

          for(unsigned int loopC = start[2]; loopC < stop[2]; loopC += step)
            {
              pointC = route.at(loopC);
              temp = __calculateOptimizePoints(pointA, pointB, pointC);

              /* wir behalten die besten Dreiecke ( taskValue[0] := bester ) */
              if(temp > taskValue[MAX_TASK_ID])
                {
                  for(unsigned int loop = MAX_TASK_ID; loop > 0; loop--)
                    {
                      if(temp > taskValue[loop - 1])
                        {
                          __moveOptimizePoint(idList, taskValue, loop - 1);
                          if(loop == 1)
                            {
                              /* temp ist der weiteste Flug */
                              taskValue[0] = temp;
                              idList[0] = loopA;
                              idList[1] = loopB;
                              idList[2] = loopC;
                            }
                        }
                      else
                        {
                          taskValue[loop] = temp;
                          idList[ 3 * loop ] = loopA;
                          idList[( 3 * loop ) + 1] = loopB;
                          idList[( 3 * loop ) + 2] = loopC;
                          break;
                        }
                    }
                }
              numSteps++;
            }
        }
    }
  return numSteps;
}

double Flight::__sectorangle(int loop, bool isDraw)
{
  /*
   * Berechnet die Winkelhalbierende des Sektors
   */
  double nextAngle = 0.0, preAngle = 0.0, sectorAngle = 0.0;
  // Art des Wendepunktes
  switch(wpList.at(loop)->type)
    {
      case Begin:
      case (TakeOff | Begin):
        // directions to the next point
        sectorAngle = __polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(NEXT_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(NEXT_ID)->projP.y() ) );
        break;
      case RouteP:
        // directions to the previous point
        preAngle = __polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(PRE_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(PRE_ID)->projP.y() ) );
        // direction to the following point:
        nextAngle = __polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(NEXT_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(NEXT_ID)->projP.y() ) );

        sectorAngle = (preAngle + nextAngle) / 2.0;
        if( ( preAngle < PI / 2.0 && nextAngle > 1.5 * PI ) ||
            ( nextAngle < PI / 2.0 && preAngle > 1.5 * PI ) )
            sectorAngle = sectorAngle - PI;
        break;
      case End:
      case (End | Landing):
        // direction to the previous point:
        sectorAngle = __polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(PRE_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(PRE_ID)->projP.y() ) );
        break;
    }

  // Nur nötig bei der Überprüfung der Wegpunkte,
  // würde beim Zeichnen zu Fehlern führen
  if(!isDraw) sectorAngle += PI;

  if(sectorAngle > (2 * PI)) sectorAngle = sectorAngle - (2 * PI);

  wpList.at(CUR_ID)->angle = sectorAngle;

  return sectorAngle;
}

bool Flight::__isVisible() const
{
  return true;
//  extern const MapMatrix _globalMapMatrix;
//  return _globalMapMatrix.isVisible(bBox);
}

void Flight::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  if(!__isVisible()) return;

  extern const MapMatrix _globalMapMatrix;

  extern double _scale[], _currentScale;
  struct flightPoint* pointA;
  struct flightPoint* pointB;
  QPoint curPointA, curPointB;
  double w1;

  if(wpList.count() > 1)
    {
      extern const MapMatrix _globalMapMatrix;
      QPoint tempP;

      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          /*
           * w1 ist die Winkelhalbierende des Sektors!!!
           *      (Angaben in 1/16 Grad)
           *
           *
           *    schein noch nicht immer zu stimmen!!!
           */
          w1 = ( ( _globalMapMatrix.map(wpList.at(loop)->angle) + PI ) / PI )
                  * 180.0 * 16.0 * -1.0;

          tempP = _globalMapMatrix.map(wpList.at(loop)->projP);
          double qx = -R1 + tempP.x();
          double qy = -R1 + tempP.y();
          double gx = -R2 + tempP.x();
          double gy = -R2 + tempP.y();

          if(loop)
            {
              bBoxTask.setLeft(MIN(tempP.x(), bBoxTask.left()));
              bBoxTask.setTop(MAX(tempP.y(), bBoxTask.top()));
              bBoxTask.setRight(MAX(tempP.x(), bBoxTask.right()));
              bBoxTask.setBottom(MIN(tempP.y(), bBoxTask.bottom()));
            }
          else
            {
              bBoxTask.setLeft(tempP.x());
              bBoxTask.setTop(tempP.y());
              bBoxTask.setRight(tempP.x());
              bBoxTask.setBottom(tempP.y());
            }

          switch(wpList.at(loop)->type)
            {
              case Flight::RouteP:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
                targetPainter->setBrush(QColor(255, 110, 110));
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 1440, 2880);
                targetPainter->setBrush(QColor(110, 255, 110));
                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Inneren Sektor erneut zeichnen, damit Trennlinien
                // zwischen Sekt. 1 und Zylinder verschwinden
//                targetPainter->setPen(QPen::NoPen);
//                targetPainter->drawEllipse(gx + 2, gy + 2,
//                    (2 * R2) - 5, (2 * R2) - 5);

                maskPainter->setPen(QPen(Qt::color1, 2));
                maskPainter->setBrush(QBrush(Qt::color1));
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 1440, 2880);
                maskPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);
                if(loop)
                  {
                    targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
                    maskPainter->setPen(QPen(Qt::color1, 3));
                    targetPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));
                    maskPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));
                  }
                break;
              case Flight::Begin:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
                targetPainter->setBrush(QBrush(QColor(255, 0, 0),
                    QBrush::BDiagPattern));
                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                maskPainter->setPen(QPen(Qt::color1, 2));
                maskPainter->setBrush(QBrush(Qt::color1, QBrush::BDiagPattern));
                maskPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                if(loop)
                  {
                    targetPainter->setPen(QPen(QColor(50, 50, 50), 3,
                        QPen::DashLine));
                    maskPainter->setPen(QPen(Qt::color1, 3,
                        QPen::DashLine));
                    targetPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));
                    maskPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));
                  }
                break;
              case Flight::End:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
                targetPainter->setBrush(QBrush(QColor(0, 0, 255),
                    QBrush::FDiagPattern));
                maskPainter->setPen(QPen(Qt::color1, 2));
                maskPainter->setBrush(QBrush(Qt::color1,
                    QBrush::FDiagPattern));

                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);
                targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
                targetPainter->drawLine(
                    _globalMapMatrix.map(wpList.at(loop-1)->projP),
                    _globalMapMatrix.map(wpList.at(loop)->projP));

                maskPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);
                maskPainter->setPen(QPen(QColor(50, 50, 50), 3));
                maskPainter->drawLine(
                    _globalMapMatrix.map(wpList.at(loop-1)->projP),
                    _globalMapMatrix.map(wpList.at(loop)->projP));
                break;
              case ( Flight::End | Flight::Landing ):
                if(loop)
                  {
                    targetPainter->setPen(QPen(QColor(50,50,50), 3));
                    maskPainter->setPen(QPen(Qt::color1, 3));

                    targetPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));
                    maskPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));
                  }
                break;
              default:
                if(loop)
                  {
                    targetPainter->setPen(QPen(QColor(50, 50, 50), 3,
                        QPen::DashLine));
                    maskPainter->setPen(QPen(Qt::color1, 3,
                        QPen::DashLine));

                    targetPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));

                    maskPainter->drawLine(
                        _globalMapMatrix.map(wpList.at(loop - 1)->projP),
                        _globalMapMatrix.map(wpList.at(loop)->projP));

                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
                    targetPainter->setBrush(QBrush::NoBrush);
                    maskPainter->setPen(QPen(Qt::color1, 2));
                    maskPainter->setBrush(QBrush::NoBrush);

                    targetPainter->drawEllipse(
                        _globalMapMatrix.map(wpList.at(loop)->projP).x() - 8,
                        _globalMapMatrix.map(wpList.at(loop)->projP).y() - 8,
                        16, 16);
                    maskPainter->drawEllipse(
                        _globalMapMatrix.map(wpList.at(loop)->projP).x() - 8,
                        _globalMapMatrix.map(wpList.at(loop)->projP).y() - 8,
                        16, 16);
                  }
                break;
            }
        }
    }

  unsigned int delta = 1;
  if(_currentScale > _scale[ID_BORDER_SMALL])          delta = 8;
  else if(_currentScale > _scale[ID_BORDER_SMALL - 1]) delta = 4;

  curPointA = _globalMapMatrix.map(route.at(0)->projP);
  bBoxFlight.setLeft(curPointA.x());
  bBoxFlight.setTop(curPointA.y());
  bBoxFlight.setRight(curPointA.x());
  bBoxFlight.setBottom(curPointA.y());

  for(unsigned int n = delta; n < route.count(); n = n + delta)
    {
      pointA = route.at(n - delta);
      pointB = route.at(n);

      curPointB = _globalMapMatrix.map(pointB->projP);

      bBoxFlight.setLeft(MIN(curPointB.x(), bBoxFlight.left()));
      bBoxFlight.setTop(MAX(curPointB.y(), bBoxFlight.top()));
      bBoxFlight.setRight(MAX(curPointB.x(), bBoxFlight.right()));
      bBoxFlight.setBottom(MIN(curPointB.y(), bBoxFlight.bottom()));

      // Strecke einzeichnen
      maskPainter->setPen(QPen(Qt::color1, 4));
      if(_currentScale < _scale[ID_BORDER_SMALL - 1])
        {
          if(pointA->dH < 0)
              targetPainter->setPen(QPen(QColor(255,0,0), 4));
          else
              targetPainter->setPen(QPen(QColor(0,255,0), 4));
        }
      else if(_currentScale < _scale[5])
          targetPainter->setPen(QPen(QColor(0,0,200), 4));
      else
        {
          maskPainter->setPen(QPen(Qt::color1, 3));
          targetPainter->setPen(QPen(QColor(0,0,200), 3));
        }

      /*******************************************************************
       **
       ** Dynamische Farben im Flug:
       **
       **   Zur Zeit nur für Vario. Sollte für alle wichtigen Werte
       **   möglich sein. Dazu muss der Wert wählbar sein. Farben
       **   als Legende ausgeben ???
       **
       ******************************************************************/
      /*
      if(pointA->dH < 0)
        {
          targetPainter->setPen(QPen(QColor(255,
              MIN((int)(10.0 * -pointA->dH), 255),
              MIN((int)(5.0 * -pointA->dH), 255)), 4));
        }
      else
        {
          targetPainter->setPen(QPen(QColor(MIN((int)(5.0 * pointA->dH), 255),
              MIN((int)(10.0 * pointA->dH), 255), 255), 4));
        }
      */

      targetPainter->drawLine(curPointA, curPointB);
      maskPainter->drawLine(curPointA, curPointB);

      // Wenn die Kringel beibehalten werden, müsste eine weitere
      // Grenze eingeführt werden ...
//      if(_currentScale < _scale[1])
//        {
        if(route.at(n)->f_state == Rechts ||
           route.at(n)->f_state == Links ||
           route.at(n)->f_state == Vermischt)
           {
              targetPainter->setBrush(QBrush::NoBrush);
              targetPainter->setPen(QPen(QColor(255,255,0), 2));
              targetPainter->drawEllipse(curPointB.x() - 10,
                  curPointB.y() - 10,20,20);
              maskPainter->drawEllipse(curPointB.x() - 10,
                  curPointB.y() - 10,20,20);
            }
//        }

      curPointA = curPointB;
    }
}

QString Flight::getID() const { return gliderID; }

QString Flight::getRouteType(bool isOrig) const
{
  /*
   * Die Ausgabetexte müssen noch ins Englische übersetzt werden ...
   */
  unsigned int type = flightType;
  if(isOrig && isOptimized())  type = origType;

  switch(type)
    {
      case NotSet: return "nicht gesetzt";
      case ZielS: return "Zielstrecke";
      case ZielR: return "Zielrückkehr";
      case FAI: return "FAI-Dreieck";
      case Dreieck: return "allg. Dreieck";
      case FAI_S: return "FAI-Dreieck (S)";
      case Dreieck_S: return "allg. Dreieck (S)";
      case Abgebrochen: return "abgebrochen";
      default: return "unbekannt";
    }
}

struct flightPoint Flight::getPointByTime(int time)
{
  int diff, n, sp, ep;

  if(getLandTime() - time < time - getStartTime())
    {
      n = -1;
      sp = route.count() - 1;
      ep = 0;
    }
  else
    {
      n = 1;
      sp = 0;
      ep = route.count() - 1;
    }

  diff = getPoint(sp).time - time;
  diff = ABS(diff);

  for(int l = sp + n; l < (int)route.count() && l >= 0; l += n)
    {
      int a = getPoint(l).time - time;
      if(ABS(a) > diff)
          return getPoint(l - n);

      diff = getPoint(l).time - time;
      diff = ABS(diff);
    }

  return getPoint(ep);
}

int Flight::getPointByTime_i(int time)
{
  // Muss noch vereinfacht werden !!
  // als Ersatz für getPointByTime
  int diff, n, sp, ep;

  if(getLandTime() - time < time - getStartTime())
    {
      n = -1;
      sp = route.count() - 1;
      ep = 0;
    }
  else
    {
      n = 1;
      sp = 0;
      ep = route.count() - 1;
    }

  diff = getPoint(sp).time - time;
  diff = ABS(diff);

  for(int l = sp + n; l < (int)route.count() && l >= 0; l += n)
    {
      int a = getPoint(l).time - time;
      if(ABS(a) > diff)
          return l - n;

      diff = getPoint(l).time - time;
      diff = ABS(diff);
    }

  return ep;
}

struct flightPoint Flight::getPoint(int n)
{
  if(n >= 0 && n < (int)route.count())  return *route.at(n);

  switch(n)
    {
      case V_MAX: return *route.at(v_max);
      case H_MAX: return *route.at(h_max);
      case VA_MAX: return *route.at(va_max);
      case VA_MIN: return *route.at(va_min);
      default:
        flightPoint ret;
        ret.gpsHeight = 0;
        ret.height = 0;
        return ret;
    }
}

QStrList Flight::getFlightValues(unsigned int start, unsigned int end)
{
  float k_height_pos_r = 0;
  float k_height_neg_r = 0;
  float k_height_pos_l = 0;
  float k_height_neg_l = 0;
  float k_height_pos_v = 0;
  float k_height_neg_v = 0;
  int kurbel_r = 0;
  int kurbel_l = 0;
  int kurbel_v = 0;
  float distance = 0;
  float s_height_pos = 0;
  float s_height_neg = 0;

//  noch abchecken, dass start => 0 und end <= fluglänge
  if(end > route.count() - 1) end = route.count() - 1;
  if(start < 0) start = 0;

  for(unsigned int n = start; n < end; n++)
    {
      switch(route.at(n)->f_state)
        {
          case Flight::Rechts:
//            warning("Rechts");
            if(route.at(n)->dH > 0)
              {
                k_height_pos_r += (float)route.at(n)->dH;
              }
            else
              {
                k_height_neg_r += (float)route.at(n)->dH;
              }

            kurbel_r += route.at(n)->dT;
            break;
          case Flight::Links:
            if(route.at(n)->dH > 0)
              {
                k_height_pos_l += (float)route.at(n)->dH;
              }
            else
              {
                k_height_neg_l += (float)route.at(n)->dH;
              }

            kurbel_l += route.at(n)->dT;
            break;
          case Flight::Vermischt:
//          warning("vermischt:");
            if(route.at(n)->dH > 0)
              {
                k_height_pos_v += (float)route.at(n)->dH;
              }
            else
              {
                k_height_neg_v += (float)route.at(n)->dH;
              }

            kurbel_v += route.at(n)->dT;
            break;
          default:
           // immer oder bloß auf Strecke ??
           distance += (float)route.at(n)->dS;

           if(route.at(n)->dH > 0)
              {
                s_height_pos += (float)route.at(n)->dH;
              }
            else
              {
                s_height_neg += (float)route.at(n)->dH;
              }
         }
    }


    QStrList ergebnis;
    QString text;

  warning("Rechts: %d", kurbel_r);
  warning("Links: %d", kurbel_l);
  warning("Vermischt: %d", kurbel_v);

    // Kreisflug
    text.sprintf("%s (%.1f %%)", (const char*)printTime(kurbel_r),
        (float)kurbel_r /
            (float)( route.at(end)->time - route.at(start)->time ) * 100.0);
    ergebnis.append(text);
    text.sprintf("%s (%.1f %%)", (const char*)printTime(kurbel_l),
        (float)kurbel_l /
            (float)( route.at(end)->time - route.at(start)->time) * 100.0);
    ergebnis.append(text);
    text.sprintf("%s (%.1f %%)", (const char*)printTime(kurbel_v),
        (float)kurbel_v /
            (float)(route.at(end)->time - route.at(start)->time ) * 100.0);
    ergebnis.append(text);
    text.sprintf("%s (%.1f %%)",
        (const char*)printTime((kurbel_r + kurbel_l + kurbel_v)),
        (float)(kurbel_r + kurbel_l + kurbel_v) /
            (float)( route.at(end)->time - route.at(start)->time ) * 100.0);
    ergebnis.append(text);


    text.sprintf("%.2f m/s",(k_height_pos_r + k_height_neg_r) /
             (route.at(end)->time - route.at(start)->time));
    ergebnis.append(text);
    text.sprintf("%.2f m/s",(k_height_pos_l + k_height_neg_l) /
             (route.at(end)->time - route.at(start)->time));
    ergebnis.append(text);
    text.sprintf("%.2f m/s",(k_height_pos_v + k_height_neg_v) /
             (route.at(end)->time - route.at(start)->time));
    ergebnis.append(text);
    text.sprintf("%.2f m/s",
             (k_height_pos_r + k_height_pos_l + k_height_pos_v +
              k_height_neg_r + k_height_neg_l + k_height_neg_v) /
             (route.at(end)->time - route.at(start)->time));
    ergebnis.append(text);

    text.sprintf("%.0f m",k_height_pos_r);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_pos_l);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_pos_v);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_pos_r + k_height_pos_l + k_height_pos_v);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_neg_r);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_neg_l);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_neg_v);
    ergebnis.append(text);
    text.sprintf("%.0f m",k_height_neg_r + k_height_neg_l + k_height_neg_v);
    ergebnis.append(text);


    // Strecke
    text.sprintf("%.0f",distance / (s_height_pos + s_height_pos));
    ergebnis.append(text);
    text.sprintf("%.1f km/h",distance /
          ((float)(route.at(end)->time - route.at(start)->time -
          (kurbel_r + kurbel_l + kurbel_v))) * 3.6);
    ergebnis.append(text);
    text.sprintf("%.0f m",s_height_pos);
    ergebnis.append(text);
    text.sprintf("%.0f m",s_height_neg);
    ergebnis.append(text);
    text.sprintf("%.0f km",distance / 1000);
    ergebnis.append(text);
    text.sprintf("%s (%.1f %%)",
        (const char*)printTime( ( route.at(end)->time - route.at(start)->time -
            ( kurbel_r + kurbel_l + kurbel_v ) ) ),
        (float)( route.at(end)->time - route.at(start)->time -
                ( kurbel_r + kurbel_l + kurbel_v ) ) /
            (float)( route.at(end)->time - route.at(start)->time ) * 100.0 );
    ergebnis.append(text);
    text.sprintf("%s",
        (const char*)printTime(route.at(end)->time - route.at(start)->time));
    ergebnis.append(text);
    text.sprintf("%.0f m",s_height_pos   + k_height_pos_r
                        + k_height_pos_l + k_height_pos_v);
    ergebnis.append(text);
    text.sprintf("%.0f m",s_height_neg   + k_height_neg_r
                        + k_height_neg_l + k_height_neg_v);
    ergebnis.append(text);


    // Rückgabe:
    // kurbelanteil r - l - v - g
    // mittleres Steigen r - l - v - g
    // Höhengewinn r - l - v - g
    // Höhenverlust r - l - v - g
    // Gleitzahl - miitlere Geschw. - Höhengewinn - höhenverlust
    // Distanz - StreckenZeit - Gesamtzeit - Ges. Höhengewinn - Ges Höhenverlust



    return ergebnis;
}

QString Flight::getDistance(bool isOrig) const
{
  unsigned int type = flightType;
  double dist = distance_tot;
  if(isOrig && isOptimized())
    {
      type = origType;
      dist = origDistanceTot;
    }

  if(type == NotSet)  return "--";

  QString distString;
  distString.sprintf("%.2f km ", dist);

  return distString;
}

QString Flight::getTaskDistance(bool isOrig) const
{
  unsigned int type = flightType;
  double dist = distance_wp;
  if(isOrig && isOptimized())
    {
      type = origType;
      dist = origDistanceWP;
    }

  if(type == NotSet)  return "--";

  QString distString;
  distString.sprintf("%.2f km ", dist);

  return distString;
}

QString Flight::getPoints(bool isOrig) const
{
  unsigned int type = flightType;
  double points = taskPoints;

  if(isOrig && isOptimized())
    {
      type = origType;
      points = origPoints;
    }

  if(type == NotSet)  return "--";

  int points1 = (int) points;
  if((int) ( (points - points1) * 10 ) > 5) points1++;

  QString pointString;
  pointString.sprintf("%d", points1);

  return pointString;
}

QString Flight::getLandSite() const { return landSite; }

int Flight::getLandTime() const { return landTime; }

QString Flight::getPilot() const { return pilotName; }

QString Flight::getStartSite() const { return startSite; }

int Flight::getStartTime() const { return startTime; }

QString Flight::getType() const { return gliderType; }

QString Flight::getDate() const { return date; }

unsigned int Flight::getTaskType() const { return flightType; }

unsigned int Flight::getRouteLength() const { return route.count(); }

const char* Flight::getFileName() const { return sourceFileName; }

bool Flight::isOptimized() const { return (origList.count() != 0); }

int Flight::searchPoint(QPoint cPoint, struct flightPoint* searchPoint)
{
  extern const double _currentScale, _scale[];
  extern const MapMatrix _globalMapMatrix;

  unsigned int delta = 1;
  int index = -1;

  double minDist = 1000.0, distance = 0.0;

  if(_currentScale > _scale[ID_BORDER_SMALL])          delta = 8;
  else if(_currentScale > _scale[ID_BORDER_SMALL - 1]) delta = 4;

  QPoint fPoint;

  for(unsigned int loop = 0; loop < route.count(); loop = loop + delta)
    {
      fPoint = _globalMapMatrix.map(route.at(loop)->projP);
      int dX = cPoint.x() - fPoint.x();
      int dY = cPoint.y() - fPoint.y();
      distance = sqrt( (dX * dX) + (dY * dY) );

      /* Maximaler Abstand: 30 Punkte */
      if(distance < 30.0)
        {
          if(distance < minDist)
            {
              minDist = distance;
              index = loop;
              searchPoint[0] = *route.at(index);
            }
        }
    }
  return index;
}

QRect Flight::getFlightRect() const { return bBoxFlight; }

QRect Flight::getTaskRect() const { return bBoxTask; }

QStrList Flight::getHeader()
{
  QStrList header;
  // Die Liste könnte doch eigentlich permanent gespeichert werden ...
  header.append(pilotName);
  header.append(gliderID);
  header.append(gliderType);
  header.append(date);
  header.append(printTime(route.last()->time - route.at(0)->time));
  header.append(getRouteType());
  header.append(getDistance());
  header.append(getPoints());

  return header;
}

void Flight::__setWaypointType()
{
  /*
   * Setzt den Status der Wendepunkte
   *
   */
  wpList.at(0)->type = Flight::TakeOff;
  wpList.at(1)->type = Flight::Begin;
  wpList.at(wpList.count() - 2)->type = Flight::End;
  wpList.at(wpList.count() - 1)->type = Flight::Landing;

  for(unsigned int n = 2; n < wpList.count() - 2; n++)
  {
    wpList.at(n)->type = Flight::RouteP;
  }

}



void Flight::__appendWaypoint(struct wayPoint* newPoint)
{
warning("Flight::__appendWaypoint");
  if(wpList.count() && dist(wpList.last(), newPoint) <= 0.1) return;
warning("------------------------> 1");
  wpList.append(newPoint);

  if(tEnd == 0 && wpList.count() > 2)
    {
warning("------------------------> 2");
      int loop = 0;

      for(int n = wpList.count() - 3; n >= 0; n--)
        {
warning("------------------------> 3 (%d)", loop);
          loop++;
          if(newPoint->origP == wpList.at(n)->origP)
            {
              bool noTask = false;

              tEnd = wpList.count() - 1;
              tBegin = n;
warning("Begin: %d / Ende: %d", tBegin, tEnd);
              wpList.at(tEnd)->type = Flight::End;
              wpList.at(tBegin)->type = Flight::Begin;


              // Wenn wpList.count() < 4 -> keine Aufgabe deklariert
              ///// Müll rausfiltern
              if(tEnd - tBegin == 2)
                {
                  if(wpList.at(tBegin)->origP == wpList.at(tBegin + 1)->origP)
                      noTask = true;
                }
              else
                {
                  for(int loop = tBegin + 2; loop < tEnd; loop++)
                    {
                      if(wpList.at(loop - 1)->origP == wpList.at(loop)->origP)
                          noTask = true;
                    }
                }

              if(noTask)
                {
                  for(int m = 0; m < tEnd; m++)
                      wpList.at(m)->type = Flight::FreeP;

                  flightType = Flight::Abgebrochen;
                  KMessageBox::error(0, i18n("The task is not valid.<BR>"
                           "All waypoints are the same point."));
                }
              else
                {
                  for(int m = 0; m < tBegin; m++)
                      wpList.at(m)->type = Flight::FreeP;

                  for(int m = tBegin + 1; m < tEnd; m++)
                      wpList.at(m)->type = Flight::RouteP;
                }
              break;
            }
        }
    }
  else
      newPoint->type = Flight::FreeP;
}

void Flight::__checkType()
{
warning("Flight::__checkType()");
  distance_tot = 0;
warning("tBegin: %d / tEnd: %d", tBegin, tEnd);
  for(int loop = tBegin + 1; loop <= tEnd; loop++)
    {
warning("Durchlauf: %d", loop);
      wpList.at(loop)->distance = dist(wpList.at(loop - 1), wpList.at(loop));
      distance_tot = distance_tot + wpList.at(loop)->distance;
    }
warning("hallo");
  switch(tEnd - tBegin)
    {
      case 0:
        break;
      case 1:           // Zielstrecke
        flightType = ZielS;
        break;
      case 2:           // Zielrückkehr
        flightType = ZielR;
        break;
      case 3:
        if(__isFAI(distance_tot,wpList.at(tBegin + 1)->distance,
              wpList.at(tBegin + 2)->distance,
              wpList.at(tBegin + 3)->distance))
           flightType = FAI;
        else
            flightType = Dreieck;
        break;
      case 4:           // Start auf Schenkel oder Vieleck
        distance_tot = distance_tot - wpList.at(tBegin + 1)->distance
               - wpList.at(tBegin + 4)->distance
               + dist(wpList.at(tBegin + 1), wpList.at(tBegin + 3));

        if(__isFAI(distance_tot, dist(wpList.at(tBegin + 1),
                wpList.at(tBegin + 3)), wpList.at(tBegin + 2)->distance,
              wpList.at(tBegin + 3)->distance))
            flightType = FAI_S;
        else
            flightType = Dreieck_S;
        break;
      default:
        flightType = Unknown;
    }
}

void Flight::__checkMaxMin()
{
  v_max = 0;
  h_max = 0;
  va_max = 0;
  va_min = 0;

  for(unsigned int loop = 0; loop < route.count(); loop++)
    {
      if(loop)
        {
          // Maximal Werte finden
          if(GET_SPEED(route.at(loop)) > GET_SPEED(route.at(v_max)))
              v_max = loop;

          if(route.at(loop)->height > route.at(h_max)->height)
              h_max = loop;

          if(GET_VARIO(route.at(loop)) > GET_VARIO(route.at(va_max)))
              va_max = loop;

          if(GET_VARIO(route.at(loop)) < GET_VARIO(route.at(va_min)))
              va_min = loop;
        }
    }
}

void Flight::__checkWaypoints()
{
  if(flightType == NotSet) return;
warning("Flight::__checkWaypoints()");

  int gliderIndex = 100, preTime = 0;
  KConfig* config = kapp->config();
  config->setGroup("FlightPoints");

  double pointFAI = config->readDoubleNumEntry("FAIPoint", 2.0);
  double pointNormal = config->readDoubleNumEntry("NormalPoint", 1.75);
  double pointCancel = config->readDoubleNumEntry("CancelPoint", 1.0);
  double pointZielS = config->readDoubleNumEntry("ZielSPoint", 1.5);
  double malusValue = config->readDoubleNumEntry("MalusValue", 15.0);
  double sectorMalus = config->readDoubleNumEntry("SectorMalus", -0.1);

  if(gliderType != 0)
    {
      config->setGroup("GliderTypes");
      gliderIndex = config->readNumEntry(gliderType, 100);
      config->setGroup(0);
    }

  for(unsigned int loop = 0; loop < route.count(); loop++)
    {
      if(loop)
        {
          if(route.at(loop)->time - preTime > 7)
            {
              /*
               *           Zeitabstand zwischen Loggerpunkten ist zu gross!
               *                      (vgl. Code Sportif 3, Ziffer 1.9.2.1)
               */
              KMessageBox::error(0,
                  i18n("The time intervall between two points<BR>"
                       "of the flight is more than 70 sec.!<BR>"
                       "Due to Code Sportif 3, Nr. 1.9.2.1,<BR>"
                       "the flight can not be valued!"));
              return;
              ////////////////////////////////////////////////////////////////
              // sonstige Reaktion ????
              ////////////////////////////////////////////////////////////////
            }
        }
      preTime = route.at(loop)->time;
    }

  // Überprüfen von Start und Landung Abstand noch überdenken!!!!!
  if(dist(wpList.at(0), route.at(1)) < 1.0)
    {
      wpList.at(0)->sector1 = route.at(0)->time;
      if(wpList.at(0)->type == FreeP)  wpList.at(0)->type = ( TakeOff );
      else  wpList.at(0)->type = ( wpList.at(0)->type | TakeOff );
    }
  else
    {
      // Was passiert, wenn der Start woanders erfolgte ???
    }

  if(dist(wpList.last(), route.last()) < 1.0)
    {
      wpList.last()->sector1 = route.last()->time;
      if(wpList.last()->type == FreeP)  wpList.last()->type = ( Landing );
      else  wpList.last()->type = ( wpList.last()->type | Landing );
    }
  else
    {
      // Was passiert, wenn die Landung woanders erfolgte ???
    }

  unsigned int startIndex = 0, dummy = 0;

  for(unsigned int loop = 0; loop < wpList.count(); loop++)
    {
      double deltaAngle = 0.0, pointAngle = 0.0, sectorAngle = 0.0;
      dummy = 0;

      sectorAngle = __sectorangle(loop, false);

      /*
       * Prüfung, ob Flugpunkte in den Sektoren liegen.
       *
       *      Ein Index 0 bei den Sektoren zeigt an, dass der Sektor
       *      _nicht_ erreicht wurde. Dies führt an Mitternacht zu
       *      einem möglichen Fehler ...
       */
      for(unsigned int pLoop = startIndex + 1; pLoop < route.count(); pLoop++)
        {
          if( wpList.at(loop)->projP == route.at(pLoop)->projP )
            {
              // Wir sind in allen Sektoren ...
              if(!wpList.at(loop)->sector1)
                  wpList.at(loop)->sector1 = route.at(pLoop)->time;

              if(!wpList.at(loop)->sector2)
                  wpList.at(loop)->sector2 = route.at(pLoop)->time;

              if(!wpList.at(loop)->sectorFAI)
                  wpList.at(loop)->sectorFAI = route.at(pLoop)->time;

              // ... daher ist ein Abbruch möglich!
              startIndex = pLoop;
              break;
            }
          else
            {
              if(dist(wpList.at(loop), route.at(pLoop)) <= 0.5)
                {
                  // Wir sind im kleinen Zylinder ...
                  if(!wpList.at(loop)->sector1)
                      wpList.at(loop)->sector1 = route.at(pLoop)->time;

                  if(!wpList.at(loop)->sector2)
                      wpList.at(loop)->sector2 = route.at(pLoop)->time;

                  if(!dummy)
                    {
                      startIndex = pLoop;
                      dummy = pLoop;
                    }
                }

              pointAngle = __polar(
                  ( wpList.at(loop)->projP.x() - route.at(pLoop)->projP.x() ),
                  ( wpList.at(loop)->projP.y() - route.at(pLoop)->projP.y() ) );

              deltaAngle = sqrt( ( pointAngle - wpList.at(loop)->angle ) *
                                 ( pointAngle - wpList.at(loop)->angle ) );

              if(deltaAngle <= (0.25 * PI))
                {
                  // Wir sind im FAI-Sektor ...
                  if(!wpList.at(loop)->sectorFAI)
                      wpList.at(loop)->sectorFAI = route.at(pLoop)->time;

                  if(dist(wpList.at(loop), route.at(pLoop)) <= 3.0)
                    {
                      // ... und in Sektor 1 ...
                      if(!wpList.at(loop)->sector1)
                        {
                          wpList.at(loop)->sector1 = route.at(pLoop)->time;
                          // ... daher ist ein Abbruch möglich!
                          startIndex = pLoop;
                          break;
                        }

                    }
                }
              else if(deltaAngle <= (0.5 * PI))
                {
                  // "nur" in Sektor 2
                  if(!wpList.at(loop)->sector2)
                      wpList.at(loop)->sector2 = route.at(pLoop)->time;

                  if(!dummy)
                    {
                      startIndex = pLoop;
                      dummy = pLoop;
                    }
                }
            }
        }
    }

  /*
   * Überprüfen der Aufgabe
   */
  int faiCount = 0, dmstCount = 0;
  double dmstMalus = 1.0, aussenlande = 0.0;
  bool home, stop = false;

  if(wpList.at(tBegin)->sector1 == 0)
    {
      KMessageBox::information(0,
          i18n("You have not reached the first point of your task!"));
      return;
    }

  for(int loop = tBegin + 1; loop < tEnd; loop++)
    {
      if(!stop)
        {
          if(wpList.at(loop)->sector1 != 0)
            {
              dmstCount++;
            }
          else if(wpList.at(loop)->sector2 != 0)
            {
              dmstMalus += sectorMalus;
              dmstCount++;
            }
          else
            {
              // Wendepunkt nicht erreicht!!
              if(loop == tBegin + 1)
                {
                  KMessageBox::information(0,
                      i18n("You have not reached the first waypoint of your task."));
                  return;
                }
              /*
               * Wertung mit 1,5 Punkten, weil Punkt nicht erreicht
               */
              stop = true;
            }
        }

      if(wpList.at(loop)->sectorFAI != 0) faiCount++;
    }

  if(wpList.at(tEnd)->sector1 == 0)
    {
      home = false;
      KMessageBox::error(0,
          i18n("You have not reached the last point of your task."));

      if(dist(wpList.at(tBegin + dmstCount), route.last()) < 1.0)
        {
          // Landung auf letztem Wegpunkt
        }
      else
          // Außenlandung -- Wertung: + 1Punkt bis zur Außenlandung
          aussenlande = dist(wpList.at(tBegin + dmstCount), route.last());
    }
  else
    {
      dmstCount++;
      faiCount++;
      home = true;
    }

  // FAI erreicht?
  if(faiCount == tEnd - tBegin)
    {
      cerr << "Nach FAI Regeln erfüllt!\n";
    }

  double wertDist = 0, F;

  if(dmstCount != tEnd - tBegin)
    {
      if(home)
        {
          wertDist = dist(wpList.at(tEnd),
              wpList.at(tBegin + dmstCount - 1));
          for(int loop = tBegin; loop < tBegin + dmstCount; loop++)
              wertDist = wertDist + wpList.at(loop)->distance;

        }
      else
        {
          for(int loop = tBegin; loop <= tBegin + dmstCount; loop++)
              wertDist = wertDist + wpList.at(loop)->distance;

        }
    }
  else
    {
      /*
       * Aufgabe vollständig erfüllt
       *        F: Punkte/km
       *        I: Index des Flugzeuges
       *        f & I noch abfragen !!!!
       */
      switch(flightType)
        {
          case Flight::ZielS:
            F = pointZielS;
            break;
          case Flight::ZielR:
          case Flight::Dreieck:
          case Flight::Dreieck_S:
            F = pointNormal;
            break;
          case Flight::FAI:
          case Flight::FAI_S:
            F = pointFAI;
            break;
          default:
            F = 0.0;
        }
      wertDist = distance_tot;
    }

  taskPoints = (wertDist * F * 100) / gliderIndex * dmstMalus +
                      (aussenlande * pointCancel * 100) / gliderIndex;
  if(origList.count())
    {
      /*
       * Optimierter Flug: 15% abziehen
       */
      taskPoints -= ( taskPoints * (malusValue / 100.0) );
      cerr << "reduziere Punkte um 15%\n";
    }

  cerr << "dmstCount " << dmstCount << endl;
  cerr << "Punkte/km " << F << endl;
  cerr << "malus: " << dmstMalus << endl;
  cerr << "Index: " << gliderIndex << endl;
  cerr << "Entfernung: " << wertDist << endl;
  cerr << taskPoints << endl;
}

QList<struct wayPoint>* Flight::getWPList(bool isOrig)
{
  if(isOrig && isOptimized()) return &origList;

  return &wpList;
}

bool Flight::optimizeTask()
{
  unsigned int curNumSteps = 0, temp, step = 0, minNumSteps = 400000000;
  unsigned int numStepsA = 0, numStepsB = 0, minA = 0, minB = 0;

  double length = 0;
  for(unsigned int curStep = 1; curStep < 100; curStep++)
    {
      /*
       * ( route.count / curStep ) gibt die Anzahl der Punkte an, die
       * zur Optimierung im ersten Anlauf verwendet werden. Wenn dieser
       * Wert unter 3 sinkt, kann kein Dreieck mehr bestimmt werden ...
       */
      if((route.count() / curStep) < 3) break;

      curNumSteps = 1;
      numStepsA = 1;
      numStepsB = 1;
      temp = 1;

      /*
       * Berechnung der Rechenschritte für den ersten Durchlauf
       */
      for(unsigned int loop = 3; loop < ( route.count() / curStep ); loop++)
        {
          curNumSteps += temp + loop;
          temp += loop;
        }

      numStepsA = curNumSteps;
      temp = 1;

      /*
       * Berechnung der Rechenschritte für den zweiten Durchlauf
       */
      for(unsigned int loop = 3; loop < ( curStep - 1 ) * 6; loop++)
        {
          curNumSteps += temp + loop;
          temp += loop;
        }
      numStepsB = curNumSteps - numStepsA;

      if(minNumSteps > curNumSteps)
        {
          minNumSteps = curNumSteps;
          step = curStep;

          minA = numStepsA;
          minB = numStepsB;
          length = ( curStep - 1 ) * 6;
        }
    }

  unsigned int numSteps = 0, totalSteps = 0, secondSteps = 0,
      start[3], stop[3];
  /*
   * in taskValues stehen jetzt die Längen der längsten Flüge.
   * in idList die Indizes der drei Punkte dieser Flüge.
   */
  unsigned int idList[NUM_TASKS_POINTS], idTempList[NUM_TASKS_POINTS];
  double taskValue[NUM_TASKS];
  for(unsigned int loop = 0; loop < NUM_TASKS; loop++)
      taskValue[loop] = 0;

  start[0] = step - 1;
  stop[0] = route.count() - ( 2 * step );
  stop[1] = route.count() - step;
  stop[2] = route.count();

  numSteps = __calculateBestTask(start, stop, step, idList, taskValue, true);
  totalSteps = numSteps;

  /* Sichern der ID's der Favoriten */
  for(unsigned int loop = 0; loop < NUM_TASKS_POINTS; loop++)
      idTempList[loop] = idList[loop];

  for(unsigned int loop = 0; loop < NUM_TASKS; loop++)
    {
      __setOptimizeRange(start, stop, idTempList, loop * 3, step);

      numSteps = __calculateBestTask(start, stop, 1, idList, taskValue, false);
      secondSteps += numSteps;
    }

  totalSteps += secondSteps;

  double dist1 = dist(route.at(idList[0]), route.at(idList[1]));
  double dist2 = dist(route.at(idList[1]), route.at(idList[2]));
  double dist3 = dist(route.at(idList[0]), route.at(idList[2]));
  double totalDist = dist1 + dist2 + dist3;

  /*
   * Dialogfenster einfügen: Soll Optimierung übernommen werden ???
   *
   * Da wir wissen, dass alle Wegpunkte erreicht worden sind, können wir
   * hier die Berechnung der Punkte vereinfachen!
   */
  QString text, distText, pointText;
  if(__isFAI(dist1, dist2, dist3, totalDist))
      pointText.sprintf(" %d (FAI)", (int)(totalDist * 2.0 * 0.85));
  else
      pointText.sprintf(" %d", (int)(totalDist * 1.75 * 0.85));

  distText.sprintf(" %.2f km  ", totalDist);
  text = i18n("The task has been optimized. The best task found is:\n\n");
  text = text + "\t1:  "
      + printPos(route.at(idList[0])->origP.y(), true) + " / "
      + printPos(route.at(idList[0])->origP.x()) + "\n\t2:  "
      + printPos(route.at(idList[1])->origP.y(), true) + " / "
      + printPos(route.at(idList[1])->origP.x()) + "\n\t3:  "
      + printPos(route.at(idList[2])->origP.y(), true) + " / "
      + printPos(route.at(idList[2])->origP.x()) + "\n\n\t"
      + i18n("Distance:") + distText + i18n("Points:") + pointText + "\n\n"
      + i18n("Do You want to use this task and replace the old?") + "\n";

//  if(KMsgBox::yesNo(0, i18n("Optimizing"), text, KMsgBox::QUESTION) == 1) {
//    origList.clear();
//    struct wayPoint* cPoint;
//    for(unsigned int loop = 0; loop < wpList.count(); loop++) {
      /*
       * Kompliziert, aber wir brauchen eine echte Kopie der Punkte!
       */
//      cPoint = new struct wayPoint;
//      cPoint->angle = wpList.at(loop)->angle;
//      cPoint->distance = wpList.at(loop)->distance;
//      cPoint->latitude = wpList.at(loop)->latitude;
//      cPoint->longitude = wpList.at(loop)->longitude;
//      cPoint->name = wpList.at(loop)->name;
//      cPoint->sector1 = wpList.at(loop)->sector1;
//      cPoint->sector2 = wpList.at(loop)->sector2;
//      cPoint->sectorFAI = wpList.at(loop)->sectorFAI;
//      cPoint->type = wpList.at(loop)->type;
//      origList.append(cPoint);
//    }
//    wpList.clear();
//    tBegin = 0;
//    tEnd = 0;
//    origDistanceWP = distance_wp;
//    origDistanceTot = distance_tot;
//    origType = flightType;
//    origPoints = taskPoints;
//    distance_tot = 0;
//    distance_wp = 0;
//    taskPoints = 0.0;
//    flightType = NotSet;

    /*
     * Der optimierte Flug hat einen versetzten Start- und Landepunkt.
     * Daher müssen hier Start und Landung mit eingetragen werden.
     */
//    __appendWaypoint("Take-Off", flightRoute[0].latitude,
//        flightRoute[0].longitude);
//    __appendWaypoint("Optimize 1", flightRoute[idList[0]].latitude,
//        flightRoute[idList[0]].longitude);
//    __appendWaypoint("Optimize 2", flightRoute[idList[1]].latitude,
//        flightRoute[idList[1]].longitude);
//    __appendWaypoint("Optimize 3", flightRoute[idList[2]].latitude,
//        flightRoute[idList[2]].longitude);
//    __appendWaypoint("Landing", flightRoute[0].latitude,
//        flightRoute[0].longitude);

//    __checkType();
//    __checkWaypoints();
//    return true;
//  }
  return false;
}
