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
#include <mapcalc.h>
#include <qregexp.h>

#include <iostream>

double dist(double lat1, double lon1, double lat2, double lon2)
{
  double pi_180 = PI / 108000000.0;
  double dlat = lat1 - lat2;
  double dlon = lon1 - lon2;

  double dist = RADIUS * sqrt( ( pi_180 * dlat * pi_180 * dlat )
    + ( pi_180 * cos( pi_180 * lat1 ) * dlon *
        pi_180 * cos( pi_180 * lat1 ) * dlon ) );

  return dist / 1000.0;
}

double dist(struct wayPoint* wp1, struct wayPoint* wp2)
{
  return ( dist( wp1->origP.x(), wp1->origP.y(),
                 wp2->origP.x(), wp2->origP.y() ) );
}

double dist(struct wayPoint* wp, struct flightPoint* fp)
{
  return ( dist( wp->origP.x(), wp->origP.y(),
                 fp->origP.x(), fp->origP.y() ) );
}

double dist(struct flightPoint* fp1, struct flightPoint* fp2)
{
  return ( dist( fp1->origP.x(), fp1->origP.y(),
                 fp2->origP.x(), fp2->origP.y() ) );
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
  if(min < 10)  posMin.sprintf(" 0%d'", min);
  else  posMin.sprintf(" %d'", min);

  sec = (int)sqrt(sec * sec);
  if(sec < 10)  posSec.sprintf(" 0%d\"", sec);
  else  posSec.sprintf(" %d\"", sec);

  if(isLat)
    {
      if(coord < 0)
        {
          posDeg.sprintf("%d°", -degree);
          pos = posDeg + posMin + posSec + " S";
        }
      else
        {
          posDeg.sprintf("%d°", degree);
          pos = posDeg + posMin + posSec + " N";
        }
    }
  else
    {
      if(coord < 0)
        {
          posDeg.sprintf("%d°", -degree);
          pos = posDeg + posMin + posSec + " W";
        }
      else
        {
          posDeg.sprintf("%d°", degree);
          pos = posDeg + posMin + posSec + " E";
        }
    }

  return pos;
}

QString printTime(int time, bool isZero)
{
  QString hour, min, sec;

  int hh = time / 3600;
  int mm = (time - (hh * 3600)) / 60;
  int ss = time - (hh * 3600) - mm * 60;

  if(isZero && hh < 10)  hour.sprintf("0%d", hh);
  else  hour.sprintf("%d", hh);

  if(mm < 10)  min.sprintf("0%d", mm);
  else  min.sprintf("%d", mm);

  if(ss < 10)  sec.sprintf("0%d", ss);
  else  sec.sprintf("%d", ss);

  return (hour + ":" + min + ":" + sec);
}
