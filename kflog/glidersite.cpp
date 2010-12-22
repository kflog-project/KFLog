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

#include <QDir>

#include "glidersite.h"

GliderSite::GliderSite(QString n, QString icao, QString gps, WGSPoint wgsPos,
        QPoint pos, unsigned int elev, const char* f, bool w)
: RadioPoint(n, icao, gps, BaseMapElement::Glidersite, wgsPos, pos, f, elev),
  winch(w),
  rwData(0)
{

}

GliderSite::~GliderSite()
{
  delete rwData;
  rwData=0;
}

QString GliderSite::getFrequency() const { return frequency; }

runway *GliderSite::getRunway(int index) const
{ 
  if (!rwData)
    return 0;

  return rwData->at(index); 
}

unsigned int GliderSite::getRunwayNumber() const 
{ 
  if (!rwData)
    return 0;

  return rwData->count(); 
}

bool GliderSite::isWinch() const { return winch; }

QString GliderSite::getInfoString() const
{
  extern MapConfig* _globalMapConfig;
  QString path = _globalMapConfig->getIconPath();
  QString text;

  // @AP: suppress an empty frequency
  QString tmp;

  if( frequency.left(1) != "0" )
    {
      tmp = frequency;
    }

  text.sprintf("%d", elevation);
  text = "<TABLE BORDER=0><TR><TD>"
      "<IMG SRC=" + path + "/" + glConfig->getPixmapName(typeID, winch) + ">" +
      "</TD><TD>" + name + " (" + icao + ")</TD></TR>" +
      "<TR><TD></TD><TD><FONT SIZE=-1>" + text + "m" +
      "<BR>" + tmp + "</FONT></TD></TR></TABLE>";

  return text;
}

void GliderSite::printMapElement(QPainter* printPainter, bool isText)
{
  if(!isVisible()) return;

  QPoint printPos(glMapMatrix->print(position));

  int iconSize = 20;

  QPen whiteP = QPen(QColor(255,255,255), 7, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);
  QPen blackP = QPen(QColor(0, 0, 0), 3, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);
  Q3PointArray pointArray(5);

  printPainter->setBrush(Qt::NoBrush);

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

  if(isText)
    {
      printPainter->setFont(QFont("helvetica", 10, QFont::Bold));
      printPainter->drawText(printPos.x() - 15,
          printPos.y() + iconSize + 4, name);
      printPainter->drawText(printPos.x() - 15,
          printPos.y() + iconSize + 14, frequency);
    }
}


void GliderSite::addRunway(runway* r)
{
    if (r) {
      if (!rwData) {
        rwData=new Q3PtrList<runway>;
        rwData->setAutoDelete(true);
      }
      rwData->append(r);
    }
}
