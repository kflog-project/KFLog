/***********************************************************************
 **
 **   flightgroup.h
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

#ifndef FLIGHTGROUP_H
#define FLIGHTGROUP_H

#include "baseflightelement.h"
#include "flight.h"

#include <QList>

/**
 *@author Harald Maier
 */

class FlightGroup : public BaseFlightElement
{
 public:
  /**
   * Constructor
   *
   * Creates a flightgroup with name @ref fName
   * @param fName name of new flightgroup
   * @returns new @ref FlightGroup object.
   */
  FlightGroup(const QString& fName);
  /**
   * Constructor
   *
   * Creates a flightgroup with name @ref fName containing the flights in list @ref fList.
   * @param fName name of new flightgroup
   * @param fList @ref QPtrList of flights to be included in the FlightGroup.
   * @returns new @ref FlightGroup object.
   */
  FlightGroup(const QList <Flight::Flight*>& fList, const QString& fName);
  /**
   * Destructor
   */
  ~FlightGroup();
  /**
   * @returns an empty waypoint list
   */
  QList<Waypoint*> getWPList();
  /**
   * Draws the flight an the task for each fligth into the given painter. Reimplemented
   * from BaseMapElement.
   * @param  targetP  The painter to draw the element into.
   * @param  maskP  The maskpainter for targetP
   */
  void drawMapElement(QPainter* targetP, QPainter* maskP);
  /**
   * Prints the flight an the task for each fligth into the given painter. Reimplemented
   * from BaseMapElement.
   * @param  targetP  The painter to draw the element into.
   * @param  isText Print text? 
   */
  void printMapElement(QPainter* targetP, bool isText);
  /**
   * @returns the list of flights contained in the group
   */
  QList<Flight::Flight*> getFlightList();
  /**
   * Removed the indicated object from the group.
   */
  void removeFlight(BaseFlightElement *f);
  /** No descriptions */
  void setFlightList(QList <Flight::Flight*> fl);
  /**
   * re-project the flights in this flightgroup. Reimplemented from BaseFlightElement.
   */
  void reProject();
 private:
  QList<Flight::Flight*> flightList;
};

#endif
