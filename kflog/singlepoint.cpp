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

SinglePoint::SinglePoint(QString n, QString abbr, unsigned int t,
      QPoint pos, bool wP)
  : BaseMapElement(n, t),
    position(pos), isWaypoint(wP), abbrev(abbr), curPos(pos)
{

}

SinglePoint::~SinglePoint()
{

}

void SinglePoint::printMapElement(QPainter* printPainter) const
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
  if(typeID == BaseMapElement::Village)
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
  int iconSize = 16;

  if(typeID == BaseMapElement::Village)
    {
      targetP->setBrush(QBrush::NoBrush);
      targetP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      return;
    }

  curPos = glMapMatrix->map(position);

  if(glMapMatrix->isSwitchScale())
      targetP->drawPixmap(curPos.x() - iconSize, curPos.y() - iconSize,
          glConfig->getPixmap(typeID));
  else
      targetP->drawPixmap(curPos.x() - iconSize, curPos.y() - iconSize,
          glConfig->getPixmap(typeID));
}

void SinglePoint::setWaypoint(bool isW)  {  isWaypoint = isW;  }

bool SinglePoint::isWayPoint() const { return isWaypoint; }

QString SinglePoint::getWPName() const { return abbrev; }

QPoint SinglePoint::getPosition() const { return position; }

QPoint SinglePoint::getMapPosition() const  { return curPos;  }

QString SinglePoint::getInfoString() const  {  return QString();  }
