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

#include "mapmatrix.h"

Isohypse::Isohypse(QPointArray pA, unsigned int elev, bool isV)
: LineElement(0, BaseMapElement::Isohypse, pA, isV),
  elevation(elev)
{
  regionStored=false;
}

Isohypse::~Isohypse()
{

}

QRegion* Isohypse::drawRegion(QPainter* targetP, QPainter* maskP)
{
  if(glMapMatrix->isVisible(bBox)) {
      targetP->drawPolygon(glMapMatrix->map(projPointArray));
      QPointArray tA = glMapMatrix->map(projPointArray);
      return new QRegion(tA);
  }
  return 0;
}

int Isohypse::getElevation() const { return elevation; }


/*!
    \fn Isohypse::getRegion
 */
QRegion* Isohypse::getRegion()
{
  //qDebug("getting rect");
  //QRect br=projPointArray.boundingRect();
  //for (int i=0; i<projPointArray.count();i++) qDebug("  %d, %d", projPointArray.point(i).x(), projPointArray.point(i).y());
  //qDebug("bounding rect: %d, %d, %d, %d", br.left(), br.top(), br.right(), br.bottom());

  //AS: This may lead to a crash with large coordinates, or so it seems.
  return new QRegion(projPointArray);
}
