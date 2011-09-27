/***********************************************************************
**
**   mapcalc.cpp
**
**   This file is part of KFLog4.
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
#include <cstdlib>

#include "mapcalc.h"
#include "mapdefaults.h"

#include "mapmatrix.h"
#include "resource.h"

#define PI2 M_PI*2

static const char *timeFormat[6] = {"%2d:%2d", "%2d:%2d:%2d", "%02d:%02d", "%02d:%02d:%02d", "%2d:%02d", "%2d:%02d:%02d"};

static const char *monthAbb[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static const double rad = M_PI / 108000000.0; // Pi / (180 degrees * 600000 KFlog degrees)
static const double pi_180 = M_PI / 108000000.0;

/**
 * Calculates the distance between two given points in km according to Pythagoras.
 * For longer distances the result is often greater as calculated by great circle.
 * Not more used in Cumulus.
 */
double distP(double lat1, double lon1, double lat2, double lon2)
{
  double dlat = lat1 - lat2;
  double dlon = lon1 - lon2;

  // lat is used to calculate the earth-radius. We use the average here.
  // Otherwise, the result would depend on the order of the parameters.
  double lat = ( lat1 + lat2 ) / 2.0;

  // Distance calculation according to Pythagoras
  double dist = RADIUS * hypot (pi_180 * dlat, pi_180 * cos( pi_180 * lat ) * dlon);

  return dist / 1000.0;
}

/**
 *  Distance calculation according to great circle. Unfit for very short
 *  distances due to rounding errors but required for longer distances according
 *  to FAI Code Sportif, Annex C. It is used as default in Cumulus.
 *  http://sis-at.streckenflug.at/2009/pdf/cs_annex_c.pdf
 */
double distG(double lat1, double lon1, double lat2, double lon2)
{
  // See here for formula: http://williams.best.vwh.net/avform.htm#Dist and
  // http://freenet-homepage.de/streckenflug/optigc.html
  // s = 6371km * acos( sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1-lon2) )
  double dlon = lon1-lon2;

  double arc = acos( sin(lat1*rad) * sin(lat2*rad) + cos(lat1*rad) * cos(lat2*rad) * cos(dlon*rad) );

  // distance in Km
  double dist = arc * RADIUS / 1000.;

  return dist;
}

/**
 * Distance calculation according to great circle. A mathematically equivalent formula,
 * which is less subject to rounding error for short distances.
 */
double dist(double lat1, double lon1, double lat2, double lon2)
{
  // See here for formula: http://williams.best.vwh.net/avform.htm#Dist
  // d = 6371km * 2 * asin( sqrt( sin((lat1-lat2)/2) ^2 + cos(lat1) * cos(lat2) * sin((lon1-lon2)/2) ^2 ) )
  double dlon = (lon1-lon2) * rad / 2;
  double dlat = (lat1-lat2) * rad / 2;

  double sinLatd = sin(dlat);
  sinLatd = sinLatd * sinLatd;

  double sinLond = sin(dlon);
  sinLond = sinLond * sinLond;

  double arc = 2 * asin( sqrt( sinLatd + cos(lat1*rad) * cos(lat2*rad) * sinLond ) );

  // distance in Km
  double dist = arc * RADIUS / 1000.;

  return dist;
}

double dist(Waypoint* wp1, Waypoint* wp2)
{
  return ( dist( wp1->origP.lat(), wp1->origP.lon(),
                 wp2->origP.lat(), wp2->origP.lon() ) );
}

double dist(Waypoint* wp, FlightPoint* fp)
{
  return ( dist( wp->origP.lat(), wp->origP.lon(),
                 fp->origP.lat(), fp->origP.lon() ) );
}

double dist(FlightPoint* fp1,  FlightPoint* fp2)
{
  return ( dist( fp1->origP.lat(), fp1->origP.lon(),
                 fp2->origP.lat(), fp2->origP.lon() ) );
}

