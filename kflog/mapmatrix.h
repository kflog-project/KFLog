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

#include <qobject.h>
#include <qwmatrix.h>

#include <projectionlambert.h>
#include <projectioncylindric.h>

/**
 * This class is used to handle WGS-coordinates. It inherits QPoint.
 * The only difference is, that the methods to access the coordinates
 * are called "lat" and "lon".
 */
class WGSPoint : public QPoint
{
  public:
    /**
     * Creates a new, empty WGSPoint.
     */
    WGSPoint();
    /**
     * Creates a new WGSPoint with the given position-data.
     */
    WGSPoint(int lat, int lon);
    /**
     * Returns the latitude in the internal format.
     */
    inline int lat() const  {  return x();  };
    /**
     * Returns the longitude in the internal format.
     */
    inline int lon() const  {  return y();  };
    /**
     * Sets the latitude.
     */
    inline void setLat(int lat)  {  setX(lat);  };
    /**
     * Sets the longitude.
     */
    inline void setLon(int lon)  {  setY(lon);  };
    /**
     * Sets the position.
     */
    inline void setPos(int lat, int lon)  {  setX(lat);  setY(lon);  };
    /**
     *
     */
    WGSPoint &operator=( const QPoint &p );
    /**
     *
     */
    friend inline bool operator==( const WGSPoint &pA, const WGSPoint &pB )
        {  return ( (QPoint)pA == (QPoint)pB );  };
    /**
     *
     */
    friend inline bool operator!=( const WGSPoint &pA, const WGSPoint &pB )
        {  return ( (QPoint)pA != (QPoint)pB );  };;
};

