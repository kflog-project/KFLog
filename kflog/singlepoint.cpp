/***********************************************************************
**
**   singlepoint.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011-2014 by Axel Pauli
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

#include "altitude.h"
#include "singlepoint.h"

SinglePoint::SinglePoint() :
  BaseMapElement(),
  elevation(0.0),
  lm_typ(0)
{
}

SinglePoint::SinglePoint( const QString& name,
                          const QString& shortName,
                          const BaseMapElement::objectType typ,
                          const WGSPoint& wgsP,
                          const QPoint& pos,
                          const float elev,
                          const QString& comment,
                          const QString& country,
                          const unsigned short secID,
                          unsigned int lmType ) :
  BaseMapElement(name, typ, secID, country),
  wgsPosition(wgsP),
  position(pos),
  shortName(shortName),
  curPos(pos),
  elevation(elev),
  lm_typ(lmType),
  comment(comment)
{
}

SinglePoint::~SinglePoint()
{
}

void SinglePoint::printMapElement(QPainter* printPainter, bool isText )
{
  Q_UNUSED( isText )

  if( !isVisible() )
    {
      return;
    }

  QPoint printPos(glMapMatrix->print(position));

  int iconSize = 16;

  if(glMapMatrix->isSwitchScale())
    {
      iconSize = 8;
    }

  /*
   * Hier sollte mal für eine bessere Qualität der Icons gesorgt werden.
   * Eventuell kann man die Icons ja hier zeichnen lassen ?!?
   */
  printPainter->setPen(QPen(QColor(0,50,50), 2));
  printPainter->setBrush(Qt::NoBrush);
  printPainter->drawEllipse(printPos.x() - 5, printPos.y() - 5, 10, 10);
  return;

  printPainter->drawPixmap( printPos.x() - iconSize,
                            printPos.x() - iconSize,
                            glConfig->getPixmap(typeID) );
}

bool SinglePoint::drawMapElement(QPainter* targetP)
{
  if(! isVisible() )
    {
      curPos = QPoint(-50000, -50000);
      return false;
    }

  curPos = glMapMatrix->map(position);

  targetP->setPen(QPen(Qt::black, 2));

  if( typeID == BaseMapElement::Village )
    {
      targetP->setBrush( Qt::NoBrush );
      targetP->drawEllipse( curPos.x() - 5, curPos.y() - 5, 10, 10 );
      return true;
    }

  if(typeID == BaseMapElement::Landmark)
   {
      switch(lm_typ)
      {
        case LM_DAM:
        case LM_LOCK:
        case LM_MINE:
        case LM_INDUSTRY:
        case LM_DEPOT:
        case LM_SALT:
        case LM_OILFIELD:
        case LM_TOWER:
        case LM_SETTLEMENT:
        case LM_BUILDING:
        case LM_CASTLE:
        case LM_RUIN:
        case LM_MONUMENT:
        case LM_LIGHTHOUSE:
        case LM_STATION:
        case LM_BRIDGE:
        case LM_FERRY:
          targetP->setPen(QPen(QColor(100, 100, 100), 3));
          break;
        case LM_UNKNOWN:
        default:
          targetP->setPen(QPen(QColor(255, 255, 255), 3));
          break;
      }

      targetP->setBrush(Qt::NoBrush);
      targetP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
      return true;
   }

  QPixmap pixmap = glConfig->getPixmap(typeID);

  int xoff = pixmap.size().width() / 2;
  int yoff = pixmap.size().height() / 2;

  if( typeID == BaseMapElement::Thermal || typeID == BaseMapElement::Turnpoint )
    {
      // The lower end of the flag shall directly point to the point at the map.
      yoff = pixmap.size().height();
    }

  targetP->drawPixmap( curPos.x() - xoff,
                       curPos.y() - yoff,
                       pixmap );

  return true;
}

QString SinglePoint::getInfoString()
{
  QString path = glConfig->getIconPath();
  QString text;
  QString text1;

  text = QString("<html><center><b>") +
         "<IMG SRC=" + path + "/" +
         glConfig->getPixmapName(typeID, false, false) + "> " +
         name;

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

  // get current unit
  Altitude::altitudeUnit currentUnit = Altitude::getUnit();

  Altitude::setUnit(Altitude::meters);
  QString meters = Altitude::getText( elevation, true, 0 );

  Altitude::setUnit(Altitude::feet);
  QString feet = Altitude::getText( elevation, true, 0 );

  // restore save unit
  Altitude::setUnit(currentUnit);

  if( currentUnit == Altitude::meters )
   {
     text += "<tr><td>" + QObject::tr("Elevation:") +
             "</td><td><b>" + meters + " / " + feet +
             "</b></td></tr>";
   }
  else
   {
     text += "<tr><td>" + QObject::tr("Elevation:") +
             "</td><td><b>" + feet + " / " + meters +
             "</b></td></tr>";
   }

  text += "</table></html>";

  return text;
}
