/***********************************************************************
**
**   flighttask.cpp
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

#include "flighttask.h"

#include <mapcalc.h>

#include <cmath>

#include <iostream.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#define PRE_ID loop - 1
#define CUR_ID loop
#define NEXT_ID loop + 1

/* Die Einstellungen können mal in die Voreinstellungsdatei wandern ... */
#define FAI_POINT 2.0
#define NORMAL_POINT 1.75
#define R1 (3000.0 / glMapMatrix->getScale())
#define R2 (500.0 / glMapMatrix->getScale())

FlightTask::FlightTask(QString fName)
  : BaseFlightElement("task", BaseMapElement::Task, fName),
    isOrig(false)
{

}

FlightTask::FlightTask(QList<wayPoint> wpL, bool isO, QString fName)
  : BaseFlightElement("task", BaseMapElement::Task, fName),
    isOrig(isO),
    wpList(wpL)
{
  __setWaypointType();

  __checkType();

  for(unsigned int loop = 0; loop < wpList.count(); loop++)
      __sectorangle(loop, false);
}

FlightTask::~FlightTask()
{

}

void FlightTask::__checkType()
{
  /**
   * Prooves the type of the task.
   **/
  distance_tot = 0;
  double distance_tot_d = 0;

  if(wpList.count() < 4)
    {
      flightType = FlightTask::NotSet;
      return;
    }

  for(unsigned int loop = 2; loop <= wpList.count() - 2; loop++)
      distance_tot = distance_tot + wpList.at(loop)->distance;

  if(dist(wpList.at(1),wpList.at(wpList.count() - 2)) < 1.0)
    {
      switch(wpList.count() - 4)
        {
          case 0:
            // Fehler
            flightType = FlightTask::NotSet;
            break;
          case 1:
            // Zielrückkehr
            flightType = FlightTask::ZielR;
            break;
          case 2:
            // FAI Dreieck
            if(isFAI(distance_tot,wpList.at(2)->distance,
                wpList.at(3)->distance, wpList.at(4)->distance))
               flightType = FlightTask::FAI;
            else
              // Dreieck
              flightType = FlightTask::Dreieck;
            break;
          case 3:
            // Start auf Schenkel oder Vieleck
            // Vieleck Ja/Nein kann endgültig erst bei der Analyse des Fluges
            // bestimmt werden!
            //
            // Erste Abfrage je nachdem ob Vieleck oder Dreieck mehr Punkte geben
            // würde
            distance_tot_d = distance_tot - wpList.at(2)->distance
                 - wpList.at(5)->distance + dist(wpList.at(2), wpList.at(4));

            if(isFAI(distance_tot_d, dist(wpList.at(2), wpList.at(4)),
                  wpList.at(3)->distance, wpList.at(4)->distance))
              {
                if(distance_tot > distance_tot_d * (1.0 + 1.0/3.0))
                    flightType = FlightTask::Vieleck;
                else
                  {
                    flightType = FlightTask::FAI_S;
                    distance_tot = distance_tot_d;
                  }
              }
            else
              {
                if(distance_tot > distance_tot_d * (1.0 + 1.0/6.0))
                    flightType = FlightTask::Vieleck;
                else
                  {
                    flightType = FlightTask::Dreieck_S;
                    distance_tot = distance_tot_d;
                  }
              }
            break;
          case 5:
            // 2x Dreieck nur als FAI gültig
            flightType = Unknown;
            if( (distance_tot / 2 <= 100) && (wpList.at(1) == wpList.at(4)) &&
                    (wpList.at(2) == wpList.at(5)) &&
                    (wpList.at(3) == wpList.at(6)) &&
                    isFAI(distance_tot / 2, wpList.at(2)->distance,
                            wpList.at(3)->distance, wpList.at(4)->distance))
                flightType = FlightTask::FAI_2;
            break;
          case 6:
            // 2x Dreieck auf Schenkel FAI
            flightType = FlightTask::Unknown;
            distance_tot = distance_tot - wpList.at(2)->distance
                 - wpList.at(5)->distance
                 + dist(wpList.at(2), wpList.at(4)) * 2;

            if( (distance_tot / 2 <= 100) &&
                    (wpList.at(2) == wpList.at(5)) &&
                    (wpList.at(3) == wpList.at(6)) &&
                    (wpList.at(4) == wpList.at(7)) &&
                    isFAI(distance_tot, dist(wpList.at(2), wpList.at(4)),
                      wpList.at(3)->distance, wpList.at(4)->distance))
              flightType = FlightTask::FAI_S2;

            break;
          case 8:
            // 3x FAI Dreieck
            flightType = Unknown;
            if( (distance_tot / 3 <= 100) &&
                    (wpList.at(1) == wpList.at(4)) &&
                    (wpList.at(2) == wpList.at(5)) &&
                    (wpList.at(3) == wpList.at(6)) &&
                    (wpList.at(1) == wpList.at(7)) &&
                    (wpList.at(2) == wpList.at(8)) &&
                    (wpList.at(3) == wpList.at(9)) &&
                    isFAI(distance_tot / 3, wpList.at(2)->distance,
                        wpList.at(3)->distance, wpList.at(4)->distance))
                flightType = FlightTask::FAI_3;
            break;
          case 9:
            // 3x FAI Dreieck Start auf Schenkel
            distance_tot = distance_tot - wpList.at(2)->distance
                 - wpList.at(5)->distance
                 + dist(wpList.at(2), wpList.at(4)) * 3;

            flightType = Unknown;
            if( (distance_tot / 3 <= 100) &&
                    (wpList.at(2) == wpList.at(5)) &&
                    (wpList.at(3) == wpList.at(6)) &&
                    (wpList.at(4) == wpList.at(7)) &&
                    (wpList.at(2) == wpList.at(8)) &&
                    (wpList.at(3) == wpList.at(9)) &&
                    (wpList.at(4) == wpList.at(10)) &&
                     isFAI(distance_tot, dist(wpList.at(2), wpList.at(4)),
                        wpList.at(3)->distance, wpList.at(4)->distance))
              flightType = FlightTask::FAI_S3;
          default:
            flightType = FlightTask::Unknown;
        }
    }
  else
    {
      if(wpList.count() <= 1 + 4)
          // Zielstrecke
          flightType = FlightTask::ZielS;
      else
          flightType = FlightTask::Unknown;
    }
}

