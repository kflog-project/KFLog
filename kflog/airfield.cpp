/***********************************************************************
 **
 **   airfield.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifdef QT_5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "airfield.h"
#include "altitude.h"

Airfield::Airfield( const QString& name,
                    const QString& icao,
                    const QString& shortName,
                    const BaseMapElement::objectType typeId,
                    const WGSPoint& wgsPos,
                    const QPoint& pos,
                    const float elevation,
                    const float frequency,
                    const QString& country,
                    const QString comment,
                    bool winch,
                    bool towing,
                    bool landable,
                    const float atis) :
  SinglePoint(name, shortName, typeId, wgsPos, pos, elevation, comment, country),
  m_icao(icao),
  m_frequency(frequency),
  m_atis(atis),
  m_winch(winch),
  m_towing(towing),
  m_rwShift(0),
  m_landable(landable)
{
}

Airfield::~Airfield()
{
}

QString Airfield::getInfoString()
{
  QString path = glConfig->getIconPath();
  QString text;
  QString text1;

  text = QString("<html><center><b>") +
         "<IMG SRC=" + path + "/" +
         glConfig->getPixmapName(typeID, hasWinch(), false) + "> " +
         name;

  if( !m_icao.isEmpty() )
    {
      text1 = " (" + m_icao;
    }

  if( !country.isEmpty() )
    {
      if( text1.isEmpty() )
        {
          text1 = " (";
        }
      else
        {
          text1 += ", ";
        }

      text1 += country + ")";
    }
  else
    {
      text1 += ")";
    }

  text += text1;

  text += " " + BaseMapElement::item2Text( typeID, QObject::tr("(unknown object)") ) +
           "</b></center>";

  text += "<table cellpadding=5 width=100%>";

  if( getRunwayNumber() > 0 )
    {
      for( int i = 0; i < getRunwayNumber(); i++ )
        {
          text += QString("<tr><td>") + QObject::tr("Runway: ") + "</td><td>";

          Runway *rw = getRunway(i);

          text += QString("<b>%1/%2</b>").arg( rw->m_heading.first, 2, 10, QChar('0') )
                                         .arg( rw->m_heading.second, 2, 10, QChar('0') );
          text += " (" + Runway::item2Text( rw->m_surface ) + ")";
          text += "</td><td>" + QObject::tr("Length:") + "</td><td><b>" +
                  QString("%1 m").arg(rw->m_length) + "</b><td></tr>";
        }
    }

  text += "<tr><td>" + QObject::tr("Frequency:") + "</td><td>";

  if( m_frequency > 0.0 )
    {
      text += "<b>" + frequencyAsString() + " " + QObject::tr("MHz") + "</b></td>";
    }
  else
    {
      text += "<b>" + QObject::tr("unknown") +"</b></td>";
    }

  // save current unit
  Altitude::altitudeUnit currentUnit = Altitude::getUnit();

  Altitude::setUnit(Altitude::meters);
  QString meters = Altitude::getText( elevation, true, 0 );

  Altitude::setUnit(Altitude::feet);
  QString feet = Altitude::getText( elevation, true, 0 );

  // restore save unit
  Altitude::setUnit(currentUnit);

  if( currentUnit == Altitude::meters )
   {
     text += "<td>" + QObject::tr("Elevation:") +
             "</td><td><b>" + meters + " / " + feet +
             "</b></td></tr>";
   }
  else
   {
     text += "<td>" + QObject::tr("Elevation:") +
             "</td><td><b>" + feet + " / " + meters +
             "</b></td></tr>";
   }

  if( ! comment.isEmpty() )
    {
      text += "<tr><td>" + QObject::tr("Comment:") + "</td><td colspan=\"3\">" +
              comment + "</td></tr>";
    }

  text += "</table></html>";

  return text;
}

Runway* Airfield::getRunway( int index )
{
  if( m_rwList.size() == 0 )
    {
      return static_cast<Runway*> ( 0 );
    }

  return &m_rwList[index];
}

bool Airfield::drawMapElement( QPainter* targetP )
{
  if ( ! isVisible() )
    {
      curPos = QPoint(-50000, -50000);
      return false;
    }

  curPos = glMapMatrix->map(position);

  if( glConfig->isRotatable( typeID ) )
    {
      QPixmap image( glConfig->getPixmapRotatable(typeID, m_winch) );

      const Runway* runway = getRunway();

      // Calculate the default runway shift in 1/10 degrees.
      // Default is E-W direction, if no runway was defined.
      int rwShift = 90/10;

      if( runway )
        {
	  ushort heading = runway->m_heading.first;

          // calculate the real runway shift in 1/10 degrees.
          if( heading <= 36 )
            {
              rwShift = (heading >= 18 ? (heading - 18) : heading);
            }
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

#ifdef _MSC_VER
#pragma message ("warning: Airport::printMapElement not yet ready to use!")
#else
#warning "Airport::printMapElement not yet ready to use!"
#endif

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
                  printPos.y() + iconSize + 14, frequencyAsString());
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
