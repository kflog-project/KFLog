/***********************************************************************
**
**   waypointlist.cpp
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

#include "waypointlist.h"


WaypointList::WaypointList()
{
}

WaypointList::~WaypointList()
{
}

int WaypointList::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
  return (((struct wayPoint *)item1)->name.compare(((struct wayPoint *)item2)->name));
}
