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

#include <kiconloader.h>
#include <kstddirs.h>

GliderSite::GliderSite(QString n, QString icao, QString gps, QPoint pos,
        unsigned int elev, const char* f, bool w)
: RadioPoint(n, icao, gps, BaseMapElement::Glidersite, pos, f),
  elevation(elev), winch(w)
{

}

GliderSite::~GliderSite()
{

}

QString GliderSite::getFrequency() const { return frequency; }

unsigned int GliderSite::getElevation() const { return elevation; }

runway GliderSite::getRunway(int index) const { return rwData[index]; }

unsigned int GliderSite::getRunwayNumber() const { return rwNum; }

bool GliderSite::isWinch() const { return winch; }

QString GliderSite::getInfoString() const
{
  QString text;
  QString path = KGlobal::dirs()->findResource("appdata", "mapicons/");

  text.sprintf("%d", elevation);
  text = "<TABLE BORDER=0><TR><TD>"
      "<IMG SRC=" + path + glConfig->getPixmapName(typeID) + ">" +
      "</TD><TD>" + name + " (" + icao + ")</TD></TR>" +
      "<TR><TD></TD><TD><FONT SIZE=-1>" + text + "m" +
      "<BR>" + frequency + "</FONT></TD></TR></TABLE>";

  return text;
}

void GliderSite::printMapElement(QPainter* printPainter) const
{
  if(!__isVisible()) return;

  QPoint printPos(glMapMatrix->print(position));

  int iconSize = 20;

  QPen whiteP = QPen(QColor(255,255,255), 7, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);
  QPen blackP = QPen(QColor(0, 0, 0), 3, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);
  QPointArray pointArray(5);

  printPainter->setBrush(QBrush::NoBrush);

  printPainter->setPen(whiteP);
  printPainter->drawEllipse(printPos.x() - (iconSize / 2),
        printPos.y() - (iconSize / 2), iconSize, iconSize);

  printPainter->setPen(blackP);
  printPainter->drawEllipse(printPos.x() - (iconSize / 2),
        printPos.y() - (iconSize / 2), iconSize, iconSize);

  printPainter->setPen(whiteP);
  pointArray.setPoint(0, printPos.x() - iconSize , printPos.y() + 2);
  pointArray.setPoint(1, printPos.x() - (iconSize / 2),
                    printPos.y() - (iconSize / 2) + 4);
  pointArray.setPoint(2, printPos.x(), printPos.y() + 2);
  pointArray.setPoint(3, printPos.x() + (iconSize / 2),
                    printPos.y() - (iconSize / 2) + 4);
  pointArray.setPoint(4, printPos.x() + iconSize , printPos.y() + 2);

  printPainter->drawPolyline(pointArray);
  printPainter->setPen(blackP);
  printPainter->drawPolyline(pointArray);

  printPainter->setFont(QFont("helvetica", 10, QFont::Bold));
  printPainter->drawText(printPos.x() - 15, printPos.y() + iconSize + 4, name);
  printPainter->drawText(printPos.x() - 15, printPos.y() + iconSize + 14, frequency);
}
