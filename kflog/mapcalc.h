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
  * Converts the longitute or latitute into the internal format
  * suitable for the Location-class. "degree" is a string in the
  * format: [g]gg.mm'ss"X where g,m,s are any digits from 0 to 9
  * and X is one of N, S, E, W.
  */
int degreeToNum(const char* degree);

/**
  * Convert internal representation to radians.
  */
double numToRad(int internal);

/**
  * Convert radians into internal data.
  */
int radToNum(double radial);

/**
  *
  */
struct drawPoint calcLambert(struct point curPoint,
        int mapCenterLon, double dX, double dY, double scale = 0);

/**
  *
  */
struct drawPoint calcLambert(struct flightPoint curPoint,
        int mapCenterLon, double dX, double dY, double scale = 0);

/**
  *
  */
struct drawPoint calcLambert(int latitude, int longitude,
        int mapCenterLon, double dX, double dY, double scale = 0);

/**
  * Returns the x-coordinate for the Lambert-projection.
  */
double calc_X_Lambert(double latitude, double longitude);

/**
  * Returns the y-coordinate for the Lambert-projection.
  */
double calc_Y_Lambert(double latitude, double longitude);

/**
  * Calculates the geographical position of a given point in the
  * map. The position is stored in "lat" and "lon" in the internal
  * format.
  */
void map2Lambert(double x, double y, struct point* loc);

/**
  * Calculates the geographical latitude of a given point in the
  * map. The position is stored in "lat" and "lon" in the internal
  * format.
  */
int map2LambertLat(double x, double y);

/**
  * Calculates the geographical longitude of a given point in the
  * map. The position is stored in "lat" and "lon" in the internal
  * format.
  */
int map2LambertLon(double x, double y);

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
  * Calculates the polar-coordinates-angle of the given point.
  * ( 0 <= value <= 360° )
  */
double polar(double x, double y);

/**
  * Converts the given coordinate into a readable string.
  * ( xxx,xxxx°[N,S,E,W] )
  */
QString printPos(int coord, bool isLat = true);

/**
  * Converts the given time (in sec.) into a readable string.
  * ( hh:mm:ss )
  */
QString printTime(int time, bool isZero = false);

/** */
int getScaleValue(double value);

/** */
int setScaleValue(int value);

#endif
