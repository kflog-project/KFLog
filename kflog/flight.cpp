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

#include <cmath>
#include <stdlib.h>

#include "flight.h"

#include <mapcalc.h>
#include <mapmatrix.h>
#include <optimizationwizard.h>

//#include <kapp.h>
//#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktextbrowser.h>
#include <qpixmap.h>
#include <qptrlist.h>

/* Anzahl der zu merkenden besten Flüge */
#define NUM_TASKS 3
#define NUM_TASKS_POINTS ( NUM_TASKS * 3 )
#define MAX_TASK_ID NUM_TASKS - 1

/* Die Einstellungen können mal in die Voreinstellungsdatei wandern ... */
#define FAI_POINT 2.0
#define NORMAL_POINT 1.75

#define GET_SPEED(a) ( (float)a->dS / (float)a->dT )
#define GET_VARIO(a) ( (float)a->dH / (float)a->dT )

/* Maximale Vergrößerung beim Prüfen! */
#define SCALE 10.0

#define APPEND_WAYPOINT(a, b, c) \
      wpL.append(new Waypoint); \
      wpL.current()->origP = route.at( a )->origP; \
      wpL.current()->projP = route.at( a )->projP; \
      wpL.current()->distance = ( b ); \
      wpL.current()->name = c; \
      wpL.current()->sector1 = 0; \
      wpL.current()->sector2 = 0; \
      wpL.current()->sectorFAI = 0; \
      wpL.current()->angle = -100;

Flight::Flight(QString fName, QString recID, QList<flightPoint> r, QString pName,
    QString gType, QString gID, int cClass, QList<Waypoint> wpL, QDate d)
  : BaseFlightElement("flight", BaseMapElement::Flight, fName),
    recorderID(recID),
    pilotName(pName),
    gliderType(gType),
    gliderID(gID),
    date(d),
    competitionClass(cClass),
    v_max(0),
    h_max(0),
    va_min(0),
    va_max(0),
    route(r),
    landTime(route.last()->time),
    startTime(route.at(0)->time),
    origTask(FlightTask(wpL, true, QString::null)),
    optimizedTask(FlightTask(QString::null)),
    optimized(false),
    nAnimationIndex(0),
    bAnimationActive(false)
{
  origTask.checkWaypoints(route, gliderType);

  __checkMaxMin();
  __flightState();

  // size and clear animation pixbuffer
  pixAnimate.resize(32,32);
  pixAnimate.fill(QPixmap::white);

  // Die Liste könnte doch eigentlich permanent gespeichert werden ...
  header.append(pilotName);
  header.append(gliderID);
  header.append(gliderType);
  header.append(KGlobal::locale()->formatDate(date, true));
  header.append(printTime(route.last()->time - route.at(0)->time));
  header.append(getTaskTypeString());
  header.append(getDistance());
  header.append(getPoints());
  header.append(recorderID);
  taskTimesSet=false;
}

Flight::~Flight()
{

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
  stop[2] = MIN(start[2] + ( 2 * step ), route.count() - 1);
}

