/***********************************************************************
**
**   mapmatrix.cpp
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

#include <cmath>

#include "mapmatrix.h"

#define VAR1   ( cos(v1) * cos(v1) )
#define VAR2   ( sin(v1) + sin(v2) )

// Projektions-Maßstab
// 10 Meter Höhe pro Pixel ist die stärkste Vergrößerung.
// Bei dieser Vergrößerung erfolgt die eigentliche Projektion
#define MAX_SCALE 10.0
#define MIN_SCALE 5000.0

#define MAX(a,b)   ( ( a > b ) ? a : b )
#define MIN(a,b)   ( ( a < b ) ? a : b )

// Mit welchem Radius müssen wir rechnen ???
#define RADIUS 6370289.509
#define PI 3.141592654
#define NUM_TO_RAD(num) ( ( PI * (double)num ) / 108000000.0 )
#define RAD_TO_NUM(rad) ( (int)(rad * 108000000.0 / PI ) )

MapMatrix::MapMatrix()
  : cScale(0), rotationArc(0)
{
  viewBorder.setTop(32000000);
  viewBorder.setBottom(25000000);
  viewBorder.setLeft(2000000);
  viewBorder.setRight(7000000);
}

MapMatrix::~MapMatrix()
{

}

QPoint MapMatrix::wgsToMap(QPoint origPoint) const
{
  return wgsToMap(origPoint.x(), origPoint.y());
}

QPoint MapMatrix::wgsToMap(int lat, int lon) const
{
  double projX = __calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon));
  double projY = __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon));

  return QPoint(projX * RADIUS / MAX_SCALE, projY * RADIUS / MAX_SCALE);
}

QPoint MapMatrix::__mapToWgs(QPoint origPoint) const
{
  return __mapToWgs(origPoint.x(), origPoint.y());
}

QPoint MapMatrix::__mapToWgs(int x, int y) const
{
  double lat = __invert_Lambert_Lat(x * MAX_SCALE / RADIUS,
                                    y * MAX_SCALE / RADIUS);
  double lon = __invert_Lambert_Lon(x * MAX_SCALE / RADIUS,
                                    y * MAX_SCALE / RADIUS);

  return QPoint((int)lon, (int)lat);
}

bool MapMatrix::isVisible(QPoint pos) const
{
  return (mapBorder.contains(pos));
}

bool MapMatrix::isVisible(QRect itemBorder) const
{
  return (mapBorder.intersects(itemBorder));
}

QPointArray MapMatrix::map(QPointArray origArray) const
{
  return worldMatrix.map(origArray);
}

QPoint MapMatrix::map(QPoint origPoint) const
{
  return worldMatrix.map(origPoint);
}

double MapMatrix::map(double arc) const
{
  return (arc + rotationArc);
}

QRect MapMatrix::getViewBorder() const  { return viewBorder; }

double MapMatrix::scale() const  { return cScale; }

void MapMatrix::centerToMouse(QPoint center)
{
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  QPoint projCenter = __mapToWgs(invertMatrix.map(center));
  mapCenterLat = projCenter.y();
  mapCenterLon = projCenter.x();
}

void MapMatrix::centerToRect(QRect center)
{
  int centerX = (center.left() + center.right()) / 2;
  int centerY = (center.top() + center.bottom()) / 2;

  centerToMouse(QPoint(centerX, centerY));
}

QPoint MapMatrix::mapToWgs(QPoint pos) const
{
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  return __mapToWgs(invertMatrix.map(pos));
}

void MapMatrix::moveMap(int dir)
{
  switch(dir)
    {
      case North:
        mapCenterLat = viewBorder.top();
        break;
      case North | West:
        mapCenterLat = viewBorder.top();
        mapCenterLon = viewBorder.left();
        break;
      case North | East:
        mapCenterLat = viewBorder.top();
        mapCenterLon = viewBorder.right();
        break;
      case West:
        mapCenterLon = viewBorder.left();
        break;
      case East:
        mapCenterLon = viewBorder.right();
        break;
      case South:
        mapCenterLat = viewBorder.bottom();
        break;
      case South | West:
        mapCenterLat = viewBorder.bottom();
        mapCenterLon = viewBorder.left();
        break;
      case South | East:
        mapCenterLat = viewBorder.bottom();
        mapCenterLon = viewBorder.right();
        break;
      case Home:
        mapCenterLat = homeLat;
        mapCenterLon = homeLon;
    }
}

void MapMatrix::createMatrix(QSize newSize)
{
  // Nicht gerde ideal, vielleicht kann _currentScale mal rausfliegen ...
  extern double _currentScale;
  _currentScale = cScale;

  const QPoint tempPoint(wgsToMap(mapCenterLat, mapCenterLon));

  worldMatrix.reset();

  /* Set rotating and scaling */
  double scale = MAX_SCALE / cScale;
  rotationArc = atan(tempPoint.x() * 1.0 / tempPoint.y() * 1.0);

  worldMatrix.setMatrix(cos(rotationArc) * scale, sin(rotationArc) * scale,
      -sin(rotationArc) * scale, cos(rotationArc) * scale, 0,0);

  /* Set the tranlation */
  QWMatrix translateMatrix(1, 0, 0, 1, newSize.width() / 2,
    ( newSize.height() / 2 ) - worldMatrix.map(tempPoint).y() );

  worldMatrix = worldMatrix * translateMatrix;

  /* Set the viewBorder */
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  /*
   * Die Berechnung der Kartengrenze funktioniert so nur auf der
   * Nordhalbkugel. Auf der Südhalbkugel stimmen die Werte nur
   * näherungsweise.
   */
  QPoint topCenter = __mapToWgs(invertMatrix.map(QPoint(newSize.width() / 2,0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, newSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(),
      newSize.height())));

  viewBorder.setTop(topCenter.y());
  viewBorder.setLeft(tlCorner.x());
  viewBorder.setRight(trCorner.x());
  viewBorder.setBottom(MIN(blCorner.y(), brCorner.y()));

  mapBorder = worldMatrix.map(viewBorder);
}

