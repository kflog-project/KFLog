/***********************************************************************
 **
 **   flighttask.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001 by Heiner Lamprecht
 **                   2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "flighttask.h"
#include "mapcalc.h"

#define PRE_ID loop - 1
#define CUR_ID loop
#define NEXT_ID loop + 1

/* Die Einstellungen können mal in die Voreinstellungsdatei wandern ... */
#define FAI_POINT 2.0
#define NORMAL_POINT 1.75
#define R1 (3000.0 / glMapMatrix->getScale())
#define R2 (500.0 / glMapMatrix->getScale())

// declare static objects used for translations
QHash<int, QString> FlightTask::taskTypeTranslations;
QStringList FlightTask::sortedTaskTypeTranslations;

FlightTask::FlightTask(const QString& fName) :
  BaseFlightElement("task", BaseMapElement::Task, fName),
  isOrig(false),
  olcPoints(0),
  taskPoints(0),
  flightType(FlightTask::NotSet),
  __planningType(Route),
  __planningDirection(leftOfRoute)
{
}

FlightTask::FlightTask(const QList<Waypoint*>& wpL, bool isO, const QString& fName) :
  BaseFlightElement("task", BaseMapElement::Task, fName),
  isOrig(isO),
  olcPoints(0),
  taskPoints(0),
  __planningType(Route),
  __planningDirection(leftOfRoute)
{
  //warning("FlightTask(QPtrList<wayPoint> wpL, bool isO, QString fName)");
  setWaypointList(wpL);
}

FlightTask::~FlightTask()
{
  // Hope that nobody holds a shallow copy of this list.
  qDeleteAll( wpList );
}

