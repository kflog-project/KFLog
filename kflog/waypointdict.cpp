/***********************************************************************
**
**   waypointdict.cpp
**
**   This file is part of KFLog2.
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
#include <kmessagebox.h>
#include <klocale.h>

#include "waypointdict.h"

WaypointDict::WaypointDict()
{
  setAutoDelete(true);
}

WaypointDict::~WaypointDict()
{
}

/** insert a new item into the list and check if waypoint already exist */
bool WaypointDict::insertItem(Waypoint *e)
{
  Waypoint *tmp;
  bool ins = true;

  if ((tmp = find(e->name)) != 0) {
    switch (KMessageBox::warningYesNoCancel(0, "<qt>" + i18n("Waypoint<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Overwrite it?").arg(e->name) + "</qt>")) {
    case KMessageBox::Yes:
      remove(e->name);
      insert(e->name, e);
      break;
    case KMessageBox::No:
      // no break !!
    case KMessageBox::Cancel:
      delete e;
      ins = false;
      break;
    }
  }
  else {
    insert(e->name, e);
  }

  return ins;
}
