/***********************************************************************
 **
 **   airfield.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtCore>

#include "airfield.h"
#include "altitude.h"
#include "map.h"

Airfield::Airfield( const QString& name,
                    const QString& icao,
                    const QString& shortName,
                    const BaseMapElement::objectType typeId,
                    const WGSPoint& wgsPos,
                    const QPoint& pos,
                    const unsigned int elevation,
                    const QString& frequency,
                    const QString comment,
                    bool winch,
                    bool towing,
                    bool landable )
    : SinglePoint(name, shortName, typeId, wgsPos, pos, elevation),
    icao(icao),
    frequency(frequency),
    comment(comment),
    winch(winch),
    towing(towing),
    landable(landable)
{
}

Airfield::~Airfield()
{
}

QString Airfield::getInfoString() const
{
  extern MapConfig* _globalMapConfig;

  QString path = _globalMapConfig->getIconPath();
  QString text, elev;

  elev = Altitude::getText( elevation, true, 0).replace(QRegExp("\\s"), "&nbsp;");

  text = "<HTML><TABLE BORDER=0><TR><TD>"
         "<IMG SRC=" + path + "/" + glConfig->getPixmapName(typeID, hasWinch(), false) + "></TD>"
         "<TD>" + name;

  qDebug() << "AirfieldInfo:" << text;

  if( !icao.isEmpty() )
    {
      text += " (" + icao + ")";
    }

  text += "<FONT SIZE=-1><BR><BR>" + elev;

  if (!frequency.isEmpty())
    {
      text += "&nbsp;/&nbsp;" + frequency + "&nbsp;Mhz.";
    }

  text += "&nbsp;&nbsp;</FONT></TD></TR></TABLE></HTML>";

  return text;
}

Runway* Airfield::getRunway( int index )
{
  if( rwData.size() == 0 )
    {
      return static_cast<Runway*> ( 0 );
    }

  return &rwData[index];
}

bool Airfield::drawMapElement( QPainter* targetP )
{
  if ( ! isVisible() )
    {
      curPos = QPoint(-5000, -5000);
      return false;
    }

  curPos = glMapMatrix->map(position);

  if( glConfig->isRotatable( typeID ) )
    {
      QPixmap image( glConfig->getPixmapRotatable(typeID, winch) );

      const Runway* runway = getRunway();
      int rwShift = 0;

      if( runway )
        {
          rwShift = runway->rwShift;
        }
      else
        {
          rwShift = 9; // Default is E-W direction, if no runway was defined
        }

      // All icons are squares. Therefore we take the height also as width.
      int ih = image.height();

      targetP->drawPixmap( curPos.x() - ih/2, curPos.y() - ih/2,
                           image,
                           rwShift * ih, 0, ih, ih );
    }
  else
    {
      QPixmap image( glConfig->getPixmap(typeID) );

      int ih = image.height();
      int iw = image.width();

      int xOffset  = iw/2;
      int yOffset  = ih/2;

      if ( typeID == BaseMapElement::Outlanding )
        {
          // The lower end of the beacon shall directly point to the
          // point at the map.
          yOffset = ih;
        }

      targetP->drawPixmap(curPos.x() - xOffset, curPos.y() - yOffset, image );
    }

  return true;
}

#warning "Airport::printMapElement not yet ready to use!"

void Airfield::printMapElement(QPainter* printPainter, bool isText)
{
  if( !isVisible() )
    {
      return;
    }

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

  switch( typeID )
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

      case BaseMapElement::Airfield:
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

      case Gliderfield:
        {
          QPen whiteP = QPen(QColor(255,255,255), 7, Qt::SolidLine,
              Qt::SquareCap, Qt::MiterJoin);
          QPen blackP = QPen(QColor(0, 0, 0), 3, Qt::SolidLine,
              Qt::SquareCap, Qt::MiterJoin);
          QPolygon pointArray(5);

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

          if( isText )
            {
              printPainter->setFont(QFont("helvetica", 10, QFont::Bold));
              printPainter->drawText(printPos.x() - 15,
                  printPos.y() + iconSize + 4, name);
              printPainter->drawText(printPos.x() - 15,
                  printPos.y() + iconSize + 14, frequency);
            }
        }

        break;

      default:
        break;
    }

  if( isText && typeID != Gliderfield )
    {
      printPainter->drawText(printPos.x() - 10, printPos.y() + iconSize + 4, name);
    }
}
