/***********************************************************************
**
**   waypointcatalog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2001-2023
 *
 * \version 1.1
 */

#pragma once

#include <QList>
#include <QSet>

class QString;
class Waypoint;
class WGSPoint;

class WaypointCatalog
{
public:

  /**
   * Defines the available filters.
   */
  enum FilterType { None=0, Radius, Area };

  WaypointCatalog(const QString& name="");

  virtual ~WaypointCatalog();

  enum FilterType getFilter() const
  {
    return activatedFilter;
  };

  void setFilter( enum FilterType filter )
  {
    activatedFilter = filter;
  };

  /** Reads a KFLog waypoint file in XML format. */
  bool readXml(const QString &catalog);
  /** Reads a KFLog waypoint file in binary format. */
  bool readBinary(const QString &catalog);
  /** No descriptions */
  bool readFilserTXT (const QString& catalog);
  /** No descriptions */
  bool writeFilserTXT (const QString& catalog);
  /** No descriptions */
  bool readFilserDA4 (const QString& catalog);
  /** Reads a SeeYou cup file, only waypoint part */
  bool readCup (const QString& catalog);
  /** Reads a Cambridge Aero Instruments turnpoint file. */
  bool readDat(const QString &catalog);
  /** Reads a Volkslogger waypoint file. */
  bool readVolkslogger(const QString& filename);

  /** Writes a KFLog waypoint file in XML format. */
  bool writeXml();
  /** Writes a KFLog waypoint file in binary format. */
  bool writeBinary();
  /** No descriptions */
  bool writeFilserDA4 (const QString& catalog);
  /** Writes a Cambridge Aero Instruments turnpoint file. */
  bool writeDat(const QString& catalog);
  /** Writes a SeeYou cup file, only waypoint part */
  bool writeCup(const QString& catalog);

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

private:

  /**
   * Splits a cup file line into its single elements.
   *
   * \param line Line to be split.
   *
   * \param ok True if split was ok otherwise false
   *
   * \return A list with the splt elements.
   */
  QList<QString> splitCupLine( QString& line, bool &ok );

public:

  /** filter values for display/import */
  bool showAll;
  bool showAirfields;
  bool showGliderfields;
  bool showNavaids;
  bool showReportings;
  bool showObstacles;
  bool showLandmarks;
  bool showOutlandings;
  bool showHotspots;

  int areaLat1;
  int areaLat2;
  int areaLong1;
  int areaLong2;

  double radiusSize;

  /** Kind of center reference. */
  int centerRef;
  QString airfieldRef;

  /**  Waypoint list belonging to catalog. */
  QList<Waypoint*> wpList;

  /** Full path name of waypoint file. */
  QString path;

  bool modified;

  /**  File name without suffix. */
  QString catalogName;

private: // Private attributes

  /** Activated filter */
  enum FilterType activatedFilter;

  /** Coordinates of center point. */
  WGSPoint centerPoint;

  /**  */
  bool onDisc;

  /** Set of existing catalog pathes. */
  static QSet<QString> catalogSet;
};
