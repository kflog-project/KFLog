/***********************************************************************
**
**   igc3dpolyhedron.h
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

#ifndef IGC3DPOLYHEDRON_H
#define IGC3DPOLYHEDRON_H

#include "igc3dview.h"
#include "igc3dviewstate.h"
#include "flight.h"

class Flight;
class Igc3DView;

/**
  * @author Thomas Nielsen
  * @version $Id$
  *
  * Based on Igc3D by Jan Max Krueger <Jan.Krueger@uni-konstanz.de>
  *
  */
class Igc3DPolyhedron // type=0 gives the square 3D box to display the flightdata in.
{
	public:
		Igc3DPolyhedron(Igc3DViewState *s, int t=0);
		~Igc3DPolyhedron();
		void adjust_size(void);
		void calculate(void);
		void draw_back(QPainter *p);
		void draw_front(QPainter *p);
		int is_front(int);
		int mytype;
	private:
		float *x, *y, *z;
		float *tx, *ty, *tz;
		float *row, *column;
		// Let's fix this for a dodecahedron, initially
		int corners, surfacetotal, surfacecorners;
		int **surfaces;
		Igc3DViewState *state;
};

#endif