void FlightTask::__checkType()
{
  /**
   * Proves the type of the task.
   **/
  distance_task = 0;
  distance_total = 0;
  double distance_task_d = 0;

  if (wpList.count() > 0) {
    for(int loop = 1; loop <= wpList.count() - 1; loop++)
      {
        distance_total += wpList.at(loop)->distance;
      }
  }

  if( wpList.count() < 4 )
    {
      flightType = FlightTask::NotSet;
      return;
    }

  distance_task = distance_total - wpList.at(1)->distance - wpList.at(wpList.count() - 1)->distance;

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
          if(isFAI(distance_task,wpList.at(2)->distance,
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
          distance_task_d = distance_task - wpList.at(2)->distance
            - wpList.at(5)->distance + dist(wpList.at(2), wpList.at(4));

          if(isFAI(distance_task_d, dist(wpList.at(2), wpList.at(4)),
                   wpList.at(3)->distance, wpList.at(4)->distance))
            {
              if(distance_task > distance_task_d * (1.0 + 1.0/3.0))
                flightType = FlightTask::Vieleck;
              else
                {
                  flightType = FlightTask::FAI_S;
                  distance_task = distance_task_d;
                }
            }
          else
            {
              if(distance_task > distance_task_d * (1.0 + 1.0/6.0))
                flightType = FlightTask::Vieleck;
              else
                {
                  flightType = FlightTask::Dreieck_S;
                  distance_task = distance_task_d;
                }
            }
          break;
        case 5:
          // 2x Dreieck nur als FAI gültig
          flightType = Unknown;
          if( (distance_task / 2 <= 100) && (wpList.at(1) == wpList.at(4)) &&
              (wpList.at(2) == wpList.at(5)) &&
              (wpList.at(3) == wpList.at(6)) &&
              isFAI(distance_task / 2, wpList.at(2)->distance,
                    wpList.at(3)->distance, wpList.at(4)->distance))
            flightType = FlightTask::FAI_2;
          break;
        case 6:
          // 2x Dreieck auf Schenkel FAI
          flightType = FlightTask::Unknown;
          distance_task = distance_task - wpList.at(2)->distance
            - wpList.at(5)->distance
            + dist(wpList.at(2), wpList.at(4)) * 2;

          if( (distance_task / 2 <= 100) &&
              (wpList.at(2) == wpList.at(5)) &&
              (wpList.at(3) == wpList.at(6)) &&
              (wpList.at(4) == wpList.at(7)) &&
              isFAI(distance_task, dist(wpList.at(2), wpList.at(4)),
                    wpList.at(3)->distance, wpList.at(4)->distance))
            flightType = FlightTask::FAI_S2;

          break;
        case 8:
          // 3x FAI Dreieck
          flightType = Unknown;
          if( (distance_task / 3 <= 100) &&
              (wpList.at(1) == wpList.at(4)) &&
              (wpList.at(2) == wpList.at(5)) &&
              (wpList.at(3) == wpList.at(6)) &&
              (wpList.at(1) == wpList.at(7)) &&
              (wpList.at(2) == wpList.at(8)) &&
              (wpList.at(3) == wpList.at(9)) &&
              isFAI(distance_task / 3, wpList.at(2)->distance,
                    wpList.at(3)->distance, wpList.at(4)->distance))
            flightType = FlightTask::FAI_3;
          break;
        case 9:
          // 3x FAI Dreieck Start auf Schenkel
          distance_task = distance_task - wpList.at(2)->distance
            - wpList.at(5)->distance
            + dist(wpList.at(2), wpList.at(4)) * 3;

          flightType = Unknown;
          if( (distance_task / 3 <= 100) &&
              (wpList.at(2) == wpList.at(5)) &&
              (wpList.at(3) == wpList.at(6)) &&
              (wpList.at(4) == wpList.at(7)) &&
              (wpList.at(2) == wpList.at(8)) &&
              (wpList.at(3) == wpList.at(9)) &&
              (wpList.at(4) == wpList.at(10)) &&
              isFAI(distance_task, dist(wpList.at(2), wpList.at(4)),
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
  double nextAngle = 0.0, preAngle = 0.0, sectorAngle = 0.0;

  // In some cases during planning, this method is called with wrong
  // loop-values. Therefore we must check the id before calculating
  // the direction
  switch(wpList.at(loop)->tpType)
    {
    case Begin:
      // directions to the next point
      if(wpList.count() >= loop + 1)
        sectorAngle = polar(
                            ( wpList.at(CUR_ID)->projP.x() - wpList.at(NEXT_ID)->projP.x() ),
                            ( wpList.at(CUR_ID)->projP.y() - wpList.at(NEXT_ID)->projP.y() ) );
      else
        sectorAngle = 0;
      break;
    case RouteP:
      if(loop >= 1 && wpList.count() >= loop + 1)
        {
          // directions to the previous point
          preAngle = polar(
                           ( wpList.at(CUR_ID)->projP.x() - wpList.at(PRE_ID)->projP.x() ),
                           ( wpList.at(CUR_ID)->projP.y() - wpList.at(PRE_ID)->projP.y() ) );
          // direction to the following point:
          nextAngle = polar(
                            ( wpList.at(CUR_ID)->projP.x() - wpList.at(NEXT_ID)->projP.x() ),
                            ( wpList.at(CUR_ID)->projP.y() - wpList.at(NEXT_ID)->projP.y() ) );

          // vector pointing to the outside of the two points
          sectorAngle = outsideVector(preAngle, nextAngle);
        }
      else
        sectorAngle = 0;
      break;
    case End:
      if(loop >= 1 && loop < wpList.count())
        {
          // direction to the previous point:
          sectorAngle = polar(
                              ( wpList.at(CUR_ID)->projP.x() - wpList.at(PRE_ID)->projP.x() ),
                              ( wpList.at(CUR_ID)->projP.y() - wpList.at(PRE_ID)->projP.y() ) );
        }
      else
        sectorAngle = 0;
      break;
    }

  // Nur nötig bei der Überprüfung der Wegpunkte,
  // würde beim Zeichnen zu Fehlern führen
  if(!isDraw) sectorAngle += M_PI;

  if(sectorAngle > (2 * M_PI)) sectorAngle = sectorAngle - (2 * M_PI);

  wpList.at(CUR_ID)->angle = sectorAngle;

  return sectorAngle;
}

void FlightTask::__setWaypointType()
{
  /*
   * Setzt den Status der Wendepunkte und die Distanzen
   */
  int cnt = wpList.count();

  if( cnt > 0 )
    {
      wpList.at( 0 )->tpType = FlightTask::FreeP;
    }

  // Distances
  for(int n = 1; n  < cnt; n++)
    {
      wpList.at(n)->distance = dist(wpList.at(n-1),wpList.at(n));
      wpList.at(n)->tpType = FlightTask::FreeP;
    }

  // Kein Wendepunkt definiert
  if (cnt < 4)  return;

  wpList.at(0)->tpType = FlightTask::TakeOff;
  wpList.at(1)->tpType = FlightTask::Begin;
  wpList.at(cnt - 2)->tpType = FlightTask::End;
  wpList.at(cnt - 1)->tpType = FlightTask::Landing;

  for( int n = 2; n + 2 < cnt; n++ )
    {
      wpList.at( n )->tpType = FlightTask::RouteP;
    }
}

int FlightTask::getTaskType() const  {  return flightType;  }

QString FlightTask::getTaskTypeString() const
{
  switch(flightType)
    {
    case FlightTask::NotSet:       return QObject::tr("not set");
    case FlightTask::ZielS:        return QObject::tr("Free Distance");
    case FlightTask::ZielR:        return QObject::tr("Free Out and Return Distance");
    case FlightTask::FAI:          return QObject::tr("FAI Triangle");
    case FlightTask::Dreieck:      return QObject::tr("Triangle");
    case FlightTask::FAI_S:        return QObject::tr("FAI Triangle Start on leg");
    case FlightTask::Dreieck_S:    return QObject::tr("Triangle Start on leg");
    case FlightTask::OLC2003:      return QObject::tr("OLC optimized (2003 rules)");
    case FlightTask::Abgebrochen:  return QObject::tr("???");
    }

  return QObject::tr("Unknown");
}

bool FlightTask::isFAI(double d_wp, double d1, double d2, double d3)
{
  if( ( d_wp < 500.0 ) &&
      ( d1 >= 0.28 * d_wp && d2 >= 0.28 * d_wp && d3 >= 0.28 * d_wp ) )
    // small FAI
    return true;
  else if( d_wp >= 500.0 &&
           ( d1 > 0.25 * d_wp && d2 > 0.25 * d_wp && d3 > 0.25 * d_wp ) &&
           ( d1 <= 0.45 * d_wp && d2 <= 0.45 * d_wp && d3 <= 0.45 * d_wp ) )
    // large FAI
    return true;

  return false;
}

bool FlightTask::drawMapElement( QPainter* targetPainter )
{
  if( wpList.count() == 0 )
    {
      return false;
    }

  double w1;
  struct faiAreaSector *sect;
  QPoint tempP;
  QRect r;
  QString label;

  // Strecke und Sektoren zeichnen
  if(flightType != NotSet)
    {
      for(int loop = 0; loop < wpList.count(); loop++)
        {
          /*
           * w1 ist die Winkelhalbierende des Sektors!!!
           *      (Angaben in 1/16 Grad)
           */
          w1 = ( ( glMapMatrix->map(wpList.at(loop)->angle) + M_PI ) / M_PI )
            * 180.0 * 16.0 * -1.0;

          tempP = glMapMatrix->map(wpList.at(loop)->projP);
          double qx = -R1 + tempP.x();
          double qy = -R1 + tempP.y();
          double gx = -R2 + tempP.x();
          double gy = -R2 + tempP.y();


          if(loop)
            {
              bBoxTask.setLeft(qMin(tempP.x(), bBoxTask.left()));
              bBoxTask.setTop(qMax(tempP.y(), bBoxTask.top()));
              bBoxTask.setRight(qMax(tempP.x(), bBoxTask.right()));
              bBoxTask.setBottom(qMin(tempP.y(), bBoxTask.bottom()));
            }
          else
            {
              bBoxTask.setLeft(tempP.x());
              bBoxTask.setTop(tempP.y());
              bBoxTask.setRight(tempP.x());
              bBoxTask.setBottom(tempP.y());
            }

          switch(wpList.at(loop)->tpType)
            {
            case FlightTask::RouteP:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
              targetPainter->setBrush(QColor(255, 110, 110));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 1440), 2880);
              targetPainter->setBrush(QColor(110, 255, 110));
              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Inneren Sektor erneut zeichnen, damit Trennlinien
              // zwischen Sekt. 1 und Zylinder verschwinden
              targetPainter->setPen(Qt::NoPen);
              targetPainter->drawEllipse((int)(gx + 2), (int)(gy + 2),
                                         (int)((2 * R2) - 4), (int)((2 * R2) - 4));

              if(loop)
                {
                  if((flightType == FAI_S || flightType == Dreieck_S) && loop == 2)
                    targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
                  else
                    targetPainter->setPen(QPen(QColor(150, 0, 200), 3));

                  targetPainter->drawLine(
                                          glMapMatrix->map(wpList.at(loop - 1)->projP),
                                          glMapMatrix->map(wpList.at(loop)->projP));
                }
              break;

            case FlightTask::Begin:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
              targetPainter->setBrush(QBrush(QColor(255, 0, 0),
                                             Qt::BDiagPattern));
              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Linie von Startpunkt zum Aufgaben Beginn
              //
              if(loop)
                {
                  targetPainter->setPen(QPen(QColor(255, 0, 0), 4));

                  targetPainter->drawLine( glMapMatrix->map(wpList.at(loop - 1)->projP),
                                           glMapMatrix->map(wpList.at(loop)->projP));
                }
              break;

            case FlightTask::End:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 2));
              targetPainter->setBrush(QBrush(QColor(0, 0, 255),
                                             Qt::FDiagPattern));

              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
              targetPainter->drawLine(glMapMatrix->map(wpList.at(loop-1)->projP),
                                      glMapMatrix->map(wpList.at(loop)->projP));

              // Strecke
              if(flightType == FAI_S || flightType == Dreieck_S)
                targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
              else
                targetPainter->setPen(QPen(QColor(150, 0, 200), 3));

              targetPainter->drawLine(glMapMatrix->map(wpList.at(loop - 1)->projP),
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

                  targetPainter->drawLine( glMapMatrix->map(wpList.at(loop - 1)->projP),
                                           glMapMatrix->map(wpList.at(loop)->projP) );
                }

              // Linie Um Start Lande Punkt
              targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
              targetPainter->setBrush(Qt::NoBrush);
              break;
            }
        }
    }

  // Strecke bei Start auf Schenkel
  if(flightType == FAI_S || flightType == Dreieck_S)
    {
      targetPainter->setPen(QPen(QColor(150, 0, 200), 3));
      targetPainter->drawLine(glMapMatrix->map(wpList.at(2)->projP),
                              glMapMatrix->map(wpList.at(wpList.count() - 3)->projP));
    }

  // Area based planning
  if (getPlanningType() == FAIArea && wpList.count() > 3) {
    for(int loop = 0; loop < FAISectList.count(); loop++) {
      sect = FAISectList.at(loop);
      sect->pos->drawMapElement(targetPainter);
      label = sect->pos->getName();
      QPolygon pp = glMapMatrix->map(sect->pos->getProjectedPolygon());

      if (label == "FAILow500Sector" || label == "FAIHigh500Sector") {
        label.sprintf("%.0f km", sect->dist);
        tempP = pp[0];
        targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
        targetPainter->setBrush(QBrush(QColor(0, 255, 128)));
        targetPainter->setBackgroundMode(Qt::OpaqueMode);
        targetPainter->drawText(tempP, label);
        targetPainter->setBackgroundMode(Qt::TransparentMode);
      }
      else {
        r = pp.boundingRect();
        tempP = r.topLeft();
        bBoxTask.setLeft(qMin(tempP.x(), bBoxTask.left()));
        bBoxTask.setTop(qMax(tempP.y(), bBoxTask.top()));
        bBoxTask.setRight(qMax(tempP.x(), bBoxTask.right()));
        bBoxTask.setBottom(qMin(tempP.y(), bBoxTask.bottom()));
        tempP = r.bottomRight();
        bBoxTask.setLeft(qMin(tempP.x(), bBoxTask.left()));
        bBoxTask.setTop(qMax(tempP.y(), bBoxTask.top()));
        bBoxTask.setRight(qMax(tempP.x(), bBoxTask.right()));
        bBoxTask.setBottom(qMin(tempP.y(), bBoxTask.bottom()));
      }
    }
  }

  return true;
}

