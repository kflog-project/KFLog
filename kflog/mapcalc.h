/***********************************************************************
**
**   mapcalc.h
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

#ifndef MAP_CALC_H
#define MAP_CALC_H

#include "wp.h"
#include "waypoint.h"
#include <time.h>

#define PI2 M_PI*2

/**
 * The earth's radius used for calculation, given in Meters
 * NOTE: We use the earth as a sphere, not as a spheroid!
 */
#define RADIUS 6371000 // FAI Radius, this was the prevoius radius ->6370290
#define RADIUS_kfl (RADIUS / (360.0 * 600000.0))

// Define nautical mile in meters according to earth radius of KFL
#define MILE_kfl  (PI2 * RADIUS / (360.0 * 60.0))

/**
 * Calculates the distance between two given points according to great circle in km.
 */
double distG(double lat1, double lon1, double lat2, double lon2);

double distP(double lat1, double lon1, double lat2, double lon2);

double dist(double lat1, double lon1, double lat2, double lon2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(QPoint* p1, QPoint* p2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(Waypoint* wp1, Waypoint* wp2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(Waypoint* wp, flightPoint* fp);

/**
 * Calculates the distance between two given points (in km).
 */
double dist( flightPoint* fp1, flightPoint* fp2);

/**
 * Converts the given time (in sec. from 1.1.1970 00:00:00) into a readable string.
 * ( hh:mm:ss )
 */
QString printTime(time_t time, bool isZero = false, bool isSecond = true);

// Conversion for time delays
QString printTime(int time, bool isZero = false, bool isSecond = true, bool noZeroHour = false);

// Compute the time_t equivalent from  year [1970 - ...], month  [ 1-12], day [1-31]
time_t timeToDay(const int year, const int month, const int day, const char *monabb = NULL);

/**
 * Calculates the vario of a given point
 */
float getVario(flightPoint p);

/**
 * Calculates the speed of a given point
 */
float getSpeed(flightPoint p);

/**
 * Calculates the bearing to the previous point
 */
float getBearing(flightPoint p1, flightPoint p2);

/**
 * Converts a x/y position into a polar-coordinate.
 */
double polar(double x, double y);

/* convert internal point into radians */
double int2rad(int deg);

/* convert radians into internal point */
double rad2int(double rad);

/* calculate true course in radians to point 2 */
double tc(double lat1, double lon1, double lat2, double lon2);

/* calculate angle between leg a and b in radians */
double angle(double a, double b, double c);

/* calculate pos of point from dist and bearing */
WGSPoint posOfDistAndBearing(double lat1, double lon1, double bearing, double dist);

/* calculate true course from prev point (p2) */
double getTrueCourse(WGSPoint p1, WGSPoint p2);

/**
 * Calculates the direction of the vector pointing to the outside
 * of the area spanned by the two vectors.
 */
double outsideVector(QPoint center, QPoint p1, QPoint p2);
double outsideVector(double angle1, double angle2);

double normalize(double angle);

int normalize(int angle);

/**
  * Calculates the difference between two angles, returning the smallest
  * angle. It returns an angle between -180 and 180 in degrees. Positive
  * in clockwise direction.
  */
int angleDiff(int ang1, int ang2);

/**
  * Calculates the difference between two angles, returning the smallest
  * angle. It returns an angle between -Pi and Pi in rad.
  */
double angleDiff(double ang1, double ang2);

/**
 * Calculates a (crude) bounding box that contains the circle of radius @arg r
 * around point @arg center. @arg r is given in kilometers.
 */
QRect areaBox(QPoint center, double r);

/**
 * Calculates the bounding box of the given tile number in KFLog coordinates.
 * The returned rectangle used the x-axis as longitude and the y-axis as latitude.
 */
QRect getTileBox(const ushort tileNo);

/**
 * Calculates the map tile number from the passed coordinate. The coordinate
 * format is decimal degree. Positive numbers are N and E, negative numbers
 * are W and S.
 *
 * @param lat Latitude in decimal degree. 90...-90
 * @param lon Longitude in decimal degree. -180...180
 * @return map tile number 0...16199
 */
int mapTileNumber( double lat, double lon );

#endif
