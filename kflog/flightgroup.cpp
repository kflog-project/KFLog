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

FlightGroup::FlightGroup(QList <Flight::Flight> fList, QString fName)
  : BaseFlightElement("flight group", BaseMapElement::FlightGroup, fName),
  flightList(fList)
{
}

FlightGroup::~FlightGroup()
{
}

//QString FlightGroup::getFlightInfoString()
//{
//  QString htmlText;
//  QString idString;
//  unsigned int loop;
//  Flight::Flight *flight;
//
//  htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
//      <TR><TD>" + i18n("Flight group") + ":</TD><TD><A HREF=EDITGROUP>" +
//      getFileName() +  + "</A></TD></TR>\
//      </TABLE><HR NOSHADE>";
//
//  if(flightList.count()) {
//    htmlText += "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
//      <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
//       i18n("Flights") + ":</B></TD></TR>";
//
//    for (loop = 0; loop < flightList.count(); loop++) {
//      flight = flightList.at(loop);
//      // store pointer of flight instead of index
//      // flight list of mapcontents will change
//      idString.sprintf("%d", flight);
//
//      htmlText += "<TR><TD><A HREF=" + idString + ">" +
//        flight->getFileName() + "</A></TD><TD ALIGN=right>" + flight->getDate().toString() + "</TD></TR>\
//        <TR><TD>" + flight->getDistance() + "</TD><TD ALIGN=right>" +
//        printTime(flight->getLandTime() - flight->getStartTime()) + "</TD></TR>";
//    }
//  }
//  else {
//    htmlText += i18n("Click on the group name to start editing");
//  }
//
//  return htmlText;
//}

QList<wayPoint> FlightGroup::getWPList()
{
  QList<wayPoint> tmp;
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
QList<Flight::Flight> FlightGroup::getFlightList()
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
void FlightGroup::setFlightList(QList <Flight::Flight> fl)
{
  flightList = fl;
}
