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
#include <kflogconfig.h>
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

  if(typeID == BaseMapElement::Lake || typeID == BaseMapElement::City)
      closed = true;
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

}

void LineElement::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  /* If the element-type should not be drawn in the actual scale, or if the
   * element is not visible, return.
   */
  if(!glConfig->isBorder(typeID) || !__isVisible()) return;

  QPen drawP(glConfig->getPen(typeID));

  QPointArray pA = glMapMatrix->map(projPointArray);

  if(valley)
    {
      maskPainter->setPen(QPen(Qt::color0, drawP.width(), drawP.style()));
      maskPainter->setBrush(QBrush(Qt::color0, QBrush::SolidPattern));
    }
  else
    {
      maskPainter->setPen(QPen(Qt::color1, drawP.width(), drawP.style()));
      maskPainter->setBrush(QBrush(Qt::color1, Qt::SolidPattern));
    }

  if(typeID == BaseMapElement::City)
    {
      /*
       * We do not draw an outline of the city directly, because otherwise
       * we will get trouble with cities lying at the edge of a map-section.
       * So we thicker draw a line into the mask-painter.
       */
      maskPainter->setPen(QPen(Qt::color1, drawP.width() * 2));
      maskPainter->drawPolygon(pA);

      QBrush drawB = glConfig->getBrush(typeID);
      targetPainter->setPen(QPen(drawB.color(), 0, Qt::NoPen));
      targetPainter->setBrush(drawB);
      targetPainter->drawPolygon(pA);

      return;
    }

  targetPainter->setPen(drawP);

  if(closed)
    {
      // Lakes do not have a brush, because they are devided into normal
      // sections and we do not want to see section-borders in a lake ...
      if(typeID == BaseMapElement::Lake)
          targetPainter->setBrush(QBrush(drawP.color(), QBrush::SolidPattern));
      else
          targetPainter->setBrush(glConfig->getBrush(typeID));

      maskPainter->drawPolygon(pA);
      targetPainter->drawPolygon(pA);
    }
  else
    {
      maskPainter->drawPolyline(pA);
      targetPainter->drawPolyline(pA);
      if(typeID == Highway && drawP.width() > 4)
        {
          // Mittellinie zeichnen
          targetPainter->setPen(QPen(QColor(255,255,255), 1));
          targetPainter->drawPolyline(pA);
        }
    }
}

bool LineElement::__isVisible() const  {  return glMapMatrix->isVisible(bBox);  }

bool LineElement::isValley() const  {  return valley;  }
