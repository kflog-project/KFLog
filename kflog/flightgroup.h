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
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef FLIGHT_GROUP_H
#define FLIGHT_GROUP_H

#include "baseflightelement.h"
#include "flight.h"

#include <QList>

/**
 * \class FlightGroup
 *
 * \author Harald Maier
 *
 * \brief Class for flight goup management.
 *
 * \date 2002
 *
 * \version $Id$
 */

class FlightGroup : public BaseFlightElement
{
 public:
  /**
   * Constructor
   *
   * Creates a flight group with name @ref fName
   * @param fName name of new flight group
   * @returns new @ref FlightGroup object.
   */
  FlightGroup(const QString& fName);
  /**
   * Constructor
   *
   * Creates a flight group with name @ref fName containing the flights in list @ref fList.
   * @param fName name of new flight group
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
   * Draws the flight and the task for each flight into the given painter. Reimplemented
   * from BaseMapElement.
   *
   * \param  targetP  The painter to draw the element into.
   *
   * \return always true
   */
  bool drawMapElement( QPainter* targetP );
  /**
   * Prints the flight an the task for each flight into the given painter. Reimplemented
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
   * re-project the flights in this flight group. Reimplemented from BaseFlightElement.
   */
  void reProject();

 private:

  QList<Flight::Flight*> flightList;
};

#endif
