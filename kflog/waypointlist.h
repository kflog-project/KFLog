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

#include "waypointelement.h"

#include <qlist.h>

/**
  *@author Harald Maier
  */

typedef QList<WaypointElement> WaypointListBase;

class WaypointList : public WaypointListBase {
public: 
	WaypointList();
	~WaypointList();
  /** No descriptions */
  virtual int compareItems(QCollection::Item e1, QCollection::Item e2);
  /** No descriptions */
  int insertItem(WaypointElement *e);
};

#endif