void FlightTask::printMapElement(QPainter* targetPainter, bool /*isText*/)
{
  double w1;
  struct faiAreaSector *sect;
  QPoint tempP;
  QString label;
  QPolygon pA;

  // Strecke und Sektoren zeichnen
  if(flightType != FlightTask::NotSet)
    {
      for(int loop = 0; loop < wpList.count(); loop++)
        {
          /*
           * w1 ist die Winkelhalbierende des Sektors!!!
           *      (Angaben in 1/16 Grad)
           */
          w1 = ( ( glMapMatrix->print(wpList.at(loop)->angle) + M_PI ) / M_PI )
            * 180.0 * 16.0 * -1.0;


          tempP = glMapMatrix->print(wpList.at(loop)->projP);
          double qx = -R1 + tempP.x();
          double qy = -R1 + tempP.y();
          double gx = -R2 + tempP.x();
          double gy = -R2 + tempP.y();

          switch(wpList.at(loop)->tpType)
            {
            case FlightTask::RouteP:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
              targetPainter->setBrush(QColor(255, 110, 110));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 1440), 2880);
              targetPainter->setBrush(QColor(110, 255, 110));
              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Inneren Sektor erneut zeichnen, damit Trennlinien
              // zwischen Sekt. 1 und Zylinder verschwinden
              targetPainter->setPen(Qt::NoPen);
              targetPainter->drawEllipse((int)(gx + 2), (int)(gy + 2),
                                         (int)((2 * R2) - 4), (int)((2 * R2) - 4));

              if(loop)
                {
                  if((flightType == FAI_S || flightType == Dreieck_S) && loop == 2)
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2, Qt::DashLine));
                  else
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                  targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->projP),
                                          glMapMatrix->print(wpList.at(loop)->projP));
                }
              break;
            case FlightTask::Begin:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
              targetPainter->setBrush(QBrush(QColor(255, 0, 0),
                                             Qt::BDiagPattern));
              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Linie von Startpunkt zum Aufgaben Beginn
              //
              if(loop)
                {
                  targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
                  targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->projP),
                                          glMapMatrix->print(wpList.at(loop)->projP));
                }
              break;

            case FlightTask::End:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
              targetPainter->setBrush(QBrush(QColor(0, 0, 255),
                                             Qt::FDiagPattern));

              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Hier wird die Linie vom letzten Wegpunkt
              // zum Endpunkt gemalt. Die gleiche Linie wird weiter
              // unten erneut gezeichnet !!!
              //                targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
              //                targetPainter->drawLine(
              //                    glMapMatrix->print(wpList.at(loop-1)->projP),
              //                    glMapMatrix->print(wpList.at(loop)->projP));

              // Strecke
              if(flightType == FAI_S || flightType == Dreieck_S)
                targetPainter->setPen(QPen(QColor(0, 0, 0), 2, Qt::DashLine));
              else
                targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

              targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->projP),
                                      glMapMatrix->print(wpList.at(loop)->projP));
              break;

            default:
              // Kann noch Start und Landepunkt sein.

              // Linie von Startpunkt zum Aufgaben Beginn
              if(loop)
                {
                  if(flightType == FAI_S || flightType == Dreieck_S)
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2, Qt::DashLine));
                  else
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                  targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->projP),
                                          glMapMatrix->print(wpList.at(loop)->projP));
                }

              // Linie Um Start Lande Punkt
              targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
              targetPainter->setBrush(Qt::NoBrush);
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

  // Area based planning
  if (getPlanningType() == FAIArea && wpList.count() > 3) {
    for (int loop = 0; loop < FAISectList.count(); loop++) {
      sect = FAISectList.at(loop);
      sect->pos->printMapElement(targetPainter, false);
      label = sect->pos->getName();
      if (label == "FAILow500Sector" || label == "FAIHigh500Sector") {
        label.sprintf("%.0f km", sect->dist);
        QPolygon pp = sect->pos->getProjectedPolygon();
        tempP = glMapMatrix->print(pp[0]);
        targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
        targetPainter->setBrush(QBrush(QColor(0, 255, 128)));
        targetPainter->setBackgroundMode(Qt::OpaqueMode);
        targetPainter->drawText(tempP, label);
        targetPainter->setBackgroundMode(Qt::TransparentMode);
      }
    }
//     targetPainter->setBrush(QBrush::NoBrush);

//     for (loop = 0; loop < FAISectList.count(); loop++) {
//       sect = FAISectList.at(loop);
//       if (sect->dist < 500.0) {
//         targetPainter->setPen(QPen(Qt::red, 2));
//       }
//       else {
//         targetPainter->setPen(QPen(Qt::green, 2));
//       }
      //      for (i = 1; i < sect->pos.count(); i++) {
      //        tempP = glMapMatrix->print(*sect->pos.at(i - 1));
      //        targetPainter->drawLine(tempP, glMapMatrix->print(*sect->pos.at(i)));
      //      }
  }
}

