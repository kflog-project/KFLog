/***********************************************************************
**
**   flightdataprint.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FLIGHTDATAPRINT_H
#define FLIGHTDATAPRINT_H

#include <QPainter>

#include "flight.h"

/**
 * @author Heiner Lamprecht
 * @version $Id$
 */
class FlightDataPrint
{
  public:
    /** */
    FlightDataPrint(Flight* currentFlight);
    /** */
    ~FlightDataPrint();

 private:
  /** */
  void __printPositionData(QPainter* painter, FlightPoint* cPoint,
          int yPos, const char* text, bool printVario = false,
          bool printSpeed = false);
  /** */
  void __printPositionData(QPainter *painter, Waypoint *cPoint, int yPos);
  unsigned int time;
  unsigned int lasttime;
};

#endif
