/***********************************************************************
**
**   igc3dpolyhedron.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by KFlog-Team
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "igc3dpolyhedron.h"
#include "igc3dviewstate.h"
#include <math.h>

/**
 * Based on 3dIgc by Jan Max Kreuger
 */

Igc3DPolyhedron::Igc3DPolyhedron(Igc3DViewState *s, int t)
{
	int i, tmp;
	
	mytype = t;
	state = s;
	
	//cout << "Igc3DPolyhedron::Igc3DPolyhedron() Type is " << type << endl;
	if(mytype == 0){ // CUBE
		corners = 8;
		surfacetotal = 6;
		surfacecorners = 4;
		
		x = new float[corners];
		y = new float[corners];
		z = new float[corners];
		tx = new float[corners]; // Temporary positions (after rotation)
		ty = new float[corners];
		tz = new float[corners];
		row = new float[corners];
		column = new float[corners];

		surfaces = new int *[surfacetotal];
		for (tmp = 0; tmp < surfacetotal; tmp++){
			surfaces[tmp] = new int [surfacecorners];
		}
		
		x[0] = x[3] = x[5] = x[6] = 40.0;
		x[1] = x[2] = x[4] = x[7] = -40.0;
		y[0] = y[1] = y[6] = y[7] = 40.0;
		y[2] = y[3] = y[4] = y[5] = -40.0;
		z[0] = z[1] = z[2] = z[3] = 40.0;
		z[4] = z[5] = z[6] = z[7] = -40.0;
	
		//Each of the six surfaces consists of 4 points, each addressed by index {x,y,z}[i]
		surfaces[0][0] = surfaces[1][0] = surfaces[2][0] = 0;
		surfaces[0][1] = surfaces[2][3] = surfaces[3][0] = 1;
		surfaces[0][2] = surfaces[3][3] = surfaces[5][0] = 2;
		surfaces[0][3] = surfaces[1][1] = surfaces[5][3] = 3;
		surfaces[3][2] = surfaces[4][0] = surfaces[5][1] = 4;
		surfaces[1][2] = surfaces[4][3] = surfaces[5][2] = 5;
		surfaces[1][3] = surfaces[2][1] = surfaces[4][2] = 6;
		surfaces[2][2] = surfaces[3][1] = surfaces[4][1] = 7;
	} else if(mytype == 1){ // OCTAHEDRON
		corners = 6;
		surfacetotal = 8;
		surfacecorners = 3;
		
		x = new float[corners];
		y = new float[corners];
		z = new float[corners];
		tx = new float[corners]; // Temporary positions (after rotation)
		ty = new float[corners];
		tz = new float[corners];
		row = new float[corners];
		column = new float[corners];

		surfaces = new int *[surfacetotal];
		for (tmp = 0; tmp < surfacetotal; tmp++){
			surfaces[tmp] = new int [surfacecorners];
		}
		
		x[0] = x[2] = x[3] = x[5] = y[0] = y[1] = y[3] = y[4] = z[1] = z[2] = z[4] = z[5] = 0;
		z[0] = x[1] = y[2] = 40.0 * sqrt(2);
		z[3] = x[4] = y[5] = - 40.0 * sqrt(2);
		
		//Each of the six surfaces consists of 4 points, each addressed by index {x,y,z}[i]
		surfaces[0][0] = surfaces[1][0] = surfaces[2][0] = surfaces[3][0] = 0;
		surfaces[0][1] = surfaces[3][2] = surfaces[4][0] = surfaces[5][0] = 1;
		surfaces[0][2] = surfaces[1][1] = surfaces[5][2] = surfaces[7][0] = 2;
		surfaces[4][2] = surfaces[5][1] = surfaces[6][0] = surfaces[7][1] = 3;
		surfaces[1][2] = surfaces[2][1] = surfaces[6][2] = surfaces[7][2] = 4;
		surfaces[2][2] = surfaces[3][1] = surfaces[4][1] = surfaces[6][1] = 5;
	
	} else if(mytype == 2){ // DODECAHEDRON
		corners = 20;
		surfacetotal = 12;
		surfacecorners = 5;
		
		x = new float[corners];
		y = new float[corners];
		z = new float[corners];
		tx = new float[corners]; // Temporary positions (after rotation)
		ty = new float[corners];
		tz = new float[corners];
		row = new float[corners];
		column = new float[corners];

		surfaces = new int *[surfacetotal];
		for (tmp = 0; tmp < surfacetotal; tmp++){
			surfaces[tmp] = new int [surfacecorners];
		}
		
		// Dodecahedron
		// Each of the twelve surfaces consists of 5 points, each addressed by index {x,y,z}[i]
		float xtmp[20] = {
			0.5257311121191336, -0.20081141588622764, -0.6498393924658126, -0.20081141588622764,
			0.5257311121191336, 0.8506508083520399, -0.3249196962329063, -1.0514622242382672,
			-0.3249196962329063, 0.8506508083520399, 0.3249196962329063, -0.8506508083520399,
			-0.8506508083520399, 0.3249196962329063, 1.0514622242382672, 0.20081141588622764,
			-0.5257311121191336, -0.5257311121191336, 0.20081141588622764, 0.6498393924658126};
		float ytmp[20] = {
			0.3819660112501051, 0.6180339887498949, 0.0, -0.6180339887498949,
			-0.3819660112501051, 0.6180339887498949, 1.0, 0.0,
			-1.0, -0.6180339887498949, 1.0, 0.6180339887498949,
			-0.6180339887498949, -1.0, 0.0, 0.6180339887498949,
			0.3819660112501051, -0.3819660112501051, -0.6180339887498949, 0.0};
		float ztmp[20] = {
			0.8506508083520399, 0.8506508083520399, 0.8506508083520399, 0.8506508083520399,
			0.8506508083520399, 0.20081141588622764, 0.20081141588622764, 0.20081141588622764,
			0.20081141588622764, 0.20081141588622764, -0.20081141588622764, -0.20081141588622764,
			-0.20081141588622764, -0.20081141588622764, -0.20081141588622764, -0.8506508083520399,
			-0.8506508083520399, -0.8506508083520399, -0.8506508083520399, -0.8506508083520399};
	

	//	         0    1    2    3    4
	//	     ----------------------------
	//	     0 {"0", "1", "2", "3", "4"},
	//	     1 {"0", "4", "9", "14", "5"},
	//	     2 {"0", "5", "10", "6", "1"},
	//	     3 {"1", "6", "11", "7", "2"},
	//	     4 {"2", "7", "12", "8", "3"},
	//	     5 {"8", "14", "9", "4", "3"},
	//	     6 {"5", "14", "19", "15", "10"},
	//	     7 {"6", "10", "15", "16", "11"},
	//	     8 {"7", "11", "16", "17", "12"},
	//	     9 {"8", "12", "17", "18", "13"},
	//	    10 {"9", "13", "18", "19", "14"},
	//	    11 {"15", "19", "18", "17", "16"}

		surfaces[0][0] = surfaces[1][0] = surfaces[2][0] = 0;
		surfaces[0][1] = surfaces[2][4] = surfaces[3][0] = 1;
		surfaces[0][2] = surfaces[3][4] = surfaces[4][0] = 2;
		surfaces[0][3] = surfaces[4][4] = surfaces[5][4] = 3;
		surfaces[0][4] = surfaces[1][1] = surfaces[5][3] = 4;
		surfaces[1][4] = surfaces[2][1] = surfaces[6][0] = 5;
		surfaces[2][3] = surfaces[3][1] = surfaces[7][0] = 6;
		surfaces[3][3] = surfaces[4][1] = surfaces[8][0] = 7;
		surfaces[4][3] = surfaces[5][0] = surfaces[9][0] = 8;
		surfaces[1][2] = surfaces[5][2] = surfaces[10][0] = 9;
		surfaces[2][2] = surfaces[6][4] = surfaces[7][1] = 10;
		surfaces[3][2] = surfaces[7][4] = surfaces[8][1] = 11;
		surfaces[4][2] = surfaces[8][4] = surfaces[9][1] = 12;
		surfaces[5][1] = surfaces[9][4] = surfaces[10][1] = 13;
		surfaces[1][3] = surfaces[6][1] = surfaces[10][4] = 14;
		surfaces[6][3] = surfaces[7][2] = surfaces[11][0] = 15;
		surfaces[7][3] = surfaces[8][2] = surfaces[11][4] = 16;
		surfaces[8][3] = surfaces[9][2] = surfaces[11][3] = 17;
		surfaces[9][3] = surfaces[10][2] = surfaces[11][2] = 18;
		surfaces[6][2] = surfaces[10][3] = surfaces[11][1] = 19;
	
		for(i = 0; i < corners; i++){
			x[i]= 40 * xtmp[i];
			y[i]= 40 * ytmp[i];
			z[i]= 40 * ztmp[i];
		}
	} else if(mytype == 3){ // ICOSAHEDRON	
		corners = 12;
		surfacetotal = 20;
		surfacecorners = 3;
		
		x = new float[corners];
		y = new float[corners];
		z = new float[corners];
		tx = new float[corners]; // Temporary positions (after rotation)
		ty = new float[corners];
		tz = new float[corners];
		row = new float[corners];
		column = new float[corners];

		surfaces = new int *[surfacetotal];
		for (tmp = 0; tmp < surfacetotal; tmp++){
			surfaces[tmp] = new int [surfacecorners];
		}
		
		// ICOSAHEDRON Surfaces
	
		//		0    1    2
		//	--------------
		//	0     {"0", "1", "2"},
		//	 1     {"0", "2", "3"},
		//	 2     {"0", "3", "4"},
		//	 3     {"0", "4", "5"},
		//	 4     {"0", "5", "1"},
		//	 5     {"1", "6", "2"},
		//	 6     {"2", "7", "3"},
		//	 7     {"3", "8", "4"},
		//	 8     {"4", "9", "5"},
		//	 9     {"5", "10", "1"},
		//	10     {"6", "7", "2"},
		//	11     {"7", "8", "3"},
		//	12     {"8", "9", "4"},
		//	13     {"9", "10", "5"},
		//	14     {"10", "6", "1"},
		//	15     {"6", "11", "7"},
		//	16     {"7", "11", "8"},
		//	17     {"8", "11", "9"},
		//	18     {"9", "11", "10"},
		//	19    {"10", "11", "6"}
	
		surfaces[0][0] = surfaces[1][0] = surfaces[2][0] = surfaces[3][0] = surfaces[4][0] = 0;
		surfaces[0][1] = surfaces[4][2] = surfaces[5][0] = surfaces[9][2] = surfaces[14][2] = 1;
		surfaces[0][2] = surfaces[1][1] = surfaces[5][2] = surfaces[6][0] = surfaces[10][2] = 2;
		surfaces[1][2] = surfaces[2][1] = surfaces[6][2] = surfaces[7][0] = surfaces[11][2] = 3;
		surfaces[2][2] = surfaces[3][1] = surfaces[7][2] = surfaces[8][0] = surfaces[12][2] = 4;
		surfaces[3][2] = surfaces[4][1] = surfaces[8][2] = surfaces[9][0] = surfaces[13][2] = 5;
		surfaces[5][1] = surfaces[10][0] = surfaces[14][1] = surfaces[15][0] = surfaces[19][2] = 6;
		surfaces[6][1] = surfaces[10][1] = surfaces[11][0] = surfaces[15][2] = surfaces[16][0] = 7;
		surfaces[7][1] = surfaces[11][1] = surfaces[12][0] = surfaces[16][2] = surfaces[17][0] = 8;
		surfaces[8][1] = surfaces[12][1] = surfaces[13][0] = surfaces[17][2] = surfaces[18][0] = 9;
		surfaces[9][1] = surfaces[13][1] = surfaces[14][0] = surfaces[18][2] = surfaces[19][0] = 10;
		surfaces[15][1] = surfaces[16][1] = surfaces[17][1] = surfaces[18][1] = surfaces[19][1] = 11;
		
		float xtmp[12] = {
			0.0, 1.0514622242382672, 0.3249196962329063, -0.8506508083520399,
			-0.8506508083520399, 0.3249196962329063, 0.8506508083520399, -0.3249196962329063,
			-1.0514622242382672, -0.3249196962329063, 0.8506508083520399, 0.0};
		float ytmp[12] = {
			0.0, 0.0, 1.0, 0.6180339887498949,
			-0.6180339887498949, -1.0, 0.6180339887498949, 1.0,
			0.0, -1.0, -0.6180339887498949, 0.0};
		float ztmp[12] = {
			1.1755705045849463, 0.5257311121191336, 0.5257311121191336, 0.5257311121191336,
			0.5257311121191336, 0.5257311121191336, -0.5257311121191336, -0.5257311121191336,
			-0.5257311121191336, -0.5257311121191336, -0.5257311121191336, -1.1755705045849463};
		
		// I don't know how else to do this, so here goes... <shame>
		for(i = 0; i < corners; i++){
			x[i]= 40 * xtmp[i];
			y[i]= 40 * ytmp[i];
			z[i]= 40 * ztmp[i];
		}
	}
	//for(i = 0; i < corners; i++){
	//	cout << x[i] << ", " << y[i] << ", " << z[i] << endl;
	//}
}

