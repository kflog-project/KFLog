/***********************************************************************
**
**   areaelement.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "areaelement.h"

#include <mapcalc.h>
#include <mapmatrix.h>

void AreaElement::printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder)
{
  extern const double _scale[];
  extern const int _scaleBorder[];

  if(!__isVisible(mapBorder) ||
      (borderIndex != -1 && scale > _scale[_scaleBorder[borderIndex]]))
    return;

  /* Hier werden einfach erstmal die gleichen Einstellungen wie zum
   * Kartenzeichnen verwendet ...
   */
  printPainter->setBrush(fillBrush);
  printPainter->setPen(drawPen);

//  printPainter->drawPolygon(__projectElement(dX, dY, mapCenterLon, scale));
}

QRegion* AreaElement::drawRegion(QPainter* targetPainter, const double dX,
    const double dY, const int mapCenterLon,
    const struct elementBorder mapBorder)
{
  extern const MapMatrix _globalMapMatrix;

  QPointArray tA = _globalMapMatrix.map(projPointArray);

  targetPainter->setBrush(fillBrush);
  targetPainter->setPen(drawPen);

  targetPainter->drawPolygon(tA);

  return (new QRegion(tA));
}

AreaElement::AreaElement(QString n, unsigned int t)
  : LineElement(n, t)
{
  switch(typeID) {
    case HugeCity:
      borderIndex = ID_HUGECITY;
      break;
    case BigCity:
      borderIndex = ID_BIGCITY;
      break;
    case MidCity:
      borderIndex = ID_MIDCITY;
      break;
    case SmallCity:
      borderIndex = ID_SMALLCITY;
      break;
  }

  if(typeID == HugeCity || typeID == BigCity ||
      typeID == MidCity || typeID == SmallCity) {
    drawPen = QPen(QColor(0,0,0), 1);
//    fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//    fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
    fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
  } else {
    drawPen = QPen(QColor(0,0,0), 1);
    fillBrush = QBrush(QBrush::NoBrush);
  }
}

AreaElement::~AreaElement()
{

}
