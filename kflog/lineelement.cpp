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
#include <mapdefaults.h>

#include <mapcalc.h>
#include <mapmatrix.h>

#include <kapp.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

#define READ_BORDER \
    border[0] = config->readBoolEntry("Border 1", true); \
    border[1] = config->readBoolEntry("Border 2", true); \
    border[2] = config->readBoolEntry("Border 3", true); \
    border[3] = config->readBoolEntry("Border 4", true); \
    border[4] = config->readBoolEntry("Border 5", true);

#define READ_PEN(c1,c2,c3,c4,p1,p2,p3,p4) \
    drawPenSize[0] = config->readNumEntry("Pen Size 1", p1); \
    drawColor[0] = config->readColorEntry("Color 1", new c1); \
    drawPenSize[1] = config->readNumEntry("Pen Size 2", p1); \
    drawColor[1] = config->readColorEntry("Color 2", new c1); \
    drawPenSize[2] = config->readNumEntry("Pen Size 3", p1); \
    drawColor[2] = config->readColorEntry("Color 3", new c1); \
    drawPenSize[3] = config->readNumEntry("Pen Size 4", p1); \
    drawColor[3] = config->readColorEntry("Color 4", new c1);

LineElement::LineElement(QString n, unsigned int t, QPointArray pA, bool isV)
: BaseMapElement(n, t), drawPenStyle(QPen::SolidLine),
  fillBrushStyle(QBrush::SolidPattern),
  projPointArray(pA), bBox(pA.boundingRect()), valley(isV), closed(false)
{
  fillColor.setRgb(100, 255, 100);

  border = new bool[5];
  drawPenSize = new int[5];
  drawColor = new QColor[5];

  // Das muss nur bei "echten" LineElementen ablaufen.
  readConfig();
}

LineElement::~LineElement()
{

}

void LineElement::readConfig()
{
  KConfig* config = KGlobal::config();

  drawColor[4] = Qt::color0;
  drawPenSize[4] = 0;


  switch(typeID)
    {
      case Highway:
        config->setGroup("Highway");
        READ_PEN(HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
            HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4)
        break;
      case Road:
        config->setGroup("Road");
        READ_PEN(ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
            ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4)
        break;
      case Railway:
        config->setGroup("Rail");
        READ_PEN(RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
            RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4)
        break;
      case Lake:
        closed = true;
      case River:
        config->setGroup("River");
        READ_PEN(RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
            RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4)
        break;
      case Canal:
        config->setGroup("Canal");
        READ_PEN(CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3, CANAL_COLOR_4,
            CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4)
        break;
      case City:
        config->setGroup("City");
        READ_PEN(CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3, CITY_COLOR_4,
            1, 1, 1, 1)
        drawColor[4] = config->readColorEntry("Outline Color",
            new CITY_COLOR_5);
        drawPenSize[4] = config->readNumEntry("Outline Size", 1);
        closed = true;
        break;
    }

  READ_BORDER
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
    case Road:
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
    case Lake:
      isClosed = true;
    case River:
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
  if(_currentScale <= _scale[0]) index = 1;
  else if(_currentScale <= _scale[1]) index = 2;
  else if(_currentScale <= _scale[2]) index = 3;

  if(valley)
    {
      maskPainter->setBrush(QBrush(Qt::color0, fillBrushStyle));
    }
  else
    {
      maskPainter->setBrush(QBrush(Qt::color1, fillBrushStyle));
    }
  maskPainter->setPen(QPen(Qt::color1, drawPenSize[index], drawPenStyle));
  maskPainter->drawPolygon(tA);

  targetPainter->setBrush(QBrush(fillColor, fillBrushStyle));
  targetPainter->setPen(QPen(drawColor[index], drawPenSize[index]));
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
  if(_currentScale <= _scale[1]) index = 0;
  else if(_currentScale <= _scale[2]) index = 1;
  else if(_currentScale <= _scale[3]) index = 2;
  else index = 3;

  if(valley)
    {
      maskPainter->setPen(QPen(Qt::color0, drawPenSize[index]));
      maskPainter->setBrush(QBrush(Qt::color0, QBrush::SolidPattern));
    }
  else
    {
      maskPainter->setPen(QPen(Qt::color1, drawPenSize[index]));
      maskPainter->setBrush(QBrush(Qt::color1, QBrush::SolidPattern));
    }

  targetPainter->setPen(QPen(drawColor[index], drawPenSize[index]));

  // Hier wird immer mit der gleichen Farbe gefüllt ...
  targetPainter->setBrush(QBrush(drawColor[index], QBrush::SolidPattern));

  QPointArray pA = _globalMapMatrix.map(projPointArray);

  /********************************/
  if(typeID == BaseMapElement::City && isFirst)
    {
      targetPainter->setPen(QPen(drawColor[4], drawPenSize[4] * 3));
      maskPainter->setPen(QPen(Qt::color1, drawPenSize[4] * 3));
      maskPainter->drawPolygon(pA);
      targetPainter->drawPolyline(pA);

      return;
    }
  else if(typeID == BaseMapElement::City && !isFirst)
    {
      targetPainter->setPen(QPen(drawColor[index], 0));
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
      if(typeID == Highway && drawPenSize[index] > 4)
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