double FlightTask::__sectorangle(int loop, bool isDraw)
{
  /*
   * Berechnet die Winkelhalbierende des Sektors
   */
  double nextAngle = 0.0, preAngle = 0.0, sectorAngle = 0.0;
  // Art des Wendepunktes
  switch(wpList.at(loop)->type)
    {
      case Begin:
        // directions to the next point
        sectorAngle = polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(NEXT_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(NEXT_ID)->projP.y() ) );
        break;
      case RouteP:
        // directions to the previous point
        preAngle = polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(PRE_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(PRE_ID)->projP.y() ) );
        // direction to the following point:
        nextAngle = polar(
            ( wpList.at(CUR_ID)->projP.x() - wpList.at(NEXT_ID)->projP.x() ),
            ( wpList.at(CUR_ID)->projP.y() - wpList.at(NEXT_ID)->projP.y() ) );

        sectorAngle = (preAngle + nextAngle) / 2.0;
        if( ( preAngle < PI / 2.0 && nextAngle > 1.5 * PI ) ||
            ( nextAngle < PI / 2.0 && preAngle > 1.5 * PI ) )
            sectorAngle = sectorAngle - PI;
        break;
      case End:
        // direction to the previous point:
        sectorAngle = polar(
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

void FlightTask::__setWaypointType()
{
  /*
   * Setzt den Status der Wendepunkte
   */

  // Kein Wendepunkt definiert
  if (wpList.count() < 4) return;

  // warning("WendePunkte: %d",wpList.count());
  wpList.at(0)->type = FlightTask::TakeOff;
  wpList.at(1)->type = FlightTask::Begin;

  for(unsigned int n = 2; n + 2 < wpList.count(); n++)
      wpList.at(n)->type = FlightTask::RouteP;

  wpList.at(wpList.count() - 2)->type = FlightTask::End;
  wpList.at(wpList.count() - 1)->type = FlightTask::Landing;
}

int FlightTask::getTaskType() const  {  return flightType;  }

bool FlightTask::isFAI(double d_wp, double d1, double d2, double d3)
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

void FlightTask::drawMapElement(QPainter* targetPainter,
    QPainter* maskPainter)
{
  double w1;
  // Strecke und Sektoren zeichnen
//  if(flightType != NotSet)
  if(flightType != 99999)
    {
      QPoint tempP;

      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          /*
           * w1 ist die Winkelhalbierende des Sektors!!!
           *      (Angaben in 1/16 Grad)
           */
          w1 = ( ( glMapMatrix->map(wpList.at(loop)->angle) + PI ) / PI )
                  * 180.0 * 16.0 * -1.0;

          tempP = glMapMatrix->map(wpList.at(loop)->projP);
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
              case FlightTask::RouteP:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
                targetPainter->setBrush(QColor(255, 110, 110));
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 1440, 2880);
                targetPainter->setBrush(QColor(110, 255, 110));
                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Inneren Sektor erneut zeichnen, damit Trennlinien
                // zwischen Sekt. 1 und Zylinder verschwinden
                targetPainter->setPen(QPen::NoPen);
                targetPainter->drawEllipse(gx + 2, gy + 2,
                    (2 * R2) - 4, (2 * R2) - 4);

                maskPainter->setPen(QPen(Qt::color1, 2));
                maskPainter->setBrush(QBrush(Qt::color1));
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 1440, 2880);
                maskPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                if(loop)
                  {
                    if((flightType == FAI_S || flightType == Dreieck_S) &&
                            loop == 2)
                        targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
                    else
                        targetPainter->setPen(QPen(QColor(150, 0, 200), 3));

                    maskPainter->setPen(QPen(Qt::color1, 3));
                    targetPainter->drawLine(
                        glMapMatrix->map(wpList.at(loop - 1)->projP),
                        glMapMatrix->map(wpList.at(loop)->projP));
                    maskPainter->drawLine(
                        glMapMatrix->map(wpList.at(loop - 1)->projP),
                        glMapMatrix->map(wpList.at(loop)->projP));
                  }
                break;

              case FlightTask::Begin:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
                targetPainter->setBrush(QBrush(QColor(255, 0, 0),
                    QBrush::BDiagPattern));
                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                maskPainter->setPen(QPen(Qt::color1, 2));
                maskPainter->setBrush(QBrush(Qt::color1, QBrush::BDiagPattern));
                maskPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Linie von Startpunkt zum Aufgaben Beginn
                //
                if(loop)
                  {
                    targetPainter->setPen(QPen(QColor(255, 0, 0), 4));
                    maskPainter->setPen(QPen(Qt::color1, 2));
                    targetPainter->drawLine(
                        glMapMatrix->map(wpList.at(loop - 1)->projP),
                        glMapMatrix->map(wpList.at(loop)->projP));
                    maskPainter->drawLine(
                        glMapMatrix->map(wpList.at(loop - 1)->projP),
                        glMapMatrix->map(wpList.at(loop)->projP));
                   warning("zeichne Linie zum Beginn loop: %d",loop);
                   cout << "von " << wpList.at(loop - 1)->name << " nach: " << wpList.at(loop)->name << endl;
                  }
                break;

              case FlightTask::End:
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
                    glMapMatrix->map(wpList.at(loop-1)->projP),
                    glMapMatrix->map(wpList.at(loop)->projP));

                maskPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                maskPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Strecke
                if(flightType == FAI_S || flightType == Dreieck_S)
                    targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
                else
                    targetPainter->setPen(QPen(QColor(150, 0, 200), 3));

                maskPainter->setPen(QPen(Qt::color1, 3));
                targetPainter->drawLine(
                  glMapMatrix->map(wpList.at(loop - 1)->projP),
                  glMapMatrix->map(wpList.at(loop)->projP));
                maskPainter->drawLine(
                  glMapMatrix->map(wpList.at(loop - 1)->projP),
                  glMapMatrix->map(wpList.at(loop)->projP));
                break;

              default:
                // Kann noch Start und Landepunkt sein.

                // Linie von Startpunkt zum Aufgaben Beginn
                if(loop)
                  {
                    if(flightType == FAI_S || flightType == Dreieck_S)
                        targetPainter->setPen(QPen(QColor(150, 0, 200), 3));
                    else
                        targetPainter->setPen(QPen(QColor(0, 0, 255), 2));

                    maskPainter->setPen(QPen(Qt::color1, 2));
                    targetPainter->drawLine(
                        glMapMatrix->map(wpList.at(loop - 1)->projP),
                        glMapMatrix->map(wpList.at(loop)->projP));
                    maskPainter->drawLine(
                        glMapMatrix->map(wpList.at(loop - 1)->projP),
                        glMapMatrix->map(wpList.at(loop)->projP));
                  }

                // Linie Um Start Lande Punkt
                targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
                targetPainter->setBrush(QBrush::NoBrush);
                maskPainter->setPen(QPen(Qt::color1, 2));
                maskPainter->setBrush(QBrush::NoBrush);
                break;
            }
        }
    }

  // Strecke bei Start auf Schenkel
  if(flightType == FAI_S || flightType == Dreieck_S)
    {
      targetPainter->setPen(QPen(QColor(150, 0, 200), 3));
      maskPainter->setPen(QPen(Qt::color1, 3));
      targetPainter->drawLine(glMapMatrix->map(wpList.at(2)->projP),
          glMapMatrix->map(wpList.at(wpList.count() - 3)->projP));
      maskPainter->drawLine(glMapMatrix->map(wpList.at(2)->projP),
          glMapMatrix->map(wpList.at(wpList.count() - 3)->projP));
    }
}

