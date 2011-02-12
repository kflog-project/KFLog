/***********************************************************************
**
**   wp.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WP_STRUCT
#define WP_STRUCT

#include <time.h>
#include <qpoint.h>
#include <qstring.h>
#include <q3ptrcollection.h>
#include <q3valuevector.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3ValueList>
#include "mapmatrix.h"

/**
 * This class contains one flight point.
 */
class flightPoint
{
public:
  // simple Constructor
  flightPoint(void);

  /** The original position of the point. Given in the internal format.
  *   set by flightloader.cpp */
  WGSPoint origP;

  /** The projected position of the point.
  *   set by flightloader.cpp */
  QPoint projP;

  /** The barometrical height, registered by the logger.
  *   set by flightloader.cpp */
  int height;

  /** The gps-height, registered by the logger.
  *   set by flightloader.cpp */
  int gpsHeight;

  /** Optional: Engine Noise Level
  *   set by flightloader.cpp */
  int engineNoise;

  // Optional: Fix Accuracy
  //float fixAccuracy

  /** The elevation of the surface of the earth beyond this point.
  *   will be filled when drawn on map
  *   set by flightloader.cpp */
  int surfaceHeight;

  /** The time, the point was registered by the logger.
  *   set by flightloader.cpp */
  time_t time;

  /** The elevation difference to the previous Point
  *   set by flight.cpp */
  int dH;

  /** The time difference to the previous Point
  *
  *   dT may change, even if the interval, in which the
  *   logger gets the position, is always the same. If the
  *   interval is f.e. 10 sec, dtime may change to 11 or 9 sec.
  *
  *   In some files the time of the current and previous point
  *   are the same. In this case we set dT = 1 to avoid
  *   a floating-point-exception ...
  *   set by flight.cpp */
  int dT;

  /** The distance between the previous and current point
  *   set by flight.cpp */
  int dS;

  /** Bearing (the average of the bearing of the line to and from this point)
  *   set by flight.cpp */
  float bearing;

  /** The difference in bearing of the previous and current point
  *   set by flight.cpp */
  float dBearing;

  /** Straight = 0, LeftTurn = 1, RightTurn = 2, MixedTurn = 3
  *   set by flight.cpp */
  unsigned int f_state;

};

// A flightPoint vector for fast iteration
typedef Q3ValueVector<flightPoint>  FixVector_TP;
typedef FixVector_TP::iterator         FixVectorIterator_TP;

// A flightPoint list for fast element exchange
typedef Q3ValueList<flightPoint>  FixList_TP;
typedef FixList_TP::iterator         FixListIterator_TP;
typedef Q3PtrList<flightPoint>       FixPtrList_TP;
typedef Q3PtrListIterator<flightPoint>      FixPtrListIterator_TP;


#endif
