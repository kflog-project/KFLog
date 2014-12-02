/***********************************************************************
**
**   distance.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by by Andre Somers
**                   2007-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include "distance.h"

//initializer for static member variable
Distance::distanceUnit Distance::_distanceUnit=kilometers;

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
    unit="SM";
    dist=getMiles();
    break;
  case nautmiles:
    unit="NM";
    dist=getNautMiles();
    break;
  default:
    unit="m";
    dist=getMeters();
    break;
  }

  QString prec;
  //see if we need to lower the precision
  if (chopOrder>0) {
    while (precision>0 && pow(10,chopOrder)<=dist) {
      precision--;
      chopOrder++;
    }
  }
  prec.setNum(precision);
  if (withUnit) {

    QString fms = QString("%1.") + prec + "f %s";
    result.sprintf( fms.toLatin1().data(), dist, unit.toLatin1().data() );
  } else {
    QString fms = QString("%1.") + prec + "f";
    result.sprintf( fms.toLatin1().data(), dist );
  }
  return result;
}

QString Distance::getText(double meters, bool withUnit, int precision)
{
  QString result, unit;
  double dist;
  uint defprec=1;

  switch (_distanceUnit)
    {
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
	unit="SM";
	dist=meters/mFromMile;
	defprec=3;
	break;
      case 4: //nautmiles:
	unit="NM";
	dist=meters/mFromNMile;
	defprec=3;
	break;
      default:
	unit="m";
	dist=meters;
	defprec=0;
	break;
    }

  if (precision < 0)
    {
      precision = defprec;
    }

  QString prec;
  prec.setNum (precision);

  if (dist < 0)
    {
      if (withUnit)
	{
	  result = unit;
	}
      else
	{
	  result = "";
	}
    }
  else
    {
      if (withUnit)
	{
	  QString fms = QString ("%1.") + prec + "f %s";
	  result.sprintf (fms.toLatin1 ().data (), dist,
			  unit.toLatin1 ().data ());
	}
      else
	{
	  QString fms = QString ("%1.") + prec + "f";
	  result.sprintf (fms.toLatin1 ().data (), dist);
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