int FlightTask::getPlannedPoints()
{
  extern QSettings _settings;

  double pointFAI    = _settings.value("/FlightPoints/FAIPoint", 2.0).toDouble();
  double pointNormal = _settings.value("/FlightPoints/NormalPoint", 1.75).toDouble();
  double pointZielS  = _settings.value("/FlightPoints/ZielSPoint", 1.5).toDouble();

  /*
   * Aufgabe vollst�ndig erf�llt
   *        F: Punkte/km
   *        I: Index des Flugzeuges
   *        f & I noch abfragen !!!!
   */
  double F;
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

  return (int)((double)distance_task * F);
  //  distance_wert = distance_task;

  //  taskPoints = (distance_wert * F * 100) / gliderIndex * dmstMalus +
  //                      (aussenlande * pointCancel * 100) / gliderIndex;
}

double FlightTask::getOlcPoints()
{
  return  olcPoints;
}

void FlightTask::checkWaypoints(QList<FlightPoint*> route, const QString& gliderType)
{
  /*
   *   �berpr�ft, ob die Sektoren der Wendepunkte erreicht wurden
   *
   *   SOLLTE NOCHMALS �BERARBEITET WERDEN
   *
   */
  bool time_error = false;

  if(flightType == FlightTask::NotSet) return;

  if(!wpList.count()) return;

  int gliderIndex = 100, preTime = 0;

  extern QSettings _settings;

  bool showWarnings = _settings.value("/GeneralOptions/ShowWaypointWarnings",true).toBool();

  double pointFAI = _settings.value("/FlightPoints/FAIPoint", 2.0).toDouble();
  double pointNormal = _settings.value("/FlightPoints/NormalPoint", 1.75).toDouble();
  double pointCancel = _settings.value("/FlightPoints/CancelPoint", 1.0).toDouble();
  double pointZielS = _settings.value("/FlightPoints/ZielSPoint", 1.5).toDouble();
  double malusValue = _settings.value("/FlightPoints/MalusValue", 15.0).toDouble();
  double sectorMalus = _settings.value("/FlightPoints/SectorMalus", -0.1).toDouble();

  if(! gliderType.isEmpty() )
    {
      gliderIndex = _settings.value("/GliderTypes/"+gliderType, 100).toInt();
    }

  for(int loop = 0; loop < route.count(); loop++)
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

  for(int loop = 0; loop < wpList.count(); loop++)
    {
      double deltaAngle = 0.0, pointAngle = 0.0;
      dummy = 0;

      __sectorangle(loop, false);
      /*
       * Pr�fung, ob Flugpunkte in den Sektoren liegen.
       *
       *      Ein Index 0 bei den Sektoren zeigt an, dass der Sektor
       *      _nicht_ erreicht wurde. Dies f�hrt an Mitternacht zu
       *      einem m�glichen Fehler ...
       */
      for(int pLoop = startIndex + 1; pLoop < route.count(); pLoop++)
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

              // ... daher ist ein Abbruch m�glich!
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

              if(deltaAngle <= (0.25 * M_PI))
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
                          // ... daher ist ein Abbruch m�glich!
                          startIndex = pLoop;
                          break;
                        }

                    }
                }
              else if(deltaAngle <= (0.5 * M_PI))
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
   * überprüfen der Aufgabe
   */
  int faiCount = 0;
  int dmstCount = 0;
  double dmstMalus = 1.0, aussenlande = 0.0;
  bool home, stop = false;

  if ((wpList.at(1)->sector1 == 0) && showWarnings)
    {
      QMessageBox::information(0, QObject::tr("Not reached first waypoint"),
                               QObject::tr("You have not reached the first waypoint of your task."), QMessageBox::Ok);
      return;
    }

  // Durchgehen der Wendepunkte
  for(int loop = 1; loop < wpList.count() - 2; loop++)
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
              if ((loop == 2) && showWarnings)
                {
                  QMessageBox::information(0, QObject::tr("Not reached first waypoint"),
                                           QObject::tr("You have not reached the first waypoint of your task."), QMessageBox::Ok);
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
      if(showWarnings)
        QMessageBox::warning(0, QObject::tr("Not reached last waypoint"),
                           QObject::tr("You have not reached the last point of your task."), QMessageBox::Ok, 0);

      if(dist(wpList.at(1 + dmstCount), route.last()) < 1.0)
        {
          // Landung auf letztem Wegpunkt
        }
      else
        // Au�enlandung -- Wertung: + 1Punkt bis zur Au�enlandung
        aussenlande = dist(wpList.at(1 + dmstCount), route.last());
    }
  else
    {
      // Zu Hause gelandet
      dmstCount++;
      faiCount++;
      home = true;
    }


  // jetzt in __setDistance noch übernehmen
  distance_wert = 0;
  double F = 1;

  if(dmstCount != wpList.count() - 2)
    {
      if(home)
        {
          distance_wert = dist(wpList.at(wpList.count() - 1),
                               wpList.at(dmstCount));
          for(int loop = 1; loop < 1 + dmstCount; loop++)
            distance_wert = distance_wert + wpList.at(loop)->distance;
        }
      else
        {
          for(int loop = 1; loop <= 1 + dmstCount; loop++)
            distance_wert = distance_wert + wpList.at(loop)->distance;
        }
    }
  else
    {
      /*
       * Aufgabe vollst�ndig erf�llt
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
      distance_wert = distance_task;
    }

  taskPoints = (distance_wert * F * 100) / gliderIndex * dmstMalus +
    (aussenlande * pointCancel * 100) / gliderIndex;

  if(!isOrig)
    {
      /*
       * Optimierter Flug: x% abziehen
       */
      taskPoints -= ( taskPoints * (malusValue / 100.0) );
    }

  if (time_error && showWarnings)
    {
      QMessageBox::warning(0, QObject::tr("Incorrect time interval"),
                         "<qt>" + QObject::tr("The time interval between two points "
                              "of the flight is more than 70 sec.!<BR>"
                              "Due to Code Sportif 3, Nr. 1.9.2.1, "
                              "the flight can not be valued!") + "</qt>", QMessageBox::Ok, 0);
    }

}

