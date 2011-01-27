/***********************************************************************
 **
 **   mapmatrix.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001      by Heiner Lamprecht
 **                   2010-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QtGui>

#include "mapmatrix.h"

// Projektions-Massstab
// 10 Meter Höhe pro Pixel ist die stärkste Vergröerung.
// Bei dieser Vergröerung erfolgt die eigentliche Projektion
#define MAX_SCALE 10.0
#define MIN_SCALE 500000.0

#define MATRIX_MOVE(a) __moveMap(a);

/**
 * The earth's radius used for calculation, given in Meters
 * NOTE: We use the earth as a sphere, not as a spheroid!
 */
#define RADIUS 6371000 // FAI Radius, this was the prevoius radius ->6370290
#define NUM_TO_RAD(num) ( ( M_PI * (double)(num) ) / 108000000.0 )
#define RAD_TO_NUM(rad) ( rad * 108000000.0 / M_PI )
#define RAD_TO_NUM_INT(rad) ( (int)rint( (rad) * 108000000.0 / M_PI ))

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
**  MapMatrix
**
*************************************************************************/
extern QSettings _settings;

MapMatrix::MapMatrix( QObject* object ) :
  QObject( object ),
  mapCenterLat(0),
  mapCenterLon(0),
  printCenterLat(0),
  printCenterLon(0),
  cScale(0),
  rotationArc(0),
  printArc(0)
{
  qDebug() << "MapMatrix()";

  viewBorder.setTop(29126344);
  viewBorder.setBottom(29124144);
  viewBorder.setLeft(5349456);
  viewBorder.setRight(5379397);

  printBorder.setTop(29126344);
  printBorder.setBottom(29124144);
  printBorder.setLeft(5349456);
  printBorder.setRight(5379397);

  int projectionType = ProjectionBase::Lambert;

  if (projectionType == ProjectionBase::Lambert)
    {
      // qDebug("MapMatrixConst: Lambert");
      currentProjection = new ProjectionLambert(32400000, 30000000, 0);
    }
  else
    {
      // qDebug("MapMatrixConst: Cylindric");
      currentProjection = new ProjectionCylindric(27000000);
    }
}


MapMatrix::~MapMatrix()
{
  qDebug() << "~MapMatrix()";
}

void MapMatrix::writeMatrixOptions()
{
  _settings.setValue("/MapData/CenterLatitude", mapCenterLat);
  _settings.setValue("/MapData/CenterLongitude", mapCenterLon);
  _settings.setValue("/MapData/MapScale", cScale);
}


QPoint MapMatrix::wgsToMap(const QPoint& origPoint) const
{
  return wgsToMap(origPoint.x(), origPoint.y());
}


