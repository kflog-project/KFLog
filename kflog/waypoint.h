/***********************************************************************
**
**   waypoint.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by Heiner Lamprecht
**                   2011-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <ctime>

#include <QPair>
#include <QPoint>
#include <QString>

#include "runway.h"
#include "wgspoint.h"

/**
  * @short Class to contain waypoint data.
  *
  * This class is used to store a waypoint.
  *
  * @author Heiner Lamprecht, Axel Pauli
  *
  * @date 2003-2013
  *
  * @version $Id$
  */

class Waypoint
{

 public:

  Waypoint() :
    sector1(0),
    sector2(0),
    sectorFAI(0),
    angle(0.0),
    type(-1),
    tpType(0),
    distance(0.0),
    elevation(0.0),
    frequency(0.0),
    importance(2),
    fixTime(0)
    {
    };

  Waypoint( QString name,
            WGSPoint oP,
            int typ,
            int tpType,
            QString _icao,
            QString _comment,
            QString _country,
            float elev = 0.0,
            float freq = 0.0,
            QPoint pP = QPoint(),
            time_t s1 = 0,
            time_t s2 = 0,
            unsigned int sFAI = 0,
            double ang = 0.0,
            double dist = 0.0,
            QString desc = "",
            unsigned short import = 2,
            time_t GPSFixTime=0 );

  Waypoint(Waypoint *p);

  Waypoint(Waypoint &p);

  virtual ~Waypoint();

  bool operator<(Waypoint &wp);

  bool operator==( const Waypoint& second ) const;

  /**
   * The name of the waypoint.
   */
  QString name;
  /**
   * The original WGS84 lat/lon-position of the waypoint.
   */
  WGSPoint origP;
  /**
   * The projected position of the waypoint.
   */
  QPoint projP;
  /**
   * The time, sector 1 has been reached.
   */
  time_t sector1;
  /**
   * The time, sector 2 has been reached.
   */
  time_t sector2;
  /**
   * The time, the fai-sector has been reached.
   */
  time_t sectorFAI;
  /**
   * The angle for the sector
   */
  double angle;
  /**
   * The type of the waypoint
   */
  int type;
  /**
   * The type as taskpoint
   */
  int tpType;
  /**
   * The distance to the previous waypoint
   */
  double distance;

  /** Improvements for planning */
  /**
   * long name or description (internal only)
   */
  QString description;

  /**
   * ICAO name
   */
  QString icao;

  /**
   * Comment
   */
  QString comment;

  /**
   * Country as two letter code where waypoint is located.
   */
  QString country;

  /**
   * A list of runways is managed by the waypoint object.
   */
  QList<Runway> rwyList;

  /**
   * Elevation of waypoint.
   */
  float elevation;

  /**
   * Frequency of waypoint.
   */
  float frequency;

  /**
    * contains an importance indication for the waypoint
    * 0=low
    * 1=normal
    * 2=high
    */
  unsigned short importance;

  time_t fixTime;
};

#endif
