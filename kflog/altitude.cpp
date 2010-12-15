/***************************************************************************
                          altitude.cpp  -  description
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers
                               2010 by Axel Pauli

    email                : axel@kflog.org

    This file is part of KFLog4.

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

#include <QtCore>
#include "altitude.h"

// initialize static value
Altitude::altitudeUnit Altitude::_altitudeUnit = meters;

Altitude::Altitude() : Distance()
{
}

Altitude::Altitude(int meters) : Distance(meters)
{}

Altitude::Altitude(double meters) : Distance(meters)
{}

/** copy constructor */
Altitude::Altitude (const Altitude& alt) : Distance()
{
  _dist    = alt._dist;
  _isValid = alt._isValid;
}

Altitude::Altitude (const Distance& dst): Distance (dst)
{}

Altitude::~Altitude()
{}


QString Altitude::getText(double meter, bool withUnit, int precision)
{
  QString result;
  double dist;
  int defprec=1;

  switch (_altitudeUnit)
  {
    case meters:
      dist=meter;
      defprec=0;
      break;
    case feet:
      dist=meter/mFromFeet;
      defprec=0;
      break;
    case flightlevel:
      dist=rint(meter/(mFromFeet*100.0));
      defprec=3;
      break;
    default:
      dist=meter;
      defprec=0;
  }

  if( precision < 0 )
    {
      precision = defprec;
    }

  if( withUnit )
    {
      if( _altitudeUnit == flightlevel )
        {
          result = QString("%1 %2").arg( getUnitText() )
                                   .arg( dist, 0, 'f', precision );

        }
      else
        {
          result = QString("%1 %2").arg( dist, 0, 'f', precision )
                                   .arg( getUnitText() );
        }
    }
  else
    {
      result = QString("%1").arg( dist, 0, 'f', precision );
    }

  return result;
}

QString Altitude::getText( bool withUnit, int precision ) const
{
  return getText( getMeters(), withUnit, precision );
}

/** Converts a distance from the current units to meters. */
double Altitude::convertToMeters(double dist)
{
  double res;

  switch (_altitudeUnit)
  {
    case meters:
      res=dist;
      break;
    case feet:
      res=dist*mFromFeet;
      break;
    case flightlevel:
      res=dist*(mFromFeet/100.0);
    default:
      res=dist;
  }

  return res;
}

QString Altitude::getUnitText()
{
  QString unit;

  switch (_altitudeUnit)
  {
    case meters:
      unit="m";
      break;
    case feet:
      unit="ft";
      break;
    case flightlevel:
      unit="FL";
      break;
    default:
      unit="m";
  }

  return unit;
}
