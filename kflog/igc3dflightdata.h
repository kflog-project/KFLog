/***********************************************************************
**
**   igc3dflightdata.h
**
**   This file is part of KFLog2.
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
/*
*    3D view was recycled from '3digc' (c) 2001 by Jan Max Krueger
*/

#ifndef IGC3DFLIGHTDATA_H
#define IGC3DFLIGHTDATA_H

#include <qpainter.h>
#include <qlist.h>

#include "igc3dviewstate.h"
#include "flight.h"

class Flight;
class Igc3DViewState;

class Igc3DFlightDataPoint
{
	public:
		Igc3DFlightDataPoint();
		~Igc3DFlightDataPoint();
		Igc3DFlightDataPoint *next, *previous;
		int runningnumber;
		int has_shadow;
		float x, y, z, z_shadow;
		//double tx, ty, tz;
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
		void read_igc_file(QString st);
		void load(Flight *flight);
		int flight_opened_flag;
		void centre_data_to_marker(void);
	
		Igc3DFlightDataPoint *firstDataPoint, *tmpDataPoint;
		int flightlength;
		
	private:
		//float *x, *y, *z;
		//float *tx, *ty, *tz;
		//float *row, *column;
		Igc3DViewState *state;
		//int arraylength;
	
	private: // Private methods
		// reset data structures
		void reset();
};

#endif
