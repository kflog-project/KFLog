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

#include <kmessagebox.h>
#include <klocale.h>

WaypointList::WaypointList()
{
  setAutoDelete(true);
}

WaypointList::~WaypointList()
{
}

/** No descriptions */
int WaypointList::compareItems(QCollection::Item e1, QCollection::Item e2)
{
  return (((WaypointElement *)e1)->name.compare(((WaypointElement *)e2)->name));
}

/** insert a new item into the list, sort inplace and return index of new item
    check if waypoint already exist */
int WaypointList::insertItem(WaypointElement *e)
{
  int idx;
  WaypointElement *tmp;

  if ((idx = find(e)) != -1) {
    switch (KMessageBox::warningYesNoCancel(0, i18n("Waypoint<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Overwrite it?").arg(e->name))) {
    case KMessageBox::Yes:
      tmp = current();
      tmp->name = e->name;
      tmp->description = e->description;
      tmp->type = e->type;
      tmp->pos = e->pos;
      tmp->elevation = e->elevation;
      tmp->icao = e->icao;
      tmp->frequency = e->frequency;
      tmp->runway = e->runway;
      tmp->length = e->length;
      tmp->surface = e->surface;
      tmp->comment = e->comment;
      break;
    case KMessageBox::Cancel:
      idx = -1;
      break;
    }
  }
  else {
    inSort(e);
    idx = findRef(e);
  }

  return idx;
}