double dist(QPoint* p1, QPoint* p2)
{
    return ( dist( double(p1->x()), double(p1->y()),
                   double(p2->x()), double(p2->y()) ));
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
  posMin.sprintf(" %02d'", min);

  sec = (int)sqrt(sec * sec);
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


QString printTime(int time, bool isZero, bool isSecond, bool noZeroHour)
{
  QString tmpbuf;

  int hh = time / 3600;
  int mm = (time - (hh * 3600)) / 60;
  int ss = time - (hh * 3600) - mm * 60;

  if(noZeroHour==true) isZero = true;

  return tmpbuf.sprintf(timeFormat[isSecond + 2*isZero + 2*noZeroHour], hh, mm, ss);
}

time_t timeToDay(const int year, const int month, const int day, const char *monabb)
{
  struct tm bt;
  int mymonth = month;

  bt.tm_sec = 0; /* Seconds.        [0-60] (1 leap second) */
  bt.tm_min = 0; /* Minutes.        [0-59] */
  bt.tm_hour = 0; /* Hours.         [0-23] */
  bt.tm_mday = day; /* Day.         [1-31] */
  if ( monabb )
  {
    mymonth = 1; // default to Jan
    for ( int i = 0; i < 12; i++ )
      if ( strcasecmp(monthAbb[i], monabb) == 0 ) {
        mymonth = i + 1;
        break;
      }
  }
  bt.tm_mon = mymonth - 1; // Month.        [0-11]
  bt.tm_year = year - 1900;   // Year        - 1900.

  // The next two members of the struct are set by the mktime routine
  bt.tm_wday = 0;   // Day of week [0-6]  I am in doubt, seems to be 1-7
  bt.tm_yday = 0;    // Day of year [0-365]
  // The following members of the struct seems to be also set by the mktime routine
  bt.tm_isdst = 0;  // DST [-1/0/1] -1: no info, 0 : no daylight save time, 1 daylight save time
  bt.tm_gmtoff = 0; // Seconds east of UTC
  bt.tm_zone = NULL; // Timezone abbreviation.

  // get UTC time
  time_t ret = timegm(&bt);

  return ret;
}

float getSpeed(FlightPoint p) { return (float)p.dS / (float)p.dT * 3.6; }

float getVario(FlightPoint p) { return (float)p.dH / (float)p.dT; }

/**
   Calculate the bearing from point p1 to point p2 from WGS84
   coordinates to avoid distortions caused by projection to the map.
   source: openairparser.cpp
*/
float getBearing(FlightPoint p1, FlightPoint p2)
{
  // Arcus computing constant for kflog corordinates. PI is devided by
  // 180 degrees multiplied with 600.000 because one degree in kflog
  // is multiplied with this resolution factor.
  const float pi_180 = M_PI / 108000000.0;

  // qDebug( "x1=%d y1=%d, x2=%d y2=%d",  p1.x(), p1.y(), p2.x(), p2.y() );

  int dx = p2.origP.x() - p1.origP.x(); // latitude
  int dy = p2.origP.y() - p1.origP.y(); // longitude

  // compute latitude distance in meters
  float latDist = dx * MILE_kfl / 10000.; // b

  // compute latitude average
  float latAv = ( ( p2.origP.x() + p1.origP.x() ) / 2.0);

  // compute longitude distance in meters
  float lonDist = dy * cos( pi_180 * latAv ) * MILE_kfl / 10000.; // a

  // compute angle
  float angle = asin( fabs(lonDist) / hypot( latDist, lonDist ) );

  // double angleOri = angle;

  // assign computed angle to the right quadrant
  if( dx >= 0 && dy < 0 ) {
    angle = (2 * M_PI) - angle;
  } else if( dx <=0 && dy <= 0 ) {
    angle =  M_PI + angle;
  } else if( dx < 0 && dy >= 0 ) {
    angle = M_PI - angle;
  }

  // no bearing can be calculated when the two points are on the same location
  if( dx==0 && dy==0) {
    angle = 0;
  }

  //qDebug( "dx=%d, dy=%d - AngleRadCorr=%f, AngleGradCorr=%f",
  //  dx, dy, angle, angle * 180/M_PI);

  return angle;
}

double getTrueCourse(WGSPoint p1, WGSPoint p2)
{
  return p1 != p2 ? polar(p1.lat() - p2.lat(), p1.lon() - p2.lon()) * 180.0 / M_PI : 0.0;
}

double polar(double x, double y)
{
  double angle = 0.0;
  //
  //  dX = 0 ???
  //
  if(x >= -0.001 && x <= 0.001)
    {
      if(y < 0.0) return ( 1.5 * M_PI );
      else  return ( 0.5 * M_PI );
    }

  // Punkt liegt auf der neg. X-Achse
  if(x < 0.0)  angle = atan( y / x ) + M_PI;
  else  angle = atan( y / x );

  // Neg. value not allowed.
  if(angle < 0.0)  angle = 2 * M_PI + angle;

  if(angle > (2 * M_PI))  angle = angle - (2 * M_PI);

  return angle;
}

double int2rad(int deg)
{
  return (double)deg * M_PI / 108000000.0;
}

double rad2int(double rad)
{
  return (int) (rad * 108000000.0 / M_PI);
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
              2.0 * M_PI) + M_PI;
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
  tLon = -fmod(lon1 - lon + M_PI, 2.0 * M_PI) + M_PI;

  return WGSPoint((int)rad2int(tLat), (int)rad2int(tLon));
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
        if (dx < 0.0) return (1.5 * M_PI);
        else return (0.5 * M_PI);
      }

      angle=atan(dx/-dy);
      if (dy>0.0) angle+= M_PI;
      if (angle<0) angle+=(2 * M_PI);
      if (angle>(2* M_PI )) angle-=(2* M_PI);

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
  double res2=res1+M_PI;

  res1=normalize(res1);
  res2=normalize(res2);


  if(res1-std::min(v1,v2)<0.5 * M_PI) {
    return res1;
  } else {
    return res2;
  }

}