QString FlightTask::getTotalDistanceString()
{
  if(flightType == FlightTask::NotSet)  return "--";

  QString distString;
  if (flightType == OLC2003)
    distString.sprintf("%.2f km",distance_wert);
  else
    distString.sprintf("%.2f km", distance_total);

  return distString;
}


QString FlightTask::getTaskDistanceString()
{
  if(flightType == FlightTask::NotSet)  return "--";

  QString distString;

  if (flightType == OLC2003)
    distString.sprintf("%.2f km",distance_wert);
  else
    if (getPlanningType() == Route) {
      distString.sprintf("%.2f km", distance_task);
    }
    else {
      distString = getFAIDistanceString();
    }
  return distString;
}

double FlightTask::getAverageSpeed()
{
  long timeDiff;
  QString averageString;
  if (wpList.count()<2) return 0.0;
  timeDiff = wpList.at(wpList.count()-2)->sectorFAI-wpList.at(1)->sectorFAI;
  return distance_wert/timeDiff*3600.0;
}

QString FlightTask::getPointsString()
{
  if(flightType == FlightTask::NotSet)  return "--";

  QString pointString;

  if (flightType == OLC2003)
    pointString.sprintf("%.2f", olcPoints);
  else{
    int points1 = (int) taskPoints;
    if((int) ( (taskPoints - points1) * 10 ) > 5) points1++;

    pointString.sprintf("%d", points1);
  }

  return pointString;
}

QRect FlightTask::getRect() const  {  return bBoxTask;  }

void FlightTask::setWaypointList(const QList<Waypoint*>& wpL)
{
  wpList = wpL;

  isOrig = false;

  __setWaypointType();
  __checkType();

  for(int loop = 0; loop < wpList.count(); loop++)
    __sectorangle(loop, false);

  if (getPlanningType() == FAIArea) {
    calcFAIArea();
  }
}

void FlightTask::setOptimizedTask(double points, double distance)
{
  distance_wert=distance;
  olcPoints=points;
  flightType=OLC2003;
}

void FlightTask::__setDMSTPoints()
{
  //  double F = 1;
  //
  //  if(dmstCount != wpList.count() - 2)
  //    {
  //      if(home)
  //        {
  //          distance_wert = dist(wpList.at(wpList.count() - 1),
  //              wpList.at(dmstCount));
  //          for(unsigned int loop = 1; loop < 1 + dmstCount; loop++)
  //              distance_wert = distance_wert + wpList.at(loop)->distance;
  //        }
  //      else
  //        {
  //          for(unsigned int loop = 1; loop <= 1 + dmstCount; loop++)
  //              distance_wert = distance_wert + wpList.at(loop)->distance;
  //        }
  //    }
  //  else
  //    {
  //      /*
  //       *  Aufgabe vollst�ndig erf�llt
  //       *        F: Punkte/km
  //       *        I: Index des Flugzeuges
  //       *        f & I noch abfragen !!!!
  //       */
  //      switch(flightType)
  //        {
  //          case FlightTask::ZielS:
  //            F = pointZielS;
  //            break;
  //          case FlightTask::ZielR:
  //          case FlightTask::Dreieck:
  //          case FlightTask::Dreieck_S:
  //            F = pointNormal;
  //            break;
  //          case FlightTask::FAI:
  //          case FlightTask::FAI_S:
  //            F = pointFAI;
  //            break;
  //          default:
  //            F = 0.0;
  //        }
  //       distance_wert = distance_tot;
  //    }
  //
  //  taskPoints = (distance_wert * F * 100) / gliderIndex * dmstMalus +
  //                      (aussenlande * pointCancel * 100) / gliderIndex;
  //
  //  if(!isOrig)
  //    {
  //      /*
  //       * Optimierter Flug: x% abziehen
  //       */
  //      taskPoints -= ( taskPoints * (malusValue / 100.0) );
  //    }
}

