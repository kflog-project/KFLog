/***********************************************************************
 **
 **   flightgroup.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Harald Maier
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "flightgroup.h"
#include "mapcalc.h"

FlightGroup::FlightGroup(const QString& fName)
  : BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName)
{
}

FlightGroup::FlightGroup(const QList <Flight::Flight*>& fList, const QString& fName)
  : BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName),
    flightList(fList)
{
}

FlightGroup::~FlightGroup()
{
}

QList<Waypoint*> FlightGroup::getWPList()
{
  QList<Waypoint*> tmp;
  return tmp;
}

bool FlightGroup::drawMapElement(QPainter* targetP, QPainter* maskP)
{
  Flight::Flight *f;
  for(int i = 0; i < flightList.count(); i++) {
    f = flightList.at(i);
    f->drawMapElement(targetP, maskP);
  }

  return true;
}

void FlightGroup::printMapElement(QPainter* targetP, bool isText)
{
  Flight::Flight *f;
  for(int i = 0; i < flightList.count(); i++) {
    f = flightList.at(i);
    f->printMapElement(targetP, isText);
  }
}
/** No descriptions */
QList<Flight::Flight*> FlightGroup::getFlightList()
{
  return flightList;
}

/** remove flight from current group */
void FlightGroup::removeFlight(BaseFlightElement *f)
{
  if (flightList.contains((Flight::Flight *)f)) {
    flightList.removeOne((Flight::Flight *)f);
  }
}

/** No descriptions */
void FlightGroup::setFlightList(QList <Flight::Flight*> fl)
{
  flightList = fl;
}

/** re-project the flights in this flightgroup. Reimplemented from BaseFlightElement. */
void FlightGroup::reProject() {
  Flight::Flight *flight;
  foreach(flight, flightList)
      flight->reProject();
}

