/***********************************************************************
**
**   wp.h
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

#ifndef WP_STRUCT
#define WP_STRUCT

#include <time.h>
#include <qpoint.h>
#include <qstring.h>
#include <qptrcollection.h>
#include <qvaluevector.h>
#include <qlist.h>
#include "mapmatrix.h"

/**
 * This class contains one flightpoint.
 */
class flightPoint
{
public:
  // simple Constructor
  flightPoint(void);
  /** The original position of the point. Given in the internal format. */
  WGSPoint origP;
  /** The projected position of the point. */
  QPoint projP;
  /** The barometrical height, registered by the logger. */
  int height;
  /** The gps-height, registered by the logger. */
  int gpsHeight;
  // Optional: Engine Noise Level
  int engineNoise;
  // Optional: Fix Accuracy
  //float fixAccuracy
  /** The elevation of the surface of the earth beyond this point.
      will be filled when drawn on map */
  int surfaceHeight;
  /** The time, the point was registered by the logger. */
  time_t time;
  /** The elevation difference to the previous Point*/
  int dH;
  /** The time difference to the previous Point*/
  int dT;
  /** The distance between the Points*/
  int dS;
  /** The Bearing to the previous Point */
  float bearing;
  /** Kreisflug 0 oder Streckenflug 1*/
  unsigned int f_state;

};

// A flightPoint vector for fast iteration
typedef QValueVector<flightPoint>  FixVector_TP;
typedef FixVector_TP::iterator         FixVectorIterator_TP;

// A flightPoint list for fast element exchange
typedef QValueList<flightPoint>  FixList_TP;
typedef FixList_TP::iterator         FixListIterator_TP;
typedef QPtrList<flightPoint>       FixPtrList_TP;
typedef QPtrListIterator<flightPoint>      FixPtrListIterator_TP;


#endif
