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

LineElement::LineElement(QString n, unsigned int t, QPointArray pA)
: BaseMapElement(n, t), drawPenStyle(QPen::SolidLine),
  projPointArray(pA), bBox(pA.boundingRect())
{
  fillColor.setRgb(0, 0, 0);

  drawThickness = new int[9];

  switch(typeID)
    {
      case Highway:
        PEN_THICKNESS(5, 5, 5, 5, 3, 3, 2, 2, 1);
        drawColor.setRgb(255, 100, 100);
        break;
      case MidRoad:
        PEN_THICKNESS(2, 2, 2, 2, 2, 1, 1, 1, 1)
        drawColor.setRgb(200, 100, 100);
        break;
      case SmallRoad:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(200, 50, 50);
        break;
      case Railway:
        PEN_THICKNESS(2, 2, 2, 2, 2, 2, 2, 1, 1)
        drawColor.setRgb(75, 75, 75);
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
      case BigRiver:
        PEN_THICKNESS(3, 3, 3, 2, 2, 1, 1, 1, 1)
        drawColor.setRgb(100, 120, 255);
        fillColor.setRgb(100, 120, 255);
        break;
      case MidLake:
      case MidRiver:
        PEN_THICKNESS(2, 2, 2, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(100, 120, 255);
        fillColor.setRgb(100, 120, 255);
        break;
      case SmallLake:
      case SmallRiver:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(100, 120, 255);
        fillColor.setRgb(100, 120, 255);
        break;
      case HugeCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        break;
      case BigCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        break;
      case MidCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        break;
      case SmallCity:
        PEN_THICKNESS(1, 1, 1, 1, 1, 1, 1, 1, 1)
        drawColor.setRgb(0, 0, 0);
//        fillBrush = QBrush(QColor(245,220,0),QBrush::Dense3Pattern);
//        fillBrush = QBrush(QColor(255,255,0), QBrush::SolidPattern);
        fillBrush = QBrush(QColor(245,240,0), QBrush::SolidPattern);
        break;
    }
}

LineElement::~LineElement()
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
    QPointArray pArray;// = __projectElement(dX, dY, mapCenterLon, scale);

    if(isClosed) {
      printPainter->drawPolygon(pArray);
    } else {
      printPainter->drawPolyline(pArray);
      if(highwayShow) {
        printPainter->setPen(QPen(QColor(255,255,255), 1));
        printPainter->drawPolyline(pArray);
      }
    }
  }
}

QRegion* LineElement::drawRegion(QPainter* targetPainter)
{
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

  targetPainter->setBrush(fillBrush);
  targetPainter->setPen(QPen(drawColor, drawThickness[index], drawPenStyle));
  targetPainter->drawPolygon(tA);

  return (new QRegion(tA));
}

void LineElement::drawMapElement(QPainter* targetPainter)
{
  extern const MapMatrix _globalMapMatrix;

//  if(!__isVisible(mapBorder)) return;

  extern const double _currentScale, _scale[];
  extern const int _scaleBorder[];

  bool show = true;
  bool highwayShow = false;
  bool isClosed = false;

  int index = 0;
  if(_currentScale <= _scale[2]) index = 1;
  else if(_currentScale <= _scale[3]) index = 2;
  else if(_currentScale <= _scale[4]) index = 3;
  else if(_currentScale <= _scale[5]) index = 4;
  else if(_currentScale <= _scale[6]) index = 5;
  else if(_currentScale <= _scale[7]) index = 6;
  else if(_currentScale <= _scale[8]) index = 7;
  else index = 8;

  targetPainter->setPen(QPen(drawColor, drawThickness[index]));

  if(typeID == Highway && _currentScale < _scale[5]) highwayShow = true;

  if(_currentScale > _scale[_scaleBorder[typeID]])
      show = false;

  // Kann mal in den Konstruktor wandern ...
  if(typeID == BigLake || typeID == MidLake || typeID == SmallLake
                       || typeID == Glacier)
    {
      isClosed = true;
      targetPainter->setBrush(QBrush(fillColor, QBrush::SolidPattern));
    }

  if(show)
    {
      QPointArray pArray = _globalMapMatrix.map(projPointArray);
      if(isClosed)
        {
          targetPainter->drawPolygon(pArray);
        }
      else
        {
          targetPainter->drawPolyline(pArray);
          if(highwayShow)
            {
              // Mittellinie zeichnen
              targetPainter->setPen(QPen(QColor(255,255,255), 1));
              targetPainter->drawPolyline(pArray);
            }
        }
    }
}

//void LineElement::setPoints(QPointArray newPA)
//{
//  projPointArray = newPA;
//}

bool LineElement::__isVisible()
{
  extern const MapMatrix _globalMapMatrix;
  return _globalMapMatrix.isVisible(bBox);
}