double Flight::__calculateOptimizePoints(flightPoint* fp1,
    flightPoint* fp2, flightPoint* fp3)
{
  double dist1 = dist(fp1, fp2);
  double dist2 = dist(fp2, fp3);
  double dist3 = dist(fp1, fp3);
  double tDist = dist1 + dist2 + dist3;

  if(FlightTask::isFAI(tDist, dist1, dist2, dist3)) return tDist * FAI_POINT;

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

      // Bedingungen für Kreisflug müssen noch geprüft werden !!!
      // Bei kleinen Zeitabständen extra Abfrage
      // Noch mehr Zeitabschnitte einfügen ab ca 25° - 35° Kursänderung
      if(( fabs(route.at(n)->bearing) > route.at(n)->dT * PI / 20.0))
        {
          weiter = 0;
          // Drehrichtung
          if(route.at(n)->bearing > 0)
              dreh++;
          else
              dreh--;

          // Kreisflug eingeleitet
          if(s_point < 0)
              s_point = n;
        }
      else if(s_point > -1 &&
              route.at(n)->time - route.at(n - weiter)->time  >= 20 &&
              route.at(n - weiter)->time - route.at(s_point)->time > 20)
        {
          // Zeit eines Kreisfluges mindestens 20s
          // Zeit zwischen zwei Kreisflügen höchstens 20s

          // Endpunkt des Kreisfluges

          e_point = n - weiter - 1;

          // Punkte zwischen s_point und e_point setzen
          for(int n = s_point; n <=  e_point; n++)
            {
              if((e_point - s_point) * 0.8 <= dreh)
                  route.at(n)->f_state = Flight::RightTurn;
              else if((e_point - s_point) * 0.8 >= - dreh)
                  route.at(n)->f_state = Flight::LeftTurn;
              else
                  // vermischt
                  route.at(n)->f_state = Flight::MixedTurn;
            }
          s_point = - 1;
          e_point = - 1;
          dreh = 0;
          weiter = 0;
        }
      else
        {
          if( (route.at(n)->time - route.at(n - weiter)->time)  >= 20)
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
  flightPoint *pointA, *pointB, *pointC;

  for(unsigned int loopA = start[0]; loopA <= MIN(stop[0], route.count() - 1); loopA += step)
    {
      pointA = route.at(loopA);

      if(isTotal) start[1] = loopA + step;

      for(unsigned int loopB = start[1]; loopB <= MIN(stop[1], route.count() - 1); loopB += step)
        {
          pointB = route.at(loopB);

          if(isTotal) start[2] = loopB + step;

          for(unsigned int loopC = start[2]; loopC <= MIN(stop[2], route.count() - 1); loopC += step)
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

bool Flight::__isVisible() const
{
  return true;
//  return glMapMatrix->isVisible(bBox);
}

void Flight::printMapElement(QPainter* targetPainter, bool isText)
{
  warning("print Flight");

  flightPoint* pointA;
  flightPoint* pointB;
  flightPoint* pointC;
  QPoint curPointA, curPointB;

  if(optimized)
      optimizedTask.printMapElement(targetPainter, isText);
  else
      origTask.printMapElement(targetPainter, isText);

  // Flugweg

  unsigned int delta = 1;
  if(!glMapMatrix->isSwitchScale())  delta = 8;

  curPointA = glMapMatrix->print(route.at(0)->projP);
  bBoxFlight.setLeft(curPointA.x());
  bBoxFlight.setTop(curPointA.y());
  bBoxFlight.setRight(curPointA.x());
  bBoxFlight.setBottom(curPointA.y());

  for(unsigned int n = delta; n < route.count(); n = n + delta)
    {
      pointA = route.at(n - delta);
      pointB = route.at(n);
      if(n + delta < route.count())
          pointC = route.at(n + delta);
      else
          pointC = route.last();

      curPointB = glMapMatrix->print(pointB->projP);

      bBoxFlight.setLeft(MIN(curPointB.x(), bBoxFlight.left()));
      bBoxFlight.setTop(MAX(curPointB.y(), bBoxFlight.top()));
      bBoxFlight.setRight(MAX(curPointB.x(), bBoxFlight.right()));
      bBoxFlight.setBottom(MIN(curPointB.y(), bBoxFlight.bottom()));

      QPen drawP = glConfig->getDrawPen(pointB);
      drawP.setCapStyle(Qt::SquareCap);
      targetPainter->setPen(drawP);

      targetPainter->drawLine(curPointA, curPointB);

      curPointA = curPointB;
    }
}

void Flight::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  if(!__isVisible()) return;

  flightPoint* pointA;
  flightPoint* pointB;
  flightPoint* pointC;
  QPoint curPointA, curPointB;
  unsigned int nStop;

  if(optimized)
      optimizedTask.drawMapElement(targetPainter, maskPainter);
  else
      origTask.drawMapElement(targetPainter, maskPainter);

  // Flugweg

  unsigned int delta = 1;
  if(!glMapMatrix->isSwitchScale())  delta = 8;

  curPointA = glMapMatrix->map(route.at(0)->projP);
  bBoxFlight.setLeft(curPointA.x());
  bBoxFlight.setTop(curPointA.y());
  bBoxFlight.setRight(curPointA.x());
  bBoxFlight.setBottom(curPointA.y());

  if (!bAnimationActive)
    nStop = route.count()-1;
  else
    nStop = nAnimationIndex;

  for(unsigned int n = delta; n < nStop; n = n + delta)
    {
      pointA = route.at(n - delta);
      pointB = route.at(n);
      if(n + delta < nStop)
          pointC = route.at(n + delta);
      else
          pointC = route.at(nStop);

      curPointB = glMapMatrix->map(pointB->projP);

      bBoxFlight.setLeft(MIN(curPointB.x(), bBoxFlight.left()));
      bBoxFlight.setTop(MAX(curPointB.y(), bBoxFlight.top()));
      bBoxFlight.setRight(MAX(curPointB.x(), bBoxFlight.right()));
      bBoxFlight.setBottom(MIN(curPointB.y(), bBoxFlight.bottom()));

      QPen drawP = glConfig->getDrawPen(pointB);
      drawP.setCapStyle(Qt::SquareCap);
//      QColor HeightC(QColor("darkblue").light(100 + (int)(pointB->height/20)));
//      drawP.setColor(HeightC);
      targetPainter->setPen(drawP);

      maskPainter->setPen(QPen(Qt::color1, drawP.width(),
          Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));

      targetPainter->drawLine(curPointA, curPointB);
      maskPainter->drawLine(curPointA, curPointB);

      curPointA = curPointB;
    }
}


QString Flight::getID() const { return gliderID; }

QString Flight::getTaskTypeString(bool isOrig) const
{
  if(isOrig || !optimized)
      return origTask.getTaskTypeString();
  else
      return optimizedTask.getTaskTypeString();
}

flightPoint Flight::getPointByTime(int time)
{
  return getPoint(getPointIndexByTime(time));
}

int Flight::getPointIndexByTime(int time)
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
          return l - n;

      diff = getPoint(l).time - time;
      diff = ABS(diff);
    }

  return ep;
}

QList<flightPoint> Flight::getRoute() const{
  return route;
}

flightPoint Flight::getPoint(int n)
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
  float k_height_pos_l = 0, k_height_pos_r = 0, k_height_pos_v = 0;
  float k_height_neg_l = 0, k_height_neg_r = 0, k_height_neg_v = 0;
  int kurbel_l = 0, kurbel_r = 0, kurbel_v = 0;
  float distance = 0;
  float s_height_pos = 0, s_height_neg = 0;

  //  noch abchecken, dass end <= fluglänge
  end = MIN(route.count() - 1, end);

  for(unsigned int n = start; n < end; n++)
    {
      switch(route.at(n)->f_state)
        {
          case Flight::RightTurn:
            if(route.at(n)->dH > 0)
                k_height_pos_r += (float)route.at(n)->dH;
            else
                k_height_neg_r += (float)route.at(n)->dH;

            kurbel_r += route.at(n)->dT;
            break;
          case Flight::LeftTurn:
            if(route.at(n)->dH > 0)
                k_height_pos_l += (float)route.at(n)->dH;
            else
                k_height_neg_l += (float)route.at(n)->dH;

            kurbel_l += route.at(n)->dT;
            break;
          case Flight::MixedTurn:
            if(route.at(n)->dH > 0)
                k_height_pos_v += (float)route.at(n)->dH;
            else
                k_height_neg_v += (float)route.at(n)->dH;

            kurbel_v += route.at(n)->dT;
            break;
          default:
           // immer oder bloß auf Strecke ??
           distance += (float)route.at(n)->dS;

           if(route.at(n)->dH > 0)
                s_height_pos += (float)route.at(n)->dH;
            else
                s_height_neg += (float)route.at(n)->dH;
         }
    }

    QStrList result;
    QString text;

    // Kreisflug
    text.sprintf("%s (%.1f %%)", (const char*)printTime(kurbel_r),
        (float)kurbel_r /
            (float)( route.at(end)->time - route.at(start)->time ) * 100.0);
    result.append(text);
    text.sprintf("%s (%.1f %%)", (const char*)printTime(kurbel_l),
        (float)kurbel_l /
            (float)( route.at(end)->time - route.at(start)->time) * 100.0);
    result.append(text);
    text.sprintf("%s (%.1f %%)", (const char*)printTime(kurbel_v),
        (float)kurbel_v /
            (float)(route.at(end)->time - route.at(start)->time ) * 100.0);
    result.append(text);
    text.sprintf("%s (%.1f %%)",
        (const char*)printTime((kurbel_r + kurbel_l + kurbel_v)),
        (float)(kurbel_r + kurbel_l + kurbel_v) /
            (float)( route.at(end)->time - route.at(start)->time ) * 100.0);
    result.append(text);

    text.sprintf("%.2f m/s",(k_height_pos_r + k_height_neg_r) /
             (route.at(end)->time - route.at(start)->time));
    result.append(text);
    text.sprintf("%.2f m/s",(k_height_pos_l + k_height_neg_l) /
             (route.at(end)->time - route.at(start)->time));
    result.append(text);
    text.sprintf("%.2f m/s",(k_height_pos_v + k_height_neg_v) /
             (route.at(end)->time - route.at(start)->time));
    result.append(text);
    text.sprintf("%.2f m/s",
             (k_height_pos_r + k_height_pos_l + k_height_pos_v +
              k_height_neg_r + k_height_neg_l + k_height_neg_v) /
             (route.at(end)->time - route.at(start)->time));
    result.append(text);

    text.sprintf("%.0f m",k_height_pos_r);
    result.append(text);
    text.sprintf("%.0f m",k_height_pos_l);
    result.append(text);
    text.sprintf("%.0f m",k_height_pos_v);
    result.append(text);
    text.sprintf("%.0f m",k_height_pos_r + k_height_pos_l + k_height_pos_v);
    result.append(text);
    text.sprintf("%.0f m",k_height_neg_r);
    result.append(text);
    text.sprintf("%.0f m",k_height_neg_l);
    result.append(text);
    text.sprintf("%.0f m",k_height_neg_v);
    result.append(text);
    text.sprintf("%.0f m",k_height_neg_r + k_height_neg_l + k_height_neg_v);
    result.append(text);

    // Strecke
    text.sprintf("%.0f",distance / (s_height_pos + s_height_pos));
    result.append(text);
    text.sprintf("%.1f km/h",distance /
          ((float)(route.at(end)->time - route.at(start)->time -
          (kurbel_r + kurbel_l + kurbel_v))) * 3.6);
    result.append(text);
    text.sprintf("%.0f m",s_height_pos);
    result.append(text);
    text.sprintf("%.0f m",s_height_neg);
    result.append(text);
    text.sprintf("%.0f km",distance / 1000);
    result.append(text);
    text.sprintf("%s (%.1f %%)",
        (const char*)printTime( ( route.at(end)->time - route.at(start)->time -
            ( kurbel_r + kurbel_l + kurbel_v ) ) ),
        (float)( route.at(end)->time - route.at(start)->time -
                ( kurbel_r + kurbel_l + kurbel_v ) ) /
            (float)( route.at(end)->time - route.at(start)->time ) * 100.0 );
    result.append(text);
    text.sprintf("%s",
        (const char*)printTime(route.at(end)->time - route.at(start)->time));
    result.append(text);
    text.sprintf("%.0f m",s_height_pos   + k_height_pos_r
                        + k_height_pos_l + k_height_pos_v);
    result.append(text);
    text.sprintf("%.0f m",s_height_neg   + k_height_neg_r
                        + k_height_neg_l + k_height_neg_v);
    result.append(text);

    // Rückgabe:
    // kurbelanteil r - l - v - g
    // mittleres Steigen r - l - v - g
    // Höhengewinn r - l - v - g
    // Höhenverlust r - l - v - g
    // Gleitzahl - mittlere Geschw. - Höhengewinn - höhenverlust
    // Distanz - StreckenZeit - Gesamtzeit - Ges. Höhengewinn - Ges Höhenverlust

    return result;
}

