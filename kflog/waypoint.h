/***********************************************************************
**
**   waypoint.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <time.h>

#include <QPair>
#include <QPoint>
#include <QString>

#include "runway.h"
#include "wgspoint.h"

/**
  * @short Class to contain waypoints
  *
  * This class is used to store a waypoint.
  *
  * @author Heiner Lamprecht, Axel Pauli
  *
  * @date 2003-2011
  *
  * @version $Id$
  */

class Waypoint
{
public:

  Waypoint( QString nam = QString::null,
            WGSPoint oP = WGSPoint(),
            int typ = -1,
            QString _icao = QString::null,
            QString _comment = QString::null,
            QString _country = QString::null,
            enum Runway::SurfaceType surf=Runway::Unknown,
            QPair<ushort, ushort> runw = (QPair<ushort, ushort>(0, 0)),
            float leng = -1.0,
            float elev = 0.0,
            float freq = 0.0,
            bool isLand = false,
            QPoint pP = QPoint(),
            time_t s1 = 0,
            time_t s2 = 0,
            unsigned int sFAI = 0,
            double ang = 0.0,
            double dist = 0.0,
            QString desc = QString::null,
            unsigned short import = 2,
            time_t GPSFixTime=0 );

  Waypoint(Waypoint *p);
  Waypoint(Waypoint &p);
  ~Waypoint();
  bool operator<(Waypoint &wp);
  /**
   * The name of the waypoint.
   */
  QString name;
  /**
   * The original lat/lon-position of the waypoint.
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
   * Country where waypoint is located.
   */
  QString country;
  /**
   * internal surface id
   */
  enum Runway::SurfaceType surface;
  /**
   *
   */
  QPair<ushort, ushort> runway;
  /**
   *
   */
  float length;
  /**
   *
   */
  float elevation;
  /**
   *
   */
  float frequency;
  /**
   * flag for landable
   */
  bool isLandable;
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