void FlightTask::printMapElement(QPainter* targetPainter, bool isText)
{
  double w1;

  // Strecke und Sektoren zeichnen
  if(flightType != FlightTask::NotSet)
    {
      QPoint tempP;

      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          /*
           * w1 ist die Winkelhalbierende des Sektors!!!
           *      (Angaben in 1/16 Grad)
           */
          w1 = ( ( glMapMatrix->print(wpList.at(loop)->angle) + PI ) / PI )
                  * 180.0 * 16.0 * -1.0;

          tempP = glMapMatrix->print(wpList.at(loop)->projP);
          double qx = -R1 + tempP.x();
          double qy = -R1 + tempP.y();
          double gx = -R2 + tempP.x();
          double gy = -R2 + tempP.y();

          switch(wpList.at(loop)->type)
            {
              case FlightTask::RouteP:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
                targetPainter->setBrush(QColor(255, 110, 110));
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 1440, 2880);
                targetPainter->setBrush(QColor(110, 255, 110));
                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Inneren Sektor erneut zeichnen, damit Trennlinien
                // zwischen Sekt. 1 und Zylinder verschwinden
                targetPainter->setPen(QPen::NoPen);
                targetPainter->drawEllipse(gx + 2, gy + 2,
                    (2 * R2) - 4, (2 * R2) - 4);

                if(loop)
                  {
                    if((flightType == FAI_S || flightType == Dreieck_S) &&
                            loop == 2)
                        targetPainter->setPen(QPen(QColor(0, 0, 0), 2,
                            Qt::DashLine));
                    else
                        targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                    targetPainter->drawLine(
                        glMapMatrix->print(wpList.at(loop - 1)->projP),
                        glMapMatrix->print(wpList.at(loop)->projP));
                  }
                break;
              case FlightTask::Begin:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
                targetPainter->setBrush(QBrush(QColor(255, 0, 0),
                    QBrush::BDiagPattern));
                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Linie von Startpunkt zum Aufgaben Beginn
                //
                if(loop)
                  {
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
                    targetPainter->drawLine(
                        glMapMatrix->print(wpList.at(loop - 1)->projP),
                        glMapMatrix->print(wpList.at(loop)->projP));
                  }
                break;

              case FlightTask::End:
                targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
                targetPainter->setBrush(QBrush(QColor(0, 0, 255),
                    QBrush::FDiagPattern));

                targetPainter->drawEllipse(gx, gy, 2 * R2, 2 * R2);
                targetPainter->drawPie(qx, qy, 2 * R1, 2 * R1, w1 - 720, 1440);

                // Hier wird die Linie vom letzten Wegpunkt
                // zum Endpunkt gemalt. Die gleiche Linie wird weiter
                // unten erneut gezeichnet !!!
//                targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
//                targetPainter->drawLine(
//                    glMapMatrix->print(wpList.at(loop-1)->projP),
//                    glMapMatrix->print(wpList.at(loop)->projP));

                // Strecke
                if(flightType == FAI_S || flightType == Dreieck_S)
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2,
                        Qt::DashLine));
                else
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                targetPainter->drawLine(
                    glMapMatrix->print(wpList.at(loop - 1)->projP),
                    glMapMatrix->print(wpList.at(loop)->projP));
                break;

              default:
                // Kann noch Start und Landepunkt sein.

                // Linie von Startpunkt zum Aufgaben Beginn
                if(loop)
                  {
                    if(flightType == FAI_S || flightType == Dreieck_S)
                        targetPainter->setPen(QPen(QColor(0, 0, 0), 2,
                            Qt::DashLine));
                    else
                        targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                    targetPainter->drawLine(
                        glMapMatrix->print(wpList.at(loop - 1)->projP),
                        glMapMatrix->print(wpList.at(loop)->projP));
                  }

                // Linie Um Start Lande Punkt
                targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
                targetPainter->setBrush(QBrush::NoBrush);
                break;
            }
        }
    }

  // Strecke bei Start auf Schenkel
  if(flightType == FAI_S || flightType == Dreieck_S)
    {
      targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
      targetPainter->drawLine(glMapMatrix->print(wpList.at(2)->projP),
          glMapMatrix->print(wpList.at(wpList.count() - 3)->projP));
    }
}

