/***********************************************************************
**
**   mapmatrix.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAP_MATRIX_H
#define MAP_MATRIX_H

#include <QObject>
#include <QTransform>
#include <QPoint>
#include <QRect>

//Added by qt3to4:
#include <Q3PointArray>

#include "projectionlambert.h"
#include "projectioncylindric.h"
#include "wgspoint.h"

/**
 * \class MapMatrix
 *
 * \brief Basic map functionality.
 *
 * This class provides functions for converting coordinates between
 * several coordinate-systems. It takes control over the map scale
 * and the projection-type. To avoid problems, there should be only
 * one element per application.
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \date 2001-2010
 *
 * \version $Id$
 */
class MapMatrix : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MapMatrix )

public:
  /**
   * Creates a new map matrix-object.
   */
  MapMatrix( QObject* object = 0 );

  /**
   * Destructor
   */
  virtual ~MapMatrix();

  /**
   * Converts the given geographic-data into the current map-projection.
   *
   * @param  point  The point to be converted. The point must be in
   *                the internal format of 1/10.000 minutes.
   *
   * @return the projected point
   */
  QPoint wgsToMap(const QPoint& point) const;

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
  QRect wgsToMap(const QRect& rect) const;

  /**
   * Maps the given projected pointarray into the current map-matrix.
   *
   * @param  pArray  The pointarray to be mapped
   *
   * @return the mapped pointarray
   */
  Q3PointArray map(const Q3PointArray& pArray) const;
  /**
   * Maps the given projected point into the current map-matrix.
   *
   * @param  point  The point to be mapped
   *
   * @return the mapped point
   */
  QPoint map(const QPoint& point) const;

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
  QPoint print(const QPoint& p) const;
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
  Q3PointArray print(const Q3PointArray& pArray) const;
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
  void createPrintMatrix(double scale, const QSize& pS, int dX = 0, int dY = 0,
      bool rotate = false);

  /**
   * Initializes the matrix for displaying the map.
   */
  void createMatrix(const QSize& newSize);

  /**
   * @return "true", if the given point in visible in the current map.
   */
  bool isVisible(const QPoint& pos) const;
  /**
   * @return "true", if the given rectangle intersects with the current map.
   */
  bool isVisible(const QRect& itemBorder) const;

  /** */
  enum MoveDirection { NotSet = 0,
                       North = 1,
                       West = 2,
                       East = 4,
                       South = 8,
                       Home = 16};
  /**
   * CurrentScale muss immer die grösste Zahl sein!
   */
  enum ScaleType {LowerLimit = 0, Border1 = 1, Border2 = 2, Border3 = 3,
                  UpperLimit = 4, SwitchScale = 5, CurrentScale = 6};
  /**
   * Centers the map to the given point.
   */
  void centerToPoint(const QPoint&);

  /**
   * Centers the map to the given rectangle and scales the map, so that
   * the rectangle will be seen completely.
   * @param QRect Rectangle to zoom to
   * @param QSize
   * @param addBorder Adds a border of 6.5 km if true.
   */
  double centerToRect(const QRect&, const QSize& = QSize(0,0), bool addBorder = true);

  /** */
  QPoint invertToMap(const QPoint& pos) const;

  /** */
  QPoint mapToWgs(const QPoint& pos) const;

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

  /**
   * @returns the coordinates of the home site
   */
  QPoint getHomeCoord() const;

  /** */
  void centerToLatLon(const QPoint& center);

  /** */
  void centerToLatLon(int latitude, int longitude);

  /** */
  void writeMatrixOptions();

  /**
   * @return the current projection type
   */
  ProjectionBase * getProjection() const
  {
      return currentProjection;
  };

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
  /** */
  void slotCenterTo(int latitude, int longitude);

signals:
  /** */
  void displayMatrixValues(int, bool);

  /** */
  void printMatrixValues(int);

  /**
   * Emitted each time the matrix has changed.
   */
  void matrixChanged();

  /**
   * Emitted each time the projection changed.
   */
  void projectionChanged();

private:
  /**
   * Moves the map into the given direction.
   */
  void __moveMap(int dir);

  /**
   */
  QPoint __mapToWgs(const QPoint&) const;

  /**
   */
  QPoint __mapToWgs(int x, int y) const;

  /**
   * Used map transformation matrix.
   */
  QTransform worldMatrix;

  /**
   * Used map invert transformation matrix.
   */
  QTransform invertMatrix;

  /**
   * Used print transformation matrix.
   */

  QTransform printMatrix;

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

  /** Current used map projection. */
  ProjectionBase* currentProjection;

  /** optimization to prevent recurring recalculation of this value */
  int _MaxScaleToCScaleRatio;
};

#endif
