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
**                :  2008 Improvements by Constantijn Neeteson
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

#include "mapcalc.h"
#include "mapmatrix.h"
#include "optimizationwizard.h"

//#include <kapp.h>
//#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktextbrowser.h>
#include <qpixmap.h>
#include <qptrlist.h>

/* Number of tasks to be handled during optimization */
#define NUM_TASKS 25
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
      wpL.current()->angle = -100; \
      wpL.current()->fixTime = 0;

#define APPEND_WAYPOINT_OLC2003(a, b, c) \
      wpL.append(new Waypoint); \
      wpL.current()->origP = route.at( a )->origP; \
      wpL.current()->projP = route.at( a )->projP; \
      wpL.current()->distance = ( b ); \
      wpL.current()->name = c; \
      wpL.current()->sector1 = 0; \
      wpL.current()->sector2 = 0; \
      wpL.current()->sectorFAI = 0; \
      wpL.current()->angle = -100; \
      wpL.current()->fixTime = route.at( a )->time;

Flight::Flight(const QString& fName, const QString& recID, const QPtrList<flightPoint>& r, const QString& pName,
   const QString& gType, const QString& gID, int cClass, const QPtrList<Waypoint>& wpL, const QDate& d)
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
    startTime(route.at(0)->time),
    landTime(route.last()->time),
    startIndex(0),
    landIndex(route.count()-1),
//    origTask(FlightTask(wpL, true, QString::null)),
//    optimizedTask(FlightTask(QString::null)),
    origTask(FlightTask(wpL, true, i18n("Original task"))),
    optimizedTask(FlightTask(i18n("Optimized task"))),
    optimized(false),
    nAnimationIndex(0),
    bAnimationActive(false)
{
  origTask.checkWaypoints(route, gliderType);

  __calculateBasicInformation();
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
  start[0] = std::max(idList[id], step) - step;
  start[1] = idList[id + 1] - step;
  start[2] = std::min(idList[id + 2] + step, route.count()) - ( 2 * step );

  stop[0] = start[0] + ( 2 * step );
  stop[1] = start[1] + ( 2 * step );
  stop[2] = std::min(start[2] + ( 2 * step ), route.count() - 1);
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

  int proceed = 0;
  unsigned int delta_T, m;
  float bearing;

  float circles = 0;
  float circles_abs = 0;

  for(unsigned int n = 0; n < route.count(); n++)
    {
      // calculate the change in bearing over 10 sec.
      delta_T = 0;
      bearing = 0;
      m = n;
      while(delta_T<10)
      {
        delta_T += route.at(m)->dT;
        bearing += fabs(route.at(m)->dBearing);
        m++;
        if(m==route.count())
          break;
      }
      // if the change in bearing is more than 65 deg in the next 10 sec., the glider will be in a thermal
      if(fabs(bearing *180*10/(M_PI*delta_T)) > 65)
      {
          proceed = 0;
          // Turn direction (Drehrichtung)
          // filter large/unrealistic bearing changes: include only changes in bearing which are smaller than 22.5 deg/sec
          if(fabs(route.at(n)->dBearing*180/(M_PI*route.at(n)->dT)) < 22.5)
          {
            circles += route.at(n)->dBearing;
            circles_abs += fabs(route.at(n)->dBearing);
          }

          // Kreisflug eingeleitet
          if(s_point < 0)
              s_point = n;
      }
      else if(s_point > -1 &&
              route.at(n)->time - route.at(n - proceed)->time  >= 20 &&
              route.at(n - proceed)->time - route.at(s_point)->time > 45)
      {
          // Circling time at least 20 s (Zeit eines Kreisfluges mindestens 20s)
          // Time between two thermals at most 20 s (Zeit zwischen zwei Kreisflügen höchstens 20s)

          // Endpoint of the thermal flight (Endpunkt des Kreisfluges)
          e_point = n - proceed - 1;

          // Punkte zwischen s_point und e_point setzen
          for(int n = s_point; n <=  e_point; n++)
            {
              // if 80% of the turns are to the right, then the thermal flight will be to the right
              if(circles>circles_abs*0.8)
                route.at(n)->f_state = Flight::RightTurn;
              else if(circles<-circles_abs*0.8)
                route.at(n)->f_state = Flight::LeftTurn;
              else
                route.at(n)->f_state = Flight::MixedTurn;
            }
          s_point = - 1;
          e_point = - 1;
          circles = 0;
          circles_abs = 0;
          proceed = 0;
      }
      else
      {
          if( (route.at(n)->time - route.at(n - proceed)->time)  >= 20)
            {
              // Kreisflug war unter 20s und wird daher nicht gewertet
              s_point = - 1;
              e_point = - 1;
              circles = 0;
              circles_abs = 0;
              proceed = 0;
            }
          // 4 Punkte warten bis wir endgültig rausgehen ;-)
          proceed++;
      }
    }
}

