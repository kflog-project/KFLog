/***********************************************************************
**
**   airspace.cpp
**
**   This file is part of KFLog
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

#include "airspace.h"

#include <klocale.h>

Airspace::Airspace(QString n, unsigned int t, QPointArray pA, unsigned int u,
      unsigned int uType, unsigned int l, unsigned int lType)
  : LineElement(n, t, pA),
    lLimit(l), lLimitType(lType), uLimit(u), uLimitType(uType)
{
  // All Airspaces are closed regions ...
  closed = true;
}

Airspace::~Airspace()
{

}

QRegion* Airspace::drawRegion(QPainter* targetP, QPainter* maskP)
{
  if(!glConfig->isBorder(typeID) || !__isVisible()) return (new QRegion());

  QPointArray tA = glMapMatrix->map(projPointArray);

  QBrush drawB = glConfig->getDrawBrush(typeID);
  QPen drawP = glConfig->getDrawPen(typeID);

  maskP->setBrush(QBrush(Qt::color1, drawB.style()));
  maskP->setPen(QPen(Qt::color1, drawP.width(), drawP.style()));
  maskP->drawPolygon(tA);

  targetP->setBrush(drawB);
  targetP->setPen(drawP);
  targetP->drawPolygon(tA);

  return (new QRegion(tA));
}

unsigned int Airspace::getUpperL() const { return uLimit; }

unsigned int Airspace::getLowerL() const { return lLimit; }

unsigned int Airspace::getUpperT() const { return uLimitType; }

unsigned int Airspace::getLowerT() const { return lLimitType; }

QString Airspace::getInfoString() const
{
  QString text, tempL, tempU;

  switch(lLimitType)
    {
      case MSL:
        tempL.sprintf("%d MSL", lLimit);
        break;
      case GND:
        if(lLimit)
            tempL.sprintf("%d GND", lLimit);
        else
            tempL = "GND";
        break;
      case FL:
        tempL.sprintf("FL %d", lLimit);
        break;
      case UNLTD:
        tempL = i18n("Unlimited");
      default: ;
    }

  switch(uLimitType)
    {
      case MSL:
        if(uLimit >= 99999)
            tempU = "unlimited";
        else
            tempU.sprintf("%d MSL", uLimit);
        break;
      case GND:
        tempU.sprintf("%d GND", uLimit);
        break;
      case FL:
        tempU.sprintf("FL %d", uLimit);
        break;
      case UNLTD:
        tempL = i18n("Unlimited");
      default: ;
    }

  switch(typeID)
    {
      case BaseMapElement::AirA:
        text = "A";
        break;
      case BaseMapElement::AirB:
        text = "B";
        break;
      case BaseMapElement::AirC:
        text = "C";
        break;
      case BaseMapElement::AirD:
        text = "D";
        break;
      case BaseMapElement::AirElow:
        text = "E (low)";
        break;
      case BaseMapElement::AirEhigh:
        text = "E (high)";
        break;
      case BaseMapElement::AirF:
        text = "F";
        break;
      case BaseMapElement::Restricted:
        text = "Restricted";
        break;
      case BaseMapElement::Danger:
        text = "Danger";
        break;
      case BaseMapElement::ControlC:
        text = "Control C";
        break;
      case BaseMapElement::ControlD:
        text = "Control D";
        break;
      case BaseMapElement::LowFlight:
        text = "Low Flight";
        break;
      case BaseMapElement::TMZ:
        text = "TMZ";
        break;
      default:
        text = "<B><EM>" + i18n("unknown") + "</EM></B>";
        break;
    }

  text = text + " " + name + "<BR>" +
      "<FONT SIZE=-1>" + tempL + " / " + tempU + "</FONT>";

  return text;
}
