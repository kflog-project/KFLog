/***********************************************************************
**
**   mapcalc.h
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

#ifndef MAPCALC_H
#define MAPCALC_H

#include <wp.h>

#define PI 3.141592654

/**
  * The earth's radius used for calculation, given in Meter.
  * NOTE: We use the earth as a sphere, not as a spheroid!
  */
#define RADIUS 6370289.509

/**
  * Calculates the distance between two given points (in km).
  */
double dist(double lat1, double lon1, double lat2, double lon2);

/**
  * Calculates the distance between two given points (in km).
  */
double dist(struct wayPoint* wp1, struct wayPoint* wp2);

/**
  * Calculates the distance between two given points (in km).
  */
double dist(struct wayPoint* wp, struct flightPoint* fp);

/**
  * Calculates the distance between two given points (in km).
  */
double dist(struct flightPoint* fp1, struct flightPoint* fp2);

/**
  * Converts the given coordinate into a readable string.
  * ( xxx,xxxx°[N,S,E,W] )
  */
QString printPos(int coord, bool isLat = true);

/**
  * Converts the given time (in sec.) into a readable string.
  * ( hh:mm:ss )
  */
QString printTime(int time, bool isZero = false, bool isSecond = true);

/**
  * Calculates the Vario of a given Point
  */
float getVario(struct flightPoint* p);
float getVario(struct flightPoint p);
/**
  * Calculates the Speed of a given Point
  */
float getSpeed(struct flightPoint* p);
float getSpeed(struct flightPoint p);

/**
  * Calculates the Bearing to the previous Point
  */
float getBearing(struct flightPoint p1, struct flightPoint p2);
double __polar(double x, double y);

#endif