void Flight::__calculateBasicInformation()
{
  /**BUG: wrong bearings are given in flights with a high log-interval and with windy conditions. This results in incorrect turning directions of thermals*/
  float        prevBearing = 0, nextBearing = 0 , diffBearing = 0, prevDiffBearing = 0;
  unsigned int points = route.count();

  for(unsigned int n = 0; n < points; n++)
  {
    if(n==0)
    {
      route.at(n)->dH = 0;
      route.at(n)->dT = 0;
      route.at(n)->dS = 0;

      route.at(n)->bearing  = getBearing(*route.at(n), *route.at(n+1));
      route.at(n)->dBearing = 0;
    }
    else if(n==(points-1))
    {
      route.at(n)->dH = route.at(n)->height - route.at(n-1)->height;
      route.at(n)->dT = std::max( (route.at(n)->time - route.at(n-1)->time), time_t(1));
      route.at(n)->dS = (int)(dist(route.at(n)->origP.lat(), route.at(n)->origP.lon(), route.at(n-1)->origP.lat(), route.at(n-1)->origP.lon()) * 1000.0);

      route.at(n)->bearing  = getBearing(*route.at(n-1), *route.at(n));
      route.at(n)->dBearing = __diffAngle(route.at(n-1)->bearing, route.at(n)->bearing);
    }
    //calculate the bearing by calculating the average between the bearing with the previous and next point
    else
    {
      route.at(n)->dH = route.at(n)->height - route.at(n-1)->height;
      route.at(n)->dT = std::max( (route.at(n)->time - route.at(n-1)->time), time_t(1));
      route.at(n)->dS = (int)(dist(route.at(n)->origP.lat(), route.at(n)->origP.lon(), route.at(n-1)->origP.lat(), route.at(n-1)->origP.lon()) * 1000.0);

      prevBearing = getBearing(*route.at(n-1), *route.at(n));
      nextBearing = getBearing(*route.at(n), *route.at(n+1));
      diffBearing = __diffAngle(prevBearing, nextBearing);

      //in windy conditions large changes in diffBearing can occur, which means that the plane suddenly changes its turn direction
      if(fabs(prevDiffBearing-diffBearing)*9/route.at(n)->dT > M_PI)
        diffBearing = -diffBearing;

      //calculate the bearing as an average of the previous and the next bearing
      if(diffBearing<0)
        route.at(n)->bearing = fabs(diffBearing)/2+nextBearing;
      else
        route.at(n)->bearing = fabs(diffBearing)/2+prevBearing;

      //be sure that the bearing is not larger than 360 degrees
      if(route.at(n)->bearing > 2.0*M_PI)
        route.at(n)->bearing  = route.at(n)->bearing - 2.0*M_PI;

      route.at(n)->dBearing = __diffAngle(route.at(n-1)->bearing, route.at(n)->bearing);
      //in windy conditions large changes in dBearing can occur, which means that the plane suddenly changes its turn direction
      if((route.at(n)->dBearing-route.at(n-1)->dBearing)*9/route.at(n)->dT>270/180*M_PI && route.at(n)->dBearing>0)
        route.at(n)->dBearing = route.at(n)->dBearing - 2*M_PI;
      else if((route.at(n)->dBearing-route.at(n-1)->dBearing)*9/route.at(n)->dT<(-270/180*M_PI) && route.at(n)->dBearing<0)
        route.at(n)->dBearing = route.at(n)->dBearing + 2*M_PI;

      prevDiffBearing = diffBearing;
    }
  }
}

