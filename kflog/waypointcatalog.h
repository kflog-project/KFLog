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
  bool writeBinary();
  /** No descriptions */
  bool importVolkslogger(QString & filename);

  /** filter for display/import */
  bool showAll;
  bool showAirports;
  bool showGliderSites;
  bool showOtherSites;
  bool showObstacle;
  bool showLandmark;
  bool showOutlanding;
  bool showStation;

  int areaLat1;
  int areaLat2;
  int areaLong1;
  int areaLong2;
  int radiusLat;
  int radiusLong;

  double radiusSize;

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
