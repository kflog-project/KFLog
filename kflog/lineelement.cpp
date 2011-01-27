/***********************************************************************
 **
 **   lineelement.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2009 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QColor>

#include "lineelement.h"

LineElement::LineElement( const QString& name,
                          const BaseMapElement::objectType oType,
                          const QPolygon& pP,
                          const bool isV,
                          const unsigned short secID )
  : BaseMapElement(name, oType, secID),
    projPolygon(pP),
    bBox(pP.boundingRect()),
    valley(isV),
    closed(false)
{
  if( typeID == BaseMapElement::Lake ||
      typeID == BaseMapElement::City ||
      typeID == BaseMapElement::Forest ||
      typeID == BaseMapElement::PackIce ||
      typeID == BaseMapElement::Glacier ||
      typeID == BaseMapElement::Lake_T )
    {
      closed = true;
    }
}

LineElement::~LineElement()
{
}

bool LineElement::drawMapElement(QPainter* targetP)
{
  // If the element-type should not be drawn in the actual scale, or if the
  // element is not visible, return.
  if( ! glConfig->isBorder(typeID) || ! isVisible() )
    {
      return false;
    }

  QPolygon mP( glMapMatrix->map( projPolygon ) );

  if(typeID == BaseMapElement::City)
    {
      // We do not draw the outline of the city directly, because otherwise
      // we will get into trouble with cities lying at the edge of a
      // map-section. So we use a thicker draw a line into the mask-painter.
      QBrush drawB = glConfig->getDrawBrush( typeID );
      targetP->setPen( QPen( drawB.color(), 0, Qt::NoPen ) );
      targetP->setBrush( drawB );
      targetP->drawPolygon( mP );
      return true;
    }

  QPen drawP( glConfig->getDrawPen( typeID ) );
  targetP->setPen(drawP);

  if( closed )
    {
      // Lakes do not have a brush, because they are divided into normal
      // sections and we do not want to see section-borders in a lake ...
      targetP->setBrush( glConfig->getDrawBrush( typeID ) );

      if( typeID == BaseMapElement::Lake )
        {
          QBrush brush = targetP->brush();
          brush.setStyle( Qt::SolidPattern );
          targetP->setBrush( brush );
        }

      targetP->drawPolygon( mP );
      return true;
    }

  targetP->drawPolyline(mP);

  if(typeID == BaseMapElement::Highway && drawP.width() > 4)
    {
      // draw the white line in the middle
      targetP->setPen(QPen(Qt::white, 1));
      targetP->drawPolyline(mP);
    }

  return true;
}

void LineElement::printMapElement( QPainter* printPainter, bool isText )
{
  Q_UNUSED( isText )

  if( !glConfig->isPrintBorder( typeID ) )
    {
      return;
    }

  printPainter->setPen( glConfig->getPrintPen( typeID ) );

  if( closed )
    {
      printPainter->setBrush( glConfig->getPrintBrush( typeID ) );
      printPainter->drawPolygon( glMapMatrix->print( projPolygon ) );
    }
  else
    {
      printPainter->drawPolyline( glMapMatrix->print( projPolygon ) );
    }
}