/**
 * This class provides functions for converting coordinates between
 * several coordinate-systems. It takes control over the mapscale
 * and the pfijection-type. To avoid problems, there should be only
 * one element per application.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapMatrix : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new mapmatrix-object.
     */
    MapMatrix();
    /**
     * Destructor
     */
    ~MapMatrix();
    /**
     * Converts the given geographic-data into the current map-projection.
     *
     * @param  point  The point to be converted. The point must be in
     *                the internal format of 1/10.000 minutes.
     *
     * @return the projected point
     */
    QPoint wgsToMap(QPoint point) const;
    /**
     * Converts the given geographic-data into the current map-projection.
     *
     * @param  lat  The latitude of the point to be converted. The point must
     *              be in the internal format of 1/10.000 minutes.
     * @param  lon  The longitude of the point to be converted. The point must
     *              be in the internal format of 1/10.000 minutes.
     *
     * @return the projected point
     */
    QPoint wgsToMap(int lat, int lon) const;
    /**
     * Converts the given geographic-data into the current map-projection.
     *
     * @param  rect  The rectangle to be converted. The points must
     *              be in the internal format of 1/10.000 minutes.
     *
     * @return the projected rectangle
     */
    QRect wgsToMap(QRect rect) const;
    /**
     * Maps the given projected pointarray into the current map-matrix.
     *
     * @param  pArray  The pointarray to be mapped
     *
     * @return the mapped pointarray
     */
    QPointArray map(QPointArray pArray) const;
    /**
     * Maps the given projected point into the current map-matrix.
     *
     * @param  point  The point to be mapped
     *
     * @return the mapped point
     */
    QPoint map(QPoint point) const;
    /**
     * Maps the given bearing into the current map-matrix.
     *
     * @param  bearing  The bearing to be mapped
     *
     * @return the mapped bearing
     */
    double map(double bearing) const;
    /**
     * Zur Zeit erwartet die Funktion noch originale Daten. Das ist
     * inkonsequent, da die andere print-Funktion bereits die
     * projezierten Daten haben will ...
     */
    QPoint print(int latitude, int longitude, double dX, double dY) const;
    /**
     */
    QPoint print(QPoint p) const;
    /**
     */
    double print(double bearing) const;
    /**
     * Maps the given pointarray into the current print-matrix.
     *
     * @param  pArray  The pointarray to be mapped
     *
     * @return the mapped pointarray
     */
    QPointArray print(QPointArray pArray) const;
    /**
     * @param  type  The type of scale to be returned.
     *
     * @return the selected scale
     */
    double getScale(unsigned int type = MapMatrix::CurrentScale);
    /**
     * @return the lat/lon-border of the current map.
     */
    QRect getViewBorder() const;
    /**
     * @return the lat/lon-border of the current print-map.
     */
    QRect getPrintBorder() const;
    /**
     * @return the lat/lon-border of the current map.
     */
    QRect getPrintBorder(double a1, double a2, double b1, double b2,
        double c1, double c2, double d1, double d2) const;
    /**
     * Initializes the matrix for printing the map.
     */
    void createPrintMatrix(double scale, QSize pS, int dX = 0, int dY = 0,
        bool rotate = false);
    /**
     * Initializes the matrix for displaying the map.
     */
    void createMatrix(QSize newSize);
    /**
     * @return "true", if the given point in visible in the current map.
     */
    bool isVisible(QPoint pos) const;
    /**
     * @return "true", if the given rectangle intersects with the current map.
     */
    bool isVisible(QRect itemBorder) const;
    /** */
    enum MoveDirection {NotSet = 0, North = 1, West = 2, East = 4,
        South = 8, Home = 16};
    /**
     * CurrentScale muss immer die größte Zahl sein!
     */
    enum ScaleType {LowerLimit = 0, Border1 = 1, Border2 = 2, Border3 = 3,
        UpperLimit = 4, SwitchScale = 5, CurrentScale = 6};
    /**
     * Centers the map to the given point.
     */
    void centerToPoint(QPoint);
    /**
     * Centers the map to the given rectangle and scales the map, so that
     * the rectangle will be seen completly.
     */
    double centerToRect(QRect, QSize = QSize(0,0));
    /** */
    QPoint mapToWgs(QPoint pos) const;
    /**
     *
     */
    int getScaleRange() const;
    /**
     * @return "true", if the current scale is smaller than the switch-scale.
     */
    bool isSwitchScale() const;
    /**
     * @return the lat/lon-position of the map-center.
     */
    QPoint getMapCenter(bool isPrint = false) const;
    /** */
    void centerToLatLon(QPoint center);
    /** */
    void centerToLatLon(int latitude, int longitude);
    /** */
    void writeMatrixOptions();

  public slots:
    /** */
    void slotInitMatrix();
    /** */
    void slotZoomIn();
    /** */
    void slotZoomOut();
    /** */
    void slotCenterToHome();
    /** */
    void slotMoveMapNW();
    /** */
    void slotMoveMapN();
    /** */
    void slotMoveMapNE();
    /** */
    void slotMoveMapW();
    /** */
    void slotMoveMapE();
    /** */
    void slotMoveMapSW();
    /** */
    void slotMoveMapS();
    /** */
    void slotMoveMapSE();
    /** */
    void slotSetScale(double);

  signals:
    /** */
    void displayMatrixValues(int, bool);
    /** */
    void printMatrixValues(int);
    /**
     * Emitted each time the matrix has changed.
     */
    void matrixChanged();

  private:
    /**
     * Moves the map into the given direction.
     */
    void __moveMap(int dir);
    /**
     * Returns the x-coordinate for the Lambert-projection.
     */
//    double __calc_X_Lambert(double latitude, double longitude) const;
    /**
     * Returns the y-coordinate for the Lambert-projection.
     */
//    double __calc_Y_Lambert(double latitude, double longitude) const;
    /**
     * Returns the latitude of the given map-point
     */
//    int __invert_Lambert_Lat(double x, double y) const;
    /**
     * Returns the longitude of the given map-point
     */
//    int __invert_Lambert_Lon(double x, double y) const;
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
//    double v1;
    /**
     *
     */
//    double v2;
//    double var1;
//    double var2;
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
    int printCenterLat;
    int printCenterLon;
    /** */
    int homeLat;
    /** */
    int homeLon;
    /**
     * Contains the geographical border of the map (lat/lon).
     */
    QRect viewBorder;
    QRect printBorder;
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
    double printArc;
    /** */
    int scaleBorders[6];
    /** */
    QSize matrixSize;

    /** */
    ProjectionBase* currentProjection;
    /** */
    ProjectionLambert lambertProjection;
    /** */
    ProjectionCylindric cylindricalProjection;
};

#endif
