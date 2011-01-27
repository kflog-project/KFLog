/***********************************************************************
 **
 **   airspace.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **   Modified:       2008      by Josua Dietze
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "airspace.h"

Airspace::Airspace(QString name, BaseMapElement::objectType oType, QPolygon pP,
                   int upper, BaseMapElement::elevationType uType,
                   int lower, BaseMapElement::elevationType lType) :
  LineElement(name, oType, pP), lLimitType(lType), uLimitType(uType)
{
  // All Airspaces are closed regions ...
  closed = true;

  // Normalize values
  double lLim=0.0;

  switch( lLimitType )
  {
    case GND:
    case MSL:
    case STD:
      lLim = Distance::mFromFeet*lower;
      break;
    case FL:
      lLim = Distance::mFromFeet*100.0*lower;
      break;
    case UNLTD:
      lLim=99999.0;
      break;
    case NotSet:
      lLim=0.0;
      break;
    default:
      lLim=0.0;
  }

  lLimit.setMeters( lLim );
  double uLim=0.0;

  switch( uLimitType )
  {
    case GND:
    case MSL:
    case STD:
      uLim = Distance::mFromFeet*upper;
      break;
    case FL:
      uLim = Distance::mFromFeet*100.0*upper;
      break;
    case UNLTD:
      lLim=99999.0;
      break;
    case NotSet:
      lLim=0.0;
      break;
    default:
      uLim=0.0;
  }

  uLimit.setMeters( uLim );
}

Airspace::~Airspace()
{
}

/**
 * Tells the caller, if the airspace is drawable or not
 */
bool Airspace::isDrawable() const
{
  return ( glConfig->isBorder(typeID) && isVisible() );
}

void Airspace::drawRegion( QPainter* targetP, const QRect &viewRect )
{
  // qDebug("Airspace::drawRegion(): TypeId=%d, opacity=%f, Name=%s",
  //         typeID, opacity, getInfoString().toLatin1().data() );
  if( ! isDrawable() )
    {
      return;
    }

  QPolygon mP = glMapMatrix->map( projPolygon );

  QBrush drawB( glConfig->getDrawBrush(typeID) );

  QPen drawP = glConfig->getDrawPen( typeID );
  drawP.setJoinStyle( Qt::RoundJoin );

  targetP->setPen(drawP);
  targetP->setBrush(drawB);
  targetP->setClipRegion( viewRect );

  // If brush SolidPattern is set, we draw transparent filled airspace areas.
  if( drawB.style() == Qt::SolidPattern )
    {
      /* Gets the opacity of the painter. The
       * value should be in the range 0.0 to 100.0%, where 0.0 is fully
       * transparent and 100.0 is fully opaque.
       */
      double opacity = (double) glConfig->getAsOpacity(typeID);

      if( uLimitType == BaseMapElement::FL && uLimit.getFL() >= 200.0 )
        {
          opacity = 0.0;
        }

      if( opacity < 100.0 && opacity > 0.0 )
        {
          // Draw airspace filled with opacity factor
          targetP->setOpacity( opacity/100.0 );
          targetP->drawPolygon(mP);
          targetP->setBrush(Qt::NoBrush);
          targetP->setOpacity( 1.0 );
        }
      else if( opacity == 0.0 )
        {
          // draw only airspace borders without any filling inside
          targetP->setBrush(Qt::NoBrush);
          targetP->setOpacity( 1.0 );
        }
    }

  // Draw the outline of the airspace with the selected brush
  targetP->drawPolygon(mP);
}

/**
 * Return a pointer to the mapped airspace region data. The caller takes
 * the ownership about the returned object.
 */
QPainterPath Airspace::createRegion()
{
  QPolygon mP = glMapMatrix->map(projPolygon);

  QPainterPath path;
  path.addPolygon(mP);
  path.closeSubpath();
  return path;
}

/**
 * Returns a text representing the type of the airspace
 */
QString Airspace::getTypeName (objectType type)
{
  switch(type)
  {
    case BaseMapElement::AirA:
      return QObject::tr("AS-A");
    case BaseMapElement::AirB:
      return QObject::tr("AS-B");
    case BaseMapElement::AirC:
      return QObject::tr("AS-C");
    case BaseMapElement::AirD:
      return QObject::tr("AS-D");
    case BaseMapElement::AirElow:
      return QObject::tr("AS-E (low)");
    case BaseMapElement::AirE:
      return QObject::tr("AS-E");
    case BaseMapElement::WaveWindow:
      return QObject::tr("Wave Window");
    case BaseMapElement::AirF:
      return QObject::tr("AS-F");
    case BaseMapElement::Restricted:
      return QObject::tr("Restricted");
    case BaseMapElement::Danger:
      return QObject::tr("Danger");
    case BaseMapElement::Prohibited:
      return QObject::tr("Prohibited");
    case BaseMapElement::ControlC:
      return QObject::tr("CTR-C");
    case BaseMapElement::ControlD:
      return QObject::tr("CTR-D");
    case BaseMapElement::LowFlight:
      return QObject::tr("Low Flight");
    case BaseMapElement::Tmz:
      return QObject::tr("TMZ");
    case BaseMapElement::GliderSector:
      return QObject::tr("Glider Sector");
    default:
      return "<B><EM>" + QObject::tr("unknown AS") + "</EM></B>";
  }
}

QString Airspace::getInfoString()
{
  QString text, tempL, tempU;

  QString type;

  switch(lLimitType)
  {
    case MSL:
      tempL.sprintf("%s MSL", lLimit.getText(true,0).toLatin1().data());
      break;
    case GND:
      if(lLimit.getMeters())
        tempL.sprintf("%s GND", lLimit.getText(true,0).toLatin1().data());
      else
        tempL = "GND";
      break;
    case FL:
      tempL.sprintf("FL %d (%s)", (int) rint(lLimit.getFeet()/100.), lLimit.getText(true,0).toLatin1().data());
      break;
    case STD:
      tempL.sprintf("%s STD", lLimit.getText(true,0).toLatin1().data());
      break;
    case UNLTD:
      tempL = QObject::tr("Unlimited");
    default:
      break;
  }

  switch(uLimitType)
  {
    case MSL:
      if(uLimit.getMeters() >= 99999)
        tempU = QObject::tr("Unlimited");
      else
        tempU.sprintf("%s MSL", uLimit.getText(true,0).toLatin1().data());
      break;
    case GND:
      tempU.sprintf("%s GND", uLimit.getText(true,0).toLatin1().data());
      break;
    case FL:
      tempU.sprintf("FL %d (%s)", (int) rint(uLimit.getFeet()/100.), uLimit.getText(true,0).toLatin1().data());
      break;
    case STD:
      tempU.sprintf("%s STD", uLimit.getText(true,0).toLatin1().data());
      break;
    case UNLTD:
      tempU = QObject::tr("Unlimited");
    default:
      break;
  }

  text = getTypeName(typeID);

  text += " " + name + "<BR>" +
          "<FONT SIZE=-1>" + tempL + " / " + tempU + "</FONT>";

  return text;
}

bool Airspace::operator < (const Airspace& other) const
{
  int a1C = getUpperL(), a2C = other.getUpperL();

  if( a1C > a2C )
    {
      return false;
    }
  else if( a1C < a2C )
    {
      return true;
    }
  else
    { //equal
      int a1F = getLowerL(), a2F = other.getLowerL();
      return (a1F < a2F);
    }
}
