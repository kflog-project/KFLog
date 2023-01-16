/***********************************************************************
 **
 **   airfield.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtGui>

#include "airfield.h"
#include "altitude.h"
#include "mapconfig.h"

QMutex Airfield::mutex;

Airfield::Airfield() :
  SinglePoint(),
  m_icao(),
  m_atis(0.0),
  m_winch(false),
  m_towing(false),
  m_ppr(false),
  m_private(false),
  m_skyDiving(false),
  m_landable(true),
  m_rwShift(9)
 {
 }

Airfield::Airfield( const QString& name,
                    const QString& icao,
                    const QString& shortName,
                    const BaseMapElement::objectType typeId,
                    const WGSPoint& wgsPos,
                    const QPoint& pos,
                    const float elevation,
                    const QList<Runway>& rwyList,
                    const QList<Frequency> frequencyList,
                    const QString country,
                    const QString comment,
                    bool hasWinch,
                    bool hasTowing,
                    bool isPPR,
                    bool isPrivate,
                    bool hasSkyDiving,
                    bool isLandable,
                    const float atis ) :
  SinglePoint( name, shortName, typeId, wgsPos, pos, elevation, country, comment ),
  m_icao(icao),
  m_frequencyList(frequencyList),
  m_atis(atis),
  m_rwyList(rwyList),
  m_winch(hasWinch),
  m_towing(hasTowing),
  m_ppr(isPPR),
  m_private(isPrivate),
  m_skyDiving(hasSkyDiving),
  m_landable(isLandable),
  m_rwShift(0)
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

  text = QString( "<html><center><b>") +
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

  if( m_rwyList.size() > 0 )
    {
      for( int i = 0; i < m_rwyList.size(); i++ )
        {
          text += QString("<tr><td>") + QObject::tr("Runway:");

          const Runway& rwy = m_rwyList.at(i);

          text += QString("&nbsp;<b>%1</b>").arg( rwy.getName() );

          if( rwy.isMainRunway() == true )
            {
              text += "&nbsp;*&nbsp;</td>";
            }

          float rwyLength = rwy.getLength();
          float rwyWidth = rwy.getWidth();
          QString rwyUnit = "m";

          if( Altitude::getUnit() != Altitude::meters )
           {
              rwyLength /= Distance::mFromFeet;
              rwyWidth /= Distance::mFromFeet;
              rwyUnit = "ft";
           }

          text += "<td> (" + Runway::item2Text( rwy.getSurface() ) + ")</td>" +
                  "<td>" + QObject::tr("Length:") + " <b>" +
                  QString("%1 %2").arg( rwyLength, 0, 'f', 0 ).arg( rwyUnit ) + "</b>" +
                  "</td>" +
                  "<td>" + QObject::tr("Width:") + " <b>" +
                  QString("%1 %2").arg( rwyWidth, 0, 'f', 0 ).arg( rwyUnit ) + "</b>";

         if( rwy.isOpen() == false )
           {
             text += "&nbsp;" + QObject::tr("Closed!");
           }

          text += "</td></tr>";
        }
    }

  if( m_frequencyList.size() > 0.0 )
    {
      for( int i = 0; i < m_frequencyList.size(); i++ )
        {
          const Frequency& fq = m_frequencyList.at(i);

          text += "<tr><td colspan=\"4\">" + QObject::tr("Frequency:");
          text += " <b>" + frequencyAsString( fq.getValue() ) + " " +
                  QObject::tr("MHz") + "</b> ";
          text += fq.getName() + "</td></tr>";
        }
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
     text += "<tr><td colspan=\"4\">" + QObject::tr("Elevation:") + "&nbsp;" +
             "<b>" + meters + " / " + feet +"</b></td></tr>";
   }
  else
   {
     text += "<tr><td colspan=\"4\">" + QObject::tr("Elevation:") + "&nbsp;" +
             "<b>" + feet + " / " + meters + "</b></td></tr>";
   }

  if( ! comment.isEmpty() )
    {
      text += "<tr><td colspan=\"4\">" + QObject::tr("Comment:") + "&nbsp;" +
              comment + "</td></tr>";
    }

  text += "</table></html>";

  return text;
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

      // Calculate the default runway shift in 1/10 degrees.
      // Default is E-W direction, if no runway data were defined.
      calculateRunwayShift();

      // All icons are squares. Therefore we take the height also as width.
      int ih = image.height();

      targetP->drawPixmap( curPos.x() - ih/2, curPos.y() - ih/2,
                           image,
                           m_rwShift * ih, 0, ih, ih );
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

void Airfield::printMapElement( QPainter* printPainter, bool isText )
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
#if 0
              printPainter->drawText(printPos.x() - 15,
                            printPos.y() + iconSize + 14, frequencyAsString());
#endif
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
