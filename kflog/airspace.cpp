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
 **                   2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "airspace.h"

Airspace::Airspace(QString n, BaseMapElement::objectType t, QPolygon pG,
                   int u, BaseMapElement::elevationType uType,
                   int l, BaseMapElement::elevationType lType)
  : LineElement(n, t, pG), lLimitType(lType), uLimitType(uType)
{
  type = t;

  // All Airspaces are closed regions ...
  closed = true;

  // Normalize values
  double lLim=0.0;

  switch( lLimitType )
  {
  case GND:
  case MSL:
  case STD:
    lLim = Distance::mFromFeet*l;
    break;
  case FL:
    lLim = Distance::mFromFeet*100.0*l;
    break;
  case UNLTD:
    lLim=99999.0;
    break;
  case NotSet:
    lLim=0.0;
    break;
  default:
    lLim=0.0;
  };

  lLimit.setMeters( lLim );
  double uLim=0.0;
  switch( uLimitType )
  {
  case GND:
  case MSL:
  case STD:
    uLim = Distance::mFromFeet*u;
    break;
  case FL:
    uLim = Distance::mFromFeet*100.0*u;
    break;
  case UNLTD:
    lLim=99999.0;
    break;
  case NotSet:
    lLim=0.0;
    break;
  default:
    uLim=0.0;
  };

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
};

QRegion* Airspace::drawRegion( QPainter* targetP, QPainter* maskP )
{
  if( !glConfig->isBorder( typeID ) || !isVisible() )
    {
      return (new QRegion());
    }

  QPolygon mP = glMapMatrix->map( projPolygon );

  if( mP.count() <= 1 )
    {
      return (new QRegion());
    }

  QBrush drawB = glConfig->getDrawBrush( typeID );
  QPen drawP   = glConfig->getDrawPen( typeID );
  drawP.setJoinStyle( Qt::RoundJoin );

  maskP->setBrush( QBrush( Qt::color1, drawB.style() ) );
  maskP->setPen( QPen( Qt::color1, drawP.width(), drawP.style(),
      drawP.capStyle(), drawP.joinStyle() ) );
  maskP->drawPolygon( mP );

  targetP->setBrush( drawB );
  targetP->setPen( drawP );
  targetP->drawPolygon( mP );

  return (new QRegion( mP ));
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
    case BaseMapElement::AirEhigh:
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
      return "<B><EM>" + QObject::tr("unknown") + "</EM></B>";
  }
}

QString Airspace::getInfoString() const
{
  QString text, tempL, tempU;

  QString type;

  switch(lLimitType) {
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
    ;
  }

  switch(uLimitType) {
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
    ;
  }

  text = getTypeName(typeID);

  text += " " + name + "<BR>" +
    "<FONT SIZE=-1>" + tempL + " / " + tempU + "</FONT>";

  return text;
}

bool Airspace::operator < (const Airspace& other) const
{
  int a1C = getUpperL(), a2C = other.getUpperL();

  if (a1C > a2C) {
    return false;
  } else if (a1C < a2C) {
    return true;
  } else { //equal
    int a1F = getLowerL(), a2F = other.getLowerL();
    return (a1F < a2F);
  }
}
