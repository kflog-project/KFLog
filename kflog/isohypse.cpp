/***********************************************************************
 **
 **   isohypse.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 by Axel Pauli, Josua Dietze
 **                   2009-2010 by Axel Pauli, Peter Turczak
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QPainterPath>
#include <QString>
#include <QSize>

#include <QtCore>

#include "isohypse.h"
#include "mapcalc.h"
#include "mapmatrix.h"

extern MapMatrix* _globalMapMatrix;
extern MapConfig* _globalMapConfig;

Isohypse::Isohypse( QPolygon elevationCoordinates,
                    const short elevation,
                    const uchar  elevationIndex,
                    const ushort secID,
                    const char typeID ) :
    LineElement( "Isoline", BaseMapElement::Isohypse, elevationCoordinates, false, secID ),
    _elevation(elevation),
    _elevationIndex(elevationIndex),
    _typeID(typeID)
{}

Isohypse::~Isohypse()
{}

QPainterPath* Isohypse::drawRegion( QPainter* targetP, const QRect &viewRect,
                                    bool really_draw, bool isolines )
{

  if( isVisible() == false )
    {
      return static_cast<QPainterPath *> (0);
    }

  QPolygon mP = glMapMatrix->map(projPolygon);

  if (really_draw)
    {
      if (mP.boundingRect().isNull())
        {
          // ignore null values and return also no region
          return static_cast<QPainterPath *> (0);
        }

      targetP->setClipRegion(viewRect);

      targetP->drawPolygon(mP);

      if( isolines )
        {
          targetP->drawPolyline(mP);
        }
    }

  QPainterPath *path = new QPainterPath;
  path->addPolygon(projPolygon);
  path->closeSubpath();
  return path;
}

bool Isohypse::isVisible() const
{
  // Check, if this isohypse tile has a map overlapping otherwise we can ignore
  // it completely.
  return getTileBox(getMapSegment()).intersects(_globalMapMatrix->getViewBorder());
}
