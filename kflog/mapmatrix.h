/***********************************************************************
**
**   mapmatrix.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPMATRIX_H
#define MAPMATRIX_H

#include <qwmatrix.h>

/**
 * This class provides functions for converting coordinates between
 * several coordinate-systems.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapMatrix
{
  public:
    /**
     *
     */
    MapMatrix();
    /**
     *
     */
    ~MapMatrix();
    /**
     * Converts the given geographic-data into the current
     * maptype.
     */
    QPoint wgsToMap(QPoint) const;
    /**
     * Converts the given geographic-data into the current
     * maptype.
     */
    QPoint wgsToMap(int lat, int lon) const;
    /** */
    QRect wgsToMap(QRect rect);
    /**
     *
     */
    QPointArray map(QPointArray) const;
    /** */
    QPoint map(QPoint) const;
    /** */
    double map(double) const;
    /**
     *
     */
    void createMatrix(QSize newSize);
    /** */
    double scale() const;
    /** */
    QRect getViewBorder() const;
    /** */
    void initMatrix(int centerLat, int centerLon, double scale,
        double v1, double v2, int hLat, int hLon);
    /** */
    void scaleAdd(QSize mapSize);
    /** */
    void scaleSub(QSize mapSize);
    /** */
    bool isVisible(QPoint pos) const;
    /** */
    bool isVisible(QRect itemBorder) const;
    /** */
    enum MoveDirection {NotSet = 0, North = 1, West = 2, East = 4,
        South = 8, Home = 16};
    /** */
    void moveMap(int dir);
    /** */
    void centerToMouse(QPoint);
    /** */
    void centerToRect(QRect);
    /** */
    QPoint mapToWgs(QPoint pos) const;
    /** */
    void setScale(double);

  private:
    /**
     * Returns the x-coordinate for the Lambert-projection.
     */
    double __calc_X_Lambert(double latitude, double longitude) const;
    /**
     * Returns the y-coordinate for the Lambert-projection.
     */
    double __calc_Y_Lambert(double latitude, double longitude) const;
    /**
     * Returns the latitude of the given map-point
     */
    int __invert_Lambert_Lat(double x, double y) const;
    /**
     * Returns the longitude of the given map-point
     */
    int __invert_Lambert_Lon(double x, double y) const;
    /**
     */
    QPoint __mapToWgs(QPoint) const;
    /**
     */
    QPoint __mapToWgs(int x, int y) const;
    /**
     */
    QWMatrix worldMatrix;
    /**
     *
     */
    double v1;
    /**
     *
     */
    double v2;
    double var1;
    double var2;
    /**
     * The mapCenter is the position displayed in the center of the map.
     * It is used in two different ways:
     * 1.: Determine the area shown in the map-widget
     * 2.: Calculating the difference in latitude between a point in the
     * map and the center.
     *
     * The latitude of the center of the map.
     */
    int mapCenterLat;
    /**
     * The longitude of the center of the map.
     */
    int mapCenterLon;
    /** */
    int homeLat;
    /** */
    int homeLon;
    /**
     * Contains the geographical border of the map (lat/lon).
     */
    QRect viewBorder;
    QRect mapBorder;
    /** */
    QSize mapViewSize;
    /** */
    double cScale;
    /** */
    double rotationArc;
};

#endif
