/***********************************************************************
**
**   radiopoint.cpp
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

#include "radiopoint.h"

#include <mapcalc.h>

#include <kapp.h>
#include <kiconloader.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

RadioPoint::RadioPoint(QString n, QString abbr, unsigned int t, QPoint pos,
    const char* f, const char* a, bool wP)
: SinglePoint(n, abbr, t, pos, wP),
  frequency(f), alias(a)
{
  switch (typeID)
    {
      case VOR:
        iconName = "vor.xpm";
        break;
      case VORDME:
        iconName = "vordme.xpm";
        break;
      case VORTAC:
        iconName = "vortac.xpm";
        break;
      case NDB:
        iconName = "ndb.xpm";
        break;
    }
}

RadioPoint::~RadioPoint()
{

}

void RadioPoint::printMapElement(QPainter* printPainter, const double dX,
      const double dY, const int mapCenterLon, const double scale,
      const struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  extern const double _scale[];
  extern const bool _showElements[];

  struct drawPoint printPos;// = __projectElement(dX, dY,
//      mapCenterLon, scale);

  printPainter->setPen(QPen(QColor(0,0,0), 2));
  char* kflog_dir = "/kflog/map/";
  int iconSize = 16;

  if(scale > _scale[ID_BORDER_SMALL]) {
    kflog_dir = "/kflog/map/small/";
    iconSize = 8;
  }

  if(_showElements[typeID]) {
//    printPainter->drawPixmap(printPos.x - iconSize, printPos.y - iconSize,
//      Icon(KApplication::kde_datadir() + kflog_dir + iconName));
  }
}