QString Flight::getDistance(bool isOrig)
{
  if(isOrig || !optimized)
      return origTask.getTotalDistanceString();
  else
      return optimizedTask.getTotalDistanceString();
}

FlightTask Flight::getTask(bool isOrig)
{
  if(isOrig || !optimized)
      return origTask;
  else
      return optimizedTask;
}

QString Flight::getTaskDistance(bool isOrig)
{
  if(isOrig || !optimized)
      return origTask.getTaskDistanceString();
  else
      return optimizedTask.getTaskDistanceString();
}

QString Flight::getPoints(bool isOrig)
{
  if(isOrig || !optimized)
      return origTask.getPointsString();
  else
      return optimizedTask.getPointsString();
}

int Flight::getCompetitionClass() const  { return competitionClass; }

int Flight::getLandTime() const { return landTime; }

QString Flight::getPilot() const { return pilotName; }

int Flight::getStartTime() const { return startTime; }

QString Flight::getType() const { return gliderType; }

QDate Flight::getDate() const { return date; }

unsigned int Flight::getRouteLength() const { return route.count(); }

bool Flight::isOptimized() const { return optimized; }

int Flight::searchPoint(QPoint cPoint, flightPoint& searchPoint)
{
  unsigned int delta = 1;
  int index = -1;

  double minDist = 1000.0, distance = 0.0;

  if(!glMapMatrix->isSwitchScale())  delta = 8;

  QPoint fPoint;

  for(unsigned int loop = 0; loop < route.count(); loop = loop + delta)
    {
      fPoint = glMapMatrix->map(route.at(loop)->projP);
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
              searchPoint = *route.at(index);
            }
        }
    }
  return index;
}

