/***********************************************************************
**
**   waypoint.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by Heiner Lamprecht
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <ctime>

#include <QList>
#include <QPoint>
#include <QString>

#include "Frequency.h"
#include "runway.h"
#include "wgspoint.h"

/**
  * @short Class to contain waypoint data.
  *
  * This class is used to store a waypoint.
  *
  * @author Heiner Lamprecht, Axel Pauli
  *
  * @date 2003-2023
  *
  * @version 1.2
  */

class Waypoint
{
 public:

  /**
   * contains an priority indication for a waypoint
   */
  enum Priority { Low=0, Normal=1, High=2, Top=3 };

  Waypoint() :
    sector1(0),
    sector2(0),
    sectorFAI(0),
    angle(0.0),
    type(-1),
    tpType(0),
    distance(0.0),
    elevation(0.0),
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

  Waypoint& operator=(const Waypoint& wp) = default;

  virtual ~Waypoint();

  bool operator<(Waypoint &wp);

  bool operator==( const Waypoint& second ) const;

  /**
   * @return The frequency list as reference.
   */
  QList<Frequency>& getFrequencyList()
    {
      return frequencyList;
    }

  /**
   * Sets a new frequency list.
   */
  void setFequencyList( QList<Frequency>& fqList )
  {
    frequencyList = fqList;
  }

  /**
   * Adds a new frequency to the frequency list.
   *
   * @param freq The frequency and its type.
   */
  void addFrequency( const Frequency& frequencyAndType )
    {
      frequencyList.append( frequencyAndType );
    }

  /**
   * @return The runway list as reference.
   */
  QList<Runway>& getRunwayList()
    {
      return rwyList;
    }

  /**
   * Sets a new runway list.
   */
  void setRunwayList( QList<Runway>& runwayList )
  {
    rwyList = runwayList;
  }

  /**
   * Adds a new runway to the runway list.
   *
   * @param rwy The runway to be added.
   */
  void addRunway( const Runway& runway )
    {
      rwyList.append( runway );
    }

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
   * The distance to the previous waypoint in kilometers
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
   * A list of runways is managed by the airfield object. If a waypoint is
   * derived temporary from an airfield object, these data is also taken over.
   */
  QList<Runway> rwyList;

  /**
   * Elevation of waypoint.
   */
  float elevation;

  /**
  * All speech frequencies with type of the airfield.
  */
  QList<Frequency> frequencyList;

  /**
    * contains an importance indication for the waypoint
    * 0=low
    * 1=normal
    * 2=high
    */
  unsigned short importance;

  time_t fixTime;
};
