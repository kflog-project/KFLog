/***********************************************************************
**
**   waypointcatalog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WaypointCatalog
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Class for waypoint catalog management.
 *
 * Class for waypoint catalog management. Different file formats can be read
 * and written by this class.
 *
 * \date 2001-2011
 *
 * \version $Id$
 */

#ifndef WAYPOINT_CATALOG_H
#define WAYPOINT_CATALOG_H

#include <QList>
#include <QSet>

class QString;
class Waypoint;
class WGSPoint;

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
  bool importVolkslogger(const QString& filename);
  /** Checks if the file exists on disk, and if not asks the user for it.
    * It then calls either write() or writeBinary(),
    * depending on the selected format.
    */
  bool save(bool alwaysAskName=false);

  /** This function calls either read or readBinary depending on the filename of the catalog. */
  bool load(const QString& catalog);

  /** insert a new waypoint into the list and check if waypoint already exist */
  bool insertWaypoint(Waypoint *newWaypoint);

  /**
   * Find a waypoint by using its name as search key.
   *
   * \param name Name of the waypoint
   *
   * \param index List index of found waypoint or -1 if not existing
   *
   * \return Pointer to found waypoint or NULL
   */
  Waypoint *findWaypoint(const QString& name, int &index );

  bool removeWaypoint(const QString& name);

  /**
   * \return The center point of the radius.
   */
  WGSPoint getCenterPoint();

  /**
   * Sets the radius center point.
   *
   * \param center New center point to be set.
   */
  void setCenterPoint( const WGSPoint& center );

  /** filter values for display/import */
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

  double radiusSize;

  /** Kind of center reference. */
  int centerRef;
  QString airfieldRef;

public:

  /**  Waypoint list belonging to catalog. */
  QList<Waypoint*> wpList;

  /** Full path name of waypoint file. */
  QString path;

  bool modified;

  /**  File name without suffix. */
  QString catalogName;

private: // Private attributes

  /** Coordinates of center point. */
  WGSPoint centerPoint;

  /**  */
  bool onDisc;

  /** Set of existing catalog pathes. */
  static QSet<QString> catalogSet;
};

#endif