QPoint MapMatrix::wgsToMap(int lat, int lon) const
{
  return QPoint((int) rint(currentProjection->projectX(NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
                      RADIUS / MAX_SCALE),
                (int) rint(currentProjection->projectY(NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
                      RADIUS / MAX_SCALE));
}


QRect MapMatrix::wgsToMap(const QRect& rect) const
{
  return QRect(wgsToMap(rect.topLeft()), wgsToMap(rect.bottomRight()));
}


QPoint MapMatrix::__mapToWgs(const QPoint& origPoint) const
{
  return __mapToWgs(origPoint.x(), origPoint.y());
}


QPoint MapMatrix::__mapToWgs(int x, int y) const
{
  double lat = RAD_TO_NUM(currentProjection->invertLat(x * (MAX_SCALE / RADIUS),
                                                       y * (MAX_SCALE / RADIUS)));
  double lon = RAD_TO_NUM(currentProjection->invertLon(x * (MAX_SCALE / RADIUS),
                                                       y * (MAX_SCALE / RADIUS)));

  return QPoint((int)rint(lon), (int)rint(lat));
}

bool MapMatrix::isVisible(const QPoint& pos) const
{
  return (mapBorder.contains(pos));
}

bool MapMatrix::isVisible(const QRect& itemBorder) const
{
  // Grenze: Nahe 15Bit
  // Vereinfachung kann zu Fehlern führen ...
  return ( ( mapBorder.intersects(itemBorder) ) &&
           ( itemBorder.width() * ( MAX_SCALE / cScale ) < 30000 ) &&
           ( itemBorder.height() * ( MAX_SCALE / cScale ) < 30000 ) );
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

QPoint MapMatrix::map(const QPoint& origPoint) const
{
  return worldMatrix.map(origPoint);
}

double MapMatrix::map(double arc) const
{
  return (arc + rotationArc);
}

QPolygon MapMatrix::print(const QPolygon& pArray) const
{
  return printMatrix.map(pArray);
}

QPoint MapMatrix::print(const QPoint& p) const
{
  return printMatrix.map(p);
}

QPoint MapMatrix::print(int lat, int lon, double dX, double dY) const
{
  QPoint temp;

  if(dX == 0 &&  dY == 0)
    {
      temp = QPoint(
        (int)(( currentProjection->projectX( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / pScale ) + dX),
        (int)(( currentProjection->projectY( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / pScale ) + dY ));
    }
  else
    {
      temp = QPoint(
        (int)(( currentProjection->projectX( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / ( pScale * 0.5 ) ) + dX),
        (int)(( currentProjection->projectY( NUM_TO_RAD(lat),
            NUM_TO_RAD(lon - mapCenterLon) ) * RADIUS / ( pScale * 0.5 ) ) + dY ));
    }
  return temp;
}

double MapMatrix::print(double arc) const
{
  return (arc + printArc);
}

QRect MapMatrix::getViewBorder() const  { return viewBorder; }

QRect MapMatrix::getPrintBorder() const  { return printBorder; }

QRect MapMatrix::getPrintBorder( double a1, double a2, double b1, double b2,
                                 double c1, double c2, double d1, double d2) const
{
  QRect temp;

  temp.setTop( RAD_TO_NUM_INT(currentProjection->invertLat(a2 * pScale / RADIUS,  a1 * pScale / RADIUS)) );
  temp.setBottom( RAD_TO_NUM_INT(currentProjection->invertLat(b2 * pScale / RADIUS, b1 * pScale / RADIUS)));
  temp.setRight( RAD_TO_NUM_INT(currentProjection->invertLon(c2 * pScale / RADIUS, c1 * pScale / RADIUS)) + printCenterLon );
  temp.setLeft( RAD_TO_NUM_INT(currentProjection->invertLon(d2 * pScale / RADIUS,  d1 * pScale / RADIUS)) + printCenterLon );

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


void MapMatrix::centerToPoint(const QPoint& center)
{
  QPoint projCenter = __mapToWgs(invertMatrix.map(center));
  mapCenterLat = projCenter.y();
  mapCenterLon = projCenter.x();
}


void MapMatrix::centerToLatLon(const QPoint& center)
{
  centerToLatLon(center.x(), center.y());
}


void MapMatrix::slotCenterTo(int latitude, int longitude)
{
  centerToLatLon(latitude, longitude);
  emit matrixChanged();
}


void MapMatrix::centerToLatLon(int latitude, int longitude)
{
  mapCenterLat = latitude;
  mapCenterLon = longitude;
}


double MapMatrix::centerToRect(const QRect& center, const QSize& pS, bool addBorder)
{
  const int centerX = (center.left() + center.right()) / 2;
  const int centerY = (center.top() + center.bottom()) / 2;

  // We add 6.5 km to ensure, that the sectors will be visible,
  // when the user centers to the task.
  double width, height;

  if(addBorder)
    {
      width = sqrt(center.width() * center.width()) + (6.5 * 1000.0 / cScale);
      height = sqrt(center.height() * center.height()) + (6.5 * 1000.0 / cScale);
    }
  else
    {
      width = center.width();
      height = center.height();
    }

  double xScaleDelta, yScaleDelta;

  if( pS == QSize( 0, 0 ) )
    {
      xScaleDelta = width / mapViewSize.width();
      yScaleDelta = height / mapViewSize.height();
    }
  else
    {
      xScaleDelta = width / pS.width();
      yScaleDelta = height / pS.height();
    }

  double tempScale = qMax( cScale * qMax( xScaleDelta, yScaleDelta ), MAX_SCALE );

  // Only change if difference is too large:
  if( (tempScale / cScale) > 1.05 || (tempScale / cScale) < 0.95 )
    {
      cScale = tempScale;
    }

  centerToPoint(QPoint(centerX, centerY));

  return cScale;
}


QPoint MapMatrix::mapToWgs(const QPoint& pos) const
{
  return __mapToWgs( invertMatrix.map( pos ) );
}

QPoint MapMatrix::invertToMap(const QPoint& pos) const
{
  return invertMatrix.map( pos );
}

void MapMatrix::__moveMap(int dir)
{
  switch(dir) {
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
    mapCenterLat = _settings.value( "/Homesite/Latitude", HOME_DEFAULT_LAT ).toInt();
    mapCenterLon = _settings.value( "/Homesite/Longitude", HOME_DEFAULT_LON ).toInt();
    }

  createMatrix(matrixSize);
  emit matrixChanged();
}

QPoint MapMatrix::getHomeCoord() const
{
  int homeLat = _settings.value( "/Homesite/Latitude", HOME_DEFAULT_LAT ).toInt();
  int homeLon = _settings.value( "/Homesite/Longitude", HOME_DEFAULT_LON ).toInt();

  return QPoint(homeLat, homeLon);
}

void MapMatrix::createMatrix( const QSize& newSize )
{
  qDebug() << "MapMatrix::createMatrix()";

  const QPoint tempPoint( wgsToMap( mapCenterLat, mapCenterLon ) );

  /* Set rotating and scaling */
  double scale = MAX_SCALE / cScale;
  rotationArc = currentProjection->getRotationArc(tempPoint.x(), tempPoint.y());

  double sinscaled = sin(rotationArc) * scale;
  double cosscaled = cos(rotationArc) * scale;
  worldMatrix = QTransform( cosscaled, sinscaled, -sinscaled, cosscaled, 0, 0 );

  /* Set the translation */
  const QPoint map = worldMatrix.map(tempPoint);

  QTransform translateMatrix( 1, 0, 0, 1,
                              currentProjection->getTranslationX(newSize.width(),map.x()),
                              currentProjection->getTranslationY(newSize.height(),map.y()));

  worldMatrix *= translateMatrix;

  // Setting the viewBorder
  bool result = true;
  invertMatrix = worldMatrix.inverted( &result );

  if( !result )
    {
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldMatrix! File=%s, Line=%d", __FILE__, __LINE__);
    }

  // Die Berechnung der Kartengrenze funktioniert so nur auf der
  // Nordhalbkugel. Auf der Südhalbkugel stimmen die Werte nur
  // näherungsweise.
  //
  QPoint tCenter  = __mapToWgs(invertMatrix.map(QPoint(newSize.width() / 2, 0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, newSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(),newSize.height())));

  viewBorder.setTop( tCenter.y() );
  viewBorder.setLeft( tlCorner.x() );
  viewBorder.setRight( trCorner.x() );
  viewBorder.setBottom( std::min( blCorner.y(), brCorner.y() ) );

  mapBorder = invertMatrix.mapRect( QRect( 0, 0, newSize.width(), newSize.height() ) );
  mapViewSize = newSize;

  emit displayMatrixValues( getScaleRange(), isSwitchScale() );
}


void MapMatrix::createPrintMatrix( double printScale,
                                   const QSize& pSize,
                                   int dX,
                                   int dY,
                                   bool rotate )
{

  pScale = printScale;

  printCenterLat = mapCenterLat;
  printCenterLon = mapCenterLon;

  // We must devide the scale by 2.0, because all printing is done
  // with a scaled matrix to get a better resolution in the printout.
  if( pScale <= 500.0 / 72 * 25.4 / 2.0 )  /* 1:500.000   */
    {
      emit printMatrixValues(0);
    }
  else
    {
      emit printMatrixValues(1);
    }

  const QPoint tempPoint( wgsToMap( printCenterLat, printCenterLon ) );

  printMatrix.reset();

  double scale = MAX_SCALE / pScale;

  printArc = atan(tempPoint.x() * 1.0 / tempPoint.y() * 1.0);

  double sinscaled = sin(printArc) * scale;
  double cosscaled = cos(printArc) * scale;
  printMatrix = QTransform( cosscaled, sinscaled, -sinscaled, cosscaled, 0, 0 );

  if( rotate )
    {
      printMatrix.rotate(90);
    }

  /* Set the translation */
  if( dX == 0 && dY == 0 )
    {
      dX = pSize.width() / 2;
      dY = ( pSize.height() / 2 ) - printMatrix.map(tempPoint).y();
    }

  QTransform translateMatrix( 1, 0, 0, 1, dX, dY );

  printMatrix *= translateMatrix;

  // Setting the viewBorder
  bool result = true;
  invertMatrix = printMatrix.inverted( &result );

  if( !result )
    {
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert printMatrix! File=%s, Line=%d", __FILE__, __LINE__);
    }

  /*
   * Die Berechnung der Kartengrenze funktioniert so nur auf der
   * Nordhalbkugel. Auf der Südhalbkugel stimmen die Werte nur
   * näherungsweise.
   */
  QPoint tCenter  = __mapToWgs(invertMatrix.map(QPoint(pSize.width() / 2, 0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(pSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, pSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(pSize.width(), pSize.height())));

  printBorder.setTop( tCenter.y() );
  printBorder.setLeft( tlCorner.x() );
  printBorder.setRight( trCorner.x() );
  printBorder.setBottom( qMin( blCorner.y(), brCorner.y() ) );

}

void MapMatrix::slotCenterToHome()  { MATRIX_MOVE( MapMatrix::Home ); }

void MapMatrix::slotMoveMapNW() { MATRIX_MOVE( MapMatrix::North | MapMatrix::West ); }

void MapMatrix::slotMoveMapN()  { MATRIX_MOVE( MapMatrix::North ); }

void MapMatrix::slotMoveMapNE() { MATRIX_MOVE( MapMatrix::North | MapMatrix::East ); }

void MapMatrix::slotMoveMapW()  { MATRIX_MOVE( MapMatrix::West ); }

void MapMatrix::slotMoveMapE()  { MATRIX_MOVE( MapMatrix::East ); }

void MapMatrix::slotMoveMapSW() { MATRIX_MOVE( MapMatrix::South | MapMatrix::West ); }

void MapMatrix::slotMoveMapS()  { MATRIX_MOVE( MapMatrix::South ); }

void MapMatrix::slotMoveMapSE() { MATRIX_MOVE( MapMatrix::South | MapMatrix::East ); }

void MapMatrix::slotZoomIn()
{
  cScale = qMax( ( cScale / 1.25 ), MAX_SCALE);
  createMatrix(matrixSize);
  emit matrixChanged();
}

void MapMatrix::slotZoomOut()
{
  cScale = qMin( ( cScale * 1.25 ), MIN_SCALE);
  createMatrix(matrixSize);
  emit matrixChanged();
}

void MapMatrix::slotSetScale(double nScale)
{
  if( nScale <= 0 )
    {
      return;
    }

  cScale = nScale;
  createMatrix( matrixSize );
  emit matrixChanged();
}

void MapMatrix::slotInitMatrix()
{
  qDebug() << "MapMatrix::slotInitMatrix()";

  // The scale is set to 0 in the constructor. Here we read the scale and
  // the map center only the first time. Otherwise the values would change
  // after configuring KFLog. Fixed 2001-12-14
  if( cScale <= 0 )
    {
      // @ee we want to center to the last position !
      mapCenterLat = _settings.value( "/MapData/CenterLatitude",
                                      HOME_DEFAULT_LAT ).toInt();

      mapCenterLon = _settings.value( "/MapData/CenterLongitude",
                                      HOME_DEFAULT_LON ).toInt();

      cScale = _settings.value( "/MapData/MapScale", 200.0 ).toDouble();
    }

  int newProjectionType = _settings.value( "/MapData/ProjectionType",
                                           ProjectionBase::Lambert ).toInt();

  bool projChanged = newProjectionType != currentProjection->projectionType();

  if( projChanged )
    {
      delete currentProjection;

      switch( newProjectionType )
        {
        case ProjectionBase::Lambert:

          currentProjection =
              new ProjectionLambert( _settings.value( "/LambertProjection/Parallel1", 32400000 ).toInt(),
                                     _settings.value( "/LambertProjection/Parallel2", 30000000 ).toInt(),
                                     _settings.value( "/LambertProjection/Origin", 0 ).toInt() );

      qDebug() << "Map projection changed to Lambert";
      break;

    case ProjectionBase::Cylindric:
    default:
      // fallback is cylindrical
      currentProjection =
          new ProjectionCylindric( _settings.value( "/CylindricalProjection/Parallel", 27000000).toInt() );

      qDebug () << "Map projection changed to Cylinder";
      break;
    }
  }

  if( projChanged )
    {
      emit projectionChanged();
    }

  scaleBorders[LowerLimit]  = _settings.value("/Scale/Lower Limit", VAL_BORDER_L).toInt();
  scaleBorders[Border1]     = _settings.value("/Scale/Border1", VAL_BORDER_1).toInt();
  scaleBorders[Border2]     = _settings.value("/Scale/Border2", VAL_BORDER_2).toInt();
  scaleBorders[Border3]     = _settings.value("/Scale/Border3", VAL_BORDER_3).toInt();
  scaleBorders[SwitchScale] = _settings.value("/Scale/SwitchScale", VAL_BORDER_S).toInt();
  scaleBorders[UpperLimit]  = _settings.value("/Scale/UpperLimit", VAL_BORDER_U).toInt();

  cScale = qMin( cScale, double( scaleBorders[UpperLimit] ) );
  cScale = qMax( cScale, double( scaleBorders[LowerLimit] ) );

  bool initChanged = false;

  if (currentProjection->projectionType() == ProjectionBase::Lambert)
    {
      initChanged = ( dynamic_cast<ProjectionLambert *>  (currentProjection) )->initProjection(
         _settings.value( "/LambertProjection/Parallel1", 32400000 ).toInt(),
         _settings.value( "/LambertProjection/Parallel2", 30000000 ).toInt(),
         _settings.value( "/LambertProjection/Origin", 0 ).toInt() );
    }
  else if (currentProjection->projectionType() == ProjectionBase::Cylindric)
    {
      initChanged = ( dynamic_cast<ProjectionCylindric*>  (currentProjection) )->initProjection(
                      _settings.value( "/CylindricalProjection/Parallel", 27000000).toInt() );
    }

  if( projChanged || initChanged )
    {
      emit projectionChanged();
    }
}
