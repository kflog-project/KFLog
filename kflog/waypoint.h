/***********************************************************************
**
**   waypoint.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "wgspoint.h"
#include <time.h>
#include <qstring.h>

/**
  *@short Class to contain waypoints
  *
  * This class is used to store a waypoint.
  *
  *@author Heiner Lamprecht
  *@version $Id$
  */

class Waypoint {
public: 
  Waypoint(QString nam = QString::null, WGSPoint oP = WGSPoint(), int typ = -1, QString _icao = QString::null,
    QString _comment = QString::null, int surf = -1, int runw = -1, int leng = -1, int elev = 0,
    double freq = 0.0, bool isLand = false, QPoint pP = QPoint(), time_t s1 = 0, time_t s2 = 0,
    unsigned int sFAI = 0, double ang = 0.0, double dist = 0.0, QString desc = QString::null, unsigned int import = 2, time_t GPSFixTime=0);
  Waypoint(Waypoint *p);
  Waypoint(Waypoint &p);
  ~Waypoint();
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
   * internal surface id
   */
  int surface;
  /**
   *
   */
  int runway;
  /**
   *
   */
  int length;
  /**
   *
   */
  int elevation;
  /**
   *
   */
  double frequency;
  /**
   * flag for landable
   */
  bool isLandable;
  /**
    * contains an importance indidation for the waypoint
    * 0=low
    * 1=normal
    * 2=high
    */
  unsigned int importance;
  time_t fixTime;
};

#endif
