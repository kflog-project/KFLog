/***********************************************************************
**
**   isohypse.cpp
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

#include "isohypse.h"

#include <mapmatrix.h>

Isohypse::Isohypse()
: LineElement(0, BaseMapElement::Isohypse, 0)
{

}

Isohypse::Isohypse(QPointArray pA, unsigned int elev, bool isV, int s_ID)
: LineElement(0, BaseMapElement::Isohypse, pA, isV),
  elevation(elev), sort_ID(s_ID)
{
  /* Farbgebung erfolgt in
   *   MapContents::drawIsoList()
   */
}

Isohypse::~Isohypse()
{

}

void Isohypse::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  if(glMapMatrix->isVisible(bBox))
      targetPainter->drawPolygon(glMapMatrix->map(projPointArray));
}

int Isohypse::getElevation() const { return elevation; }

int Isohypse::sortID() const { return sort_ID; }
