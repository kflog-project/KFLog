/***********************************************************************
**
**   WGSPoint.cpp - general position representations
**
**   This file is part of KFLog4
**
************************************************************************
**
**   Copyright (c):  2008-2010 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author André Somers, Axel Pauli
 *
 * \short Class to handle WGS84 coordinates
 *
 * This class is used to handle WGS84 coordinates. It inherits QPoint. The only
 * difference is, that the methods to access the coordinates are called "lat"
 * and "lon". Furthermore it controls the unit to be used for position
 * representation.
 */

#include <stdlib.h>
#include <cmath>
#include <QRegExp>

#include "wgspoint.h"
#include "mapcalc.h"

// set static format variable to default (degrees, minutes, seconds)
WGSPoint::Format WGSPoint::_format = WGSPoint::DMS;


WGSPoint::WGSPoint() : QPoint()
{}

WGSPoint::WGSPoint(int lat, int lon) : QPoint(lat, lon)
{}

/**
 * Creates a new WGSPoint with the given position.
 */
WGSPoint::WGSPoint(const QPoint& pos) : QPoint(pos.x(), pos.y())
{}

WGSPoint &WGSPoint::operator=( const QPoint &p )
{
  setPos(p.x(), p.y());
  return *this;
}

/**
 * Converts the given integer KFLog coordinate into real degrees, minutes and seconds.
 */
void WGSPoint::calcPos (int coord, int& degree, int& min, int &sec)
{
  degree = coord / 600000;
  min = (coord % 600000) / 10000;
  sec = (coord % 600000) % 10000;
  sec = (int) rint((sec * 60) / 10000.0);

  // @AP: Rounding of seconds can lead to unwanted results. Therefore this is
  // checked and corrected here.
  if( sec > 59 )
    {
      sec = 59;
    }
  else if( sec < -59 )
    {
      sec = -59;
    }
}

/**
 * Converts the given integer KFLog coordinate into real degrees and minutes.
 */
void WGSPoint::calcPos (int coord, int& degree, double& min)
{
  degree = coord / 600000;
  min = (coord % 600000) / 10000.0;
  // qDebug("Coord=%d, degree=%d, decMin=%f", coord, degree, min);
}

/**
 * Converts the given integer KFLog coordinate into a real degree value.
 */
void WGSPoint::calcPos (int coord, double& degree)
{
  degree = coord / 600000.0;
  // qDebug("Coord=%d, Degree=%f", coord, degree);
}

/**
 * Calculates the other position and distance in relation to the own position.
 *
 * @param own Own position in KFLog WGS84 coordinates.
 * @param north Relative position in meter true north from own position
 * @param east Relative position in meter true east from own position
 * @param other Calculated other position in KFLog WGS84 coordinates.
 * @param distance Calculated distance between own and other position in meters.
 * @returns true (success) or false (error occurred)
 */
bool WGSPoint::calcFlarmPos( QPoint& own, int north, int east,
                             QPoint& other, double& distance )
{
  other.setX(0);
  other.setY(0);
  distance = -1;

  // Convert own position from KFLog degree into decimal degree
  double ownLat = own.x() / 600000.;
  double ownLon = own.y() / 600000.;

  // Latitude 90N or 90S causes division by zero.
  if( ownLat >=90. || ownLat <=-90. || ownLon < -180. || ownLon > 180. )
    {
      return false;
    }

  const double rad = M_PIl/180;
  const double degree = 180/M_PIl;

  const double northD = (double) north;
  const double eastD  = (double) east;

  // Calculate length in degree along the latitude and the longitude.
  // For the calculation the circle formula is used.
  double deltaLat = degree * northD/RADIUS;
  double deltaLon = degree * eastD/(RADIUS * cos ( rad * ownLat ));

  double flarmLat = ownLat + deltaLat;
  double flarmLon = ownLon + deltaLon;

  if( flarmLat >=90. || flarmLat <=-90. )
    {
      // Assuming we do not fly in pole region
      return false;
    }

  // Check for crossing E/W longitude border
  if( flarmLon > 180. )
    {
      flarmLon = -(180. - (flarmLon - 180.));
    }
  else if( flarmLon < -180. )
    {
      flarmLon = (180. + (flarmLon + 180.));
    }

  // store Flarm position
  other.setX( static_cast<int>(rint(flarmLat * 600000)) );
  other.setY( static_cast<int>(rint(flarmLon * 600000)) );

  // Calculate distance in meters according to Pythagoras
  distance = sqrt( (north*north) + (east*east) );

  return true;
}

/**
 * Calculates the other position in relation to the own position by using
 * bearing and radius distance. Only usable for short distances.
 *
 * @param radius Distance from own position in meters to other position
 * @param bearing True bearing (0...359) from own position to other position
 * @param own Own position in KFLog WGS84 coordinates
 * @param other Calculated other position in KFLog WGS84 coordinates.
 * @returns true (success) or false (error occurred)
 */
