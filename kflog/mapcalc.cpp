/***********************************************************************
**
**   mapcalc.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>
#include "mapcalc.h"

#include "mapmatrix.h"
#include "resource.h"


static const char *timeFormat[4] = {"%2d:%2d", "%2d:%2d:%2d", "%02d:%02d", "%02d:%02d:%02d"};

static const char *monthAbb[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

double dist(double lat1, double lon1, double lat2, double lon2)
{
  double pi_180 = PI / 108000000.0;
  double dlat = lat1 - lat2;
  double dlon = lon1 - lon2;

  // lat is used to calculate the earth-radius. We use the average here.
  // Otherwise, the result would depend on the order of the parameters.
  double lat = ( lat1 + lat2 ) / 2.0;

  double dist = RADIUS * sqrt( ( pi_180 * dlat * pi_180 * dlat )
    + ( pi_180 * cos( pi_180 * lat ) * dlon *
        pi_180 * cos( pi_180 * lat ) * dlon ) );

  return dist / 1000.0;
}

double dist(Waypoint* wp1, Waypoint* wp2)
{
  return ( dist( wp1->origP.lat(), wp1->origP.lon(),
                 wp2->origP.lat(), wp2->origP.lon() ) );
}

double dist(Waypoint* wp, flightPoint* fp)
{
  return ( dist( wp->origP.lat(), wp->origP.lon(),
                 fp->origP.lat(), fp->origP.lon() ) );
}

double dist(flightPoint* fp1,  flightPoint* fp2)
{
  return ( dist( fp1->origP.lat(), fp1->origP.lon(),
                 fp2->origP.lat(), fp2->origP.lon() ) );
}

/*
 * Die Funktion scheint noch Probleme zu haben, wenn die Position nahe an
 * 0° W/E liegt.
 */
QString printPos(int coord, bool isLat)
{
  QString pos, posDeg, posMin, posSec;

  int degree = coord / 600000;
  int min = (coord - (degree * 600000)) / 10000;
  int sec = (coord - (degree * 600000) - (min * 10000));
  sec = (sec * 60) / 10000;

  min = (int)sqrt(min * min);
  //if(min < 10)  posMin.sprintf(" 0%d'", min);
  //else
  posMin.sprintf(" %02d'", min);

  sec = (int)sqrt(sec * sec);
  //if(sec < 10)  posSec.sprintf(" 0%d\"", sec);
  //else
  posSec.sprintf(" %02d\"", sec);

  if(isLat)
    {
      if(coord < 0)
        {
          posDeg.sprintf("%02d°", -degree);
          pos = posDeg + posMin + posSec + " S";
        }
      else
        {
          posDeg.sprintf("%02d°", degree);
          pos = posDeg + posMin + posSec + " N";
        }
    }
  else
    {
      if(coord < 0)
        {
          posDeg.sprintf("%03d°", -degree);
          pos = posDeg + posMin + posSec + " W";
        }
      else
        {
          posDeg.sprintf("%03d°", degree);
          pos = posDeg + posMin + posSec + " E";
        }
    }

  return pos;
}

QString printTime(time_t time, bool isZero, bool isSecond)
{
  QString tmpbuf;
  struct tm lt;

  gmtime_r (&time, &lt);

  return tmpbuf.sprintf(timeFormat[isSecond + 2*isZero], lt.tm_hour, lt.tm_min, lt.tm_sec);

}


QString printTime(int time, bool isZero, bool isSecond)
{
  QString tmpbuf;

  int hh = time / 3600;
  int mm = (time - (hh * 3600)) / 60;
  int ss = time - (hh * 3600) - mm * 60;

  return tmpbuf.sprintf(timeFormat[isSecond + 2*isZero], hh, mm, ss);
}

time_t timeToDay(const int year, const int month, const int day, const char *monabb)
{
  struct tm bt;
  int mymonth = month;

  bt.tm_sec = 0; /* Seconds.	[0-60] (1 leap second) */
  bt.tm_min = 0; /* Minutes.	[0-59] */
  bt.tm_hour = 0; /* Hours.	[0-23] */
  bt.tm_mday = day; /* Day.		[1-31] */
  if ( monabb )
  {
    mymonth = 1; // default to Jan
    for ( int i = 0; i < 12; i++ )
      if ( strcasecmp(monthAbb[i], monabb) == 0 ) {
        mymonth = i + 1;
        break;
      }
  }
  bt.tm_mon = mymonth - 1; /* Month.	[0-11] */
  bt.tm_year = year - 1900; /* Year	- 1900.  */
  bt.tm_wday = 0; /* Day of week.	[0-6] */
  bt.tm_yday = 0; /* Day of year.[0-365]	*/
  bt.tm_isdst = 0; /* DST.		[-1/0/1]*/
  bt.tm_gmtoff = 0; /* Seconds east of UTC.  */
  bt.tm_zone = NULL; /* "GMT" Timezone abbreviation.  */

  return mktime(&bt);
}

