/***********************************************************************
**
**   waypointcatalog.h
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

#ifndef WAYPOINTCATALOG_H
#define WAYPOINTCATALOG_H

#include "waypointlist.h"

/**
  *@author Harald Maier
  */

class WaypointCatalog {
public: 
	WaypointCatalog();
	~WaypointCatalog();
  /** No descriptions */
  bool read(QString &catalog);
  /** No descriptions */
  bool write();
public: // Public attributes
  /**  */
  WaypointList wpList;
  QString path;
  bool modified;
  /**  */
  bool onDisc;
private: // Private attributes
};

#endif
