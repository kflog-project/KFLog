/***********************************************************************
**
**   glidersite.cpp
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

#include "glidersite.h"

#include <mapcalc.h>

#include <kapp.h>
#include <kiconloader.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

GliderSite::GliderSite(QString n, QString abbr, QPoint pos, unsigned int elev,
        const char* f, bool w, bool wP)
: RadioPoint(n, abbr, BaseMapElement::Glidersite, pos, f, 0, wP),
  elevation(elev), winch(w)
{
  if(winch) iconName = "glider.xpm";
  else iconName = "glider2.xpm";
}

GliderSite::~GliderSite()
{

}

QString GliderSite::getFrequency() const { return frequency; }

unsigned int GliderSite::getElevation() const { return elevation; }

struct runway GliderSite::getRunway(int index) const { return rwData[index]; }

unsigned int GliderSite::getRunwayNumber() const { return rwNum; }

bool GliderSite::isWinch() const { return winch; }

void GliderSite::printMapElement(QPainter* printPainter)
{

}

void GliderSite::printMapElement(QPainter* printPainter, const double dX,
    const double dY, const int mapCenterLon, const double scale,
    const struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  struct drawPoint printPos;// = __projectElement(dX, dY, mapCenterLon, scale);

  int iconSize = 20;

  QPointArray pointArray(5);

  printPainter->setBrush(QBrush::NoBrush);

  printPainter->setPen(QPen(QColor(255,255,255), 7));
  printPainter->drawEllipse(printPos.x - (iconSize / 2),
        printPos.y - (iconSize / 2), iconSize, iconSize);

  printPainter->setPen(QPen(QColor(0,0,0), 3));
  printPainter->drawEllipse(printPos.x - (iconSize / 2),
        printPos.y - (iconSize / 2), iconSize, iconSize);

  printPainter->setPen(QPen(QColor(255,255,255), 7));
  pointArray.setPoint(0, printPos.x - iconSize , printPos.y + 2);
  pointArray.setPoint(1, printPos.x - (iconSize / 2),
                    printPos.y - (iconSize / 2) + 4);
  pointArray.setPoint(2, printPos.x, printPos.y + 2);
  pointArray.setPoint(3, printPos.x + (iconSize / 2),
                    printPos.y - (iconSize / 2) + 4);
  pointArray.setPoint(4, printPos.x + iconSize , printPos.y + 2);

  printPainter->drawPolyline(pointArray);
  printPainter->setPen(QPen(QColor(0, 0, 0), 3));
  printPainter->drawPolyline(pointArray);

  printPainter->setFont(QFont("helvetica", 10, QFont::Bold));
  printPainter->drawText(printPos.x - 15, printPos.y + iconSize + 4, name);
  printPainter->drawText(printPos.x - 15, printPos.y + iconSize + 14, frequency);
}