bool WGSPoint::calcFlarmPos( int radius, int bearing, QPoint& own, QPoint& other )
{
  const double rad    = M_PIl/180;
  const double degree = 180/M_PIl;

  // Convert own position from KFLog degree into decimal degree
  double ownLat = own.x() / 600000.;
  double ownLon = own.y() / 600000.;

  double rx = degree * radius/RADIUS;
  double ry = degree * radius/(RADIUS * cos ( rad * ownLat ));

  double phi = bearing * rad;

  // calculate polar coordinates
  double x = (cos(phi) * rx) + ownLat;
  double y = (sin(phi) * ry) + ownLon;

  // store other position
  other.setX( static_cast<int>(rint(x * 600000)) );
  other.setY( static_cast<int>(rint(y * 600000)) );

  return true;
}

/**
 * The function seems to have problems, if the position is near 0° W/E.
 */
QString WGSPoint::printPos(int coord, bool isLat)
{
  QString pos, posDeg, posMin, posSec;
  int degree, min, sec;
  double decDegree, decMin;

  if ( getFormat() == WGSPoint::DMS )
    {
      // default is always degrees, minutes, seconds
      calcPos (coord, degree, min, sec);

      if (isLat)
        {
          posDeg.sprintf("%02d\260 ", (degree < 0)  ? -degree : degree);
        }
      else
        {
          posDeg.sprintf("%03d\260 ", (degree < 0)  ? -degree : degree);
        }

      min = abs(min);
      posMin.sprintf("%02d'", min);

      sec = abs(sec);
      posSec.sprintf(" %02d\"", sec);
    }
  else if ( getFormat() == WGSPoint::DDM )
    {
      // degrees and decimal minutes
      calcPos (coord, degree, decMin);

      if (isLat)
        {
          posDeg.sprintf("%02d\260 ", (degree < 0)  ? -degree : degree);
        }
      else
        {
          posDeg.sprintf("%03d\260 ", (degree < 0)  ? -degree : degree);
        }

      decMin = fabs(decMin);

      posMin.sprintf("%.3f'", decMin);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if ( decMin < 10.0 )
        {
          posMin.insert(0, "0");
        }
    }
  else if ( getFormat() == WGSPoint::DDD )
    {
      // decimal degrees
      calcPos (coord, decDegree);

      posDeg.sprintf("%.5f\260", (decDegree < 0)  ? -decDegree : decDegree);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if (isLat)
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "0");
            }
        }
      else
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "00");
            }
          else if ( decDegree < 100.0 )
            {
              posDeg.insert(0, "0");
            }
        }
    }

  pos = posDeg + posMin + posSec;

  if (isLat)
    {
      if (coord < 0)
        {
          pos += " S";
        }
      else
        {
          pos += " N";
        }
    }
  else
    {
      if (coord < 0)
        {
          pos += " W";
        }
      else
        {
          pos += " E";
        }
    }

  // qDebug( "Pos=%s", pos.toLatin1().data() );

  return pos;
}

/** Converts the degree input string into the internal KFLog format */
int WGSPoint::degreeToNum(QString inDegree)
{
  /*
   * needed formats:
   *
   *  [g]gg° mm' ss"
   *  [g]gg° mm.mmmm'
   *  [g]gg.ggggg°
   *  dddddddddd
   */

  // to prevent trouble with the degree sign coding
  QChar degreeChar = Qt::Key_degree;
  QString degreeString( degreeChar );
  QString input = inDegree;

  QRegExp degreeDMS("^[0-1]?[0-9]?[0-9]" + degreeString + "\\s[0-5]?[0-9]'[ ]*[0-5]?[0-9]\"\\s[neswNESW]$");
  QRegExp degreeDDM("^[0-1]?[0-9]?[0-9]" + degreeString + "\\s[0-5]?[0-9].[0-9][0-9]?[0-9]?'\\s[neswNESW]$");
  QRegExp degreeDDD("^[0-1]?[0-9]?[0-9].[0-9][0-9]?[0-9]?[0-9]?[0-9]?" + degreeString + "\\s[neswNESW]$");
  QRegExp number("^-?[0-9]+$");

  if (number.indexIn(inDegree) != -1)
    {
      return inDegree.toInt();
    }

  int result = 0;

  if (degreeDMS.indexIn(inDegree) != -1)
    {
      int deg = 0, min = 0, sec = 0;

      QRegExp deg1(degreeString);
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg3("\"");
      sec = inDegree.mid(0, deg3.indexIn(inDegree)).toInt();

      result = (int)rint((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));
    }
  else if ( degreeDDM.indexIn(inDegree) != -1)
    {
      int deg = 0;
      double min = 0;

      QRegExp deg1(degreeString);
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.indexIn(inDegree)).toDouble();
      inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

      result = (int)rint((600000.0 * deg) + (10000.0 * (min)));
    }
  else if ( degreeDDD.indexIn(inDegree) != -1)
    {
      double deg = 0;

      QRegExp deg1(degreeString);
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toDouble();

      result = (int) rint( 600000.0 * deg );
    }
  else
    {
      // @AP: inform the user that something has going wrong
      qWarning("%s(%d) degreeToNum(): Wrong input format %s",
               __FILE__, __LINE__, inDegree.toLatin1().data() );

      return 0; ; // Auweia! That is a pitfall, all is set to zero on error.
    }

  QRegExp dir("[swSW]$");

  if (dir.indexIn(inDegree) >= 0)
    {
      result = -result;
    }

  // qDebug("WGSPoint::degreeToNum(%s)=%d", input.toLatin1().data(), result);

  return result;
}
