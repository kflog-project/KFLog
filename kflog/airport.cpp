/***********************************************************************
**
**   airport.cpp
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

#include "airport.h"

#include <mapcalc.h>

#include <kapp.h>
#include <kiconloader.h>
#include <klocale.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

Airport::Airport(QString n, QString a, QString abbr, unsigned int t,
  QPoint pos, unsigned int e, const char* f, bool v, bool wP)
: RadioPoint(n, abbr, t, pos, f, 0, wP),
  elevation(e), vdf(v)
{
  switch(typeID)
    {
      case BaseMapElement::Airport:
        iconName = "airport.xpm";
        break;
      case MilAirport:
        iconName = "milairport.xpm";
        break;
      case CivMilAirport:
        iconName = "civmilair.xpm";
        break;
      case Airfield:
        iconName = "airfield_c.xpm";
        break;
      case ClosedAirfield:
        iconName = "closed.xpm";
        break;
      case CivHeliport:
        iconName = "civheliport.xpm";
        break;
      case MilHeliport:
        iconName = "milheliport.xpm";
        break;
      case AmbHeliport:
        iconName = "ambheliport.xpm";
        break;
    }
}

Airport::~Airport()
{

}

QString Airport::getFrequency() const { return frequency; }

unsigned int Airport::getElevation() const { return elevation; }

struct runway Airport::getRunway(int index) const { return rwData[index]; }

unsigned int Airport::getRunwayNumber() const { return rwNum; }

QString Airport::getInfoString() const
{
  QString text, temp;

  temp.sprintf("%d", elevation);
  text = "<TABLE BORDER=0><TR><TD>"
      "<IMG SRC=/opt/kde/share/apps/kflog/map_icons/" + iconName + ">" +
      "</TD><TD>" + name + " (" + alias + ")</TD></TR>" +
      "<TR><TD></TD><TD><FONT SIZE=-1>" + temp + "m" +
      "<BR>" + frequency + "</FONT></TD></TR></TABLE>";

  return text;
}

void Airport::printMapElement(QPainter* printPainter)
{
warning("Drucke Luftraum ...");
}

void Airport::printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  extern const double _scale[];

  struct drawPoint printPos;// = __projectElement(dX, dY,
//      mapCenterLon, scale);

  printPainter->setPen(QPen(QColor(0,0,0), 3));
  printPainter->setBrush(QBrush::NoBrush);
  printPainter->setFont(QFont("helvetica", 10));

  char* kflog_dir = "/kflog/map/";
  int iconSize = 20;

  if(scale > _scale[ID_BORDER_SMALL]) {
    kflog_dir = "/kflog/map/small/";
    iconSize = 10;
  }

  QString iconName;
  switch(typeID) {
    case BaseMapElement::Airport:
      printPainter->setPen(QPen(QColor(255,255,255), 7));
      printPainter->drawLine(printPos.x, printPos.y - iconSize + 4,
          printPos.x, printPos.y - iconSize / 2);
      printPainter->drawLine(printPos.x, printPos.y + iconSize - 4,
          printPos.x, printPos.y + iconSize / 2);
      printPainter->drawLine(printPos.x - iconSize + 4, printPos.y,
          printPos.x - iconSize / 2, printPos.y);
      printPainter->drawLine(printPos.x + iconSize - 4, printPos.y,
          printPos.x + iconSize / 2, printPos.y);
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);

      printPainter->setPen(QPen(QColor(0,0,0), 3));
      printPainter->drawLine(printPos.x, printPos.y - iconSize + 4,
          printPos.x, printPos.y - iconSize / 2);
      printPainter->drawLine(printPos.x, printPos.y + iconSize - 4,
          printPos.x, printPos.y + iconSize / 2);
      printPainter->drawLine(printPos.x - iconSize + 4, printPos.y,
          printPos.x - iconSize / 2, printPos.y);
      printPainter->drawLine(printPos.x + iconSize - 4, printPos.y,
          printPos.x + iconSize / 2, printPos.y);
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawText(printPos.x - 10, printPos.y + iconSize + 4, name);
      break;
    case MilAirport:
      printPainter->setPen(QPen(QColor(255,255,255), 7));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawEllipse(printPos.x - iconSize/4,
          printPos.y - iconSize/4, iconSize/2, iconSize/2);

      printPainter->setPen(QPen(QColor(0,0,0), 3));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);

      printPainter->setPen(QPen(QColor(0,0,0), 1));
      printPainter->drawEllipse(printPos.x - iconSize/4,
          printPos.y - iconSize/4, iconSize/2, iconSize/2);
      printPainter->drawText(printPos.x - 10, printPos.y + iconSize + 4, name);
      break;
    case CivMilAirport:
      printPainter->setPen(QPen(QColor(255,255,255), 7));
      printPainter->drawLine(printPos.x, printPos.y - iconSize + 4,
          printPos.x, printPos.y - iconSize / 2);
      printPainter->drawLine(printPos.x, printPos.y + iconSize - 4,
          printPos.x, printPos.y + iconSize / 2);
      printPainter->drawLine(printPos.x - iconSize + 4, printPos.y,
          printPos.x - iconSize / 2, printPos.y);
      printPainter->drawLine(printPos.x + iconSize - 4, printPos.y,
          printPos.x + iconSize / 2, printPos.y);
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawEllipse(printPos.x - iconSize/4,
          printPos.y - iconSize/4, iconSize/2, iconSize/2);

      printPainter->setPen(QPen(QColor(0,0,0), 3));
      printPainter->drawLine(printPos.x, printPos.y - iconSize + 4,
          printPos.x, printPos.y - iconSize / 2);
      printPainter->drawLine(printPos.x, printPos.y + iconSize - 4,
          printPos.x, printPos.y + iconSize / 2);
      printPainter->drawLine(printPos.x - iconSize + 4, printPos.y,
          printPos.x - iconSize / 2, printPos.y);
      printPainter->drawLine(printPos.x + iconSize - 4, printPos.y,
          printPos.x + iconSize / 2, printPos.y);
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawEllipse(printPos.x - iconSize/4,
          printPos.y - iconSize/4, iconSize/2, iconSize/2);
      printPainter->drawText(printPos.x - 10, printPos.y + iconSize + 4, name);
      break;
    case Airfield:
      iconSize += 2;
      printPainter->setPen(QPen(QColor(255,255,255), 7));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawLine(printPos.x - iconSize + 4, printPos.y,
          printPos.x + iconSize - 4, printPos.y);

      printPainter->setPen(QPen(QColor(0,0,0), 3));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawLine(printPos.x - iconSize + 4, printPos.y,
          printPos.x + iconSize - 4, printPos.y);
      printPainter->drawText(printPos.x - 10, printPos.y + iconSize + 4, name);
      break;
    case ClosedAirfield:
      warning("ClosedAirfield");
      break;
    case CivHeliport:
      printPainter->setPen(QPen(QColor(255,255,255), 5));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);

      printPainter->setPen(QPen(QColor(0,0,0), 2));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->setFont(QFont("helvetica", 13, QFont::Bold));
      printPainter->drawText(printPos.x - 5, printPos.y + 5, "H");
      break;
    case MilHeliport:
      iconSize += 2;
      printPainter->setPen(QPen(QColor(255,255,255), 5));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->drawEllipse(printPos.x - iconSize/4,
          printPos.y - iconSize/4, iconSize/2, iconSize/2);

      printPainter->setPen(QPen(QColor(0,0,0), 2));
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->setPen(QPen(QColor(0,0,0), 1));
      iconSize -= 8;
      printPainter->drawEllipse(printPos.x - iconSize/2,
          printPos.y - iconSize/2, iconSize, iconSize);
      printPainter->setFont(QFont("helvetica", 9, QFont::Bold));
      printPainter->drawText(printPos.x - 3, printPos.y + 3, "H");
      break;
    case AmbHeliport:
      printPainter->setPen(QPen(QColor(255,255,255), 1));
      printPainter->setBrush(QBrush(QColor(255,255,255), QBrush::SolidPattern));
      printPainter->drawRect(printPos.x - 9, printPos.y - 9, 18, 18);

      printPainter->setPen(QPen(QColor(0,0,0), 1));
      printPainter->setBrush(QBrush(QColor(0,0,0), QBrush::SolidPattern));
      printPainter->drawRect(printPos.x - 7, printPos.y - 7, 14, 14);
      printPainter->setPen(QPen(QColor(255,255,255), 1));
      printPainter->setFont(QFont("helvetica", 13, QFont::Bold));
      printPainter->drawText(printPos.x - 5, printPos.y + 5, "H");
      break;
  }
}