Igc3DPolyhedron::~Igc3DPolyhedron()
{

}

void Igc3DPolyhedron::adjust_size(void)
{
	int i;
	float polyhedronmax = -1e300;
	float scale = -1e300;
	if(mytype == 0){ // CUBE
		x[0] = x[3] = x[5] = x[6] = state->maxx;
		x[1] = x[2] = x[4] = x[7] = state->minx;
		y[0] = y[1] = y[6] = y[7] = state->maxy;
		y[2] = y[3] = y[4] = y[5] = state->miny;
		z[0] = z[1] = z[2] = z[3] = state->maxz;
		z[4] = z[5] = z[6] = z[7] = state->minz;
	} else if(mytype > 0){
		// find maximum and normalize all before scaling with maximum[[x,y,z].max]
		for(i = 0; i < corners; i++){
			if(x[i] > polyhedronmax){
				polyhedronmax = x[i];
			}
			if(y[i] > polyhedronmax){
				polyhedronmax = y[i];
			}
			if(z[i] > polyhedronmax){
				polyhedronmax = z[i];
			}
		}
		for(i = 0; i < corners; i++){
			x[i] = x[i] / polyhedronmax;
			y[i] = y[i] / polyhedronmax;
			z[i] = z[i] / polyhedronmax;
		}
		
		if(state->minx > scale) scale = state->minx;
		if(state->miny > scale) scale = state->miny;
		if(state->minz > scale) scale = state->minz;
		if(state->maxx > scale) scale = state->maxx;
		if(state->maxy > scale) scale = state->maxy;
		if(state->maxz > scale) scale = state->maxz;
		
		for(i = 0; i < corners; i++){
			x[i]= 1.5 * scale * x[i];
			y[i]= 1.5 * scale * y[i];
			z[i]= 1.5 * scale * z[i];
		}
	}
}