void MapMatrix::scaleAdd(QSize mapSize)
{
  cScale = MAX( ( cScale / 1.25 ), MAX_SCALE);
  createMatrix(mapSize);
}

void MapMatrix::scaleSub(QSize mapSize)
{
  cScale = MIN( ( cScale * 1.25 ), MIN_SCALE);
  createMatrix(mapSize);
}

void MapMatrix::setScale(double nScale)  {  cScale = nScale;  }

void MapMatrix::initMatrix(int centerLat, int centerLon, double scale,
        double nv1, double nv2, int hLat, int hLon)
{
  v1 = nv1;
  v2 = nv2;

  var1 = cos(v1)*cos(v1);
  var2 = sin(v1)+sin(v2);

  mapCenterLat = centerLat;
  mapCenterLon = centerLon;

  homeLat = hLat;
  homeLon = hLon;

  cScale = scale;
}

double MapMatrix::__calc_Y_Lambert(double latitude, double longitude) const
{
  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 )
               / var2 )
             * cos( var2 * longitude / 2 ) );
}

double MapMatrix::__calc_X_Lambert(double latitude, double longitude) const
{
  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 )
               / var2 )
             * sin( var2 * longitude / 2 ) );
}


int MapMatrix::__invert_Lambert_Lat(double x, double y) const
{
  double lat = -asin(
              ( -4.0 * pow(cos(v1), 2.0) - 4.0 * pow(sin(v1), 2.0)
                -4.0 * sin(v1) * sin(v2)
                + x * x * pow(sin(v1), 2.0) + pow(sin(v1), 2.0)* y * y
                + 2.0 * x * x * sin(v1) * sin(v2) + 2.0 * sin(v1)
                * sin(v2) * y * y + x * x * pow(sin(v2), 2.0)
                + pow(sin(v2), 2.0) * y * y
                ) /
              ( sin(v1) + sin(v2) ) / 4 );
  return RAD_TO_NUM(lat);
}

int MapMatrix::__invert_Lambert_Lon(double x, double y) const
{
  double lon = 2.0 * atan( x / y ) / ( sin(v1) + sin(v2) );
  return RAD_TO_NUM(lon);
}
