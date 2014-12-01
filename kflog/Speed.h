/***********************************************************************
**
**   Speed.h
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

#ifndef SPEED_H
#define SPEED_H

#include <QString>

/**
 * \class Speed
 *
 * \author Andre Somers, Axel Pauli
 *
 * \brief This class handles different speed units and arithmetics.
 *
 * This class can contain a speed. It provides set and get functions for
 * all popular units. For this class, there are two default units you can
 * set: the horizontal and the vertical unit. The horizontal unit is used
 * for the airspeed, the vertical unit is used for climb, McCready settings,
 * etc. There are convenience functions to access the value using both
 * appropriate units.
 *
 * \date 2002-2014
 */
class Speed
{

public:
    /**
     * The speed unit enumeration lists the units that apply to speed.
     */
    enum speedUnit
    {
      metersPerSecond=0,   /** meters per second */
      kilometersPerHour=1, /** kilometers per hour */
      knots=2,             /** knots (nautical miles per hour) */
      milesPerHour=3,      /** statute miles per hour */
      feetPerMinute=4      /** feet per minute */
    };

    /**
     * The following constants define the factor by which to multiply meters per
     * second to get the indicated unit.
     */
    static const double toKph  = 3.6;
    static const double toMph  = 2.2369;
    static const double toKnot = 1.9438;
    static const double toFpm  = 196.8504;

    /**
     * Constructor
     */
    Speed();

    /**
     * Constructor
     *
     * Sets the speed to meters per second.
     */
    Speed(double Mps);

    /**
     * Destructor
     */
    virtual ~Speed();

    /**
     * Get speed in Knots.
     */
    double getKnots() const
    {
      return (_speed * toKnot);
    };

    /**
     * Get speed in Kilometers per hour
     */
    double getKph() const
    {
      return (_speed * toKph);
    };

    /**
     * Get speed in Meters per Second
     */
    double getMps() const
    {
      return (_speed);
    };

    /**
     * Get speed in statute miles per hour
     */
    double getMph() const
    {
      return (_speed * toMph);
    };

    /**
     * Get speed in Feed per minute
     */
    double getFpm() const
    {
      return (_speed * toFpm);
    };

    /**
     * Set speed in statute miles per hour
     */
    void setMph(double speed)
    {
      _speed = speed / toMph;
      _isValid = true;
    };

    /**
     * Set speed in knots
     */
    void setKnot(double speed)
    {
      _speed = speed / toKnot;
      _isValid = true;
    };

    /**
     * Set speed in Kilometers per hour
     */
    void setKph(double speed)
    {
      _speed = speed / toKph;
      _isValid = true;
    };

    /**
     * Set speed in meters per second.
     */
    void setMps(double speed)
    {
      _speed = speed;
      _isValid = true;
    };

    /**
     * Set speed in feet per minute.
     */
    void setFpm(double speed)
    {
      _speed = speed / toFpm;
      _isValid = true;
    };

    /**
     * Set speed in selected horizontal unit.
     */
    void setHorizontalValue(double speed)
    {
      setValueInUnit(speed, _horizontalUnit);
    };

    /**
     * Set speed in selected vertical unit.
     */
    void setVerticalValue(double speed)
    {
      setValueInUnit(speed, _verticalUnit);
    };

    /**
     * Set speed in selected wind unit.
     */
    void setWindValue(double speed)
    {
      setValueInUnit(speed, _windUnit);
    };

    /**
     * set the horizontal unit
     */
    static void setHorizontalUnit(speedUnit unit)
    {
      _horizontalUnit=unit;
    };

    /**
     * set the vertical unit
     */
    static void setVerticalUnit(speedUnit unit)
    {
      _verticalUnit=unit;
    };

    /**
     * set the wind unit
     */
    static void setWindUnit(speedUnit unit)
    {
      _windUnit=unit;
    };

    /**
     * get the horizontal unit
     */
    static speedUnit getHorizontalUnit()
    {
      return _horizontalUnit;
    };

    /**
     * get the vertical unit
     */
    static speedUnit getVerticalUnit()
    {
      return _verticalUnit;
    };

    /**
     * get the wind unit
     */
    static speedUnit getWindUnit()
    {
      return _windUnit;
    }