//TEST
void FlightTask::printMapElement(QPainter* targetPainter, bool /*isText*/, double dX, double dY)
{
  double w1;
  struct faiAreaSector *sect;
  QPoint tempP;
  QString label;

  // Strecke und Sektoren zeichnen
  if(flightType != FlightTask::NotSet)
    {
      for(int loop = 0; loop < wpList.count(); loop++)
        {
          /*
           * w1 ist die Winkelhalbierende des Sektors!!!
           *      (Angaben in 1/16 Grad)
           */
          w1 = ( ( glMapMatrix->print(wpList.at(loop)->angle) + M_PI ) / M_PI )
            * 180.0 * 16.0 * -1.0;

          tempP = glMapMatrix->print(wpList.at(loop)->origP.lat(), wpList.at(loop)->origP.lon() , dX, dY);
          double qx = -R1 + tempP.x();
          double qy = -R1 + tempP.y();
          double gx = -R2 + tempP.x();
          double gy = -R2 + tempP.y();

          switch(wpList.at(loop)->tpType)
            {
            case FlightTask::RouteP:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
              targetPainter->setBrush(QColor(255, 110, 110));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 1440), 2880);
              targetPainter->setBrush(QColor(110, 255, 110));
              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Inneren Sektor erneut zeichnen, damit Trennlinien
              // zwischen Sekt. 1 und Zylinder verschwinden
              targetPainter->setPen(Qt::NoPen);
              targetPainter->drawEllipse((int)(gx + 2), (int)(gy + 2),
                                         (int)((2 * R2) - 4), (int)((2 * R2) - 4));

              if(loop)
                {
                  if((flightType == FAI_S || flightType == Dreieck_S) && loop == 2)
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2, Qt::DashLine));
                  else
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                  targetPainter->drawLine( glMapMatrix->print(wpList.at(loop - 1)->origP.lat(),wpList.at(loop - 1)->origP.lon() , dX, dY),
                                          glMapMatrix->print(wpList.at(loop)->origP.lat(),wpList.at(loop)->origP.lon() , dX, dY));
                }
              break;
            case FlightTask::Begin:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
              targetPainter->setBrush(QBrush(QColor(255, 0, 0), Qt::BDiagPattern));
              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Linie von Startpunkt zum Aufgaben Beginn
              //
              if(loop)
                {
                  targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
                  targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->origP.lat(),wpList.at(loop - 1)->origP.lon() , dX, dY),
                                          glMapMatrix->print(wpList.at(loop)->origP.lat(),wpList.at(loop)->origP.lon() , dX, dY));
                }
              break;

            case FlightTask::End:
              targetPainter->setPen(QPen(QColor(50, 50, 50), 1));
              targetPainter->setBrush(QBrush(QColor(0, 0, 255), Qt::FDiagPattern));

              targetPainter->drawEllipse((int)gx, (int)gy, (int)(2 * R2), (int)(2 * R2));
              targetPainter->drawPie((int)qx, (int)qy, (int)(2 * R1), (int)(2 * R1), (int)(w1 - 720), 1440);

              // Hier wird die Linie vom letzten Wegpunkt
              // zum Endpunkt gemalt. Die gleiche Linie wird weiter
              // unten erneut gezeichnet !!!
              //                targetPainter->setPen(QPen(QColor(50, 50, 50), 3));
              //                targetPainter->drawLine(
              //                    glMapMatrix->print(wpList.at(loop-1)->projP),
              //                    glMapMatrix->print(wpList.at(loop)->projP));

              // Strecke
              if(flightType == FAI_S || flightType == Dreieck_S)
                targetPainter->setPen(QPen(QColor(0, 0, 0), 2, Qt::DashLine));
              else
                targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

              targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->origP.lat(),wpList.at(loop - 1)->origP.lon() , dX, dY),
                                      glMapMatrix->print(wpList.at(loop)->origP.lat(),wpList.at(loop)->origP.lon() , dX, dY));
              break;

            default:
              // Kann noch Start und Landepunkt sein.

              // Linie von Startpunkt zum Aufgaben Beginn
              if(loop)
                {
                  if(flightType == FAI_S || flightType == Dreieck_S)
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2, Qt::DashLine));
                  else
                    targetPainter->setPen(QPen(QColor(0, 0, 0), 2));

                  targetPainter->drawLine(glMapMatrix->print(wpList.at(loop - 1)->origP.lat(),wpList.at(loop - 1)->origP.lon() , dX, dY),
                                          glMapMatrix->print(wpList.at(loop)->origP.lat(),wpList.at(loop)->origP.lon() , dX, dY));
                }

              // Linie Um Start Lande Punkt
              targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
              targetPainter->setBrush(Qt::NoBrush);
              break;
            }
        }
    }

  // Strecke bei Start auf Schenkel
  if(flightType == FAI_S || flightType == Dreieck_S)
    {
      targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
      targetPainter->drawLine(glMapMatrix->print(wpList.at(2)->origP.lat(), wpList.at(2)->origP.lon(), dX, dY),
                              glMapMatrix->print(wpList.at(wpList.count() - 3)->origP.lat(), wpList.at(wpList.count() - 3)->origP.lon() , dX, dY));
    }

  // Area based planning
  if(getPlanningType() == FAIArea && wpList.count() > 3) {
    for(int loop = 0; loop < FAISectList.count(); loop++) {
      sect = FAISectList.at(loop);
      sect->pos->printMapElement(targetPainter, false);
      label = sect->pos->getName();
      if(label == "FAILow500Sector" || label == "FAIHigh500Sector") {
        label.sprintf("%.0f km", sect->dist);
        QPolygon pp = sect->pos->getProjectedPolygon();
        tempP = glMapMatrix->print(pp[0]);
        targetPainter->setPen(QPen(QColor(0, 0, 0), 2));
        targetPainter->setBrush(QBrush(QColor(0, 255, 128)));
        targetPainter->setBackgroundMode(Qt::OpaqueMode);
        targetPainter->drawText(tempP, label);
        targetPainter->setBackgroundMode(Qt::TransparentMode);
      }
    }
  }
}

void FlightTask::setPlanningType(int type)
{
  __planningType = type;
  __setWaypointType();
  if (getPlanningType() == FAIArea) {
    calcFAIArea();
  }
}