float getSpeed(flightPoint p) { return (float)p.dS / (float)p.dT * 3.6; }

float getVario(flightPoint p) { return (float)p.dH / (float)p.dT; }

float getBearing(flightPoint p1, flightPoint p2)
{
  return (float)polar( ( p2.projP.x() - p1.projP.x() ),
                       ( p2.projP.y() - p1.projP.y() ) );
}

double getTrueCourse(WGSPoint p1, WGSPoint p2)
{
  return p1 != p2 ? polar(p1.lat() - p2.lat(), p1.lon() - p2.lon()) * 180.0 / PI : 0.0;
}

double polar(double x, double y)
{
  double angle = 0.0;
  //
  //  dX = 0 ???
  //
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

double int2rad(int deg)
{
  return (double)deg * PI / 108000000.0;
}

double rad2int(double rad)
{
  return (int) (rad * 108000000.0 / PI);
}

double angle(double a, double b, double c)
{
  double a1, b1, c1, tmp;
  a1 = a / RADIUS * 1000.0;
  b1 = b / RADIUS * 1000.0;
  c1 = c / RADIUS * 1000.0;
  
  tmp = (cos(c1) - cos(a1) * cos(b1)) / sin(a1) / sin(b1);
  if (tmp > 1.0) {
    tmp = 1.0;
  }
  else if (tmp < -1.0) {
    tmp = -1.0;
  }

  return acos(tmp);
}

double tc(double lat1, double lon1, double lat2, double lon2)
{
  return fmod(atan2(sin(lon1-lon2)*cos(lat2), 
		    cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon1-lon2)),
	      2.0 * PI) + PI;
}

WGSPoint posOfDistAndBearing(double lat1, double lon1, double bearing, double dist)
{
  double tmp, lon;
  double tLat, tLon;

  dist = dist / RADIUS * 1000.0;
  tmp = sin(lat1) * cos(dist) + cos(lat1) * sin(dist) * cos(bearing);

  if (tmp > 1.0) {
    tmp = 1.0;
  }
  else if (tmp < -1.0) {
    tmp = -1.0;
  }

  tLat = asin(tmp);
  

  lon = atan2(sin(bearing) * sin(dist) * cos(lat1),
	      cos(dist) - sin(lat1) * (sin(lat1) * cos(dist) + 
				       cos(lat1) * sin(dist) * cos(bearing)));
  tLon = -fmod(lon1 - lon + PI, 2.0 * PI) + PI;

  return WGSPoint(rad2int(tLat), rad2int(tLon));
}

float getBearing(QPoint p1, QPoint p2)
{
  extern MapMatrix * _globalMapMatrix;
    QPoint pp1, pp2;
    double angle=0.0;
    double dx=0.0;
    double dy=0.0;

    pp1 = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(p1.x(), p1.y()));
    pp2 = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(p2.x(), p2.y()));

    dx=pp2.x()-pp1.x();
    dy=pp2.y()-pp1.y();

    if (dy>=-0.001 && dy<=0.001)
      {
        if (dx < 0.0) return (1.5 * PI);
        else return (0.5 * PI);
      }

      angle=atan(dx/-dy);
      if (dy>0.0) angle+= PI;
      if (angle<0) angle+=(2 * PI);
      if (angle>(2* PI )) angle-=(2* PI);

      return angle;

}



/**
 * Calculates the direction of the vector pointing to the outside
 * of the area spanned by the two vectors.
 */
double outsideVector(QPoint center, QPoint p1, QPoint p2){
  double v1=getBearing(center, p1);
  double v2=getBearing(center, p2);

  double res1=(v1+v2)/2;
  double res2=res1+PI;

  res1=normalize(res1);
  res2=normalize(res2);


  if(res1-MIN(v1,v2)<0.5 * PI) {
    return res1;
  } else {
    return res2;
  }

}

double outsideVector(double angle1, double angle2) {
  double res1=(angle1+angle2)/2;
  double res2=res1+PI;

  res1=normalize(res1);
  res2=normalize(res2);


  if(res1-MIN(angle1,angle2)<0.5 * PI) {
    return res1;
  } else {
    return res2;
  }
}

double normalize(double angle) {
  if (angle<0) return normalize(angle+PI2);
  if (angle>=PI2) return normalize(angle-PI2);
  return angle;
}

int normalize(int angle) {
  if (angle<0) return normalize(angle+360);
  if (angle>=360) return normalize(angle-360);
  return angle;
}

int angleDiff(int ang1, int ang2) {
  int a1=normalize(ang1);
  int a2=normalize(ang2);
  int a=a2-a1;
  if (a>180) return(a-360);
  if (a<-180) return(a+360);
  return a;
}

double angleDiff(double ang1, double ang2) {
  double a1=normalize(ang1);
  double a2=normalize(ang2);
  double a=a2-a1;
  if (a>PI) return(a-PI2);
  if (a<-PI) return(a+PI2);
  return a;
}

