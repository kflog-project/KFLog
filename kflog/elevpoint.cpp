/***********************************************************************
**
**   elevpoint.cpp
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

#include "elevpoint.h"

#include <mapcalc.h>

#include <kapp.h>
#include <kiconloader.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

ElevPoint::ElevPoint(QString n, QString g, unsigned int t, QPoint pos,
        unsigned int e)
: SinglePoint(n, g, t, pos),
  elevation(e)
{

}

ElevPoint::~ElevPoint()
{

}

void ElevPoint::printMapElement(QPainter* printPainter)
{
  if(!__isVisible()) return;
}

void ElevPoint::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  if(!__isVisible())  return;

  QString temp;

  if(typeID == Spot) {
//      // we don't have an icon for the spots ...
//      QPointArray pointArray(3);
//      pointArray.setPoint(0, QPoint(curPos.x() - 3, curPos.y()));
//      pointArray.setPoint(1, QPoint(curPos.x() + 3, curPos.y()));
//      pointArray.setPoint(2, QPoint(curPos.x(), curPos.y() - 5));
//      targetPainter->setPen(QPen(QColor(0,0,0), 1));
//      targetPainter->setBrush(QBrush(QBrush::SolidPattern));
//      targetPainter->drawPolygon(pointArray);
//      temp.sprintf("%d", elevation);
//      targetPainter->drawText(curPos.x() - 15, curPos.y() - 8,temp);
    return;
  }

  targetPainter->setPen(QPen(QColor(0,0,0), 2));
}

unsigned int ElevPoint::getElevation() const { return elevation; }