double outsideVector(double angle1, double angle2) {
  double res1=(angle1+angle2)/2;
  double res2=res1+M_PI;

  res1=normalize(res1);
  res2=normalize(res2);


  if(res1-std::min(angle1,angle2)<0.5 * M_PI) {
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
  if (a>M_PI) return(a-PI2);
  if (a<-M_PI) return(a+PI2);
  return a;
}

/**
 * Calculates a (crude) bounding box that contains the circle of radius @arg r
 * around point @arg center. @arg r is given in kilometers.
 */
QRect areaBox(QPoint center, double r)
{
    const double pi_180 = M_PI / 108000000.0;
    int delta_lat = (int) rint(250.0 * r/RADIUS_kfl);
    int delta_lon = (int) rint(r*250.0 / (RADIUS_kfl *cos (pi_180 * center.x())));

    //  qDebug("delta_lat=%d, delta_lon=%d, reach=%f, center=(%d, %d)",delta_lat, delta_lon,r,center.x(),center.y());
    //  return QRect(center.x()-delta_lat, center.y()-delta_lon,
    //                      center.x()+delta_lat, center.y()+delta_lon);
    return QRect(center.x()-delta_lat, center.y()-delta_lon,
                 2*delta_lat, 2*delta_lon);
}

/**
 * Calculates the bounding box of the given tile number in KFLog coordinates.
 * The returned rectangle used the x-axis as longitude and the y-axis as latitude.
 */
QRect getTileBox(const ushort tileNo)
{
  if( tileNo > (180*90) )
    {
      qWarning("Tile %d is out of range", tileNo);
      return QRect();
    }

  // Positive result means N, negative result means S
  int lat = 90 - ((tileNo / 180) * 2);

  // Positive result means E, negative result means W
  int lon = ((tileNo % 180) * 2) - 180;

  // Tile bounding rectangle starting at upper left corner with:
  // X: longitude until longitude + 2 degrees
  // Y: latitude  until latitude - 2 degrees
  QRect rect( lon*600000, lat*600000, 2*600000, -2*600000 );

/*
  qDebug("Tile=%d, Lat=%d, Lon=%d, X=%d, Y=%d, W=%d, H=%d",
          tileNo, lat, lon, rect.x(), rect.y(),
          (rect.x()+rect.width())/600000,
          (rect.y()+rect.height())/600000 );
*/

 return rect;
}

/**
 * Calculates the map tile number from the passed coordinate. The coordinate
 * format is decimal degree. Positive numbers are N and E, negative numbers
 * are W and S.
 *
 * @param lat Latitude in decimal degree. 90...-90
 * @param lon Longitude in decimal degree. -180...180
 * @return map tile number 0...16199
 */
int mapTileNumber( double lat, double lon )
{
  // check and correct input ranges
  if( lat <= -90 ) lat = -88;
  if( lon >= 180 ) lon = 178;

  int latTile = (90 - (int) ceil(lat) + ((int) ceil(lat) % 2)) * 180 / 2;

  int lonTile = ((int) ceil(lon) + ((int) ceil(lon) % 2) + 180) / 2;

  int tile = lonTile + latTile;

  return tile;
}
