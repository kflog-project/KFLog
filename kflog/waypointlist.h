/***********************************************************************
**
**   waypointlist.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINTLIST_H
#define WAYPOINTLIST_H

#include "wp.h"

#include <qdict.h>

/**
  *@author Harald Maier
  */
typedef QDict<wayPoint> WaypointListBase;

class WaypointList :public WaypointListBase {
public: 
	WaypointList();
	~WaypointList();
  /** No descriptions */
  bool insertItem(wayPoint *e);
};

#endif