void Igc3DPolyhedron::calculate()
{
	int i;
	//float tmpx, tmpy, tmpz;
	float imagex, imagez;
	float sinalpha, cosalpha, sinbeta, cosbeta, singamma, cosgamma;
	
	sinalpha = sin(state->alpha * M_PI / 180.0);
	cosalpha = cos(state->alpha * M_PI / 180.0);
	sinbeta = sin(state->beta * M_PI / 180.0);
	cosbeta = cos(state->beta * M_PI / 180.0);
	singamma = sin(state->gamma * M_PI / 180.0);
	cosgamma = cos(state->gamma * M_PI / 180.0);

	for (i=0; i < corners; i++){
		
		// ROTATIONS
		// Euler angles (phi, theta, psi) = (gamma, beta, alpha)
		// rotate: psi around old z, theta around intermediate x, phi around new z'.
		// In Mathematica, do:
		// MatrixForm[RotationMatrix3D[\[Gamma], \[Beta], \[Alpha]] . {x, y, z}]
		tx[i] = z[i] * sinbeta * sinalpha
			+ y[i] * (cosalpha * singamma + cosgamma * cosbeta * sinalpha)
			+ x[i] * (cosgamma * cosalpha - cosbeta * singamma * sinalpha);

		ty[i] = z[i] * cosalpha * sinbeta
			- x[i] * (cosbeta * cosalpha * singamma + cosgamma * sinalpha)
			+ y[i] * (cosgamma * cosbeta * cosalpha - singamma * sinalpha);

		tz[i] = z[i] * cosbeta - y[i] * cosgamma * sinbeta + x[i] * singamma * sinbeta;
		
		// SHIFTING AND PROJECTION
		// shift
		tx[i] = tx[i] + state->deltax;
		ty[i] = ty[i] + state->deltay;
		tz[i] = tz[i] + state->deltaz;
		
		ty[i] = - fabs(ty[i]);
		
		// Project into image plane
		imagex = - (tx[i] / ty[i]) * state->dist;
		imagez = - (tz[i] / ty[i]) * state->dist;
		
		// calculate screen coordinates
		column[i] = (((state->height + 80)/ 2)) + state->mag * imagex;
			// slightly above centre...
		row[i] = (state->width / 2 ) - state->mag * imagez; //zeile
	}
}

