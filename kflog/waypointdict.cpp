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
#include "klineeditdlg.h"

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
  bool OK = false;
  QString newName;
  
  if ((tmp = find(e->name)) != 0) {
    switch (KMessageBox::warningYesNoCancel(0,
                                                                       "<qt>" + i18n("Waypoint<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Do you want to overwrite it or rename the waypoint you are adding?").arg(e->name) + "</qt>",
                                                                       i18n("Add waypoint"),
                                                                        i18n("Overwrite"),
                                                                        i18n("Rename")
                                                                      )) {
    case KMessageBox::Yes:   //overwrite
      remove(e->name);
      insert(e->name, e);
      break;
    case KMessageBox::No:    //rename
       newName=KLineEditDlg::getText(i18n("Waypoint name"), i18n("Please enter a new name for the waypoint"), e->name, &OK);
       if (OK) {
         e->name=newName;
         return insertItem(e); //recursive call!
         break;
       }
       //no break!
    case KMessageBox::Cancel:   //cancel
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
