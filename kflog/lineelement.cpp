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

LineElement::LineElement(QString name, unsigned int t, QPointArray pA, bool isV)
  : BaseMapElement(name, t),
    projPointArray(pA), bBox(pA.boundingRect()), valley(isV), closed(false)
{
  if(typeID == BaseMapElement::Lake || typeID == BaseMapElement::City ||
      typeID == BaseMapElement::Forest)
      closed = true;
}

LineElement::~LineElement()
{

}

void LineElement::printMapElement(QPainter* printPainter, bool isText) const
{
  if(!glConfig->isPrintBorder(typeID)) return;

  printPainter->setPen(glConfig->getPrintPen(typeID));

  if(closed)
    {
      printPainter->setBrush(glConfig->getPrintBrush(typeID));
      printPainter->drawPolygon(glMapMatrix->print(projPointArray));
    }
  else
    {
      printPainter->drawPolyline(glMapMatrix->print(projPointArray));
    }
}

void LineElement::drawMapElement(QPainter* targetP, QPainter* maskP)
{
  //
  // If the element-type should not be drawn in the actual scale, or if the
  // element is not visible, return.
  //
  if(!glConfig->isBorder(typeID) || !__isVisible()) return;

  QPen drawP(glConfig->getDrawPen(typeID));

  QPointArray pA = glMapMatrix->map(projPointArray);

  if(valley)
    {
      maskP->setPen(QPen(Qt::color0, drawP.width(), drawP.style()));
      maskP->setBrush(QBrush(Qt::color0, QBrush::SolidPattern));
    }
  else
    {
      maskP->setPen(QPen(Qt::color1, drawP.width(), drawP.style()));
//      maskP->setBrush(QBrush(Qt::color1, Qt::SolidPattern));
      maskP->setBrush(QBrush(Qt::color1, glConfig->getDrawBrush(typeID).style()));
    }

  if(typeID == BaseMapElement::City)
    {
      //
      // We do not draw the outline of the city directly, because otherwise
      // we will get into trouble with cities lying at the edge of a
      // map-section. So we use a thicker draw a line into the mask-painter.
      //
      maskP->setPen(QPen(Qt::color1, drawP.width() * 2));
      maskP->drawPolygon(pA);

      QBrush drawB = glConfig->getDrawBrush(typeID);
      targetP->setPen(QPen(drawB.color(), 0, Qt::NoPen));
      targetP->setBrush(drawB);
      targetP->drawPolygon(pA);

      return;
    }

  targetP->setPen(drawP);

  if(closed)
    {
      //
      // Lakes do not have a brush, because they are devided into normal
      // sections and we do not want to see section-borders in a lake ...
      //
      if(typeID == BaseMapElement::Lake)
          targetP->setBrush(QBrush(drawP.color(), QBrush::SolidPattern));
      else
          targetP->setBrush(glConfig->getDrawBrush(typeID));

      //
      // Forests do not have an outline.
      //
      if(typeID == BaseMapElement::Forest)
        {
          maskP->setPen(QPen(Qt::color1, 0, Qt::NoPen));
          targetP->setPen(QPen(drawP.color(), 0, Qt::NoPen));
        }

      maskP->drawPolygon(pA);
      targetP->drawPolygon(pA);
    }
  else
    {
      maskP->drawPolyline(pA);
      targetP->drawPolyline(pA);
      if(typeID == Highway && drawP.width() > 4)
        {
          // draw the white line in the middle
          targetP->setPen(QPen(QColor(255,255,255), 1));
          targetP->drawPolyline(pA);
        }
    }
}

bool LineElement::__isVisible() const {  return glMapMatrix->isVisible(bBox);  }

bool LineElement::isValley() const  {  return valley;  }
