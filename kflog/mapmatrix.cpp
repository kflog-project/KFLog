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

#include <kapp.h>
#include <kconfig.h>

#include "mapmatrix.h"

// Projektions-Maßstab
// 10 Meter Höhe pro Pixel ist die stärkste Vergrößerung.
// Bei dieser Vergrößerung erfolgt die eigentliche Projektion
#define MAX_SCALE 10.0
#define MIN_SCALE 5000.0

#define MAX(a,b)   ( ( a > b ) ? a : b )
#define MIN(a,b)   ( ( a < b ) ? a : b )

#define MATRIX_MOVE(a)  \
    __moveMap(a);

// Mit welchem Radius müssen wir rechnen ???
#define RADIUS 6370289.509
#define PI 3.141592654
#define NUM_TO_RAD(num) ( ( PI * (double)(num) ) / 108000000.0 )
#define RAD_TO_NUM(rad) ( (int)( (rad) * 108000000.0 / PI ) )

// the scale-borders
#define VAL_BORDER_L                      10
#define VAL_BORDER_U                    1500
#define VAL_BORDER_1                     100
#define VAL_BORDER_2                     500
#define VAL_BORDER_3                    1000
#define VAL_BORDER_S                     250

// default Start-Position (Poltringen)
#define HOME_DEFAULT_LAT 29125200
#define HOME_DEFAULT_LON 5364500


/*************************************************************************
**
** WGSPoint
**
*************************************************************************/

WGSPoint::WGSPoint()
  : QPoint()
{

}

WGSPoint::WGSPoint(int lat, int lon)
  : QPoint(lat, lon)
{

}

WGSPoint &WGSPoint::operator=( const QPoint &p )
{
  setPos(p.x(), p.y());
  return *this;
}


/*************************************************************************
**
**  MapMatrix
**
*************************************************************************/

MapMatrix::MapMatrix()
  : mapCenterLat(0), mapCenterLon(0), printCenterLat(0), printCenterLon(0),
    cScale(0), rotationArc(0), printArc(0)
{
  viewBorder.setTop(32000000);
  viewBorder.setBottom(25000000);
  viewBorder.setLeft(2000000);
  viewBorder.setRight(7000000);

  printBorder.setTop(32000000);
  printBorder.setBottom(25000000);
  printBorder.setLeft(2000000);
  printBorder.setRight(7000000);

  currentProjection = &lambertProjection;
//  currentProjection = &cylindricalProjection;
}

MapMatrix::~MapMatrix()
{

}

void MapMatrix::writeMatrixOptions()
{
  KConfig *config = kapp->config();

  config->setGroup("Map Data");
  config->writeEntry("Center Latitude", mapCenterLat);
  config->writeEntry("Center Longitude", mapCenterLon);
  config->writeEntry("Map Scale", cScale);

  config->setGroup(0);
}

QPoint MapMatrix::wgsToMap(QPoint origPoint) const
{
   return wgsToMap(origPoint.x(), origPoint.y());
}