QRect Flight::getFlightRect() const { return bBoxFlight; }

QRect Flight::getTaskRect() const
{
  if(optimized)
      return optimizedTask.getRect();
  else
      return origTask.getRect();
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

QList<Waypoint> Flight::getWPList()
{
  if(!optimized)
    return origTask.getWPList();
  else
    return optimizedTask.getWPList();
}

QList<Waypoint> Flight::getOriginalWPList()  {  return origTask.getWPList();  }

bool Flight::optimizeTaskOLC(Map* map)
{
  OptimizationWizard* wizard = new OptimizationWizard(0,i18n("Optimization for OLC"));
  wizard->setMapContents(map);
  int wizard_ret = wizard->exec();
  if (wizard_ret==QDialog::Rejected) {
    delete wizard;
    return false;
  }

  unsigned int idList[7];
  double points;
  double distance = wizard->optimizationResult(idList,&points);

  if (distance<0.0) // optimization was canceled
    return false;

//  QString text, distText, rawPointText;
//  rawPointText.sprintf(" %.2f", points);
//  distText.sprintf(" %.2f km  ", distance);
//  text = i18n("The task has been optimized for the OLC.\nThe best task found is:\n\n");
//  text = text + "\t1:  "
//      + printPos(route.at(idList[0])->origP.lat()) + " / "
//      + printPos(route.at(idList[0])->origP.lon(), false)
//      + QString("\n\t2:  ")
//      + printPos(route.at(idList[1])->origP.lat()) + " / "
//      + printPos(route.at(idList[1])->origP.lon(), false)
//      + QString(" (%1km)\n\t3:  ").arg(dist(route.at(idList[0]),route.at(idList[1])),0,'f',2)
//      + printPos(route.at(idList[2])->origP.lat()) + " / "
//      + printPos(route.at(idList[2])->origP.lon(), false)
//      + QString(" (%1km)\n\t4:  ").arg(dist(route.at(idList[1]),route.at(idList[2])),0,'f',2)
//      + printPos(route.at(idList[3])->origP.lat()) + " / "
//      + printPos(route.at(idList[3])->origP.lon(), false)
//      + QString(" (%1km)\n\t5:  ").arg(dist(route.at(idList[2]),route.at(idList[3])),0,'f',2)
//      + printPos(route.at(idList[4])->origP.lat()) + " / "
//      + printPos(route.at(idList[4])->origP.lon(), false)
//      + QString(" (%1km)\n\t6:  ").arg(dist(route.at(idList[3]),route.at(idList[4])),0,'f',2)
//      + printPos(route.at(idList[5])->origP.lat()) + " / "
//      + printPos(route.at(idList[5])->origP.lon(), false)
//      + QString(" (%1km)\n\t7:  ").arg(dist(route.at(idList[4]),route.at(idList[5])),0,'f',2)
//      + printPos(route.at(idList[6])->origP.lat()) + " / "
//      + printPos(route.at(idList[6])->origP.lon(), false)
//      + QString(" (%1km):  ").arg(dist(route.at(idList[5]),route.at(idList[6])),0,'f',2)
//      + i18n("\n\nDistance:\t") + distText + "\n" + i18n("Points(raw):\t") + rawPointText + "\n\n"
//      + i18n("Do You want to use this task and replace the old?");
//
//  if(KMessageBox::questionYesNo(0, text, i18n("Optimizing")) ==
//        KMessageBox::Yes)
//    {
      QList<Waypoint> wpL;

      APPEND_WAYPOINT(0, 0, i18n("Take-Off"))
      APPEND_WAYPOINT(idList[0], dist(route.at(idList[0]), route.at(0)),
          i18n("Begin of Task"))
      APPEND_WAYPOINT(idList[1], dist(route.at(idList[1]),
          route.at(idList[0])), i18n("Optimize 1"))
      APPEND_WAYPOINT(idList[2], dist(route.at(idList[2]),
          route.at(idList[1])), i18n("Optimize 2"))
      APPEND_WAYPOINT(idList[3], dist(route.at(idList[3]),
          route.at(idList[2])), i18n("Optimize 3"))
      APPEND_WAYPOINT(idList[4], dist(route.at(idList[4]),
          route.at(idList[3])), i18n("Optimize 4"))
      APPEND_WAYPOINT(idList[5], dist(route.at(idList[5]),
          route.at(idList[4])), i18n("Optimize 5"))
      APPEND_WAYPOINT(idList[6], dist(route.at(idList[6]),
          route.at(idList[5])), i18n("End of Task"))
      APPEND_WAYPOINT(0, 0, i18n("Landing"))

      optimizedTask.setWaypointList(wpL);
      optimizedTask.checkWaypoints(route, gliderType);
      optimizedTask.setOptimizedTask(points,distance);
      optimized = true;

      delete wizard;
      return true;
//    }
//
//  delete wizard;
//  return false;
}


bool Flight::optimizeTask()
{
  if( route.count() < 10)  return false;

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

  start[0] = 0;
  start[1] = step - 1;
  start[2] = (2 * step) - 1;
  stop[0] = route.count() - ( 2 * step );
  stop[1] = route.count() - step;
  stop[2] = route.count();

  for(unsigned int loop = 0; loop < NUM_TASKS; loop++)
      taskValue[loop] = 0;

  // steps muss noch besser berechnet werden!!!
  step = step * 3;

  numSteps = __calculateBestTask(start, stop, step, idList, taskValue, true);
  totalSteps = numSteps;

  // Sichern der ID's der Favoriten
  for(unsigned int loop = 0; loop < NUM_TASKS_POINTS; loop++)
      idTempList[loop] = idList[loop];

  unsigned int stepB = MAX(step / 6, 4);

  for(unsigned int loop = 0; loop < NUM_TASKS; loop++)
    {
      __setOptimizeRange(start, stop, idTempList, loop * 3, step);

      numSteps = __calculateBestTask(start, stop, stepB, idList, taskValue, false);
      secondSteps += numSteps;
    }

  step = stepB;

  secondSteps = 0;
  // Sichern der ID's der Favoriten
  for(unsigned int loop = 0; loop < NUM_TASKS_POINTS; loop++)
      idTempList[loop] = idList[loop];

  for(unsigned int loop = 0; loop < NUM_TASKS / 2; loop++)
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
  if(FlightTask::isFAI(totalDist, dist1, dist2, dist3))
      pointText.sprintf(" %d (FAI)", (int)(totalDist * 2.0 * 0.85));
  else
      pointText.sprintf(" %d", (int)(totalDist * 1.75 * 0.85));

  distText.sprintf(" %.2f km  ", totalDist);
  text = i18n("The task has been optimized. The best task found is:\n\n");
  text = text + "\t1:  "
      + printPos(route.at(idList[0])->origP.lat()) + " / "
      + printPos(route.at(idList[0])->origP.lon(), false) + "\n\t2:  "
      + printPos(route.at(idList[1])->origP.lat()) + " / "
      + printPos(route.at(idList[1])->origP.lon(), false) + "\n\t3:  "
      + printPos(route.at(idList[2])->origP.lat()) + " / "
      + printPos(route.at(idList[2])->origP.lon(), false) + "\n\n\t"
      + i18n("Distance:") + distText + i18n("Points:") + pointText + "\n\n"
      + i18n("Do You want to use this task and replace the old?");

  if(KMessageBox::questionYesNo(0, text, i18n("Optimizing")) ==
        KMessageBox::Yes)
    {
      QList<Waypoint> wpL;

      APPEND_WAYPOINT(0, 0, i18n("Take-Off"))
      APPEND_WAYPOINT(0, 0, i18n("Begin of Task"))
      APPEND_WAYPOINT(idList[0], dist(route.at(idList[0]), route.at(0)),
          i18n("Optimize 1"))
      APPEND_WAYPOINT(idList[1], dist(route.at(idList[1]),
          route.at(idList[0])), i18n("Optimize 2"))
      APPEND_WAYPOINT(idList[2], dist(route.at(idList[2]),
          route.at(idList[1])), i18n("Optimize 3"))
      APPEND_WAYPOINT(0, dist(route.at(0), route.at(idList[1])),
          i18n("End of Task"))
      APPEND_WAYPOINT(0, 0, i18n("Landing"))

      optimizedTask.setWaypointList(wpL);
      optimizedTask.checkWaypoints(route, gliderType);
      optimized = true;

      return true;
    }

  return false;
}

/** Get the next FlightPoint after number 'index' */
int Flight::searchGetNextPoint(int index, flightPoint& searchPoint)
{
  // only move to next if not at last point
  if ((index < (int)route.count() - 1) && (index >= 0))
                  index += 1;

        // now update searchPoint struct
  searchPoint = *route.at(index);
  return index;
}

/** Get the previous FlightPoint before number 'index' */
int Flight::searchGetPrevPoint(int index, flightPoint& searchPoint)
{
        // only move to next is not first point
  if ((index > 1) && (index <= (int)route.count() - 1))
                  index -= 1;
         // now update searchPoint struct
  searchPoint = *route.at(index);
  return index;
}

/**
 * Get the contents of the previous FlightPoint 'step' indexes before number 'index'
 */
int Flight::searchStepNextPoint(int index, flightPoint & fP, int step)
{
  if (index + step < (int)getRouteLength() - 1)
      index += step;
  else
            index = getRouteLength() - 1;

  return searchGetNextPoint(index, fP);
}

/**
 * Get the contents of the previous FlightPoint 'step' indexes before number 'index'
 */
int Flight::searchStepPrevPoint(int index,  flightPoint & fP, int step)
{
  if (index - step > 0)
      index -= step;
  else
            index = 1;

  return searchGetPrevPoint(index, fP);
}

QStrList Flight::getHeader()  {  return header;  }

/** Sets the nAnimationIndex member to 'n' */
void Flight::setAnimationIndex(int n)
{
  if ((n >= 0) && (getRouteLength() > (unsigned int)n))
            nAnimationIndex = n;
}

/** Sets task begin and end time */
void Flight::setTaskByTimes(int timeBegin,int timeEnd)
{
  taskBegin = getPointIndexByTime(timeBegin);
  taskEnd = getPointIndexByTime(timeEnd);
  taskTimesSet=true;
}

/** Increments the nAnimationIndex member */
void Flight::setAnimationNextIndex(void)
{
        if (getRouteLength() > (unsigned int)nAnimationIndex+1)
            nAnimationIndex++;
  else
    {
                  nAnimationIndex = (int)getRouteLength()-1;
      bAnimationActive = false;        //stop the animation of this flight
    }
}

/** sets the bAnimationActive flag */
void Flight::setAnimationActive(bool b)  {  bAnimationActive = b;  }

/** returns the bAnimationActive flag */
bool Flight::isAnimationActive(void)  {  return bAnimationActive;  }

/** No descriptions */
int Flight::getAnimationIndex()  {  return nAnimationIndex;  }

/** No descriptions */
void Flight::setLastAnimationPos(QPoint pos)  {  preAnimationPos = pos;  }

/** No descriptions */
QPoint Flight::getLastAnimationPos(void)  {  return preAnimationPos;  }

/** No descriptions */
QPixmap Flight::getLastAnimationPixmap(void)  {  return pixAnimate;  }

/** No descriptions */
void Flight::setLastAnimationPixmap(QPixmap pix)  {  pixAnimate = pix;  }
