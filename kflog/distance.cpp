/***********************************************************************
**
**   distance.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002      by Andre Somers
**                   2007-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include "distance.h"

// Initialize static constants.
const double Distance::mFromKm    = 1000.0; // 1000.0 meters in 1 km.
const double Distance::mFromMile  = 1609.3; // 1609.3 meters in a mile
const double Distance::mFromNMile = 1852.0; // 1852 meters in a nautical mile
const double Distance::mFromFeet  = 0.3048; // a foot is a bit more than 30 cm

// The distance default unit is kilometers.
Distance::distanceUnit Distance::_distanceUnit = kilometers;

Distance::Distance()
{
  _dist=0;
  _isValid=false;
}


Distance::Distance(int meter)
{
  _dist=double(meter);
  _isValid=true;
}


Distance::Distance(double meter)
{
  _dist=meter;
  _isValid=true;
}


Distance::Distance(const Distance& dst)
{
  _dist = dst._dist;
  _isValid=dst._isValid;
}

/**
 * Copy assignment operator
 */
Distance& Distance::operator=(const Distance& dst)
{
  _dist = dst._dist;
  _isValid = dst._isValid;
  return *this;
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
void Distance::setMeters(int meter)
{
  _dist=double(meter);
  _isValid=true;
}


/** Set the distance in meters */
void Distance::setMeters(double meter)
{
  _dist=meter;
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

/**
 * @returns a string for the currently set distance unit.
 */
QString Distance::getUnitText()
{
  QString unit;

  switch( _distanceUnit )
    {
    case meters:
      unit = "m";
      break;
    case feet:
      unit = "ft";
      break;
    case kilometers:
      unit = "km";
      break;
    case miles:
      unit = "SM";
      break;
    case nautmiles:
      unit = "NM";
      break;
    default:
      unit = "m";
      break;
    }

  return unit;
}

QString Distance::getText( bool withUnit, uint precision, uint chopOrder ) const
{
  QString result;
  double dist;

  switch( _distanceUnit )
    {
    case meters:
      dist = getMeters();
      break;
    case feet:
      dist = getFeet();
      break;
    case kilometers:
      dist = getKilometers();
      break;
    case miles:
      dist = getMiles();
      break;
    case nautmiles:
      dist = getNautMiles();
      break;
    default:
      dist = getMeters();
      break;
    }

  // see if we need to lower the precision
  if( chopOrder > 0 )
    {
      while( precision > 0 && pow( 10, chopOrder ) <= dist )
        {
          precision--;
          chopOrder++;
        }
    }

  if( withUnit )
    {
      result = QString("%1 %2").arg( dist, 0, 'f', precision )
                               .arg( getUnitText() );
    }
  else
    {
      result = QString("%1").arg( dist, 0, 'f', precision );
    }

  return result;
}

QString Distance::getText(double meters, bool withUnit, int precision)
{
  QString result;
  double dist;
  int defprec;

  switch( _distanceUnit )
    {
    case 0: // meters:
      dist = meters;
      defprec = 0;
      break;
    case 1: // feet:
      dist = meters / mFromFeet;
      defprec = 0;
      break;
    case 2: // kilometers:
      dist = meters / mFromKm;
      defprec = ( dist < 1.0 ) ? 2 : 1;
      break;
    case 3: // statute miles:
      dist = meters / mFromMile;
      defprec = ( dist < 1.0 ) ? 3 : 2;
      break;
    case 4: // nautical miles:
      dist = meters / mFromNMile;
      defprec = ( dist < 1.0 ) ? 3 : 2;
      break;
    default:
      dist = meters;
      defprec = 0;
      break;
    }

  if( precision < 0 )
    {
      precision = defprec;
    }

  if( dist < 0 )
    {
      if( withUnit )
        {
          result = getUnitText();
        }
      else
        {
          result = "";
        }
    }
  else
    {
      if( withUnit )
        {
          result = QString("%1 %2").arg( dist, 0, 'f', precision )
                                   .arg( getUnitText() );
        }
      else
        {
          result = QString("%1").arg( dist, 0, 'f', precision );
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

  switch (_distanceUnit)
  {
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
      break;
  }

  return res;
}

void Distance::setValueInCurrentUnit( const double value )
{
  switch( _distanceUnit )
    {
    case meters:
      setMeters(value);
      break;
    case feet:
      setFeet(value);
      break;
    case kilometers:
      setKilometers(value);
      break;
    case miles:
      setMiles(value);
      break;
    case nautmiles:
      setNautMiles(value);
      break;
    default:
      setMeters(value);
      break;
    }
}

double Distance::getValueOfCurrentUnit() const
{
  double dist = 0.0;

  switch( _distanceUnit )
    {
    case meters:
      dist = getMeters();
      break;
    case feet:
      dist = getFeet();
      break;
    case kilometers:
      dist = getKilometers();
      break;
    case miles:
      dist = getMiles();
      break;
    case nautmiles:
      dist = getNautMiles();
      break;
    default:
      dist = getMeters();
      break;
    }

   return dist;
}
