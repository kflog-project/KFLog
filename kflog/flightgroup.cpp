/***********************************************************************
 **
 **   flightgroup.cpp
 **
 **   This file is part of KFLog2.
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

#include <klocale.h>

FlightGroup::FlightGroup(QString fName)
  : BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName)
{
}

FlightGroup::FlightGroup(QPtrList <Flight::Flight> fList, QString fName)
  : BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName),
    flightList(fList)
{
}

FlightGroup::~FlightGroup()
{
}

QPtrList<Waypoint> FlightGroup::getWPList()
{
  QPtrList<Waypoint> tmp;
  return tmp;
}

void FlightGroup::drawMapElement(QPainter* targetP, QPainter* maskP)
{
  unsigned int i;
  Flight::Flight *f;
  for (i = 0; i < flightList.count(); i++) {
    f = flightList.at(i);
    f->drawMapElement(targetP, maskP);
  }
}

void FlightGroup::printMapElement(QPainter* targetP, bool isText)
{
  unsigned int i;
  Flight::Flight *f;
  for (i = 0; i < flightList.count(); i++) {
    f = flightList.at(i);
    f->printMapElement(targetP, isText);
  }
}
/** No descriptions */
QPtrList<Flight::Flight> FlightGroup::getFlightList()
{
  return flightList;
}

/** remove flight from current group */
void FlightGroup::removeFlight(BaseFlightElement *f)
{
  if (flightList.containsRef((Flight::Flight *)f)) {
    flightList.take();
  }
}

/** No descriptions */
void FlightGroup::setFlightList(QPtrList <Flight::Flight> fl)
{
  flightList = fl;
}

/** re-project the flights in this flightgroup. Reimplemented from BaseFlightElement. */
void FlightGroup::reProject() {
  QPtrListIterator<Flight::Flight> it(flightList);

  for ( ; it.current(); ++it ) {
      Flight::Flight *f = it.current();
      f->reProject();
  }

}

