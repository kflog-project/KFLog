/***********************************************************************
**
**   lineelement.cpp
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

#include "lineelement.h"

#include <mapcalc.h>
#include <mapmatrix.h>

#include <kapp.h>
#include <kiconloader.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

LineElement::LineElement(QString n, unsigned int t, QPointArray pA, bool isV)
: BaseMapElement(n, t), drawPenStyle(QPen::SolidLine),
  fillBrushStyle(QBrush::SolidPattern),
  projPointArray(pA), bBox(pA.boundingRect()), valley(isV), closed(false)
{
  fillColor.setRgb(0, 0, 0);

  drawThickness = new int[9];

  switch(typeID)
    {
      case Highway:
        PEN_THICKNESS(5, 5, 5, 5, 3, 2, 2, 2, 1);
        drawColor.setRgb(255, 100, 100);
        break;
      case MidRoad:
        PEN_THICKNESS(2, 2, 2, 2, 2, 1, 1, 1, 1)
        drawColor.setRgb(255, 100, 100);
        break;
      case SmallRoad:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(255, 50, 50);
        break;
      case Railway:
        PEN_THICKNESS(2, 2, 2, 2, 2, 1, 1, 1, 1)
        drawColor.setRgb(80, 80, 80);
        break;
      case AerialRailway:
        PEN_THICKNESS(2, 2, 2, 2, 2, 2, 2, 1, 1)
        drawColor.setRgb(0, 0, 0);
        break;
      case Coast:
        PEN_THICKNESS(2, 2, 2, 2, 2, 2, 2, 1, 1)
        drawColor.setRgb(50, 200, 255);
        break;
      case BigLake:
        closed = true;
      case BigRiver:
        PEN_THICKNESS(3, 3, 3, 2, 2, 1, 1, 1, 1)
        drawColor.setRgb(70, 70, 195);
        fillColor.setRgb(96,128,248);
        break;
      case MidLake:
        closed = true;
      case MidRiver:
        PEN_THICKNESS(2, 2, 2, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(70, 70, 195);
        fillColor.setRgb(96,128,248);
        break;
      case SmallLake:
        closed = true;
      case SmallRiver:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(70, 70, 195);
        fillColor.setRgb(96,128,248);
        break;
      case HugeCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
        fillColor.setRgb(255,250,100);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
//        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        closed = true;
        break;
      case BigCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
        fillColor.setRgb(255,250,100);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
//        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        closed = true;
        break;
      case MidCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
        fillColor.setRgb(255,250,100);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
//        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        closed = true;
        break;
      case SmallCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
        fillColor.setRgb(245,240,0);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
//        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        closed = true;
        break;
    }
}

LineElement::~LineElement()
{

}

void LineElement::printMapElement(QPainter* printPainter)
{

}

void LineElement::printMapElement(QPainter* printPainter, const double dX,
      const double dY, const int mapCenterLon, const double scale,
      const struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  extern const double _scale[];
  extern const int _scaleBorder[];

  bool show = true;
  bool highwayShow = false;
  bool isClosed = false;

  switch(typeID) {
    case Highway:
      show = true;
      if(scale < _scale[5]) {
        printPainter->setPen(QPen(QColor(255,100,100), 5));
        highwayShow = true;
      } else if(scale < _scale[6]){
        printPainter->setPen(QPen(QColor(255,100,100), 3));
      } else if(scale < _scale[7]){
        printPainter->setPen(QPen(QColor(255,100,100), 2));
      } else {
        printPainter->setPen(QPen(QColor(255,100,100), 1));
      }
      break;
    case MidRoad:
      show = false;
      printPainter->setPen(QPen(QColor(200,100,100), 1));
      if(scale <= _scale[_scaleBorder[typeID]]) {
        if(scale <= _scale[6]) {
          show = true;
          printPainter->setPen(QPen(QColor(200,100,100), 2));
        } else if(scale <= _scale[7]) {
          show = true;
          printPainter->setPen(QPen(QColor(200,100,100), 1));
        }
      }
      printPainter->setBrush(QBrush::NoBrush);
      break;
    case SmallRoad:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        show = true;
        printPainter->setPen(QPen(QColor(200,50,50), 1));
      }
      printPainter->setBrush(QBrush::NoBrush);
      break;
    case Railway:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        if(scale <= _scale[6]) {
          printPainter->setPen(QPen(QColor(75,75,75), 2, QPen::DashLine));
        } else {
          printPainter->setPen(QPen(QColor(75,75,75), 1, QPen::DashLine));
        }
        show = true;
        printPainter->setBrush(QBrush::NoBrush);
      }
      break;
    case AerialRailway:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        show = true;
        printPainter->setBrush(QBrush::NoBrush);
        printPainter->setPen(QPen(QColor(0,0,0), 2, QPen::DashDotLine));
      }
      break;
    case Coast:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        show = true;
        printPainter->setBrush(QBrush(QColor(50,200,255), QBrush::SolidPattern));
        printPainter->setPen(QPen(QColor(50,50,255), 1));
      }
      break;
    case BigLake:
      isClosed = true;
    case BigRiver:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        show = true;
        if(scale <= _scale[5]) {
          printPainter->setPen(QPen(QColor(100,100,255), 3, QPen::DotLine));
        } else if(scale <= _scale[6]) {
          printPainter->setPen(QPen(QColor(100,100,255), 2, QPen::DotLine));
        } else {
          printPainter->setPen(QPen(QColor(100,100,255), 1, QPen::DotLine));
        }
        printPainter->setBrush(QBrush(QColor(100,200,255), QBrush::SolidPattern));
      }
      break;
    case MidLake:
      isClosed = true;
    case MidRiver:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        show = true;
        if(scale <= _scale[6]) {
          printPainter->setPen(QPen(QColor(100,100,255), 2, QPen::DotLine));
        } else {
          printPainter->setPen(QPen(QColor(100,100,255), 1, QPen::DotLine));
        }
        printPainter->setBrush(QBrush(QColor(75,200,255), QBrush::SolidPattern));
      }
      break;
    case SmallLake:
      isClosed = true;
    case SmallRiver:
      show = false;
      if(scale <= _scale[_scaleBorder[typeID]]) {
        show = true;
        printPainter->setPen(QPen(QColor(100,100,255), 1, QPen::DotLine));
        printPainter->setBrush(QBrush(QColor(50,200,255), QBrush::SolidPattern));
      }
      break;
  }

  if(show) {
    QPointArray pA;// = __projectElement(dX, dY, mapCenterLon, scale);

    if(isClosed) {
      printPainter->drawPolygon(pA);
    } else {
      printPainter->drawPolyline(pA);
      if(highwayShow) {
        printPainter->setPen(QPen(QColor(255,255,255), 1));
        printPainter->drawPolyline(pA);
      }
    }
  }
}

QRegion* LineElement::drawRegion(QPainter* targetPainter, QPainter* maskPainter)
{
  if(!__isVisible()) return (new QRegion());

  extern const MapMatrix _globalMapMatrix;

  QPointArray tA = _globalMapMatrix.map(projPointArray);

  extern const double _currentScale, _scale[];

  int index = 0;
  if(_currentScale <= _scale[2]) index = 1;
  else if(_currentScale <= _scale[3]) index = 2;
  else if(_currentScale <= _scale[4]) index = 3;
  else if(_currentScale <= _scale[5]) index = 4;
  else if(_currentScale <= _scale[6]) index = 5;
  else if(_currentScale <= _scale[7]) index = 6;
  else if(_currentScale <= _scale[8]) index = 7;
  else index = 8;

  if(valley)
    {
      maskPainter->setBrush(QBrush(Qt::color0, fillBrushStyle));
    }
  else
    {
      maskPainter->setBrush(QBrush(Qt::color1, fillBrushStyle));
    }
  maskPainter->setPen(QPen(Qt::color1, drawThickness[index], drawPenStyle));
  maskPainter->drawPolygon(tA);

  targetPainter->setBrush(QBrush(fillColor, fillBrushStyle));
  targetPainter->setPen(QPen(drawColor, drawThickness[index], drawPenStyle));
  targetPainter->drawPolygon(tA);

  return (new QRegion(tA));
}

void LineElement::drawMapElement(QPainter* targetPainter, QPainter* maskPainter,
    bool isFirst)
{
  if(!__isVisible()) return;

  extern const MapMatrix _globalMapMatrix;
  extern const double _currentScale, _scale[];

  int index = 0;
  if(_currentScale <= _scale[2]) index = 1;
  else if(_currentScale <= _scale[3]) index = 2;
  else if(_currentScale <= _scale[4]) index = 3;
  else if(_currentScale <= _scale[5]) index = 4;
  else if(_currentScale <= _scale[6]) index = 5;
  else if(_currentScale <= _scale[7]) index = 6;
  else if(_currentScale <= _scale[8]) index = 7;
  else index = 8;

  if(valley)
    {
      maskPainter->setPen(QPen(Qt::color0, drawThickness[index]));
      maskPainter->setBrush(QBrush(Qt::color0, QBrush::SolidPattern));
    }
  else
    {
      maskPainter->setPen(QPen(Qt::color1, drawThickness[index]));
      maskPainter->setBrush(QBrush(Qt::color1, QBrush::SolidPattern));
    }

  targetPainter->setPen(QPen(drawColor, drawThickness[index]));
  targetPainter->setBrush(QBrush(fillColor, QBrush::SolidPattern));

  QPointArray pA = _globalMapMatrix.map(projPointArray);

  /********************************/
  if(typeID == 37 && isFirst)
    {
//      targetPainter->setPen(QPen(drawColor, drawThickness[index]));
      targetPainter->setPen(QPen(drawColor, drawThickness[index] * 3));
      maskPainter->setPen(QPen(Qt::color1, drawThickness[index] * 3));
      maskPainter->drawPolygon(pA);
      targetPainter->drawPolyline(pA);

      return;
    }
  else if(typeID == 37 && !isFirst)
    {
      targetPainter->setPen(QPen(fillColor, 0));
      targetPainter->drawPolygon(pA);

      return;
    }
  /********************************/

  if(closed)
    {
      maskPainter->drawPolygon(pA);
      targetPainter->drawPolygon(pA);
    }
  else
    {
      maskPainter->drawPolyline(pA);
      targetPainter->drawPolyline(pA);
      if(typeID == Highway && _currentScale < _scale[5])
        {
          // Mittellinie zeichnen
          targetPainter->setPen(QPen(QColor(255,255,255), 1));
          targetPainter->drawPolyline(pA);
        }
    }
}

bool LineElement::__isVisible() const
{
  extern const MapMatrix _globalMapMatrix;
  return _globalMapMatrix.isVisible(bBox);
}

bool LineElement::isValley() const  {  return valley;  }
