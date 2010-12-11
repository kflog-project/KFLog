/***********************************************************************
**
**   waypointdict.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINTDICT_H
#define WAYPOINTDICT_H

#include "waypoint.h"

#include <qdict.h>

/**
  *@author Harald Maier
  */
typedef QDict<Waypoint> WaypointDictBase;

class WaypointDict :public WaypointDictBase {
public:
  WaypointDict();
  ~WaypointDict();
  /** No descriptions */
  bool insertItem(Waypoint *e);
};

#endif