void FlightTask::setPlanningDirection(int dir)
{
  __planningDirection = dir;
  if (getPlanningType() == FAIArea) {
    calcFAIArea();
  }
}

QString FlightTask::getFAIDistanceString()
{
  QString txt;
  double dist;
  struct faiRange range;

  if (wpList.count() < 4) {
    txt = "0.0 km - 0.0 km";
  }
  else {
    dist = wpList.at(2)->distance;
    range = getFAIDistance(dist);
    txt.sprintf("%.2f km - %.2f km",
                range.minLength28 < 500.0 ? range.minLength28 : range.minLength25,
                range.maxLength25 > 500.0 ? range.maxLength25 : range.maxLength28);
  }

  return txt;
}

struct faiRange FlightTask::getFAIDistance(double leg)
{
  struct faiRange r;

  r.minLength28 = qMin(leg + leg / 44.0 * 56.0, 500.0); // maximal 500
  r.maxLength28 = qMin(leg + leg / 28.0 * 72.0, 500.0); // maximal 500
  r.minLength25 = qMax(leg + leg / 45.0 * 55.0, 500.0); // minimal 500
  r.maxLength25 = qMax(4.0 * leg, 500.0);               // minimal 500

  return r;
}

void FlightTask::calcFAIArea()
{
  Waypoint *wp1;
  Waypoint *wp2;
  double minDist;
  double trueCourse;
  double tmpDist;
  QPolygon pointArray;
  struct faiAreaSector *areaSector;
  QVector<bool> sides;
  bool isRightOfRoute;

  if (wpList.count() > 2) {
    wp1 = wpList.at(1);
    wp2 = wpList.at(2);
    double lat1 = int2rad(wp1->origP.lat());
    double lon1 = -int2rad(wp1->origP.lon());
    double lat2 = int2rad(wp2->origP.lat());
    double lon2 = -int2rad(wp2->origP.lon());
    double leg = wp2->distance;

    struct faiRange faiR = getFAIDistance(leg);

    minDist = faiR.minLength28 < 500.0 ? faiR.minLength28 : faiR.minLength25 ;

    trueCourse = tc(lat1, lon1, lat2, lon2);
    qWarning("%03.0f", polar(wp2->origP.lat() - wp1->origP.lat(), wp2->origP.lon() - wp1->origP.lon()) * 180.0 / M_PI);
    FAISectList.clear();

    // determine with sides to calculate
    if (getPlanningDirection() & leftOfRoute) {
      sides.push_back(false);
    }

    if (getPlanningDirection() & rightOfRoute) {
      sides.push_back(true);
    }

    for (int i = 0; i < sides.size(); i++) {
      pointArray.resize(0);
      isRightOfRoute = sides[i];

      if (faiR.minLength28 < faiR.maxLength28) {
        // first calc the surrounding area of FAI < 500 km
        // first sector
        calcFAISector(leg, trueCourse, 28.0, 44.0, 0.02, minDist, lat2, lon2, &pointArray, false, isRightOfRoute);
        // first side upwards
        calcFAISectorSide(leg, trueCourse, minDist, faiR.maxLength28, 1, lat2, lon2, true, &pointArray, true, isRightOfRoute);
        // last sector
        calcFAISector(leg, trueCourse, 28.0, 44.0, 0.02, faiR.maxLength28, lat2, lon2, &pointArray, true, isRightOfRoute);
        // second side downwards
        calcFAISectorSide(leg, trueCourse, faiR.maxLength28, minDist, 1, lat2, lon2, true, &pointArray, false, isRightOfRoute);

        if (!pointArray.isEmpty()) {
          areaSector = new faiAreaSector;
          areaSector->dist = minDist;
          areaSector->pos = new LineElement("FAILow500Area", BaseMapElement::FAIAreaLow500, pointArray, false, true);
          FAISectList.append(areaSector);
        }

        // now calc all sectors for FAI < 500 km
        tmpDist = minDist;
        while (tmpDist < faiR.maxLength28) {
          pointArray.resize(0);
          calcFAISector(leg, trueCourse, 28.0, 44.0, 0.02, tmpDist, lat2, lon2, &pointArray, true, isRightOfRoute);
          if (!pointArray.isEmpty()) {
            areaSector = new faiAreaSector;
            areaSector->dist = tmpDist;
            areaSector->pos = new LineElement("FAILow500Sector", BaseMapElement::FAIAreaLow500, pointArray, false, false);
            FAISectList.append(areaSector);
          }

          tmpDist += (50.0 - fmod(tmpDist, 50.0));
          tmpDist = qMin(tmpDist, faiR.maxLength28);
        }
        // last sector for < 500 km FAI
        if (faiR.minLength28 < faiR.maxLength28) {
          pointArray.resize(0);
          calcFAISector(leg, trueCourse, 28.0, 44.0, 0.02, tmpDist, lat2, lon2, &pointArray, true, isRightOfRoute);
          if (!pointArray.isEmpty()) {
            areaSector = new faiAreaSector;
            areaSector->dist = tmpDist;
            areaSector->pos = new LineElement("FAILow500Sector", BaseMapElement::FAIAreaLow500, pointArray, false, false);
            FAISectList.append(areaSector);
          }
        }
      }
      if (faiR.minLength25 < faiR.maxLength25) {
        pointArray.resize(0);
        // first calc the surrounding area of FAI > 500 km
        // first sector
        calcFAISector(leg, trueCourse, 25.0, 45.0, 0.02, faiR.minLength25, lat2, lon2, &pointArray, false, isRightOfRoute);
        // first side upwards
        calcFAISectorSide(leg, trueCourse, faiR.minLength25, faiR.maxLength25, 1, lat2, lon2, false, &pointArray, true, isRightOfRoute);
        // last sector
        calcFAISector(leg, trueCourse, 25.0, 45.0, 0.02, faiR.maxLength25, lat2, lon2, &pointArray, true, isRightOfRoute);
        // second side downwards
        calcFAISectorSide(leg, trueCourse, faiR.maxLength25, faiR.minLength25, 1, lat2, lon2, false, &pointArray, false, isRightOfRoute);

        if (!pointArray.isEmpty()) {
          areaSector = new faiAreaSector;
          areaSector->dist = faiR.minLength25;
          areaSector->pos = new LineElement("FAIHigh500Area", BaseMapElement::FAIAreaHigh500, pointArray, false, true);
          FAISectList.append(areaSector);
        }

        tmpDist = faiR.minLength25;
        while (tmpDist < faiR.maxLength25) {
          pointArray.resize(0);
          calcFAISector(leg, trueCourse, 25.0, 45.0, 0.02, tmpDist, lat2, lon2, &pointArray, true, isRightOfRoute);
          if (!pointArray.isEmpty()) {
            areaSector = new faiAreaSector;
            areaSector->dist = tmpDist;
            areaSector->pos = new LineElement("FAIHigh500Sector", BaseMapElement::FAIAreaHigh500, pointArray, false, false);
            FAISectList.append(areaSector);
          }
          tmpDist += (50.0 - fmod(tmpDist, 50.0));
          tmpDist = qMin(tmpDist, faiR.maxLength25);
        }
        // last sector for > 500 km FAI
        if (faiR.minLength25 < faiR.maxLength25) {
          pointArray.resize(0);
          calcFAISector(leg, trueCourse, 25.0, 45.0, 0.02, tmpDist, lat2, lon2, &pointArray, true, isRightOfRoute);
          if (!pointArray.isEmpty()) {
            areaSector = new faiAreaSector;
            areaSector->dist = tmpDist;
            areaSector->pos = new LineElement("FAIHigh500Sector", BaseMapElement::FAIAreaHigh500, pointArray, false, false);
            FAISectList.append(areaSector);
          }
        }
      }
    }
  }
}

