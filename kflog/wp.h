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

/**
 * This struct contains one point. The struct is used to represent the
 * Cartesian-coordinates. The resolution used in KFLog is 1/10,000th of a
 * minute (approx. 0.1852m on a great circle).
 * <P>
 * We decided to use minutes
 * instead of seconds as base unit, because our GPS-Logger used
 * them likewise ;-)
 * </P>
 * <P>
 * <TABLE>
 * <TR><TD>Therefore:</TD>
 *     <TD>1 second</TD><TD>=</TD><TD ALIGN="right"><TT>~167</TT></TD></TR>
 * <TR><TD></TD>
 *     <TD>1 minute</TD><TD>=</TD><TD ALIGN="right"><TT>10,000</TT></TD></TR>
 * <TR><TD></TD>
 *     <TD>1 degree</TD><TD>=</TD><TD ALIGN="right"><TT>600,000</TT></TD></TR>
 * </TABLE>
 * </P>
 * <P>
 * The Earth's circumference (1 degree * 360) is 216,000,000, so
 * we can use 32 bit integer vars to represent these values!
 * Positive values mean North or East and negative values South
 * or West.
 * </P>
 */
struct point
{
  /** The latitude ;-) */
  int latitude;
  /** The longitude ;-) */
  int longitude;
};

/**
 *
 */
struct elementBorder
{
  int north;
  int east;
  int west;
  int south;
};

/**
 * Contains the two coordinates for drawing the point into the map.
 */
struct drawPoint
{
  /** */
  double x;
  /** */
  double y;
};

/**
 * This struct contains one flightpoint.
 */
struct flightPoint
{
  /** The original position of the point. Given in the internal format. */
  QPoint origP;
  /** The projected position of the point. */
  QPoint projP;
  /** */
  QPoint drawP;
  /** The barometrical height, registered by the logger. */
  int height;
  /** The gps-height, registered by the logger. */
  int gpsHeight;
  /** The time, the point was registered by the logger. */
  unsigned int time;
  /** The elevation difference to the previous Point*/
  int dH;
  /** The time difference to the previous Point*/
  int dT;
  /** The distance between the Points*/
  int dS;

  int latitude;
  int longitude;
  int dh;
  int dt;
  int distance;
};

/**
 * This struct contains the data of one waypoint given to the logger.
 */
struct wayPoint
{
  /** The name of the waypoint. */
  QString name;
  /** The original position of the waypoint. */
  QPoint origP;
  /** The projected position of the waypoint. */
  QPoint projP;
  /** The latitude ;-) */
  int latitude;
  /** The longitude ;-) */
  int longitude;
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
};

#endif
