/***********************************************************************
**
**   flightdataprint.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FLIGHT_DATA_PRINT_H
#define FLIGHT_DATA_PRINT_H

#include <QPainter>
#include <QString>

#include "flight.h"

/**
 * \class FlightDataPrint
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Prints out the data of the current flight.
 *
 * Prints out the data of the current flight.
 *
 * \date 2002-2011
 *
 * \version $Id$
 */
class FlightDataPrint
{
 public:

  FlightDataPrint( Flight* currentFlight );

  virtual ~FlightDataPrint();

 private:

  void __printPositionData( QPainter* painter, FlightPoint* cPoint,
                            int yPos, QString text, bool printVario = false,
                            bool printSpeed = false);

  void __printPositionData(QPainter *painter, Waypoint *cPoint, int yPos);

  unsigned int time;

  unsigned int lasttime;
};

#endif
