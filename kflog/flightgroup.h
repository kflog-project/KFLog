/***********************************************************************
**
**   flightgroup.h
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

#ifndef FLIGHTGROUP_H
#define FLIGHTGROUP_H

#include "baseflightelement.h"
#include "flight.h"

#include <qlist.h>

/**
  *@author Harald Maier
  */

class FlightGroup : public BaseFlightElement
{
public: 
	FlightGroup(QString fName);
	FlightGroup(QList <Flight::Flight> fList, QString fName);
	~FlightGroup();
//  QString getFlightInfoString();
  QList<wayPoint> getWPList();
  /**
  * Draws the flight an the task for each fligth into the given painter. Reimplemented
  * from BaseMapElement.
  * @param  targetP  The painter to draw the element into.
  * @param  maskP  The maskpainter for targetP
  */
  void drawMapElement(QPainter* targetP, QPainter* maskP);
  /** */
  void printMapElement(QPainter* targetP, bool isText);
  /** No descriptions */
  QList<Flight::Flight> getFlightList();
  void removeFlight(BaseFlightElement *f);

private:
  QList<Flight::Flight> flightList;
};

#endif
