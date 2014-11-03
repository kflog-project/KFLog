/***********************************************************************
**
**   waypoint.cpp
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

#include "flight.h"
#include "waypoint.h"

Waypoint::Waypoint( QString name,
                    WGSPoint oP,
                    int typ,
                    int _tpType,
                    QString _icao,
                    QString _comment,
                    QString _country,
                    float elev,
                    float freq,
                    QPoint pP,
                    time_t s1,
                    time_t s2,
                    unsigned int sFAI,
                    double ang,
                    double dist,
                    QString desc,
                    unsigned short import,
                    time_t GPSFixTime ) :
  name(name),
  origP(oP),
  projP(pP),
  sector1(s1),
  sector2(s2),
  sectorFAI(sFAI),
  angle(ang),
  type(typ),
  tpType(_tpType),
  distance(dist),
  description(desc),
  icao(_icao),
  comment(_comment),
  country(_country),
  elevation(elev),
  frequency(freq),
  importance(import),
  fixTime(GPSFixTime)
{
}

Waypoint::~Waypoint()
{
}

Waypoint::Waypoint(Waypoint *p)
{
  *this = *p;
}

Waypoint::Waypoint(Waypoint &p)
{
  *this = p;
}

bool Waypoint::operator<(Waypoint &wp)
{
  return name < wp.name;
}

bool Waypoint::operator==( const Waypoint& second ) const
{
  if( name == second.name &&
      type == second.type &&
      description == second.description &&
      origP == second.origP )
    {
      return true;
    }

  return false;
}
