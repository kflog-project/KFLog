/***********************************************************************
**
**   igc3dviewstate.h
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

#ifndef IGC3DVIEWSTATE_H
#define IGC3DVIEWSTATE_H

#include "igc3dviewstate.h"

/**
  * @author Thomas Nielsen
  * @version $Id$
  *
  * Based on Igc3D by Jan Max Krueger <Jan.Krueger@uni-konstanz.de>
  *
  */

class Igc3DViewState
{
	public:
		Igc3DViewState();
		~Igc3DViewState();
		
		void reset();
		
		float height, width;
		float alpha, beta, gamma, deltax, deltay, deltaz;
		float deltayoffset;
		float mag, dist;
		int flag, polyhedron_back, polyhedron_front, flight_trace, flight_shadow; //FLAGS
		float maxx, maxy, maxz, minx, miny, minz;
		int zfactor;
		int timerflag, ms_timer;
		float rotate_fract; // Tenth of degrees to rotate every timer-step
		int flight_marker_position;
		int centering;
};

#endif
