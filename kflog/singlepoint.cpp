/***********************************************************************
**
**   singlepoint.cpp
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

#include "singlepoint.h"

#include <kstddirs.h>

SinglePoint::SinglePoint(QString n, QString gps, unsigned int t,
      WGSPoint wgsP, QPoint pos, unsigned int elev, unsigned int ind)
  : BaseMapElement(n, t),
    wgsPosition(wgsP), position(pos), gpsName(gps), curPos(pos), elevation(elev),
    index(ind)
{

}


SinglePoint::~SinglePoint()
{

}

void SinglePoint::printMapElement(QPainter* printPainter, bool isText)
{
  if(!__isVisible()) return;

  QPoint printPos(glMapMatrix->print(position));

  int iconSize = 16;

  if(glMapMatrix->isSwitchScale())
      iconSize = 8;

  /*
   * Hier sollte mal für eine bessere Qualität der Icons gesorgt werden.
   * Eventuell kann man die Icons ja hier zeichnen lassen ?!?
   */
  if(typeID == BaseMapElement::PopulationPlace)
    {
      printPainter->setPen(QPen(QColor(0,0,0), 2));
      printPainter->setBrush(QBrush::NoBrush);
      printPainter->drawEllipse(printPos.x() - 5, printPos.y() - 5, 10, 10);
      return;
    }

  printPainter->drawPixmap(printPos.x() - iconSize, printPos.x() - iconSize,
      glConfig->getPixmap(typeID));
}

bool SinglePoint::__isVisible() const
{
  return glMapMatrix->isVisible(position);
}

void SinglePoint::drawMapElement(QPainter* targetP, QPainter* maskP)
{
  if(!__isVisible())
    {
      curPos = QPoint(-50, -50);
      return;
    }

  targetP->setPen(QPen(QColor(0,0,0), 2));
  int iconSize = 8;

  if(typeID == BaseMapElement::PopulationPlace)
    {
      targetP->setBrush(QBrush::NoBrush);
      targetP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      return;
    }

  curPos = glMapMatrix->map(position);

  if(glMapMatrix->isSwitchScale())
      iconSize = 16;

  targetP->drawPixmap(curPos.x() - iconSize, curPos.y() - iconSize,
      glConfig->getPixmap(typeID));
}

QString SinglePoint::getWPName() const { return gpsName; }

QPoint SinglePoint::getPosition() const { return position; }

WGSPoint SinglePoint::getWGSPosition() const { return wgsPosition; }

QPoint SinglePoint::getMapPosition() const  { return curPos;  }

QString SinglePoint::getInfoString() const  {  return QString();  }

unsigned int SinglePoint::getElevation() const { return elevation; }
