/***********************************************************************
**
**   waypointdict.cpp
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
#include <qmessagebox.h>

#include "waypointdict.h"

WaypointDict::WaypointDict()
{
  setAutoDelete(true);
}

WaypointDict::~WaypointDict()
{
}

bool notEqual (const Waypoint& wp1, const Waypoint& wp2)
{
  return
    wp1.name != wp2.name ||
    wp1.angle != wp2.angle ||
    wp1.comment != wp2.comment ||
    wp1.description != wp2.description ||
    wp1.distance != wp2.distance ||
    wp1.elevation != wp2.elevation ||
    wp1.fixTime != wp2.fixTime ||
    wp1.frequency != wp2.frequency ||
    wp1.icao != wp2.icao ||
    wp1.importance != wp2.importance ||
    wp1.isLandable != wp2.isLandable ||
    wp1.length != wp2.length ||
    wp1.origP != wp2.origP ||
    wp1.runway != wp2.runway ||
    wp1.type != wp2.type ||
    wp1.surface != wp2.surface;
}

/** insert a new item into the list and check if waypoint already exist */
bool WaypointDict::insertItem(Waypoint *e)
{
  Waypoint *tmp;
  bool ins = true;
  bool OK = false;
  QString newName;
  
  if (tmp = find(e->name))
  {
    if (notEqual (*tmp, *e))
    {
      switch (QMessageBox::warning(0, "Waypoint exists",
            "<qt>" + QObject::tr("A waypoint with the name<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Do you want to overwrite the existing waypoint?").arg(e->name) + "</qt>",
            QObject::tr("Cancel"),
            QObject::tr("&Overwrite")
           )) {
      case QMessageBox::Abort:   //cancel
        delete e;
        ins = false;
        break;
      case QMessageBox::Ok:      //overwrite, old version
        remove(e->name);
        insert(e->name, e);
        break;
      }
    }
  }
  else {
    insert(e->name, e);
  }

  return ins;
}

