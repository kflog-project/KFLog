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

ElevPoint::ElevPoint(QString n, QString abbr, unsigned int t, QPoint pos,
        unsigned int e, bool wP)
: SinglePoint(n, abbr, t, pos, wP),
  elevation(e)
{
  switch(typeID)
    {
      case Outlanding:
        iconName = "outlanding.xpm";
        break;
      case Obstacle:
        iconName = "obstacle.xpm";
        break;
      case LightObstacle:
        iconName = "obst_light.xpm";
        break;
      case ObstacleGroup:
        iconName = "obst_group.xpm";
        break;
      case LightObstacleGroup:
        iconName = "obst_group_light.xpm";
        break;
    }
}

ElevPoint::~ElevPoint()
{

}

void ElevPoint::printMapElement(QPainter* printPainter)
{

}

void ElevPoint::printMapElement(QPainter* printPainter, const double dX,
    const double dY, const int mapCenterLon, const double scale,
    const struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  extern const double _scale[];
  extern const int _scaleBorder[];

  struct drawPoint printPos;// = __projectElement(dX, dY, mapCenterLon, scale);

  printPainter->setPen(QPen(QColor(0,0,0), 2));
  QString height;
  height.sprintf("%d", elevation);
  printPainter->setFont(QFont("helvetica", 10, QFont::Normal, true));
  printPainter->setPen(QPen(QColor(0,0,0), 1));

  switch(typeID) {
    case Outlanding:
      warning("outlanding");
      return;
    case LightObstacle:
      // Kennzeichnung des Lichtes ???
    case Obstacle:
      if(scale <= _scale[_scaleBorder[typeID]]) {
        printPainter->drawLine(printPos.x - 6, printPos.y + 1, printPos.x, printPos.y - 16);
        printPainter->drawLine(printPos.x + 6, printPos.y + 1, printPos.x, printPos.y - 16);
        printPainter->drawPoint(printPos.x, printPos.y);
        printPainter->drawText(printPos.x - 10, printPos.y + 12, height);
      }
      return;
    case LightObstacleGroup:
      // Kennzeichnung des Lichtes ???
    case ObstacleGroup:
      if(scale <= _scale[_scaleBorder[typeID]]) {
        printPainter->drawLine(printPos.x - 9, printPos.y + 1, printPos.x - 3, printPos.y - 16);
        printPainter->drawLine(printPos.x + 3, printPos.y + 1, printPos.x - 3, printPos.y - 16);

        printPainter->drawLine(printPos.x - 3, printPos.y + 1, printPos.x + 3, printPos.y - 16);
        printPainter->drawLine(printPos.x + 9, printPos.y + 1, printPos.x + 3, printPos.y - 16);

        printPainter->drawPoint(printPos.x, printPos.y);
        printPainter->drawText(printPos.x - 10, printPos.y + 12, height);
      }
      return;
    case Spot:
      if(scale <= _scale[_scaleBorder[typeID]]) {
        printPainter->setPen(QPen(QColor(0,0,0), 2));
        printPainter->drawEllipse(printPos.x - 1, printPos.y - 1, 2, 2);
        printPainter->drawText(printPos.x - 10, printPos.y - 4, height);
      }
      return;
  }
}

void ElevPoint::drawMapElement(QPainter* targetPainter, QPainter* maskPainter)
{
  if(!__isVisible())  return;

  extern const double _currentScale, _scale[];
  extern const int _scaleBorder[];

//  curPos = __projectElement(dX, dY, mapCenterLon, _currentScale);
  QString temp;

  if(typeID == Spot) {
    if( _currentScale <= _scale[_scaleBorder[typeID]]) {
      // we don't have an icon for the spots ...
      QPointArray pointArray(3);
      pointArray.setPoint(0, QPoint(curPos.x() - 3, curPos.y()));
      pointArray.setPoint(1, QPoint(curPos.x() + 3, curPos.y()));
      pointArray.setPoint(2, QPoint(curPos.x(), curPos.y() - 5));
      targetPainter->setPen(QPen(QColor(0,0,0), 1));
      targetPainter->setBrush(QBrush(QBrush::SolidPattern));
      targetPainter->drawPolygon(pointArray);
      temp.sprintf("%d", elevation);
      targetPainter->drawText(curPos.x() - 15, curPos.y() - 8,temp);
    }
    return;
  }

  targetPainter->setPen(QPen(QColor(0,0,0), 2));
  char* kflog_dir = "/kflog/map/";
  int iconHeight = 28;  // Positionierung des Punktes im Icon
  int iconWidth = 15;   // auf die richtige Stelle

  if(_currentScale > _scale[ID_BORDER_SMALL])
    {
      kflog_dir = "/kflog/map/small/";
      iconHeight = 16;
      iconWidth = 8;
    }

  if(_currentScale <= _scale[_scaleBorder[typeID]])
    {
      temp.sprintf("%d", elevation);
      targetPainter->drawText(curPos.x() - iconWidth, curPos.y() + 15,temp);
    }
}

unsigned int ElevPoint::getElevation() const { return elevation; }
