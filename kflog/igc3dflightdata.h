/***********************************************************************
**
**   igc3dflightdata.h
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
/*
*    3D view was recycled from '3digc' (c) 2001 by Jan Max Krueger
*/

#ifndef IGC_3D_FLIGHT_DATA_H
#define IGC_3D_FLIGHT_DATA_H

#include "igc3dviewstate.h"
#include "flight.h"

class QPainter;
class QString;

class Flight;
class Igc3DViewState;

class Igc3DFlightDataPoint
{
	public:

		Igc3DFlightDataPoint()
		{
		  next = previous = 0;
		  x = y = z = z_shadow = 0.0;
		  has_shadow = 0;
		};

		Igc3DFlightDataPoint *next, *previous;
		int runningnumber;
		int has_shadow;
		float x, y, z, z_shadow;
		float row, column;
		float shadow_row, shadow_column;
		float latdeg, londeg;
		float pressureheight, gpsheight;
		float timesec;
};

class Igc3DFlightData
{
	public:

		Igc3DFlightData(Igc3DViewState *s);

		~Igc3DFlightData();

		void flatten_data(void);
		void change_zfactor(void);
		void calculate_min_max(void);
		void calculate_flight(void);
		void calculate_shadow(void);
		void draw_flight(QPainter *p);
		void draw_shadow(QPainter *p);
		void draw_marker(QPainter *p);
		void koord2dist(void);
		void load(Flight *flight);
		int flight_opened_flag;
		void centre_data_to_marker(void);
	
		Igc3DFlightDataPoint *firstDataPoint, *tmpDataPoint;
		int flightlength;
		
  private: // Private methods

    /** reset data structures. */
    void reset();

	private:

		Igc3DViewState *state;
	};

#endif
