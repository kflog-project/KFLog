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

int degreeToNum(const char* degree)
{
  int deg = 0, min = 0, sec = 0;
  int result;
  int count = 0;

  QRegExp number("^-?[0-9]+$");
  QRegExp deg1("[°.]");
  QRegExp deg2(",");
  QRegExp deg3("'");
  QRegExp dir("[swSW]$");
  if(number.match(degree) != -1)
    {
      unsigned int id = 1;
      if(degree[0] == '-')
        {
          result = degree[1] - '0';
          id++;
        }
      else
        result = degree[0] - '0';

      for(unsigned int loop = id ; loop < strlen(degree); loop++)
        result = 10 * result + (degree[loop] - '0');

      if(id == 2) return -result;

      return result;
    }

  switch(deg1.match(degree))
    {
      case 1:
        deg = degree[0] - '0';
        degree += 2;
        break;
      case 2:
        deg = 10 * (degree[0] - '0') + (degree[1] - '0');
        degree += 3;
        break;
      case 3:
        deg = 100 * (degree[0] - '0') + 10 * (degree[1] - '0')
            + (degree[2] - '0');
        degree += 4;
        break;
      default:
        if(deg1.match(degree) > 3)  return 0;    // << degree is not correct!
        switch(strlen(degree))
          {
            case 1:
              deg = degree[0] - '0';
              break;
            case 2:
              deg = 10 * (degree[0] - '0') + (degree[1] -'0');
              break;
            case 3:
              deg = 100 * (degree[0] - '0') + 10 * (degree[1] - '0')
                    + (degree[2] - '0');
              break;
            default:
              return 0;                           // << degree is not correct!
          }
    }

  if( deg2.match(degree) != -1 )
    {
      // Minuten mit Nachkommastellen!
      switch(deg2.match(degree))
        {
          case 1:
            min = degree[0] - '0';
            for(unsigned int loop = 2; loop < strlen(degree); loop++)
              if( ( degree[loop] >= '0' ) && ( degree[loop] <= '9' ) )
                {
                  sec = 10 * sec + (degree[loop] - '0');
                  count++;
                }
            break;
          case 2:
            min = 10 * (degree[0] - '0') + (degree[1] -'0');
            for(unsigned int loop = 3; loop < strlen(degree); loop++)
              if( ( degree[loop] >= '0' ) && ( degree[loop] <= '9' ) )
                {
                  sec = (degree[loop] - '0') + (sec / 10);
                  count++;
                }
            break;
          default:
            if( ( deg2.match(degree) > 2 ) || ( deg2.match(degree) == 0 ) )
                return 0;    // << degree is not correct!
        }
    }
  else if( deg3.match(degree) != -1 )
    {
      // es folgen "echte" Sekunden
      switch( deg3.match(degree) )
        {
          case 1:
            min = degree[0] - '0';
            for(unsigned int loop = 2; loop < strlen(degree); loop++)
              if( ( degree[loop] >= '0' ) && ( degree[loop] <= '9' ) )
                {
                  sec = sec * 10 + (degree[loop] - '0');
                  count++;
                }
            break;
          case 2:
            min = 10 * (degree[0] - '0') + (degree[1] -'0');
            for(unsigned int loop = 3; loop < strlen(degree); loop++)
              if( ( degree[loop] >= '0' ) && ( degree[loop] <= '9' ) )
                {
                  sec = sec * 10 + (degree[loop] - '0');
                  count++;
                }
            break;
          default:
            if( ( deg2.match(degree) > 2 ) || ( deg2.match(degree) == 0 ) )
                return 0;    // << degree is not correct!
        }
    }
  result = (int) ((600000 * deg) + (10000 * (min + (sec * pow(10,-count)))));

  if(dir.match(degree) >= 0) return -result;

  return result;
}

double numToRad(int internal) { return ( PI * internal ) / 108000000.0; }

int radToNum(double radial) { return (int) (radial * 108000000.0 / PI ); }

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

double polar(double x, double y)
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

int setScaleValue(int value)
{
  if(value <= 40) return (value * 5);
  else if(value <= 70) return (200 + (value - 40) * 10);
  else if(value <= 95) return (500 + (value - 70) * 20);
  else if(value <= 105) return (1000 + (value - 95) * 50);
  else return (2000 + (value - 105) * 100);
}

int getScaleValue(double scale)
{
  if(scale <= 200) return ((int) scale / 5);
  else if(scale <= 500) return (((int) scale - 200) / 10 + 40);
  else if(scale <= 1000) return (((int) scale - 500) / 20 + 70);
  else if(scale <= 2000) return (((int) scale - 1000) / 50 + 95);
  else return (((int) scale - 2000) / 100 + 125);
}

float getSpeed(struct flightPoint* p)
{
  return (float)p->dS / (float)p->dT * 3.6;
}

float getSpeed(struct flightPoint p)
{
  return (float)p.dS / (float)p.dT * 3.6;
}

float getVario(struct flightPoint* p)
{
  return (float)p->dH / (float)p->dT;
}

float getVario(struct flightPoint p)
{
  return (float)p.dH / (float)p.dT;
}