float Flight::__diffAngle(float firstAngle, float secondAngle) {
  float diffAngle = secondAngle-firstAngle;
  //make the absolute value of bearing smaller than 180 degree
  if(diffAngle > M_PI)
    diffAngle-= 2.0*M_PI;
  else if(diffAngle < -M_PI)
    diffAngle+= 2.0*M_PI;
  return diffAngle;
}

unsigned int Flight::__calculateBestTask(unsigned int start[],
    unsigned int stop[], unsigned int step, unsigned int idList[],
    double taskValue[], bool isTotal)
{
  unsigned int numSteps = 0;
  double temp = 0;
  flightPoint *pointA, *pointB, *pointC;

  for(unsigned int loopA = start[0]; loopA <= std::min(stop[0], route.count() - 1); loopA += step)
    {
      pointA = route.at(loopA);

      if(isTotal) start[1] = loopA + step;

      for(unsigned int loopB = start[1]; loopB <= std::min(stop[1], route.count() - 1); loopB += step)
        {
          pointB = route.at(loopB);

          if(isTotal) start[2] = loopB + step;

          for(unsigned int loopC = start[2]; loopC <= std::min(stop[2], route.count() - 1); loopC += step)
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

  float vario_min = getPoint(VA_MIN).dH/getPoint(VA_MIN).dT;
  float vario_max = getPoint(VA_MAX).dH/getPoint(VA_MAX).dT;
  int altitude_max = getPoint(H_MAX).height;
  float speed_max = getPoint(V_MAX).dS/getPoint(V_MAX).dT;

  for(unsigned int n = delta; n < route.count(); n = n + delta)
    {
      pointA = route.at(n - delta);
      pointB = route.at(n);
      if(n + delta < route.count())
          pointC = route.at(n + delta);
      else
          pointC = route.last();

      curPointB = glMapMatrix->print(pointB->projP);

      bBoxFlight.setLeft(std::min(curPointB.x(), bBoxFlight.left()));
      bBoxFlight.setTop(std::max(curPointB.y(), bBoxFlight.top()));
      bBoxFlight.setRight(std::max(curPointB.x(), bBoxFlight.right()));
      bBoxFlight.setBottom(std::min(curPointB.y(), bBoxFlight.bottom()));

      QPen drawP = glConfig->getDrawPen(pointB, vario_min, vario_max, altitude_max, speed_max);
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

  float vario_min = getPoint(VA_MIN).dH/getPoint(VA_MIN).dT;
  float vario_max = getPoint(VA_MAX).dH/getPoint(VA_MAX).dT;
  int altitude_max = getPoint(H_MAX).height;
  float speed_max = getPoint(V_MAX).dS/getPoint(V_MAX).dT;

  for(unsigned int n = delta; n < nStop; n = n + delta)
    {
      pointA = route.at(n - delta);
      pointB = route.at(n);
      if(n + delta < nStop)
          pointC = route.at(n + delta);
      else
          pointC = route.at(nStop);

      curPointB = glMapMatrix->map(pointB->projP);

      bBoxFlight.setLeft(std::min(curPointB.x(), bBoxFlight.left()));
      bBoxFlight.setTop(std::max(curPointB.y(), bBoxFlight.top()));
      bBoxFlight.setRight(std::max(curPointB.x(), bBoxFlight.right()));
      bBoxFlight.setBottom(std::min(curPointB.y(), bBoxFlight.bottom()));

      QPen drawP = glConfig->getDrawPen(pointB, vario_min, vario_max, altitude_max, speed_max);
      drawP.setCapStyle(Qt::SquareCap);
      targetPainter->setPen(drawP);

      maskPainter->setPen(QPen(Qt::color1, drawP.width(),
          Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));

      targetPainter->drawLine(curPointA, curPointB);
      maskPainter->drawLine(curPointA, curPointB);

      /* tries to find the elevation of the surface under the point */
      // send a signal with the curPointA, and the index of the point [ n * delta - (delta - 1) ] */

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

flightPoint Flight::getPointByTime(time_t time)
{
  return getPoint(getPointIndexByTime(time));
}

int Flight::getPointIndexByTime(time_t time)
{
 int diff, n, sp, ep;

  // Estimate a near point on the route to reduce linear search
  diff = (route.last()->time - route.at(0)->time) / route.count();
  sp = (time - route.at(0)->time) / diff;
  if ( sp < 0 )
    sp = 0;
  if ( sp > ((int)route.count())-1 )
    sp = route.count()-1;

  // sp is now hopefully an index near to the wanted fix time
  if( route.at(sp)->time < time ) {
    n = 1;
    ep = route.count() - 1;
  }
  else {
    n = -1;
    ep = 0;
  }

  diff = route.at(sp)->time - time;
  diff = abs(diff);

  if ( sp != ep )
  {
    for(int l = sp+n; l != ep; l += n) // l < (int)route.count() && l >= 0; l += n)
    {
      int a = route.at(l)->time - time;
      a = abs(a);
      if( a > diff )
        return l-n;

      diff = a;
    }
  }

  return ep;
}

QPtrList<flightPoint> Flight::getRoute() const{
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
  end = std::min(route.count() - 1, end);

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

    // Kreisflug / Circling
    //index: 0 right turn time
    text.sprintf("<small>%.1f%%</small> %s",
                 (float) kurbel_r / (float)(kurbel_r + kurbel_l + kurbel_v) * 100.0,
                 (const char*)printTime(kurbel_r, true, true, true) );
    result.append(text);
    //index: 1 left turn time
    text.sprintf("<small>%.1f%%</small> %s",
                 (float) kurbel_l / (float)(kurbel_r + kurbel_l + kurbel_v) * 100.0,
                 (const char*)printTime(kurbel_l, true, true, true) );
    result.append(text);
    //index: 2 mixed turn time
    text.sprintf("<small>%.1f%%</small> %s",
                 (float) kurbel_v / (float)(kurbel_r + kurbel_l + kurbel_v) * 100.0,
                 (const char*)printTime(kurbel_v, true, true, true) );
    result.append(text);
    //index: 3 total turn time
    text.sprintf("<small>%.1f%%</small> %s",
                 (float)(kurbel_r + kurbel_l + kurbel_v) / (float)( route.at(end)->time - route.at(start)->time ) * 100.0,
                 (const char*)printTime((kurbel_r + kurbel_l + kurbel_v), true, true, true) );
    result.append(text);

    //index: 4 right turn vario
    text.sprintf("%.2f m/s",(k_height_pos_r + k_height_neg_r) /
             (kurbel_r));
    result.append(text);
    //index: 5 left turn vario
    text.sprintf("%.2f m/s",(k_height_pos_l + k_height_neg_l) /
             (kurbel_l));
    result.append(text);
    //index: 6 mixed turn vario
    text.sprintf("%.2f m/s",(k_height_pos_v + k_height_neg_v) /
             (kurbel_v));
    result.append(text);
    //index: 7 total turn vario
    text.sprintf("%.2f m/s",
             (k_height_pos_r + k_height_pos_l + k_height_pos_v +
              k_height_neg_r + k_height_neg_l + k_height_neg_v) /
             (kurbel_r + kurbel_l + kurbel_v));
    result.append(text);

    //index: 8 right turn dH
    text.sprintf("%.0f m",k_height_pos_r);
    result.append(text);
    //index: 9 left turn dH
    text.sprintf("%.0f m",k_height_pos_l);
    result.append(text);
    //index: 10 mixed turn dH
    text.sprintf("%.0f m",k_height_pos_v);
    result.append(text);
    //index: 11 total turn dH
    text.sprintf("%.0f m",k_height_pos_r + k_height_pos_l + k_height_pos_v);
    result.append(text);
    //index: 12 right turn -dH
    text.sprintf("%.0f m",k_height_neg_r);
    result.append(text);
    //index: 13 left turn -dH
    text.sprintf("%.0f m",k_height_neg_l);
    result.append(text);
    //index: 14 mixed turn -dH
    text.sprintf("%.0f m",k_height_neg_v);
    result.append(text);
    //index: 15 total turn -dH
    text.sprintf("%.0f m",k_height_neg_r + k_height_neg_l + k_height_neg_v);
    result.append(text);
    //index: 16 right turn dH netto
    text.sprintf("%.0f m",k_height_pos_r + k_height_neg_r);
    result.append(text);
    //index: 17 left turn dH netto
    text.sprintf("%.0f m",k_height_pos_l + k_height_neg_l);
    result.append(text);
    //index: 18 mixed turn dH netto
    text.sprintf("%.0f m",k_height_pos_v + k_height_neg_v);
    result.append(text);
    //index: 19 total turn dH netto
    text.sprintf("%.0f m",k_height_pos_r + k_height_pos_l + k_height_pos_v  +  k_height_neg_r + k_height_neg_l + k_height_neg_v);
    result.append(text);

    // Strecke / Straight
    //index: 20 straight L/D
    text.sprintf("%.0f",distance / (s_height_pos + s_height_neg) *-1);
    result.append(text);
    //index: 21 straight speed
    text.sprintf("%.1f km/h",distance /
          ((float)(route.at(end)->time - route.at(start)->time -
          (kurbel_r + kurbel_l + kurbel_v))) * 3.6);
    result.append(text);
    //index: 22 straight dH
    text.sprintf("%.0f m",s_height_pos);
    result.append(text);
    //index: 23 straight -dH
    text.sprintf("%.0f m",s_height_neg);
    result.append(text);
    //index: 24 straight dH netto
    text.sprintf("%.0f m",s_height_pos + s_height_neg);
    result.append(text);
    //index: 25 straight total distance
    text.sprintf("%.0f km",distance / 1000);
    result.append(text);
    //index: 26 straight time
    text.sprintf("<small>%.1f%%</small> %s",
        (float)( route.at(end)->time - route.at(start)->time -
            ( kurbel_r + kurbel_l + kurbel_v ) ) /
                (float)( route.at(end)->time - route.at(start)->time ) * 100.0,
        (const char*)printTime( (int)( route.at(end)->time - route.at(start)->time -
            ( kurbel_r + kurbel_l + kurbel_v ) ) , true, true, true) );
    result.append(text);

    //Total
    //index: 27 total time
    text.sprintf("%s",
        (const char*)printTime((int)(route.at(end)->time - route.at(start)->time), true, true, true));
    result.append(text);
    //index: 28 total dH
    text.sprintf("%.0f m",s_height_pos   + k_height_pos_r
                        + k_height_pos_l + k_height_pos_v);
    result.append(text);
    //index: 29 total -dH
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

QPtrList<statePoint> Flight::getFlightStates(unsigned int start, unsigned int end)
{
  int dH_pos = 0, dH_neg = 0;
  int duration = 0;
  int n_start = 0;
  int distance = 0;
  float circ_angle_sum = 0;
  float vario = 0;
  unsigned int state = 9;
  QPtrList<statePoint> state_list;
  statePoint state_info;

  //  noch abchecken, dass end <= fluglÃ¯Â¿Â½nge
  end = std::min(route.count() - 1, end);

  for(unsigned int n = start; n < end; n++)
    {
      // copy info about previous state into state_list, when state changes
      if(state!=route.at(n)->f_state)
      {
        if(state!=9)
        {
            state_info.f_state = state;
            state_info.start_time = route.at(n_start)->time;
            state_info.end_time = route.at(n)->time;
            state_info.duration = duration;
            if(state==Flight::Straight)
              //cruising:
              //distance based on dS
              state_info.distance = distance/1000.0;
            else
              //circling:
              //distance of a straight line between start and end point
              state_info.distance = dist(route.at(n_start), route.at(n));
            state_info.speed = state_info.distance/duration*3600.0;
            state_info.L_D = state_info.distance*1000.0/(route.at(n_start)->height-route.at(n)->height);
            state_info.circles = fabs(circ_angle_sum/(2*M_PI));
            if(duration>0) //to prevent a buffer overflow
                vario = (route.at(n)->height-route.at(n_start)->height)/((float) duration);
            //cout << state << " " << distance << " " << dist(route.at(n_start), route.at(n)) << endl;
            state_info.vario = vario;
            state_info.dH_pos = dH_pos;
            state_info.dH_neg = dH_neg;

            state_list.append(new statePoint);
            *(state_list.last()) = state_info;
        }

        //reset for next state
        state = route.at(n)->f_state;
        dH_pos = 0;
        dH_neg = 0;
        duration = 0;
        distance = 0;
        circ_angle_sum = 0;
        n_start = n;
      }
      if(route.at(n)->dH > 0)
        dH_pos += route.at(n)->dH;
      else
        dH_neg += route.at(n)->dH;
      duration += route.at(n)->dT;
      distance += route.at(n)->dS;
      circ_angle_sum += route.at(n)->bearing;
    }

    return state_list;
}

QString Flight::getDistance(bool isOrig)
{
  if(isOrig || !optimized)
      return origTask.getTotalDistanceString();
  else
      return optimizedTask.getTotalDistanceString();
}

FlightTask Flight::getTask(bool isOrig) const
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

time_t Flight::getLandTime() const { return landTime; }

QString Flight::getPilot() const { return pilotName; }

time_t Flight::getStartTime() const { return startTime; }

int Flight::getStartIndex() const { return startIndex; }

int Flight::getLandIndex() const { return landIndex; }

QString Flight::getType() const { return gliderType; }

QDate Flight::getDate() const { return date; }

unsigned int Flight::getRouteLength() const { return route.count(); }

bool Flight::isOptimized() const { return optimized; }

int Flight::searchPoint(const QPoint& cPoint, flightPoint& searchPoint)
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
  float tmp, refv = .0, refh = .0, refva1 = .0 , refva2 = 500.;

  QPtrListIterator<flightPoint> fp(route);
  for( unsigned int loop = 0; (*fp); ++fp, loop++ )
    {
      if(loop)
        {
          // Fetch extreme values
          if( (tmp=GET_SPEED((*fp))) > refv )
              { v_max = loop; refv = tmp; }

          if( (tmp=(*fp)->height) > refh )
              { h_max = loop; refh = tmp; }

          if( (tmp=GET_VARIO((*fp))) > refva1 )
              { va_max = loop; refva1 = tmp; }

          if( (tmp=GET_VARIO((*fp))) < refva2 )
              { va_min = loop; refva2 = tmp; }
        }
    }
}

QPtrList<Waypoint> Flight::getWPList()
{
  if(!optimized)
    return origTask.getWPList();
  else
    return optimizedTask.getWPList();
}

QPtrList<Waypoint> Flight::getOriginalWPList() {  return origTask.getWPList();  }

bool Flight::optimizeTaskOLC(Map* map)
{
  OptimizationWizard* wizard = new OptimizationWizard(0,i18n("Optimization for OLC"));
  wizard->setMapContents(map);
  int wizard_ret = wizard->exec();
  if (wizard_ret==QDialog::Rejected) {
    delete wizard;
    return false;
  }

  unsigned int idList[LEGS+3];
  double points;
  double distance = wizard->optimizationResult(idList,&points);

  if (distance<0.0) // optimization was canceled
    return false;

      QPtrList<Waypoint> wpL;

      APPEND_WAYPOINT_OLC2003(startIndex, 0, i18n("Take-Off"))
      APPEND_WAYPOINT_OLC2003(idList[0], dist(route.at(idList[0]), route.at(0)),
          i18n("Begin of Soaring"))
      APPEND_WAYPOINT_OLC2003(idList[1], dist(route.at(idList[1]), route.at(1)),
          i18n("Begin of Task"))
      APPEND_WAYPOINT_OLC2003(idList[2], dist(route.at(idList[2]),
          route.at(idList[1])), i18n("Optimize 1"))
      APPEND_WAYPOINT_OLC2003(idList[3], dist(route.at(idList[3]),
          route.at(idList[2])), i18n("Optimize 2"))
      APPEND_WAYPOINT_OLC2003(idList[4], dist(route.at(idList[4]),
          route.at(idList[3])), i18n("Optimize 3"))
      APPEND_WAYPOINT_OLC2003(idList[5], dist(route.at(idList[5]),
          route.at(idList[4])), i18n("Optimize 4"))
      APPEND_WAYPOINT_OLC2003(idList[6], dist(route.at(idList[6]),
          route.at(idList[5])), i18n("Optimize 5"))
      APPEND_WAYPOINT_OLC2003(idList[7], dist(route.at(idList[7]),
          route.at(idList[6])), i18n("End of Task"))
      APPEND_WAYPOINT_OLC2003(idList[8], dist(route.at(idList[8]),
          route.at(idList[7])), i18n("End of Soaring"))
      APPEND_WAYPOINT_OLC2003(landIndex, dist(route.getLast(),
          route.at(idList[8])), i18n("Landing"))

      optimizedTask.setWaypointList(wpL);
      optimizedTask.checkWaypoints(route, gliderType);
      optimizedTask.setOptimizedTask(points,distance);
      optimized = true;

      delete wizard;
      return true;
}

/*
 * The optimization is done in two runs. During the first run, we go
 * with a special step-width, which is calculated
 */
bool Flight::optimizeTask()
{
  if( route.count() < 10)  return false;

  unsigned int curNumSteps = 0, temp, step = 0, minNumSteps = 400000000;
  unsigned int numStepsA = 0, numStepsB = 0, minA = 0, minB = 0;

  double length = 0;
  for(unsigned int curStep = 1; curStep < 100; curStep++)
    {
      /*
       * ( route.count / curStep ) is the number of points used for the
       * first optimization-run. If this number is smaller than 3, we
       * cannot create a triangle.
       */
      if((route.count() / curStep) < 3) break;

      curNumSteps = 1;
      numStepsA = 1;
      numStepsB = 1;
      temp = 1;

      /*
       * Calculating the step-width for the first run.
       */
      for(unsigned int loop = 3; loop < ( route.count() / curStep ); loop++)
        {
          curNumSteps += temp + loop;
          temp += loop;
        }

      numStepsA = curNumSteps;
      temp = 1;

      /*
       * Calculating the number of steps for the second run.
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

  unsigned int stepB = std::max((int)step / 6, 4);

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
      QPtrList<Waypoint> wpL;

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

/** Re-calculates all projections for this flight. */
void Flight::reProject(){

  QPtrListIterator<flightPoint> it(route);
  extern MapMatrix _globalMapMatrix;

  for ( ; it.current(); ++it ) {
      flightPoint *fp = it.current();
      fp->projP = _globalMapMatrix.wgsToMap(fp->origP);
  }

  origTask.reProject();
  optimizedTask.reProject();

}
