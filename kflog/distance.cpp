/***************************************************************************
                          distance.cpp  -  description
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers, 2007 Axel Pauli
    email                : andre@kflog.org

    $Id$

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "distance.h"
#include <cmath>

//initializer for static membervariable
Distance::distanceUnit Distance::_distanceUnit=kilometers;

const double Distance::mFromKm=1000.0;    // 1000.0 meters in 1 km.
const double Distance::mFromMile=1609.3;  // 1609.3 meters in a mile
const double Distance::mFromNMile=1852.0; // 1852 meters in a nautical mile
const double Distance::mFromFeet=0.3048;  // a foot is a bit more than 30 cm


Distance::Distance()
{
  _dist=0;
  _isValid=false;
}


Distance::Distance(int meters)
{
  _dist=double(meters);
  _isValid=true;
}


Distance::Distance(double meters)
{
  _dist=meters;
  _isValid=true;
}


Distance::Distance(const Distance& dst)
{
  _dist = dst._dist;
  _isValid=dst._isValid;
}


Distance::~Distance()
{}


/** Returns distance in meters */
double Distance::getMeters() const
{
  return _dist;
}


/** Get distance in feet */
double Distance::getFeet() const
{
  return _dist / mFromFeet;
}


/** Set the distance in meters */
void Distance::setMeters(int meters)
{
  _dist=double(meters);
  _isValid=true;
}


/** Set the distance in meters */
void Distance::setMeters(double meters)
{
  _dist=meters;
  _isValid=true;
}


/** Set distance in feet */
void Distance::setFeet (int feet)
{
  _dist=double(feet)*mFromFeet;
  _isValid=true;
}


/** Set distance in feet */
void Distance::setFeet (double feet)
{
  _dist=feet*mFromFeet;
  _isValid=true;
}


/** implements == operator for distance */
bool Distance::operator == (const Distance& x) const
{
  return (_dist == x._dist && _isValid && x._isValid);
}


/** implements != operator for distance */
bool Distance::operator != (const Distance& x) const
{
  return(_dist != x._dist);
}


/** implements minus operator */
Distance Distance::operator - (const Distance& op) const
{
  return Distance (_dist - op._dist);
}


/** implements divide operator */
double Distance::operator / (const Distance& op) const
{
  return _dist / op._dist;
}


QString Distance::getText(bool withUnit, uint precision, uint chopOrder) const
{
  QString result, unit;
  double dist;

  switch (_distanceUnit) {
  case meters:
    unit="m";
    dist=getMeters();
    break;
  case feet:
    unit="ft";
    dist=getFeet();
    break;
  case kilometers:
    unit="Km";
    dist=getKilometers();
    break;
  case miles:
    unit="M.";
    dist=getMiles();
    break;
  case nautmiles:
    unit="nM.";
    dist=getNautMiles();
    break;
  default:
    unit="m";
    dist=getMeters();
  }

  QString prec;
  //see if we need to lower the precission
  if (chopOrder>0) {
    while (precision>0 && pow(10,chopOrder)<=dist) {
      precision--;
      chopOrder++;
    }
  }
  prec.setNum(precision);
  if (withUnit) {

    QString fms = QString("%1.") + prec + "f %s";
    result.sprintf( fms.latin1(), dist, unit.latin1() );
  } else {
    QString fms = QString("%1.") + prec + "f";
    result.sprintf( fms.latin1(), dist );
  }
  return result;
}


QString Distance::getText(double meters, bool withUnit, int precision)
{
  QString result, unit;
  double dist;
  uint defprec=1;

  switch (_distanceUnit) {
  case 0: //meters:
    unit="m";
    dist=meters;
    defprec=0;
    break;
  case 1: //feet:
    unit="ft";
    dist=meters/mFromFeet;
    defprec=0;
    break;
  case 2: //kilometers:
    unit="Km";
    dist=meters/mFromKm;
    defprec=2;
    break;
  case 3: //miles:
    unit="M.";
    dist=meters/mFromMile;
    defprec=3;
    break;
  case 4: //nautmiles:
    unit="nM.";
    dist=meters/mFromNMile;
    defprec=3;
    break;
  default:
    unit="m";
    dist=meters;
    defprec=0;
  }

  if (precision<0)
    precision=defprec;

  QString prec;
  prec.setNum(precision);

  if (dist<0) {
    if (withUnit) {
      result=unit;
    } else {
      result="";
    }
  } else {
    if (withUnit) {
      QString fms = QString("%1.") + prec + "f %s";
      result.sprintf( fms.latin1(), dist, unit.latin1() );
    } else {
      QString fms = QString("%1.") + prec + "f";
      result.sprintf( fms.latin1(), dist);
    }
  }
  return result;
}


/** returns the distance in kilometers */
double Distance::getKilometers() const
{
  return _dist/mFromKm;
}


/** Sets the distance in kilometers */
void Distance::setKilometers(int km)
{
  _dist=double(km)*mFromKm;
  _isValid=true;
}


/** Sets the distance in kilometers */
void Distance::setKilometers(double km)
{
  _dist=km*mFromKm;
  _isValid=true;
}


/** returns the distance in miles */
double Distance::getMiles() const
{
  return _dist/mFromMile;
}


/** returns the distance in Nautical Miles */
double Distance::getNautMiles() const
{
  return _dist/mFromNMile;
}


void Distance::setUnit(distanceUnit unit)
{
  _distanceUnit=unit;
}


/** Return the currently set unit for distances */
Distance::distanceUnit Distance::getUnit()
{
  return _distanceUnit;
}


/** Sets the distance in miles */
void Distance::setMiles(double value)
{
  _dist=value*mFromMile;
  _isValid=true;
}


/** sets the distance in Nauticle miles */
void Distance::setNautMiles(double value)
{
  _dist=value*mFromNMile;
  _isValid=true;
}

/** Converts a distance from the current units to meters. */
double Distance::convertToMeters(double dist)
{
  double res;

  switch (_distanceUnit) {
  case 0: //meters:
    res=dist;
    break;
  case 1: //feet:
    res=dist*mFromFeet;
    break;
  case 2: //kilometers:
    res=dist*mFromKm;
    break;
  case 3: //miles:
    res=dist*mFromMile;
    break;
  case 4: //nautmiles:
    res=dist*mFromNMile;
    break;
  default:
    res=dist;
  }

  return res;
}
