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

#include <QDir>

Airport::Airport(QString n, QString i, QString abbr, BaseMapElement::objectType t,
  WGSPoint wgsPos, QPoint pos, unsigned int e, const char* f, bool v)
  : RadioPoint(n, i, abbr, t, wgsPos, pos, f, e),
    vdf(v),
    rwData(0)
{
}

Airport::~Airport()
{
  delete rwData;
  rwData=0;
}

QString Airport::getFrequency() const { return frequency; }

runway* Airport::getRunway(int index) const
{
  if (!rwData)
    return 0;

  return rwData->at(index);
}

unsigned int Airport::getRunwayNumber() const
{
  if (!rwData)
    return 0;

  return rwData->count();
}

QString Airport::getInfoString() const
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
      "<IMG SRC=" + path + "/" + glConfig->getPixmapName(typeID) + ">" +
      "</TD><TD>" + name + " (" + icao + ")</TD></TR>" +
      "<TR><TD></TD><TD><FONT SIZE=-1>" + text + "m" +
      "<BR>" + tmp + "</FONT></TD></TR></TABLE>";

  return text;
}

void Airport::printMapElement(QPainter* printPainter, bool isText)
{
  if(!isVisible()) return;

  QPoint printPos(glMapMatrix->print(position));

  printPainter->setPen(QPen(QColor(0,0,0), 3));
  printPainter->setBrush(Qt::NoBrush);
  printPainter->setFont(QFont("helvetica", 10));

  int iconSize = 20;

  QPen whiteP = QPen(QColor(255,255,255), 7, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);
  QPen blackP = QPen(QColor(0, 0, 0), 3, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);
  QPen smallBlackP = QPen(QColor(0, 0, 0), 1, Qt::SolidLine,
      Qt::SquareCap, Qt::MiterJoin);

  QString iconName;

  switch(typeID)
    {
      case BaseMapElement::IntAirport:
      case BaseMapElement::Airport:
        printPainter->setPen(whiteP);
        printPainter->drawLine(printPos.x(), printPos.y() - iconSize + 4,
            printPos.x(), printPos.y() - iconSize / 2);
        printPainter->drawLine(printPos.x(), printPos.y() + iconSize - 4,
            printPos.x(), printPos.y() + iconSize / 2);
        printPainter->drawLine(printPos.x() - iconSize + 4, printPos.y(),
            printPos.x() - iconSize / 2, printPos.y());
        printPainter->drawLine(printPos.x() + iconSize - 4, printPos.y(),
            printPos.x() + iconSize / 2, printPos.y());
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);

        printPainter->setPen(blackP);
        printPainter->drawLine(printPos.x(), printPos.y() - iconSize + 4,
            printPos.x(), printPos.y() - iconSize / 2);
        printPainter->drawLine(printPos.x(), printPos.y() + iconSize - 4,
            printPos.x(), printPos.y() + iconSize / 2);
        printPainter->drawLine(printPos.x() - iconSize + 4, printPos.y(),
            printPos.x() - iconSize / 2, printPos.y());
        printPainter->drawLine(printPos.x() + iconSize - 4, printPos.y(),
            printPos.x() + iconSize / 2, printPos.y());
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        break;
      case MilAirport:
        printPainter->setPen(whiteP);
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->drawEllipse(printPos.x() - iconSize/4,
            printPos.y() - iconSize/4, iconSize/2, iconSize/2);

        printPainter->setPen(blackP);
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);

        printPainter->setPen(smallBlackP);
        printPainter->drawEllipse(printPos.x() - iconSize/4,
            printPos.y() - iconSize/4, iconSize/2, iconSize/2);
        break;
      case CivMilAirport:
        printPainter->setPen(whiteP);
        printPainter->drawLine(printPos.x(), printPos.y() - iconSize + 4,
            printPos.x(), printPos.y() - iconSize / 2);
        printPainter->drawLine(printPos.x(), printPos.y() + iconSize - 4,
            printPos.x(), printPos.y() + iconSize / 2);
        printPainter->drawLine(printPos.x() - iconSize + 4, printPos.y(),
            printPos.x() - iconSize / 2, printPos.y());
        printPainter->drawLine(printPos.x() + iconSize - 4, printPos.y(),
            printPos.x() + iconSize / 2, printPos.y());
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->drawEllipse(printPos.x() - iconSize/4,
            printPos.y() - iconSize/4, iconSize/2, iconSize/2);

        printPainter->setPen(blackP);
        printPainter->drawLine(printPos.x(), printPos.y() - iconSize + 4,
            printPos.x(), printPos.y() - iconSize / 2);
        printPainter->drawLine(printPos.x(), printPos.y() + iconSize - 4,
            printPos.x(), printPos.y() + iconSize / 2);
        printPainter->drawLine(printPos.x() - iconSize + 4, printPos.y(),
            printPos.x() - iconSize / 2, printPos.y());
        printPainter->drawLine(printPos.x() + iconSize - 4, printPos.y(),
            printPos.x() + iconSize / 2, printPos.y());
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->drawEllipse(printPos.x() - iconSize/4,
            printPos.y() - iconSize/4, iconSize/2, iconSize/2);
        break;
      case Airfield:
        iconSize += 2;
        printPainter->setPen(whiteP);
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->drawLine(printPos.x() - iconSize + 4, printPos.y(),
            printPos.x() + iconSize - 4, printPos.y());

        printPainter->setPen(blackP);
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->drawLine(printPos.x() - iconSize + 4, printPos.y(),
            printPos.x() + iconSize - 4, printPos.y());
        break;
      case ClosedAirfield:
        qWarning("ClosedAirfield");
        isText = false;
        break;
      case CivHeliport:
        printPainter->setPen(QPen(QColor(255,255,255), 5));
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);

        printPainter->setPen(QPen(QColor(0,0,0), 2));
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->setFont(QFont("helvetica", 13, QFont::Bold));
        printPainter->drawText(printPos.x() - 5, printPos.y() + 5, "H");
        isText = false;
        break;
      case MilHeliport:
        iconSize += 2;
        printPainter->setPen(QPen(QColor(255,255,255), 5));
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->drawEllipse(printPos.x() - iconSize/4,
            printPos.y() - iconSize/4, iconSize/2, iconSize/2);

        printPainter->setPen(QPen(QColor(0,0,0), 2));
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->setPen(QPen(QColor(0,0,0), 1));
        iconSize -= 8;
        printPainter->drawEllipse(printPos.x() - iconSize/2,
            printPos.y() - iconSize/2, iconSize, iconSize);
        printPainter->setFont(QFont("helvetica", 9, QFont::Bold));
        printPainter->drawText(printPos.x() - 3, printPos.y() + 3, "H");
        isText = false;
        break;
      case AmbHeliport:
        printPainter->setPen(QPen(QColor(255,255,255), 1));
        printPainter->setBrush(QBrush(QColor(255,255,255), Qt::SolidPattern));
        printPainter->drawRect(printPos.x() - 9, printPos.y() - 9, 18, 18);

        printPainter->setPen(QPen(QColor(0,0,0), 1));
        printPainter->setBrush(QBrush(QColor(0,0,0), Qt::SolidPattern));
        printPainter->drawRect(printPos.x() - 7, printPos.y() - 7, 14, 14);
        printPainter->setPen(QPen(QColor(255,255,255), 1));
        printPainter->setFont(QFont("helvetica", 13, QFont::Bold));
        printPainter->drawText(printPos.x() - 5, printPos.y() + 5, "H");
        isText = false;
        break;

      default:
        break;
    }

  if(isText)
    {
      printPainter->drawText(printPos.x() - 10, printPos.y() + iconSize + 4, name);
    }
}

void Airport::addRunway(runway* r)
{
    if (r) {
      if (!rwData) {
        rwData=new Q3PtrList<runway>;
        rwData->setAutoDelete(true);
      }
      rwData->append(r);
    }
}

