/***********************************************************************
**
**   mapmatrix.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
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

class MapConfig;

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
     * Zur Zeit erwartet die Funktion noch originale Daten. Das ist
     * inkonsequent, da die andere print-Funktion bereits die
     * projezierten Daten haben will ...
     */
    QPoint print(int latitude, int longitude, double dX, double dY) const;
    /** */
    QPoint print(QPoint p) const;
    /** */
    QPointArray print(QPointArray pArray) const;
    /**
     *
     */
    void createMatrix(QSize newSize);
    /** */
    double getScale(unsigned int type = MapMatrix::CurrentScale);
    /** */
    QRect getViewBorder() const;
    /** */
    QRect getPrintBorder(double a1, double a2, double b1, double b2,
        double c1, double c2, double d1, double d2) const;
    /** */
    void initMatrix(MapConfig* mConf);
    /** */
    void saveMatrix();
    /** */
    QWMatrix* createPrintMatrix(double scale, QSize pS, int dX = 0, int dY = 0,
        bool rotate = false);
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
    /**
     * CurrentScale muss immer die größte Zahl sein!
     */
    enum ScaleType {LowerLimit = 0, Border1 = 1, Border2 = 2, Border3 = 3,
        UpperLimit = 4, SwitchScale = 5, CurrentScale = 6};
    /** */
    void moveMap(int dir);
    /** */
    void centerToPoint(QPoint);
    /** */
    void centerToRect(QRect);
    /** */
    QPoint mapToWgs(QPoint pos) const;
    /** */
    void setScale(double);
    /** */
    int getScaleRange() const;
    /** */
    bool isSwitchScale() const;

    QPoint getMapCenter() const;

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
     */
    QWMatrix printMatrix;
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
    double pScale;
    /** */
    double rotationArc;
    /** */
    int scaleBorders[6];
    /** */
    MapConfig* mapConfig;
};

#endif
