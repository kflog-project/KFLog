/***********************************************************************
 **
 **   flightgroup.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Harald Maier
 **                   2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "flightgroup.h"
#include "mapcalc.h"

FlightGroup::FlightGroup(const QString& fName) :
  BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName)
{
}

FlightGroup::FlightGroup(const QList<class Flight *>& fList, const QString& fName) :
  BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName),
  flightList(fList)
{
}

FlightGroup::~FlightGroup()
{
}

bool FlightGroup::drawMapElement( QPainter* targetP )
{
  class Flight *f;

  for( int i = 0; i < flightList.count(); i++ )
    {
      f = flightList.at( i );
      f->drawMapElement( targetP );
    }

  return true;
}

void FlightGroup::printMapElement(QPainter* targetP, bool isText)
{
  class Flight *f;

  for( int i = 0; i < flightList.count(); i++ )
    {
      f = flightList.at( i );
      f->printMapElement( targetP, isText );
    }
}

/** remove flight from current group */
void FlightGroup::removeFlight(BaseFlightElement *f)
{
  class Flight *flight = dynamic_cast<class Flight *> (f);

  if( ! flight )
    {
      return;
    }

  if( flightList.contains( flight ) )
    {
      flightList.removeOne( flight );
    }
}

void FlightGroup::reProject()
{
  class Flight *flight;

  foreach(flight, flightList)
    {
      flight->reProject();
    }
}