    /**
     * Returns a formatted string for the default vertical speed units.
     * The string includes the value and optionally the unit.
     * @param withUnit set to true (default) to have the returned string
     *    include the unit, false otherwise
     * @param prec set to the number of digits after the decimal point you
     *    want in the string
     */
    QString getVerticalText(bool withUnit=true, uint prec=1) const;

    /**
     * Returns a formatted string for the default horizontal speed unit.
     * The string includes the value and optionally the unit.
     * @param withUnit set to true (default) to have the returned string
     *    include the unit, false otherwise
     * @param prec set to the number of digits after the decimal point you
     *    want in the string
     */
    QString getHorizontalText(bool withUnit=true, uint prec=1) const;

    /**
     * Returns a formatted string for the default wind speed unit.
     * The string includes the value and optionally the unit.
     * @param withUnit set to true (default) to have the returned string
     *    include the unit, false otherwise
     * @param prec set to the number of digits after the decimal point you
     *    want in the string
     */
    QString getWindText(bool withUnit=true, uint prec=1) const;

    /**
     * @returns a string for the unit requested. This string only represents
     *    the unit, not the value.
     * @param unit the type of unit you want the string for.
     */
    static QString getUnitText(speedUnit unit);

    /**
     * @returns a string for the horizontal unit requested. This string only
     * represents the unit, not the value.
     */
    static QString getHorizontalUnitText()
    {
      return getUnitText( _horizontalUnit );
    };

    static QString getVerticalUnitText()
    {
      return getUnitText( _verticalUnit );
    };

    static QString getWindUnitText()
    {
      return getUnitText( _windUnit );
    };

    /**
     * Get the value for the vertical speed in the currently active unit.
     * @returns the current speed value
     */
    double getVerticalValue() const
    {
       return getValueInUnit(_verticalUnit);
    };

    /**
     * Get the value for the horizontal speed in the currently active unit.
     * @returns the current speed value
     */
    double getHorizontalValue() const
    {
      return getValueInUnit(_horizontalUnit);
    };

    /**
     * Get the value for the wind speed in the currently active unit.
     * @returns the current speed value
     */
    double getWindValue() const
    {
      return getValueInUnit(_windUnit);
    };

    /**
     * Get the value in the requested unit.
     * @returns the value converted to the requested unit
     * @param unit the unit in which to return the value
     */
    double getValueInUnit(speedUnit unit) const;

    /**
     * Set the value in the indicated unit.
     * @param speed the new speed to set
     * @param unit the unit in which to set the value
     */
    void setValueInUnit(double speed, speedUnit unit);

    /**
     * Sets the distance to be invalid
     */
    void setInvalid()
    {
        _isValid=false;
        _speed=0;
    };

    /**
     * Gets if the distance is valid
     */
    bool isValid() const
    {
        return _isValid;
    };

    /*Operators */
    /**
     * + operator for speed.
     */
    Speed operator + (const Speed& x) const;

    /**
     * - operator for speed.
     */
    Speed operator - (const Speed& x) const;

    /**
     * / operator for speed.
     */
    double operator / (const Speed& x) const;

    /**
     * * operator for speed.
     */
    double operator * (const Speed& x) const;

    /**
     * != operator for Speed
     */
    bool operator != (const Speed& x) const;

    /**
     * == operator for Speed
     */
    bool operator == (const Speed& x) const;

    /**
     * minus prefix operator for speed
     */
    Speed operator - () const;

protected:
    /**
     * This attribute contains the stored speed in meters per second.
     */
    double _speed;

    /**
     * value valid?
     */
    bool _isValid;

    /**
     * Contains the unit used for horizontal speeds
     */
    static speedUnit _horizontalUnit;

    /**
     * Contains the unit used for vertical speeds
     */
    static speedUnit _verticalUnit;

    /**
     * Contains the unit used for wind speeds
     */
    static speedUnit _windUnit;
};

/**
 * - operator for speed.
 */
Speed operator - (double left, const Speed& right);

/**
 * + operator for speed.
 */
Speed operator + (double left, const Speed& right);

/**
 * * operator for speed.
 */
Speed operator * (double left, const Speed& right);

#endif