QPoint MapMatrix::wgsToMap(int lat, int lon) const
{
  return QPoint(currentProjection->projectX(NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
                    RADIUS / MAX_SCALE,
      currentProjection->projectY(NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
                    RADIUS / MAX_SCALE);

//  return QPoint(__calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
//                    RADIUS / MAX_SCALE,
//                __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
//                    RADIUS / MAX_SCALE);
}

QRect MapMatrix::wgsToMap(QRect rect) const
{
  return QRect(wgsToMap(rect.topLeft()), wgsToMap(rect.bottomRight()));
}

QPoint MapMatrix::__mapToWgs(QPoint origPoint) const
{
  return __mapToWgs(origPoint.x(), origPoint.y());
}

QPoint MapMatrix::__mapToWgs(int x, int y) const
{
//  double lat = __invert_Lambert_Lat(x * MAX_SCALE / RADIUS,
//                                    y * MAX_SCALE / RADIUS);
//  double lon = __invert_Lambert_Lon(x * MAX_SCALE / RADIUS,
//                                    y * MAX_SCALE / RADIUS);

  double lat = RAD_TO_NUM(currentProjection->invertLat(x * MAX_SCALE / RADIUS,
      y * MAX_SCALE / RADIUS));
  double lon = RAD_TO_NUM(currentProjection->invertLon(x * MAX_SCALE / RADIUS,
      y * MAX_SCALE / RADIUS));

  return QPoint((int)lon, (int)lat);
}

bool MapMatrix::isVisible(QPoint pos) const
{
  return (mapBorder.contains(pos));
}

bool MapMatrix::isVisible(QRect itemBorder) const
{
  // Grenze: Nahe 15Bit
  // Vereinfachung kann zu Fehlern führen ...
  return ( ( mapBorder.intersects(itemBorder) ) &&
           ( itemBorder.width() * ( MAX_SCALE / cScale ) < 30000 ) &&
           ( itemBorder.height() * ( MAX_SCALE / cScale ) < 30000 ) );

//  return ( ( mapBorder.intersects(itemBorder) ) &&
//           ( itemBorder.width() * ( MAX_SCALE / cScale ) > 2 ) &&
//           ( itemBorder.height() * ( MAX_SCALE / cScale ) > 2 ) );
}

int MapMatrix::getScaleRange()  const
{
  if(cScale <= scaleBorders[Border1])
      return LowerLimit;
   else if(cScale <= scaleBorders[Border2])
      return Border1;
   else if(cScale <= scaleBorders[Border3])
      return Border2;
   else
      return Border3;
}

bool MapMatrix::isSwitchScale() const
{
  return cScale <= scaleBorders[SwitchScale];
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

QPointArray MapMatrix::print(QPointArray pArray) const
{
  return printMatrix.map(pArray);
}

QPoint MapMatrix::print(QPoint p) const
{
  return printMatrix.map(p);
}

QPoint MapMatrix::print(int lat, int lon, double dX, double dY) const
{
  QPoint temp;

//  if(dX == 0 &&  dY == 0)
//    {
//      temp = QPoint(
//        ( __calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
//            * RADIUS / pScale ) + dX,
//        ( __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
//             * RADIUS / pScale ) + dY );
//    }
//  else
//    {
//      temp = QPoint(
//        ( __calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
//            * RADIUS / ( pScale * 0.5 ) ) + dX,
//        ( __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
//             * RADIUS / ( pScale * 0.5 ) ) + dY );
//    }

  if(dX == 0 &&  dY == 0)
    {
      temp = QPoint(
        ( currentProjection->projectX( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / pScale ) + dX,
        ( currentProjection->projectY( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / pScale ) + dY );
    }
  else
    {
      temp = QPoint(
        ( currentProjection->projectX( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / ( pScale * 0.5 ) ) + dX,
        ( currentProjection->projectY( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / ( pScale * 0.5 ) ) + dY );
    }
  return temp;
}

double MapMatrix::print(double arc) const
{
  return (arc + printArc);
}

QRect MapMatrix::getViewBorder() const  { return viewBorder; }

QRect MapMatrix::getPrintBorder() const  { return printBorder; }

QRect MapMatrix::getPrintBorder(double a1, double a2, double b1, double b2,
        double c1, double c2, double d1, double d2) const
{
  QRect temp;

//  temp.setTop( __invert_Lambert_Lat(a2 * pScale / RADIUS,
//      a1 * pScale / RADIUS) );
//  temp.setBottom( __invert_Lambert_Lat(b2 * pScale / RADIUS,
//      b1 * pScale / RADIUS) );
//  temp.setRight( __invert_Lambert_Lon(c2 * pScale / RADIUS,
//      c1 * pScale / RADIUS) + printCenterLon );
//  temp.setLeft( __invert_Lambert_Lon(d2 * pScale / RADIUS,
//      d1 * pScale / RADIUS) + printCenterLon );

  temp.setTop(RAD_TO_NUM(currentProjection->invertLat(a2 * pScale / RADIUS,
      a1 * pScale / RADIUS)));
  temp.setBottom(RAD_TO_NUM(currentProjection->invertLat(b2 * pScale / RADIUS,
      b1 * pScale / RADIUS)));
  temp.setRight(RAD_TO_NUM(currentProjection->invertLon(c2 * pScale / RADIUS,
      c1 * pScale / RADIUS)) + printCenterLon);
  temp.setLeft(RAD_TO_NUM(currentProjection->invertLon(d2 * pScale / RADIUS,
      d1 * pScale / RADIUS)) + printCenterLon);

  return temp;
}

QPoint MapMatrix::getMapCenter(bool isPrint) const
{
  if(isPrint)
      return QPoint(printCenterLat, printCenterLon);
  else
      return QPoint(mapCenterLat, mapCenterLon);
}

double MapMatrix::getScale(unsigned int type)
{
  if(type == MapMatrix::CurrentScale)
      return cScale;
  else if(type < MapMatrix::CurrentScale)
      return scaleBorders[type];
  else
      qFatal("MapMatrix::getScale(): Value too large!");

  return 0.0;
}

void MapMatrix::centerToPoint(QPoint center)
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

void MapMatrix::centerToLatLon(QPoint center)
{
  centerToLatLon(center.x(), center.y());
}

void MapMatrix::centerToLatLon(int latitude, int longitude)
{
  mapCenterLat = latitude;
  mapCenterLon = longitude;
}

double MapMatrix::centerToRect(QRect center, QSize pS)
{
  const int centerX = (center.left() + center.right()) / 2;
  const int centerY = (center.top() + center.bottom()) / 2;

  // We add 6.5 km to ensure, that the sectors will be visible,
  // when the user centers to the task.
  const double width = sqrt(center.width() * center.width()) +
      (6.5 * 1000.0 / cScale);
  const double height = sqrt(center.height() * center.height()) +
      (6.5 * 1000.0 / cScale);

  double xScaleDelta, yScaleDelta;

  if(pS == QSize(0,0))
    {
      xScaleDelta = width / mapViewSize.width();
      yScaleDelta = height / mapViewSize.height();
    }
  else
    {
      xScaleDelta = width / pS.width();
      yScaleDelta = height / pS.height();
    }

  double tempScale = MAX(cScale * MAX(xScaleDelta, yScaleDelta),
      MAX_SCALE);

  // Änderung nur, wenn Unterschied zu gross:
  if((tempScale / cScale) > 1.05 || (tempScale / cScale) < 0.95)
      cScale = tempScale;

  centerToPoint(QPoint(centerX, centerY));

  return cScale;
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

void MapMatrix::__moveMap(int dir)
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

  createMatrix(matrixSize);
  emit matrixChanged();
}

void MapMatrix::createMatrix(QSize newSize)
{
  const QPoint tempPoint(wgsToMap(mapCenterLat, mapCenterLon));

  worldMatrix.reset();

  /* Set rotating and scaling */
  double scale = MAX_SCALE / cScale;
  rotationArc = currentProjection->getRotationArc(tempPoint.x(), tempPoint.y());

  worldMatrix.setMatrix(cos(rotationArc) * scale, sin(rotationArc) * scale,
      -sin(rotationArc) * scale, cos(rotationArc) * scale, 0, 0);

  /* Set the tranlation */
  QWMatrix translateMatrix(1, 0, 0, 1,
      currentProjection->getTranslationX(newSize.width(),
          worldMatrix.map(tempPoint).x()),
      currentProjection->getTranslationY(newSize.height(),
          worldMatrix.map(tempPoint).y()));

  worldMatrix = worldMatrix * translateMatrix;

  // Setting the viewBorder
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  //
  // Die Berechnung der Kartengrenze funktioniert so nur auf der
  // Nordhalbkugel. Auf der Südhalbkugel stimmen die Werte nur
  // näherungsweise.
  //
  QPoint tCenter = __mapToWgs(invertMatrix.map(QPoint(newSize.width() / 2, 0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, newSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(),
      newSize.height())));

  viewBorder.setTop(tCenter.y());
  viewBorder.setLeft(tlCorner.x());
  viewBorder.setRight(trCorner.x());
  viewBorder.setBottom(MIN(blCorner.y(), brCorner.y()));

  mapBorder = invertMatrix.map(QRect(0,0, newSize.width(), newSize.height()));
  mapViewSize = newSize;

  emit displayMatrixValues(getScaleRange(), isSwitchScale());
//  emit matrixChanged();
}

void MapMatrix::createPrintMatrix(double printScale, QSize pSize, int dX,
    int dY, bool rotate)
{
  pScale = printScale;

  printCenterLat = mapCenterLat;
  printCenterLon = mapCenterLon;

  //
  // We must devide the scale by 2.0, because all printing is done
  // with a scaled matrix to get a better resolution in the printout.
  //
  if(pScale <= 500.0 / 72 * 25.4 / 2.0)          /* 1:500.000   */
      emit printMatrixValues(0);
  else
      emit printMatrixValues(1);

  const QPoint tempPoint(wgsToMap(printCenterLat, printCenterLon));
  printMatrix.reset();

  double scale = MAX_SCALE / pScale;
  printArc = atan(tempPoint.x() * 1.0 / tempPoint.y() * 1.0);

  printMatrix.setMatrix(cos(printArc) * scale, sin(printArc) * scale,
      -sin(printArc) * scale, cos(printArc) * scale, 0, 0);

  if(rotate)
      printMatrix.rotate(90);

  /* Set the translation */
  if(dX == 0 && dY == 0)
    {
      QWMatrix translateMatrix(1, 0, 0, 1, pSize.width() / 2,
        ( pSize.height() / 2 ) - printMatrix.map(tempPoint).y() );

      printMatrix = printMatrix * translateMatrix;
    }
  else
    {
      QWMatrix translateMatrix(1, 0, 0, 1, dX, dY );
      printMatrix = printMatrix * translateMatrix;
    }

  /* Set the viewBorder */
  bool result = true;
  QWMatrix invertMatrix = printMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  /*
   * Die Berechnung der Kartengrenze funktioniert so nur auf der
   * Nordhalbkugel. Auf der Südhalbkugel stimmen die Werte nur
   * näherungsweise.
   */
  QPoint tCenter = __mapToWgs(invertMatrix.map(QPoint(pSize.width() / 2, 0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(pSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, pSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(pSize.width(),
      pSize.height())));

  printBorder.setTop(tCenter.y());
  printBorder.setLeft(tlCorner.x());
  printBorder.setRight(trCorner.x());
  printBorder.setBottom(MIN(blCorner.y(), brCorner.y()));

//  return &printMatrix;
}

void MapMatrix::slotCenterToHome()  { MATRIX_MOVE( MapMatrix::Home ) }

void MapMatrix::slotMoveMapNW() { MATRIX_MOVE( MapMatrix::North | MapMatrix::West ) }

void MapMatrix::slotMoveMapN()  { MATRIX_MOVE( MapMatrix::North ) }

void MapMatrix::slotMoveMapNE() { MATRIX_MOVE( MapMatrix::North | MapMatrix::East ) }

void MapMatrix::slotMoveMapW()  { MATRIX_MOVE( MapMatrix::West ) }

void MapMatrix::slotMoveMapE()  { MATRIX_MOVE( MapMatrix::East ) }

void MapMatrix::slotMoveMapSW() { MATRIX_MOVE( MapMatrix::South | MapMatrix::West ) }

void MapMatrix::slotMoveMapS()  { MATRIX_MOVE( MapMatrix::South ) }

void MapMatrix::slotMoveMapSE() { MATRIX_MOVE( MapMatrix::South | MapMatrix::East ) }

void MapMatrix::slotZoomIn()
{
  cScale = MAX( ( cScale / 1.25 ), MAX_SCALE);
  createMatrix(matrixSize);
  emit matrixChanged();
}

void MapMatrix::slotZoomOut()
{
  cScale = MIN( ( cScale * 1.25 ), MIN_SCALE);
  createMatrix(matrixSize);
  emit matrixChanged();
}

void MapMatrix::slotSetScale(double nScale)
{
  if (nScale <= 0)  return;

  cScale = nScale;
  createMatrix(matrixSize);
  emit matrixChanged();
}

void MapMatrix::slotInitMatrix()
{
  KConfig *config = kapp->config();

  config->setGroup("Map Data");
  //
  // The scale is set to 0 in the constructor. Here we read the scale and
  // the mapcenter only the first time. Otherwise the values would change
  // after configuring KFLog.
  //
  //                                                Fixed 2001-12-14
  if(cScale <= 0)
    {
      mapCenterLat = config->readNumEntry("Center Latitude", HOME_DEFAULT_LAT);
      mapCenterLon = config->readNumEntry("Center Longitude", HOME_DEFAULT_LON);
      cScale = config->readDoubleNumEntry("Map Scale", 200);
    }

  homeLat = config->readNumEntry("Homesite Latitude", HOME_DEFAULT_LAT);
  homeLon = config->readNumEntry("Homesite Longitude", HOME_DEFAULT_LON);

  config->setGroup("Scale");
  scaleBorders[LowerLimit] = config->readNumEntry("Lower Limit", VAL_BORDER_L);
  scaleBorders[Border1] = config->readNumEntry("Border 1", VAL_BORDER_1);
  scaleBorders[Border2] = config->readNumEntry("Border 2", VAL_BORDER_2);
  scaleBorders[Border3] = config->readNumEntry("Border 3", VAL_BORDER_3);
  scaleBorders[SwitchScale] = config->readNumEntry("Switch Scale", VAL_BORDER_S);
  scaleBorders[UpperLimit] = config->readNumEntry("Upper Limit", VAL_BORDER_U);

  cScale = MIN(cScale, scaleBorders[UpperLimit]);
  cScale = MAX(cScale, scaleBorders[LowerLimit]);

  config->setGroup("Lambert Projection");
  lambertProjection.initProjection(config->readNumEntry("Parallel1", 32400000),
      config->readNumEntry("Parallel2", 30000000),
      config->readNumEntry("Origin", 0));

  config->setGroup("Cylindrical Projection");
  cylindricalProjection.initProjection(config->readNumEntry("Parallel", 27000000));

  config->setGroup(0);

}

//double MapMatrix::__calc_Y_Lambert(double latitude, double longitude) const
//{
//  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 ) / var2 )
//             * cos( var2 * longitude / 2 ) );
//}

//double MapMatrix::__calc_X_Lambert(double latitude, double longitude) const
//{
//  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 ) / var2 )
//             * sin( var2 * longitude / 2 ) );
//}

//int MapMatrix::__invert_Lambert_Lat(double x, double y) const
//{
//  double lat = -asin(
//              ( -4.0 * pow(cos(v1), 2.0) - 4.0 * pow(sin(v1), 2.0)
//                -4.0 * sin(v1) * sin(v2)
//                + x * x * pow(sin(v1), 2.0) + pow(sin(v1), 2.0)* y * y
//                + 2.0 * x * x * sin(v1) * sin(v2) + 2.0 * sin(v1)
//                * sin(v2) * y * y + x * x * pow(sin(v2), 2.0)
//                + pow(sin(v2), 2.0) * y * y
//                ) /
//              ( sin(v1) + sin(v2) ) / 4 );
//  return RAD_TO_NUM(lat);
//}

//int MapMatrix::__invert_Lambert_Lon(double x, double y) const
//{
//  double lon = 2.0 * atan( x / y ) / ( sin(v1) + sin(v2) );
//  return RAD_TO_NUM(lon);
//}
