/***********************************************************************
**
**   igc3dflightdata.cpp
**
**   This file is part of KFLog.
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

#include <math.h>
#include <ctype.h>
#include <stdio.h>

#include <QtGui>

#include "igc3dflightdata.h"
#include "igc3dviewstate.h"
#include "mapcalc.h"
#include "wgspoint.h"

class Igc3DViewState;

Igc3DFlightData::Igc3DFlightData( Igc3DViewState *s )
{
  state = s;
  firstDataPoint = 0;
  tmpDataPoint = 0;
  flight_opened_flag = 0;
  flightlength = 0;
}

Igc3DFlightData::~Igc3DFlightData()
{
  tmpDataPoint = firstDataPoint;

  for( int i = 0; i < flightlength; i++ )
    {
      Igc3DFlightDataPoint *delpt = tmpDataPoint;
      tmpDataPoint = tmpDataPoint->next;
      delete delpt;
    }
}

void Igc3DFlightData::calculate_flight(void)
{
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

  for( int i=0; i < flightlength; i++)
    {
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

  for( int i = 0; i < flightlength; i+=3 )
    {
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

      for( int j = 0; j < 3; j++ )
        {
          tmpDataPoint = tmpDataPoint->next;
        }
  }
}

void Igc3DFlightData::flatten_data(void)
{
  float tmpx, tmpy, tmpz;

  if( flight_opened_flag )
    {
      tmpDataPoint = firstDataPoint;
      tmpx = (state->maxx + state->minx) / 2.0;
      tmpy = (state->maxy + state->miny) / 2.0;
      tmpz = (state->maxz + state->minz) / 2.0;

      for( int i = 0; i < flightlength; i++ )
        {
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
  float tmpx, tmpy;

  if( flight_opened_flag )
    {
      tmpDataPoint = firstDataPoint;

      for( int i = 0; i < state->flight_marker_position; i++ )
        {
          tmpDataPoint = tmpDataPoint->next;
        }
      tmpx = tmpDataPoint->x;
      tmpy = tmpDataPoint->y;
      tmpDataPoint = firstDataPoint;

      for( int i = 0; i < flightlength; i++ )
        {
          tmpDataPoint->x = tmpDataPoint->x - tmpx;
          tmpDataPoint->y = tmpDataPoint->y - tmpy;
          tmpDataPoint = tmpDataPoint->next;
        }
    }
}

void Igc3DFlightData::change_zfactor(void)
{
  if( flight_opened_flag )
    {
      tmpDataPoint = firstDataPoint;

      for( int i = 0; i < flightlength; i++ )
        {
          tmpDataPoint->z = (float) (tmpDataPoint->pressureheight * state->zfactor) / 1000.0;
          tmpDataPoint = tmpDataPoint->next;
        }

      calculate_min_max();
      flatten_data();
      calculate_min_max();
    }
}

void Igc3DFlightData::calculate_min_max(void)
{
  float tmpx, tmpy, tmpz;

  if( flight_opened_flag )
    {
      tmpDataPoint = firstDataPoint;
      state->maxx = state->maxy = state->maxz = -1e300;
      state->minx = state->miny = state->minz = 1e300;

      for( int i = 0; i < flightlength; i++ )
        {
          if( tmpDataPoint->x < state->minx )
            state->minx = tmpDataPoint->x;
          if( tmpDataPoint->y < state->miny )
            state->miny = tmpDataPoint->y;
          if( tmpDataPoint->z < state->minz )
            state->minz = tmpDataPoint->z;
          if( tmpDataPoint->x > state->maxx )
            state->maxx = tmpDataPoint->x;
          if( tmpDataPoint->y > state->maxy )
            state->maxy = tmpDataPoint->y;
          if( tmpDataPoint->z > state->maxz )
            state->maxz = tmpDataPoint->z;
          tmpDataPoint = tmpDataPoint->next;
        }
    }
  // Make sure that object is behind the projection plane
  // even if far corner is pointed right at us.
  //state->deltayoffset = - sqrt(state->maxx * state->maxx
  //    + state->maxy * state->maxy + state->maxz * state->maxz) - 0.001;

  (fabs(state->maxx) > fabs(state->minx)) ? tmpx = state->maxx : tmpx = state->minx;
  (fabs(state->maxy) > fabs(state->miny)) ? tmpy = state->maxy : tmpy = state->miny;
  (fabs(state->maxz) > fabs(state->minz)) ? tmpz = state->maxz : tmpz = state->minz;

  state->deltay = -300;
  state->deltayoffset = - sqrt(tmpx * tmpx + tmpy * tmpy + tmpz * tmpz) - 0.1;
  state->deltay = state->deltay + state->deltayoffset;
}

void Igc3DFlightData::draw_flight( QPainter *p )
{

  p->setPen( QColor( 255, 0, 0 ) ); // Color red

  tmpDataPoint = firstDataPoint;

  for( int i = 0; i < flightlength - 1; i++ )
    {
      p->drawLine( (int) tmpDataPoint->row, (int) tmpDataPoint->column,
                   (int) tmpDataPoint->next->row, (int) tmpDataPoint->next->column );

      tmpDataPoint = tmpDataPoint->next;
    }
}

void Igc3DFlightData::draw_marker( QPainter *p )
{
  Igc3DFlightDataPoint *shadow;

  p->setPen( QColor( 15, 125, 55 ) ); // green
  tmpDataPoint = firstDataPoint;

  for( int i = 0; i < state->flight_marker_position; i++ )
    {
      tmpDataPoint = tmpDataPoint->next;
    }

  if( state->flight_trace && state->flight_shadow )
    {
      shadow = tmpDataPoint;

      while( !shadow->has_shadow )
        {
          shadow = shadow->next;
        }

      p->drawLine( (int) tmpDataPoint->row, (int) tmpDataPoint->column,
                   (int) shadow->shadow_row, (int) shadow->shadow_column );
    }
  else if( state->flight_trace )
    {
      p->drawLine( (int) tmpDataPoint->row, (int) tmpDataPoint->column - 15,
                  (int) tmpDataPoint->row, (int) tmpDataPoint->column + 15 );
      p->drawLine( (int) tmpDataPoint->row - 15, (int) tmpDataPoint->column,
                   (int) tmpDataPoint->row + 15, (int) tmpDataPoint->column );
    }
}

void Igc3DFlightData::draw_shadow(QPainter *p)
{
  p->setPen( QColor( 10, 10, 10 ) ); // gray
  tmpDataPoint = firstDataPoint;

  for( int i = 0; i < flightlength - 3; i += 3 )
    {
      p->drawLine( (int) tmpDataPoint->shadow_row,
                   (int) tmpDataPoint->shadow_column,
                   (int) tmpDataPoint->next->next->next->shadow_row,
                   (int) tmpDataPoint->next->next->next->shadow_column );

      tmpDataPoint = tmpDataPoint->next->next->next;
    }
}

void Igc3DFlightData::koord2dist(void)
{
  float centrex, centrey; //x,y
  float rho = 6371.0;
  float tmpx, tmpy;

  // first find 'geographical center of flight'
  calculate_min_max();
  centrex = (state->minx + state->maxx) / 2.0;
  centrey = (state->miny + state->maxy) / 2.0;

  // Calculate x,y distances to center point
  tmpDataPoint = firstDataPoint;

  for( int i = 0; i < flightlength; i++ )
    {
      tmpx = tmpDataPoint->x;
      tmpy = tmpDataPoint->y;

      tmpDataPoint->x = rho * acos( cos( centrey * M_PI / 180.0 ) * cos(
          centrey * M_PI / 180.0 ) * cos( (tmpx - centrex) * M_PI / 180.0 )
          + sin( centrey * M_PI / 180.0 ) * sin( centrey * M_PI / 180.0 ) );

      if( centrex > tmpx )
        {
          tmpDataPoint->x = tmpDataPoint->x * (-1.0);
        }

      tmpDataPoint->y = rho * acos( cos( tmpy * M_PI / 180.0 ) * cos( centrey
          * M_PI / 180.0 ) * cos( (centrex - centrex) * M_PI / 180.0 ) + sin(
          tmpy * M_PI / 180.0 ) * sin( centrey * M_PI / 180.0 ) );

      if( centrey > tmpy )
        {
          tmpDataPoint->y = tmpDataPoint->y * (-1.0);
        }

      tmpDataPoint = tmpDataPoint->next;
    }
}

void Igc3DFlightData::load(Flight* flight)
{
  int fixlines = 0;
  QString r, s, t;

  float hh, mm, ss, lat, latmin, latsec, lon, lonmin, lonsec, hightlocal, gpshightlocal;
  char NS, AV, EW; //flags for North/South, A=Valid/V=navwarning, East/West

  FlightPoint cP;

  reset();

  if( flight && flight->getTypeID() == BaseMapElement::Flight )
    {
      for( int i = 0; i < flight->getRouteLength() - 2; i++ )
        {
          flight->searchGetNextPoint( i, cP );
          hightlocal = cP.height;
          gpshightlocal = cP.gpsHeight;

          //time
          t = printTime( cP.time, false, true );
          sscanf( t.toLatin1().data(), "%02f:%02f:%02f", &hh, &mm, &ss );

          r = WGSPoint::printPos( cP.origP.lat(), true );

          sscanf( r.toLatin1().data(), "%2f%c %2f%c %2f%c %c", &lat, &AV, &latmin,
                  &AV, &latsec, &AV, &NS );
          s = WGSPoint::printPos( cP.origP.lon(), false );

          sscanf( s.toLatin1().data(), "%3f%c %2f%c %2f%c %c", &lon, &AV, &lonmin,
                  &AV, &lonsec, &AV, &EW );

          if( firstDataPoint == 0 )
            {
              firstDataPoint = new Igc3DFlightDataPoint;
              tmpDataPoint = firstDataPoint;
              tmpDataPoint->runningnumber = 1;
            }
          else
            {
              tmpDataPoint->next = new Igc3DFlightDataPoint;
              tmpDataPoint->next->previous = tmpDataPoint;
              tmpDataPoint->next->runningnumber = tmpDataPoint->runningnumber + 1;
              tmpDataPoint = tmpDataPoint->next;
            }

          tmpDataPoint->z = hightlocal / 1000.0;
          tmpDataPoint->pressureheight = hightlocal;
          tmpDataPoint->gpsheight = gpshightlocal;
          tmpDataPoint->timesec = hh * 3600 + mm * 60 + ss;
          tmpDataPoint->y = lat + (latmin / 60.0) + (latsec / 3600.0);

          if( NS == 'S' )
            {
              tmpDataPoint->y = tmpDataPoint->y * (-1.0);
            }

          tmpDataPoint->latdeg = tmpDataPoint->y;
          tmpDataPoint->x = lon + (lonmin / 60.0) + (lonsec / 3600.0);

          if( EW == 'W' )
            {
              tmpDataPoint->x = tmpDataPoint->x * (-1.0);
            }

          tmpDataPoint->londeg = tmpDataPoint->x;
          fixlines++;
        }

      tmpDataPoint->next = firstDataPoint;
      firstDataPoint->previous = tmpDataPoint; // Closing the loop.
      flightlength = fixlines;
      flight_opened_flag = 1;
    }
}

/** reset data structures */
void Igc3DFlightData::reset()
{
  flight_opened_flag = 0;
  flightlength = 0;

  if( firstDataPoint != 0 )
    {
      Igc3DFlightDataPoint *tmp = firstDataPoint;

      while( tmp->next != firstDataPoint )
        {
          tmp = tmp->next;
          delete tmp->previous;
        }

      delete tmp;
      firstDataPoint = 0;
    }
}
