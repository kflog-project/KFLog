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

void RadioPoint::printMapElement(QPainter* printPainter)
{

}

void RadioPoint::printMapElement(QPainter* printPainter, const double dX,
      const double dY, const int mapCenterLon, const double scale,
      const struct elementBorder mapBorder)
{

}
