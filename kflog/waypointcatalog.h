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

#include "waypointdict.h"

/**
  *@author Harald Maier
  */

class WaypointCatalog {
public: 
	WaypointCatalog();
	~WaypointCatalog();
  /** No descriptions */
  bool read(QString &catalog);
  bool readBinairy(QString &catalog);
  /** No descriptions */
  bool write();
  bool writeBinairy();
  /** No descriptions */
  bool importVolkslogger(QString & filename);
  /** Checks if the file exists on disk, and if not asks the user for it.
    * It then calls either write() or writeBinary(),
    * depending on the selected format.
    */
  bool save(bool alwaysAskName=false);
  /** This function calls either read or readBinairy depending on the filename of the catalog. */
  bool load(QString & catalog);

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
  WaypointDict wpList;
  QString path;
  bool modified;
private: // Private attributes
  /**  */
  bool onDisc;
};

#endif
