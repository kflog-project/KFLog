/***********************************************************************
**
**   igc3dflightdata.cpp
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

#include <math.h>
#include <ctype.h>
#include <stdio.h>

#include <qstring.h>

#include <igc3dflightdata.h>
#include <igc3dviewstate.h>
#include <mapcalc.h>

class Igc3DViewState;

Igc3DFlightData::Igc3DFlightData(Igc3DViewState *s)
{
	state = s;
	//this->x = new float[20000]; // Room for Box and shadow
	//this->y = new float[20000];
	//this->z = new float[20000];
	//this->tx = new float[20000]; // rotated position of points
	//this->ty = new float[20000];
	//this->tz = new float[20000];
	//this->row = new float[20000];
	//this->column = new float[20000];
		
	firstDataPoint = NULL;
	tmpDataPoint = NULL;
	flight_opened_flag = 0;
	flightlength = 0;
}

Igc3DFlightData::~Igc3DFlightData()
{
	int i;
	Igc3DFlightDataPoint *delpt;
	
	tmpDataPoint = firstDataPoint;
	for(i = 0; i< flightlength; i++){
		delpt = tmpDataPoint;
		tmpDataPoint = tmpDataPoint->next;
		delete delpt;
	}
}

void Igc3DFlightData::calculate_flight(void)
{
	int i;
	float imagex, imagez;
	float sinalpha, cosalpha, sinbeta, cosbeta, singamma, cosgamma;
	float tx, ty, tz;
	
	sinalpha = sin(state->alpha * M_PI / 180.0);
	cosalpha = cos(state->alpha * M_PI / 180.0);
	sinbeta = sin(state->beta * M_PI / 180.0);
	cosbeta = cos(state->beta * M_PI / 180.0);
	singamma = sin(state->gamma * M_PI / 180.0);
	cosgamma = cos(state->gamma * M_PI / 180.0);

	tmpDataPoint = firstDataPoint;
	for (i=0; i < flightlength; i++){
		
		// ROTATIONS
		// Euler angles (phi, theta, psi) = (gamma, beta, alpha)
		// rotate: psi around old z, theta around intermediate x, phi around new z'.
		// In Mathematica, do:
		// MatrixForm[RotationMatrix3D[\[Gamma], \[Beta], \[Alpha]] . {x, y, z}]
		tx = tmpDataPoint->z * sinbeta * sinalpha
			+ tmpDataPoint->y * (cosalpha * singamma + cosgamma * cosbeta * sinalpha)
			+ tmpDataPoint->x * (cosgamma * cosalpha - cosbeta * singamma * sinalpha);

		ty = tmpDataPoint->z * cosalpha * sinbeta
			- tmpDataPoint->x * (cosbeta * cosalpha * singamma + cosgamma * sinalpha)
			+ tmpDataPoint->y * (cosgamma * cosbeta * cosalpha - singamma * sinalpha);

		tz = tmpDataPoint->z * cosbeta - tmpDataPoint->y * cosgamma * sinbeta + tmpDataPoint->x * singamma * sinbeta;
		
		// SHIFTING AND PROJECTION
		// shift
		tx = tx + state->deltax;
		ty = ty + state->deltay;
		tz = tz + state->deltaz;
		
		ty = - fabs(ty);
		
		// Project into image plane
		imagex = - (tx / ty) * state->dist;
		imagez = - (tz / ty) * state->dist;
		
		// calculate screen coordinates
		tmpDataPoint->column = (((state->height + 80)/ 2)) + state->mag * imagex;
		// slightly above centre...
		
		tmpDataPoint->row = (state->width / 2 ) - state->mag * imagez;
		
		tmpDataPoint = tmpDataPoint->next;
	}
}

void Igc3DFlightData::calculate_shadow(void)
{
	int i,j;
	float imagex, imagez;
	float sinalpha, cosalpha, sinbeta, cosbeta, singamma, cosgamma;
	float tx, ty, tz;
	
	sinalpha = sin(state->alpha * M_PI / 180.0);
	cosalpha = cos(state->alpha * M_PI / 180.0);
	sinbeta = sin(state->beta * M_PI / 180.0);
	cosbeta = cos(state->beta * M_PI / 180.0);
	singamma = sin(state->gamma * M_PI / 180.0);
	cosgamma = cos(state->gamma * M_PI / 180.0);

	tmpDataPoint = firstDataPoint;
	for (i = 0; i < flightlength; i+=3){
		
		// ROTATIONS
		// Euler angles (phi, theta, psi) = (gamma, beta, alpha)
		// rotate: psi around old z, theta around intermediate x, phi around new z'.
		// In Mathematica, do:
		// MatrixForm[RotationMatrix3D[\[Gamma], \[Beta], \[Alpha]] . {x, y, z}]
		tx = tmpDataPoint->z_shadow * sinbeta * sinalpha
			+ tmpDataPoint->y * (cosalpha * singamma + cosgamma * cosbeta * sinalpha)
			+ tmpDataPoint->x * (cosgamma * cosalpha - cosbeta * singamma * sinalpha);

		ty = tmpDataPoint->z_shadow * cosalpha * sinbeta
			- tmpDataPoint->x * (cosbeta * cosalpha * singamma + cosgamma * sinalpha)
			+ tmpDataPoint->y * (cosgamma * cosbeta * cosalpha - singamma * sinalpha);

		tz = tmpDataPoint->z_shadow * cosbeta - tmpDataPoint->y * cosgamma * sinbeta + tmpDataPoint->x * singamma * sinbeta;
		
		// SHIFTING AND PROJECTION
		// shift
		tx = tx + state->deltax;
		ty = ty + state->deltay;
		tz = tz + state->deltaz;
		
		ty = - fabs(ty);
		
		// Project into image plane
		imagex = - (tx / ty) * state->dist;
		imagez = - (tz / ty) * state->dist;
		
		// calculate screen coordinates
		tmpDataPoint->shadow_column = (((state->height + 80)/ 2)) + state->mag * imagex;
		// slightly above centre...
		
		tmpDataPoint->shadow_row = (state->width / 2 ) - state->mag * imagez;
		tmpDataPoint->has_shadow = 1;
		
		for(j = 0; j < 3; j++){
			tmpDataPoint = tmpDataPoint->next;
		}
	}
}

void Igc3DFlightData::flatten_data(void)
{
	int i;
	float tmpx, tmpy, tmpz;
	if(flight_opened_flag){
		tmpDataPoint = firstDataPoint;
		tmpx = ( state->maxx + state->minx ) / 2.0;
		tmpy = ( state->maxy + state->miny ) / 2.0;
		tmpz = ( state->maxz + state->minz ) / 2.0;
		for(i = 0; i < flightlength; i++){
			tmpDataPoint->x = tmpDataPoint->x - tmpx;
			tmpDataPoint->y = tmpDataPoint->y - tmpy;
			tmpDataPoint->z = tmpDataPoint->z - tmpz;
			tmpDataPoint->z_shadow = state->minz - tmpz; // shadow at lowest point of flight
			tmpDataPoint = tmpDataPoint->next;
		}
	}
}

void Igc3DFlightData::centre_data_to_marker(void)
{
	int i;
	float tmpx, tmpy;
	if(flight_opened_flag){
		tmpDataPoint = firstDataPoint;
		for (i = 0; i < state->flight_marker_position; i++){
			tmpDataPoint = tmpDataPoint->next;
		}
		tmpx = tmpDataPoint->x;
		tmpy = tmpDataPoint->y;
		tmpDataPoint = firstDataPoint;
		for(i = 0; i < flightlength; i++){
			tmpDataPoint->x = tmpDataPoint->x - tmpx;
			tmpDataPoint->y = tmpDataPoint->y - tmpy;
			tmpDataPoint = tmpDataPoint->next;
		}
	}
}

void Igc3DFlightData::change_zfactor(void)
{
	int i;
	if(flight_opened_flag){
		tmpDataPoint = firstDataPoint;
		for(i = 0; i < flightlength; i++){
			tmpDataPoint->z = (float)  (tmpDataPoint->pressureheight * state->zfactor) / 1000.0;
			tmpDataPoint = tmpDataPoint->next;
		}
		calculate_min_max();
		flatten_data();
		calculate_min_max();
	}
}

void Igc3DFlightData::calculate_min_max(void)
{
	int i;
	float tmpx, tmpy, tmpz;

	if(flight_opened_flag){
		tmpDataPoint = firstDataPoint;
		state->maxx = state->maxy = state->maxz = -1e300;
		state->minx = state->miny = state->minz = 1e300;
		for(i = 0; i < flightlength; i++){
			if(tmpDataPoint->x < state->minx) state->minx = tmpDataPoint->x;
			if(tmpDataPoint->y < state->miny) state->miny = tmpDataPoint->y;
			if(tmpDataPoint->z < state->minz) state->minz = tmpDataPoint->z;
			if(tmpDataPoint->x > state->maxx) state->maxx = tmpDataPoint->x;
			if(tmpDataPoint->y > state->maxy) state->maxy = tmpDataPoint->y;
			if(tmpDataPoint->z > state->maxz) state->maxz = tmpDataPoint->z;
			tmpDataPoint = tmpDataPoint->next;
		}
	}
	// Make sure that object is behind the projection plane
	// even if far corner is pointed right at us.
	//state->deltayoffset = - sqrt(state->maxx * state->maxx
	//	+ state->maxy * state->maxy + state->maxz * state->maxz) - 0.001;
	
	(fabs(state->maxx) > fabs(state->minx)) ? tmpx = state->maxx : tmpx = state->minx;
	(fabs(state->maxy) > fabs(state->miny)) ? tmpy = state->maxy : tmpy = state->miny;
	(fabs(state->maxz) > fabs(state->minz)) ? tmpz = state->maxz : tmpz = state->minz;
	
	state->deltay = -300;
	state->deltayoffset = - sqrt(tmpx * tmpx + tmpy * tmpy + tmpz * tmpz) - 0.1;
	state->deltay = state->deltay + state->deltayoffset;
}

void Igc3DFlightData::draw_flight(QPainter *p)
{
	int i;
	QColor red(255,0,0);
	p->setPen(red);
	tmpDataPoint = firstDataPoint;
	for (i=0; i < flightlength - 1; i++){
		p->drawLine(tmpDataPoint->row, tmpDataPoint->column,
				 tmpDataPoint->next->row, tmpDataPoint->next->column);
		tmpDataPoint = tmpDataPoint->next;
	}
}

void Igc3DFlightData::draw_marker(QPainter *p)
{
	int i;
	QColor green(15,125,55);
	Igc3DFlightDataPoint *shadow;
	
	p->setPen(green);
	tmpDataPoint = firstDataPoint;
	for (i = 0; i < state->flight_marker_position; i++){
		tmpDataPoint = tmpDataPoint->next;
	}
	if(state->flight_trace && state->flight_shadow){
		shadow = tmpDataPoint;
		while(!shadow->has_shadow){
			shadow = shadow->next;
		}
		p->drawLine(tmpDataPoint->row, tmpDataPoint->column,
			 shadow->shadow_row, shadow->shadow_column);
	} else if(state->flight_trace){
		p->drawLine(tmpDataPoint->row, tmpDataPoint->column - 15,
			tmpDataPoint->row, tmpDataPoint->column + 15);
		p->drawLine(tmpDataPoint->row - 15, tmpDataPoint->column,
			tmpDataPoint->row + 15, tmpDataPoint->column);
	}
}

void Igc3DFlightData::draw_shadow(QPainter *p)
{
	int i;
	QColor grey(10,10,10);
	p->setPen(grey);
	tmpDataPoint = firstDataPoint;
	for (i=0; i < flightlength - 3; i+=3){
		p->drawLine(tmpDataPoint->shadow_row, tmpDataPoint->shadow_column,
				 tmpDataPoint->next->next->next->shadow_row,
				 tmpDataPoint->next->next->next->shadow_column);
		tmpDataPoint = tmpDataPoint->next->next->next;
	}
}

void Igc3DFlightData::koord2dist(void)
{
	float centrex, centrey; //x,y
	float rho = 6371.0;
	float tmpx, tmpy;
	int i;
	
	// first find 'geographical centre of flight'
	calculate_min_max();
	centrex = (state->minx + state->maxx)/2.0;
	centrey = (state->miny + state->maxy)/2.0;
	
	// Calculate x,y distances to centre point
	tmpDataPoint = firstDataPoint;
	for(i = 0; i < flightlength; i++){
		tmpx = tmpDataPoint->x;
		tmpy = tmpDataPoint->y;
		tmpDataPoint->x = rho * acos( cos(centrey * M_PI / 180.0) * cos (centrey * M_PI / 180.0)
				* cos((tmpx - centrex) * M_PI / 180.0)
			  + sin(centrey * M_PI / 180.0) * sin(centrey * M_PI / 180.0) );
		if(centrex > tmpx){
			tmpDataPoint->x = tmpDataPoint->x * (-1.0);
		}
		tmpDataPoint->y = rho * acos( cos(tmpy * M_PI / 180.0) * cos (centrey * M_PI / 180.0)
				* cos((centrex - centrex) * M_PI / 180.0)
			  + sin(tmpy * M_PI / 180.0) * sin(centrey * M_PI / 180.0) );
		if(centrey > tmpy){
			tmpDataPoint->y = tmpDataPoint->y * (-1.0);
		}
		tmpDataPoint = tmpDataPoint->next;
	}
	// my $dist=dist($centrey,$DECLON[$i],$DECLAT[$i],$DECLON[$i]);
	// dist =  $rho * acos(cos( $lat0 ) * cos( $lat1 ) * cos( $lon0 - $lon1 ) +
        // 					sin( $lat0 ) * sin( $lat1 ) );
	// if($centrey>$DECLAT[$i]){ $dist = -$dist; }
}

void Igc3DFlightData::read_igc_file(QString st)  // Cleanly handled with QString now.
{
/*
	QString line;
	char startchar;
	QFile IGCFile (  st );
	int linenum = 0;
	int fixlines = 0;

	float hh, mm, ss, lat, latmin, latmindez, lon, lonmin, lonmindez, hightlocal, gpshightlocal;
	char NS, AV, EW;	//flags for North/South, A=Valid/V=navwarning, East/West
	
	linenum = 0;
	
	
	if ( IGCFile.open(IO_ReadOnly) ) {    // file opened successfully
		QTextStream t( &IGCFile );        // use a text stream
        	while ( !t.atEnd() ) {        // until end of file...
			line = t.readLine();       // line of text excluding '\n'
			++linenum;
			sscanf ((const char *) line, "%1s", &startchar);
			if (startchar == 'B') {
				sscanf ((const char *) line, "%*c%2f%2f%2f%2f%2f%3f%c%3f%2f%3f%c%c%5f%5f",
					&hh, &mm, &ss, &lat, &latmin, &latmindez, &NS, &lon,
					&lonmin, &lonmindez, &EW, &AV, &hightlocal, &gpshightlocal);
				if (AV == 'V'){ // Unsicherer Wert
					continue;
				}
				if(firstDataPoint == NULL){
					firstDataPoint = new Igc3DFlightDataPoint;
					tmpDataPoint = firstDataPoint;
					tmpDataPoint->runningnumber = 1;
				} else {
					tmpDataPoint->next = new Igc3DFlightDataPoint;
					tmpDataPoint->next->previous = tmpDataPoint->next;
					tmpDataPoint->next->runningnumber = tmpDataPoint->runningnumber + 1;
					tmpDataPoint = tmpDataPoint->next;
				}
				
				tmpDataPoint->z = hightlocal / 1000.0;
				tmpDataPoint->pressureheight = hightlocal;
				tmpDataPoint->gpsheight = gpshightlocal;
				
				tmpDataPoint->timesec = hh * 3600 + mm * 60 + ss;
				
				tmpDataPoint->y = lat + (latmin / 60.0) + (latmindez / 60000.0);
				if (NS == 'S'){
					tmpDataPoint->y =  tmpDataPoint->y * (-1.0);
				}
				tmpDataPoint->latdeg = tmpDataPoint->y;

				tmpDataPoint->x = lon + (lonmin / 60.0) + (lonmindez / 60000.0);
				if (EW == 'W'){
					 tmpDataPoint->x = tmpDataPoint->x * (-1.0);
				}
				tmpDataPoint->londeg = tmpDataPoint->x;
				
				fixlines++;
			}
		}
        	IGCFile.close();
        	tmpDataPoint->next = firstDataPoint;
        	firstDataPoint->previous = tmpDataPoint; // Closing the loop.
	}
	flightlength = fixlines;
	//arraylength = flightlength;
	flight_opened_flag = 1;
*/
}

