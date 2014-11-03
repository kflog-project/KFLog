/***********************************************************************
**
**   elevationfinder.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c): 2004 by André Somers <andre@kflog.org>
**                  2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef ELEVATION_FINDER_H
#define ELEVATION_FINDER_H

#include <QObject>
#include <QPoint>
#include <QTimer>
#include <QFile>
#include <QDataStream>

/**
 * \class ElevationFinder
 *
 * \author André Somers, Axel Pauli
 *
 * \short Finds the ground elevation for a given coordinate.
 *
 * This singleton class can use multiple (well, currently two) methods
 * to find the elevation for a given coordinate. 
 * 1. Using a DEM file configured for openGLIGCexplorer
 * 2. Using the internal isohypses
 * The preferred method is number 1, as it is much faster and more detailed
 * in comparison to the latter. 
 *
 * \date 2004-2011
 *
 * \version $Id$
 */
class ElevationFinder : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( ElevationFinder )

  /**
   * This class is a singleton, therefore the constructor is made private.
   * Use method \ref instance to get a class instance.
   */
  ElevationFinder(QObject *parent = 0);

 public:

  virtual ~ElevationFinder();
  /**
   * Find an elevation.
   * @returns Elevation in meters or -1 if no valid result was found.
   * @args coordinates Coordinate of the point in wgs format, that is, unprojected.
   */
  int elevationWgs(const QPoint& coordinates);
  /**
   * Find an elevation.
   * @returns Elevation in meters or -1 if no valid result was found.
   * @args coordinates Coordinate of the point in map projection format.
   */
  int elevationMap(const QPoint& coordinates);
  /**
   * Find an elevation.
   * @returns Elevation in meters or -1 if no valid result was found.
   * @args WgsCoordinates Coordinate of the point in wgs format, that is, unprojected.
   * @args MapCoordinates Coordinate of the point in map projection format.
   * Both arguments should represent the same point. The format that results in the
   * fastest lookup for the method in use will be used. This is the preferred
   * method of getting the elevation.
   */
  int elevation(const QPoint& WgsCoordinates, const QPoint& MapCoordinates);
  /**
   * @returns A pointer to the instance of the object to use. Use only this static
   * method to get an ElevationFinder object!
   */
  static ElevationFinder *instance();
  /**
   * @returns true if KFLog we rely on the isohypses to find the elevation for a position.
   */
  bool useIsohypseForElevation() {return !useOGIE;};

private slots:

  void timeout();

private:

  int findDEMelevation(const QPoint&);
  bool tryOpenGLIGCexplorer();

  bool useOGIE;

  //name of DEM (Digital Elevation Model) file
  QString demFileName;
  //top left coordinate of Dem file
  QPoint demTL;
  //bottom right coordinate of Dem file
  QPoint demBR;
  int demRows;
  int demCols;
  int demGridLat;
  int demGridLon;
  QTimer * timer;
  QFile * demFile;
  QDataStream * demStream;
};

#endif