void FlightTask::checkWaypoints(QList<flightPoint> route,
    QString gliderType)
{
  /*
   *   Überprüft, ob die Sektoren der Wendepunkte erreicht wurden
   *
   *   SOLLTE NOCHMALS ÜBERARBEITET WERDEN
   *
   */
  bool time_error = false;

  if(flightType == FlightTask::NotSet) return;

  int gliderIndex = 100, preTime = 0;
  KConfig* config = KGlobal::config();
  config->setGroup("FlightPoints");

  double pointFAI = config->readDoubleNumEntry("FAIPoint", 2.0);
  double pointNormal = config->readDoubleNumEntry("NormalPoint", 1.75);
  double pointCancel = config->readDoubleNumEntry("CancelPoint", 1.0);
  double pointZielS = config->readDoubleNumEntry("ZielSPoint", 1.5);
  double malusValue = config->readDoubleNumEntry("MalusValue", 15.0);
  double sectorMalus = config->readDoubleNumEntry("SectorMalus", -0.1);

  if(gliderType != 0L)
    {
      config->setGroup("GliderTypes");
      gliderIndex = config->readNumEntry(gliderType, 100);
    }

  config->setGroup(0);

  for(unsigned int loop = 0; loop < route.count(); loop++)
    {
      if(loop && (route.at(loop)->time - preTime > 70))
          /*
           *           Zeitabstand zwischen Loggerpunkten ist zu gross!
           *                      (vgl. Code Sportif 3, Ziffer 1.9.2.1)
           */
          time_error = true;

      preTime = route.at(loop)->time;
    }

  unsigned int startIndex = 0, dummy = 0;

  for(unsigned int loop = 0; loop < wpList.count(); loop++)
    {
      double deltaAngle = 0.0, pointAngle = 0.0;
      dummy = 0;

      __sectorangle(loop, false);
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

              pointAngle = polar(
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
  int faiCount = 0;
  unsigned int dmstCount = 0;
  double dmstMalus = 1.0, aussenlande = 0.0;
  bool home, stop = false;

  if(wpList.at(1)->sector1 == 0)
    {
      KMessageBox::information(0,
          i18n("You have not reached the first waypoint of your task."));
      return;
    }

  // Durchgehen der Wendepunkte
  for(unsigned int loop = 1; loop < wpList.count() - 2; loop++)
    {
      if(!stop)
        {
          if(wpList.at(loop)->sector1 != 0)
              dmstCount++;
          else if(wpList.at(loop)->sector2 != 0)
            {
              dmstMalus += sectorMalus;
              dmstCount++;
            }
          else
            {
              // Wendepunkt nicht erreicht!!
              if(loop == 2)
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

      if(wpList.at(loop)->sectorFAI != 0)
          faiCount++;
    }

  if(wpList.at(wpList.count() - 2)->sector1 == 0)
    {
      home = false;
      KMessageBox::error(0,
          i18n("You have not reached the last point of your task."));

      if(dist(wpList.at(1 + dmstCount), route.last()) < 1.0)
        {
          // Landung auf letztem Wegpunkt
        }
      else
          // Außenlandung -- Wertung: + 1Punkt bis zur Außenlandung
          aussenlande = dist(wpList.at(1 + dmstCount), route.last());
    }
  else
    {
      // Zu Hause gelandet
      dmstCount++;
      faiCount++;
      home = true;
    }

  double wertDist = 0, F = 1;

  if(dmstCount != wpList.count() - 2)
    {
      if(home)
        {
          wertDist = dist(wpList.at(wpList.count() - 1),
              wpList.at(dmstCount));
          for(unsigned int loop = 1; loop < 1 + dmstCount; loop++)
              wertDist = wertDist + wpList.at(loop)->distance;
        }
      else
        {
          for(unsigned int loop = 1; loop <= 1 + dmstCount; loop++)
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
          case FlightTask::ZielS:
            F = pointZielS;
            break;
          case FlightTask::ZielR:
          case FlightTask::Dreieck:
          case FlightTask::Dreieck_S:
            F = pointNormal;
            break;
          case FlightTask::FAI:
          case FlightTask::FAI_S:
            F = pointFAI;
            break;
          default:
            F = 0.0;
        }
      wertDist = distance_tot;
    }

  taskPoints = (wertDist * F * 100) / gliderIndex * dmstMalus +
                      (aussenlande * pointCancel * 100) / gliderIndex;

  if(!isOrig)
    {
      /*
       * Optimierter Flug: x% abziehen
       */
      taskPoints -= ( taskPoints * (malusValue / 100.0) );
    }

  if (time_error)
   {
     KMessageBox::error(0,
        i18n("The time intervall between two points<BR>"
             "of the flight is more than 70 sec.!<BR>"
             "Due to Code Sportif 3, Nr. 1.9.2.1,<BR>"
             "the flight can not be valued!"));
   }

}

QString FlightTask::getRouteType() const
{
  /*
   * Die Ausgabetexte müssen noch ins Englische übersetzt werden ...
   */
  switch(flightType)
    {
      case NotSet:      return "nicht gesetzt";
      case ZielS:       return "Zielstrecke";
      case ZielR:       return "Zielrückkehr";
      case FAI:         return "FAI-Dreieck";
      case Dreieck:     return "allg. Dreieck";
      case FAI_S:       return "FAI-Dreieck (S)";
      case Dreieck_S:   return "allg. Dreieck (S)";
      case Abgebrochen: return "abgebrochen";
      default:          return "unbekannt";
    }
}

QString FlightTask::getDistanceString() const
{
  if(flightType == FlightTask::NotSet)  return "--";

  QString distString;
  distString.sprintf("%.2f km", distance_tot);

  return distString;
}

QString FlightTask::getTaskDistanceString() const
{
  if(flightType == FlightTask::NotSet)  return "--";

  QString distString;
  distString.sprintf("%.2f km ", distance_wp);

  return distString;
}

QString FlightTask::getPointsString() const
{
  if(flightType == FlightTask::NotSet)  return "--";

  int points1 = (int) taskPoints;
  if((int) ( (taskPoints - points1) * 10 ) > 5) points1++;

  QString pointString;
  pointString.sprintf("%d", points1);

  return pointString;
}

QRect FlightTask::getRect() const  {  return bBoxTask;  }

void FlightTask::setWaypointList(QList<wayPoint> wpL)
{
  wpList = wpL;

  isOrig = false;

  __checkType();

  for(unsigned int loop = 0; loop < wpList.count(); loop++)
      __sectorangle(loop, false);

  __setWaypointType();
}
/** No descriptions */
QString FlightTask::getFlightInfoString()
{
  QString htmlText;

  htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
      <TR><TD>" + i18n("Task") + ":</TD><TD><A HREF=EDITTASK>" +
      getFileName() +  + "</A></TD></TR>\
      </TABLE><HR NOSHADE>";

  QList<wayPoint> wpList = getWPList();

  if(wpList.count()) {
    htmlText += "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
      <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
       i18n("Waypoints") + ":</B></TD></TR>";

    for(unsigned int loop = 0; loop < wpList.count(); loop++) {
      if(loop > 0) {
        QString tmp;
        tmp.sprintf("%.2f km",wpList.at(loop)->distance);

        htmlText += "<TR><TD ALIGN=center COLSPAN=3 BGCOLOR=#EEEEEE>" +
          tmp + "</TD></TR>";
      }

      QString idString;
      idString.sprintf("%d", loop);

      htmlText += "<TR><TD COLSPAN=2><A HREF=" + idString + ">" +
        wpList.at(loop)->name + "</A></TD>\
        <TR><TD WIDTH=15></TD>\
        <TD>" + printPos(wpList.at(loop)->origP.x()) + "</TD>\
        <TD ALIGN=right>" + printPos(wpList.at(loop)->origP.y(), false) +
        "</TD></TR></TABLE>";
    }

    htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("total Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" + getDistanceString() + "</TD></TR>\
      </TABLE>";
  }
  else {
    htmlText += i18n("Select Flight/graphical Taskplanning from the menu and draw your task on the map<BR><EM>or</EM><BR>click on the task name to start editing");
  }

  return htmlText;
}
