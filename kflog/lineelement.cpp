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

}

void LineElement::drawMapElement(QPainter* targetPainter, QPainter* maskPainter,
    bool isFirst)
{
  if(!__isVisible()) return;

  int index = glMapMatrix->getScaleRange();

  if(!border[index]) return;

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

  QPointArray pA = glMapMatrix->map(projPointArray);

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

bool LineElement::__isVisible() const  {  return glMapMatrix->isVisible(bBox);  }

bool LineElement::isValley() const  {  return valley;  }
