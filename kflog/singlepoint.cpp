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

SinglePoint::SinglePoint( const QString& n,
                          const QString& shortName,
                          const BaseMapElement::objectType t,
                          const WGSPoint& wgsP,
                          const QPoint& pos,
                          const unsigned int elev,
                          const unsigned short secID,
                          unsigned int lmType )
  : BaseMapElement(n, t, secID),
    wgsPosition(wgsP),
    position(pos),
    shortName(shortName),
    curPos(pos),
    elevation(elev),
    lm_typ(lmType)
{
}

SinglePoint::~SinglePoint()
{
}

void SinglePoint::printMapElement(QPainter* printPainter, bool isText )
{
  Q_UNUSED( isText )

  if( !isVisible() )
    {
      return;
    }

  QPoint printPos(glMapMatrix->print(position));

  int iconSize = 16;

  if(glMapMatrix->isSwitchScale())
      iconSize = 8;

  /*
   * Hier sollte mal für eine bessere Qualität der Icons gesorgt werden.
   * Eventuell kann man die Icons ja hier zeichnen lassen ?!?
   */
  printPainter->setPen(QPen(QColor(0,50,50), 2));
  printPainter->setBrush(Qt::NoBrush);
  printPainter->drawEllipse(printPos.x() - 5, printPos.y() - 5, 10, 10);
  return;

  printPainter->drawPixmap( printPos.x() - iconSize, printPos.x() - iconSize,
                            glConfig->getPixmap(typeID) );
}

bool SinglePoint::drawMapElement(QPainter* targetP, QPainter* maskP)
{
  if(! isVisible() )
    {
      curPos = QPoint(-5000, -5000);
      return false;
    }

  curPos = glMapMatrix->map(position);

  targetP->setPen(QPen(QColor(0,255,255), 3));
  int iconSize = 8;

  if(typeID == BaseMapElement::Village)
   {
      targetP->setPen(QPen(QColor(0, 0, 0), 3));
      maskP->setPen(QPen(Qt::color1, 2));
      maskP->setBrush(Qt::NoBrush);
      maskP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      targetP->setBrush(Qt::NoBrush);
      targetP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      return true;
   }
  else if(typeID == BaseMapElement::Landmark)
   {
      switch(lm_typ)
      {
        case LM_DAM:
        case LM_LOCK:
        case LM_MINE:
        case LM_INDUSTRY:
        case LM_DEPOT:
        case LM_SALT:
        case LM_OILFIELD:
        case LM_TOWER:
        case LM_SETTLEMENT:
        case LM_BUILDING:
        case LM_CASTLE:
        case LM_RUIN:
        case LM_MONUMENT:
        case LM_LIGHTHOUSE:
        case LM_STATION:
        case LM_BRIDGE:
        case LM_FERRY:
          targetP->setPen(QPen(QColor(100, 100, 100), 3));
          break;
        case LM_UNKNOWN:
        default:
          targetP->setPen(QPen(QColor(255, 255, 255), 3));
          break;
      }
      maskP->setPen(QPen(Qt::color1, 2));
      maskP->setBrush(Qt::NoBrush);
      maskP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      targetP->setBrush(Qt::NoBrush);
      targetP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      return true;
   }

  if(glMapMatrix->isSwitchScale())
      iconSize = 16;

  targetP->drawPixmap(curPos.x() - iconSize, curPos.y() - iconSize,
      glConfig->getPixmap(typeID));

  return true;
}
