/***********************************************************************
**
**   waypoint.cpp
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

#include "flight.h"
#include "waypoint.h"

Waypoint::Waypoint( QString nam,
                    WGSPoint oP,
                    int typ,
                    QString _icao,
                    QString _comment,
                    QString _country,
                    enum Runway::SurfaceType surf,
                    QPair<ushort, ushort> runw,
                    float leng,
                    float elev,
                    float freq,
                    bool isLand,
                    QPoint pP,
                    time_t s1,
                    time_t s2,
                    unsigned int sFAI,
                    double ang,
                    double dist,
                    QString desc,
                    unsigned short import,
                    time_t GPSFixTime)
{
  name = nam;
  origP = oP;
  projP = pP;
  sector1 = s1;
  sector2 = s2;
  sectorFAI = sFAI;
  angle = ang;
  type = typ;
  distance = dist;
  description = desc;
  icao = _icao;
  comment = _comment;
  country = _country;
  surface = surf;
  runway = runw;
  length = leng;
  elevation = elev;
  frequency = freq;
  isLandable = isLand;
  importance = import;
  fixTime = GPSFixTime;
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
    return name<wp.name;
}