int Igc3DPolyhedron::is_front(int surf)
{
	float nx, ny, nz;
	float x1, y1, z1;
	float x2, y2, z2;
	float result; //scalar product
	
	x1 = tx[surfaces[surf][1]] - tx[surfaces[surf][0]];
	y1 = ty[surfaces[surf][1]] - ty[surfaces[surf][0]];
	z1 = tz[surfaces[surf][1]] - tz[surfaces[surf][0]];
	
	x2 = tx[surfaces[surf][2]] - tx[surfaces[surf][1]];
	y2 = ty[surfaces[surf][2]] - ty[surfaces[surf][1]];
	z2 = tz[surfaces[surf][2]] - tz[surfaces[surf][1]];
	
	nx = y1 * z2 - z1 * y2;
	ny = z1 * x2 - x1 * z2;
	nz = x1 * y2 - y1 * x2;
	
	result = - tx[surfaces[surf][1]] * nx
		- ty[surfaces[surf][1]] * ny
		- tz[surfaces[surf][1]] * nz;
	
	if(result > 0){
		return 1;
	} else {
		return 0;
	}
}

void Igc3DPolyhedron::draw_back(QPainter *p)
{
	//QColor c(
	//	(int) (255.0*rand()/(RAND_MAX+1.0)),
	//	(int) (255.0*rand()/(RAND_MAX+1.0)),
	//	(int) (255.0*rand()/(RAND_MAX+1.0))
	//   );
	//p->setBrush(c);
	//p->drawRect( 200, 200, 100, 100 );

	int i, j;

	QColor blue(100,100,255);
	p->setPen(blue);
	
	for(i = 0; i < surfacetotal; i++){
		if(!is_front(i)){
			for(j = 0; j < surfacecorners; j++){
				p->drawLine(row[surfaces[i][j]], column[surfaces[i][j]],
					row[surfaces[i][(j+1)%surfacecorners]], column[surfaces[i][(j+1)%surfacecorners]]);	
			}
		}
	}
}

void Igc3DPolyhedron::draw_front(QPainter *p)
{
	int i, j;

	QColor blue(0,0,255);
	p->setPen(blue);
	
	//cout << "Drawing front: Surfaces ";
	for(i = 0; i < surfacetotal; i++){
		if(is_front(i)){
			//cout << i << ",";
			for(j = 0; j < surfacecorners; j++){
				p->drawLine(row[surfaces[i][j]], column[surfaces[i][j]],
					row[surfaces[i][(j+1)%surfacecorners]], column[surfaces[i][(j+1)%surfacecorners]]);	
			}
		}
	}
	//cout << endl;
}