void FlightTask::calcFAISector(double leg, double legBearing, double from, double to, double step, double dist, double toLat,
                               double toLon, QPolygon *pp, bool upwards, bool isRightOfRoute)
{
  extern MapMatrix *_globalMapMatrix;

  double percent, maxDist, minDist;
  double b, c;
  double w;
  unsigned int i;
  WGSPoint p;

  minDist = dist * from / 100.0;
  maxDist = dist * to / 100.0;

  i = pp->size();
  if (upwards) {
    percent = from;
  }
  else {
    percent = to;
  }
  while (!((upwards && percent > to) || (!upwards && percent < from))) {
    b = percent * dist / 100.0;
    c = dist - leg - b;

    if (c >= minDist && c <= maxDist) {
      w = angle(leg, b, c);
      p = _globalMapMatrix->wgsToMap(posOfDistAndBearing(toLat, toLon, isRightOfRoute ? legBearing - w : legBearing + w, b));
      pp->putPoints(i++, 1, p.lat(), p.lon());
    }

    if (upwards) {
      percent += step;
    }
    else {
      percent -= step;
    }
  }
}

void FlightTask::calcFAISectorSide(double leg, double legBearing, double from, double to, double step, double toLat,
                                   double toLon, bool less500, QPolygon *pp, bool upwards, bool isRightOfRoute)
{
  extern MapMatrix *_globalMapMatrix;

  double dist = from;
  double b, c;
  double w;
  double minPercent, maxPercent;
  unsigned int i;
  WGSPoint p;

  i = pp->size();

  if (less500) {
    minPercent = 0.28;
    maxPercent = 0.44;
  }
  else {
    minPercent = 0.25;
    maxPercent = 0.45;
  }

  dist = from;
  while (!((upwards && (dist > to)) || (!upwards && (dist < to)))) {
    if (less500) {
      b = dist * minPercent >= leg ? (dist * 0.56) - leg : dist * minPercent;
    }
    else {
      b = dist * 0.3 >= leg ? (dist * 0.55) - leg : dist * minPercent;
    }
    c = dist - leg - b;

    if (c >= dist * minPercent && c <= dist * maxPercent) {
      if (upwards) {
        w = angle(leg, b, c);
      }
      else {
        w = angle(leg, c, b);
      }
      p = _globalMapMatrix->wgsToMap(posOfDistAndBearing(toLat, toLon, isRightOfRoute ? legBearing - w : legBearing + w,
                                                        upwards ? b : c));
      pp->putPoints(i++, 1, p.lat(), p.lon());
    }

    if (upwards) {
      dist += step;
    }
    else {
      dist -= step;
    }
  }
}
/** set new task name */
void FlightTask::setTaskName(const QString& fName)
{
  sourceFileName = fName;
}

/** No descriptions */
QString FlightTask::getPlanningTypeString()
{
  return ttItem2Text( __planningType );
}

/** re-projects the points along the route to make sure the route is drawn correctly if the projection changes. */
void FlightTask::reProject(){
  extern MapMatrix *_globalMapMatrix;

  FlightPoint *fp;
  foreach(fp, flightRoute)
      fp->projP = _globalMapMatrix->wgsToMap(fp->origP);

  Waypoint *wp;
  foreach(wp, wpList)
      wp->projP = _globalMapMatrix->wgsToMap(wp->origP);
}

/**
 * Get translation string for task type.
 */
QString FlightTask::ttItem2Text( const int item, QString defaultValue )
{
  if( taskTypeTranslations.isEmpty() )
    {
      loadTaskTypeTranslations();
    }

  return taskTypeTranslations.value( item, defaultValue );
}

/**
 * Get task type for translation string.
 */
int FlightTask::ttText2Item( const QString& text )
{
  if( taskTypeTranslations.isEmpty() )
    {
      // Load object - translation data
      loadTaskTypeTranslations();
    }

  return taskTypeTranslations.key( text );
}

void FlightTask::loadTaskTypeTranslations()
{
  // Load translation data to hash dictionary
  taskTypeTranslations.insert( FlightTask::Route, QObject::tr("Traditional Route") );
  taskTypeTranslations.insert( FlightTask::FAIArea, QObject::tr("FAI Area") );
  // taskTypeTranslations.insert( FlightTask::AAT, QObject::tr("Area Assigned") );

  // load sorted translation strings
  sortedTaskTypeTranslations = QStringList( taskTypeTranslations.values() );
  sortedTaskTypeTranslations.sort();
}

/**
 * Get sorted translations
 */
QStringList& FlightTask::ttGetSortedTranslationList()
{
  if( taskTypeTranslations.isEmpty() )
    {
      // Load translation data
      loadTaskTypeTranslations();
    }

  return sortedTaskTypeTranslations;
}
