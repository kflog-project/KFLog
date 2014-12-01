/***********************************************************************
**
**   Speed.cpp
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

#include "Speed.h"

// initialize static values
Speed::speedUnit Speed::_horizontalUnit = kilometersPerHour;
Speed::speedUnit Speed::_verticalUnit   = metersPerSecond;
Speed::speedUnit Speed::_windUnit       = kilometersPerHour;

Speed::Speed() :
  _speed(0),
  _isValid(false)
{
}

Speed::Speed(double Mps) :
  _speed(Mps),
  _isValid(true)
{
}

Speed::~Speed()
{
}

/** + operator for speed. */
Speed
Speed::operator +(const Speed& x) const
{
  return Speed(x._speed + _speed);
}

/** - operator for speed. */
Speed
Speed::operator -(const Speed& x) const
{
  return Speed(_speed - x._speed);
}

/** - operator for speed. */
Speed
operator -(double left, const Speed& right)
{
  return Speed(left - right.getMps());
}

/** + operator for speed. */
Speed
operator +(double left, const Speed& right)
{
  return Speed(left + right.getMps());
}

/** * operator for speed. */
Speed
operator *(double left, const Speed& right)
{
  return Speed(left * right.getMps());
}

/** / operator for speed. */
double
Speed::operator /(const Speed& x) const
{
  return _speed / x._speed;
}

/** * operator for speed. */
double
Speed::operator *(const Speed& x) const
{
  return _speed * x._speed;
}

/** == operator for Speed */
bool
Speed::operator ==(const Speed& x) const
{
  return (x._speed == _speed);
}

/** != operator for Speed */
bool
Speed::operator !=(const Speed& x) const
{
  return (x._speed != _speed);
}

/** - prefix operator for speed */
Speed
Speed::operator -() const
{
  return Speed(-_speed);
}

/** Returns a formatted string for the default horizontal unit setting. */
QString Speed::getHorizontalText(bool withUnit, uint precision) const
{
  QString result;

  if (withUnit)
    {
      result = QString("%1 %2").arg( getHorizontalValue(), 0, 'f', precision )
                               .arg( getUnitText(_horizontalUnit) );
    }
  else
    {
      result = QString("%1").arg( getHorizontalValue(), 0, 'f', precision );
    }

  return result;
}

/** Returns a formatted string for the default wind unit setting. */
QString Speed::getWindText(bool withUnit, uint precision) const
{
  QString result;

  if (withUnit)
    {
      result = QString("%1 %2").arg( getWindValue(), 0, 'f', precision )
                               .arg( getUnitText(_windUnit) );
    }
  else
    {
      result = QString("%1").arg( getWindValue(), 0, 'f', precision );
    }

  return result;
}

/** Returns a formatted string for the default vertical speed units. */
QString Speed::getVerticalText(bool withUnit, uint precision) const
{
  QString result;

  //@JD: If unit is feet/minute set precision to 0. Saves display space
  //     and should really be sufficient (1 foot/min is 0.005 m/s ...)
  if (_verticalUnit == feetPerMinute)
    {
      precision = 0;
    }

  // @AP: Negative values near zero rounded to zero. The QString arg formatter
  //      cuts not the minus sign, if a negative value is rounded to zero.
  double p10a[] = { 1., 10., 100., 1000. };

  double p10;

  if( precision < 4 )
    {
      p10 = p10a[precision];
    }
  else
    {
      p10 = pow( 10.0, precision);
    }

  double vv;

  if( precision == 0 )
    {
      vv = rint( getVerticalValue() );
    }
  else
    {
      vv = rint( getVerticalValue() * p10 ) / p10;
    }

  if (withUnit)
    {
      result = QString("%1 %2").arg( vv, 0, 'f', precision )
                               .arg( getUnitText(_verticalUnit) );
    }
  else
    {
      result = QString("%1").arg( vv, 0, 'f', precision );
    }

  return result;
}

void Speed::setValueInUnit(double speed, speedUnit unit)
{
  switch (unit)
    {
      case knots:
        setKnot(speed);
        break;
      case milesPerHour:
        setMph(speed);
        break;
      case metersPerSecond:
        setMps(speed);
        break;
      case kilometersPerHour:
        setKph(speed);
        break;
      case feetPerMinute:
        setFpm(speed);
        break;
      default:
        setMps(speed);
        break;
    }
}

QString Speed::getUnitText(speedUnit unit)
{
  switch (unit)
    {
      case knots:
        return "Kt"; // can also be abbreviated as Kn
        break;
      case milesPerHour:
        return "Mph";
        break;
      case metersPerSecond:
        return "m/s";
        break;
      case kilometersPerHour:
        return "Km/h";
        break;
      case feetPerMinute:
        return "Fpm";
        break;
      default:
        return "m/s";
    }
}

double Speed::getValueInUnit(speedUnit unit) const
{
  switch (unit)
    {
      case knots:
        return getKnots();
        break;
      case milesPerHour:
        return getMph();
        break;
      case metersPerSecond:
        return getMps();
        break;
      case kilometersPerHour:
        return getKph();
        break;
      case feetPerMinute:
        return getFpm();
        break;
      default:
        return getMps();
    }
}
