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

#include <qpoint.h>
#include <qstring.h>
#include <qptrcollection.h>
#include "mapmatrix.h"

/**
 * This struct contains one flightpoint.
 */
struct flightPoint
{
  /** The original position of the point. Given in the internal format. */
  WGSPoint origP;
  /** The projected position of the point. */
  QPoint projP;
  /** The barometrical height, registered by the logger. */
  int height;
  /** The gps-height, registered by the logger. */
  int gpsHeight;
  /** The elevation of the surface of the earth beyond this point.
      will be filled when drawn on map */
  int surfaceHeight;
  /** The time, the point was registered by the logger. */
  unsigned int time;
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

/**
 * This struct contains the data of one waypoint given from the logger.
 */
struct wayPoint_
{
  /** The name of the waypoint. */
  QString name;
  /** The original lat/lon-position of the waypoint. */
  WGSPoint origP;
  /** The projected position of the waypoint. */
  QPoint projP;
  /** The time, sector 1 has been reached. */
  unsigned int sector1;
  /** The time, sector 2 has been reached. */
  unsigned int sector2;
  /** The time, the fai-sector has been reached. */
  unsigned int sectorFAI;
  /** The angle for the sector */
  double angle;
  /** The type of the waypoint */
  int type;
  /** The distance to the previous waypoint */
  double distance;

  /** Improvements for planning */
  /** long name or description (internal only) */
  QString description;
  /** ICAO name */
  QString icao;
  /** */
  QString comment;
  /** internal surface id */
  int surface;
  /** */
  int runway;
  /** */
  int length;
  /** */
  int elevation;
  /** */
  double frequency;
  /** flag for landable*/
  bool isLandable;
  /** contains an importance indidation for the waypoint
    * 0=low
    * 1=normal
    * 2=high  */
  unsigned int importance;
};

#endif
