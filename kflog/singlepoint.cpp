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
#include <mapmatrix.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

SinglePoint::SinglePoint(QString n, QString abbr, unsigned int t,
  QPoint pos, bool wP)
: BaseMapElement(n, t),
  position(pos), isWaypoint(wP), abbrev(abbr), curPos(pos)
{
  switch(typeID)
    {
      case UltraLight:
        iconName = "ul.xpm";
        break;
      case HangGlider:
        iconName = "paraglider.xpm";
        break;
      case Parachute:
        iconName = "jump.xpm";
        break;
      case Ballon:
        iconName = "ballon.xpm";
        break;
      case CompPoint:
        iconName = "compoint.xpm";
        break;
      case Landmark:
        iconName = "landmark.xpm";
        break;
      default:
        iconName = "";
        break;
    }
}

SinglePoint::~SinglePoint()
{

}

void SinglePoint::printMapElement(QPainter* printPainter)
{

}

void SinglePoint::printMapElement(QPainter* printPainter, const double dX,
      const double dY, const int mapCenterLon, const double scale,
      const struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  struct drawPoint printPos;// = __projectElement(dX, dY, mapCenterLon, scale);

  printPainter->setPen(QPen(QColor(0,0,0), 2));
  int iconSize = 16;

  QString iconName;
  bool show = true;

  /*
   * Hier sollte mal für eine bessere Qualität der Icons gesorgt werden.
   * Eventuell kann man die Icons ja hier zeichnen lassen ?!?
   */
  if(iconName == 0)
    {
      switch(typeID)
        {
          case Village:
            printPainter->setBrush(QBrush::NoBrush);
            printPainter->drawEllipse(printPos.x - 5, printPos.y - 5, 10, 10);
            return;
        }
    }

//    printPainter->drawPixmap(printPos.x - iconSize, printPos.y - iconSize,
//              Icon(KApplication::kde_datadir() + kflog_dir + iconName));
}

bool SinglePoint::__isVisible() const
{
//  return glMapMatrix->isVisible(glMapMatrix->map(position));
  return glMapMatrix->isVisible(position);
}

void SinglePoint::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  if(!__isVisible())
    {
      curPos = QPoint(-50, -50);
      return;
    }

  targetPainter->setPen(QPen(QColor(0,0,0), 2));
  int iconSize = 16;
  bool show = true;

  if(iconName == 0)
    {
      /* Hier ist eine weitere virtuelle Funktion nötig, die
       * das Element zeichnet!
       */
      switch(typeID)
        {
          case Village:
            targetPainter->setBrush(QBrush::NoBrush);
            targetPainter->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
            return;
        }
    }

  curPos = glMapMatrix->map(position);

  if(glMapMatrix->isSwitchScale())
    {
      targetPainter->drawPixmap(curPos.x() - iconSize, curPos.y() - iconSize,
      QPixmap(KGlobal::dirs()->findResource("appdata", "mapicons/") + iconName));
    }
  else
    {
      targetPainter->drawPixmap(curPos.x() - iconSize / 2, curPos.y() - iconSize / 2,
      QPixmap(KGlobal::dirs()->findResource("appdata", "mapicons/small/") + iconName));
    }
}

void SinglePoint::setWaypoint(bool isW)
{
  isWaypoint = isW;
}

bool SinglePoint::isWayPoint() const { return isWaypoint; }

QString SinglePoint::getWPName() const { return abbrev; }

QPoint SinglePoint::getPosition() const { return position; }

QPoint SinglePoint::getMapPosition() const  { return curPos;  }

QString SinglePoint::getInfoString() const
{
  QString text;

  return text;
}
