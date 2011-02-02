/***********************************************************************
**
**   waypointcatalog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINTCATALOG_H
#define WAYPOINTCATALOG_H

/**
  *@author Harald Maier
  */

class WaypointCatalog
{
public:

  WaypointCatalog(const QString& name = QString::null);

  ~WaypointCatalog();
  /** No descriptions */
  bool read(const QString &catalog);
  /** No descriptions */
  bool readBinary(const QString &catalog);
  /** No descriptions */
  bool readFilserTXT (const QString& catalog);
  /** No descriptions */
  bool writeFilserTXT (const QString& catalog);
  /** No descriptions */
  bool readFilserDA4 (const QString& catalog);
  /** No descriptions */
  bool writeFilserDA4 (const QString& catalog);
  /** read SeeYou cup file, only waypoint part */
  bool readCup (const QString& catalog);
  /** No descriptions */
  bool write();
  /** No descriptions */
  bool writeBinary();
  /** No descriptions */
  bool importVolkslogger(const QString & filename);
  /** Checks if the file exists on disk, and if not asks the user for it.
    * It then calls either write() or writeBinary(),
    * depending on the selected format.
    */
  bool save(bool alwaysAskName=false);
  /** This function calls either read or readBinary depending on the filename of the catalog. */
  bool load(const QString & catalog);
  /** insert a new waypoint into the list and check if waypoint already exist */
  bool insertWaypoint(Waypoint *newWaypoint);
  Waypoint *findWaypoint(QString name);
  bool removeWaypoint(QString name);

  /** filter for display/import */
  bool showAll;
  bool showAirfields;
  bool showGliderfields;
  bool showOtherSites;
  bool showObstacles;
  bool showLandmarks;
  bool showOutlandings;
  bool showStations;

  int areaLat1;
  int areaLat2;
  int areaLong1;
  int areaLong2;
  int radiusLat;
  int radiusLong;

  double radiusSize;

public: // Public attributes
  /**  */
  QList<Waypoint*> wpList;
  QString path;
  bool modified;
  /**  */
  QString catalogName;

private: // Private attributes
  /**  */
  bool onDisc;
};

#endif