void Igc3DFlightData::load(Flight* flight)
{
  int linenum = 0;
  int fixlines = 0;
  int i;
  QString r,s,t;

  float hh, mm, ss, lat, latmin, latmindez, lon, lonmin, lonmindez, hightlocal, gpshightlocal;
  char NS, AV, EW;	//flags for North/South, A=Valid/V=navwarning, East/West
	
  linenum = 0;

  struct flightPoint cP;

  reset();

  if (flight && flight->getTypeID() == BaseMapElement::Flight) {
    for (i=0; i < ((int)flight->getRouteLength())-2; i++){
       flight->searchGetNextPoint(i, cP);
       hightlocal = cP.height;
       gpshightlocal = cP.gpsHeight;
       //time
       t = printTime(cP.time, false, true);
       sscanf ((const char *) t, "%02f:%02f:%02f", &hh, &mm, &ss);

       r = printPos(cP.origP.x(),true);
       sscanf ((const char *) r, "%2f%c %2f%c %2f%c %c", &lat, &AV, &latmin, &AV, &latmindez, &AV, &NS);
       s = printPos(cP.origP.y(),false);
       sscanf ((const char *) s, "%3f%c %2f%c %2f%c %c",&lon, &AV, &lonmin, &AV, &lonmindez, &AV, &EW);

       if(firstDataPoint == NULL){
         firstDataPoint = new Igc3DFlightDataPoint;
         tmpDataPoint = firstDataPoint;
         tmpDataPoint->runningnumber = 1;
       }
       else {
         tmpDataPoint->next = new Igc3DFlightDataPoint;
         tmpDataPoint->next->previous = tmpDataPoint;
         tmpDataPoint->next->runningnumber = tmpDataPoint->runningnumber + 1;
         tmpDataPoint = tmpDataPoint->next;
       }
 				
       tmpDataPoint->z = hightlocal / 1000.0;
       tmpDataPoint->pressureheight = hightlocal;
       tmpDataPoint->gpsheight = gpshightlocal;
       				
       tmpDataPoint->timesec = hh * 3600 + mm * 60 + ss;
       				
       tmpDataPoint->y = lat + (latmin / 60.0) + (latmindez / 60000.0);
       if (NS == 'S'){
         tmpDataPoint->y =  tmpDataPoint->y * (-1.0);
       }
       tmpDataPoint->latdeg = tmpDataPoint->y;

       tmpDataPoint->x = lon + (lonmin / 60.0) + (lonmindez / 60000.0);
       if (EW == 'W'){
         tmpDataPoint->x = tmpDataPoint->x * (-1.0);
       }
       tmpDataPoint->londeg = tmpDataPoint->x;
       				
       fixlines++;
	 }
  tmpDataPoint->next = firstDataPoint;
  firstDataPoint->previous = tmpDataPoint; // Closing the loop.

  flightlength = fixlines;
  //arraylength = flightlength;
  flight_opened_flag = 1;
  }
}

/** reset data structures */
void Igc3DFlightData::reset()
{
  flight_opened_flag = 0;
  flightlength = 0;
  Igc3DFlightDataPoint *tmp;
    	
	if (firstDataPoint != 0) {
	  tmp = firstDataPoint;
	  while (tmp->next != firstDataPoint) {
	    tmp = tmp->next;
	    delete tmp->previous;
	  }
	  delete tmp;
	  firstDataPoint = 0;
	}
}

Igc3DFlightDataPoint::Igc3DFlightDataPoint()
{
	next = NULL;
	previous = NULL;
	z_shadow = 0.0;
	has_shadow = 0;
}

Igc3DFlightDataPoint::~Igc3DFlightDataPoint()
{
}
