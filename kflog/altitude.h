/***************************************************************************
                          altitude.h  -  general altitude representation
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers
                               2010-2023 by Axel Pauli

    This file is part of KFLog.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include <cmath>

#include "distance.h"

/**
 * \class Altitude
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Class to calculate altitudes in different units.
 *
 * This class handles different altitude units and arithmetics.
 *
 * \date 2002-2023
 */

class Altitude : public Distance
{
public:
    /*
     * Supported altitude units
     */
    enum altitudeUnit{ meters=0, feet=1, flightlevel=2 };

public:

    Altitude();
    /**
     * Constructor
     *
     * Initializes the object to meters.
     */
    Altitude(int meter);

    /**
     * Constructor
     *
     * Initializes the object to meters.
     */
    Altitude(double meter);

    /**
     * copy constructor
     */
    Altitude(const Altitude&);

    /**
     * copy constructor
     */
    Altitude(const Distance&);

    /**
     * copy assignment operator.
     */
    Altitude& operator=( const Altitude& a );

    /**
     * destuctor
     */
    virtual ~Altitude();

    /**
     * Sets the unit for altitudes. This unit is used to return the correct string
     * in @ref getText.
     */
    static void setUnit(altitudeUnit unit)
    {
      _altitudeUnit=unit;
    };

    /**
     * returns the current altitude unit
     */
    static altitudeUnit getUnit()
    {
        return _altitudeUnit;
    };

    /**
     * Represent an altitude as a string.
     *
     * @return A string containing the altitude in the set unit (see @ref setUnit).
     * The string that identifies the unit used is added if withUnit is true.
     *
     * example:
     * <pre>
     *    getText(1,true,1)
     * </pre>
     * with the altitude unit set to feet would return "3.2 ft".
     *
     * @param meter The altitude expressed in meters
     * @param withUnit determines if the unit-string is included in the output
     * @param precision number of digits after the decimal separator
     */
    static QString getText( double meter, bool withUnit, int precision=1 );

    /**
     * Basically the same as @ref getText, but returns the internally stored altitude.
     */
    QString getText( bool withUnit, int precision=1 ) const;

    /**
     * Get current unit as text
     */
    static QString getUnitText();

    /**
     * Converts a distance from the current units to meters.
     */
    static double convertToMeters(double dist);

    /**
     * Get altitude as flight level based on standard pressure 1013.25hPa
     */
    double getFL() const
    {
      return rint( Distance::getFeet() / 100.0 );
    };

    /**
     * implements == operator for altitude
     */
    bool operator == (const Altitude& x) const
    {
        return _dist == x.getMeters();
    };

    /**
     * implements < operator for altitude
     */
    bool operator < (const Altitude& x) const
    {
        return _dist < x.getMeters();
    };

    /**
     * implements > operator for altitude
     */
    bool operator > (const Altitude& x) const
    {
        return _dist > x.getMeters();
    };

    /**
     * implements >= operator for altitude
     */
    bool operator >= (const Altitude& x) const
    {
        return _dist >= x.getMeters();
    };

    /**
     * implements <= operator for altitude
     */
    bool operator <= (const Altitude& x) const
    {
        return _dist <= x.getMeters();
    };

    /**
     * implements != operator for altitude
     */
    bool operator != (const Altitude& x) const
    {
        return _dist != x.getMeters();
    };

    /**
     * implements minus altitude
     */
    Altitude operator - (const Altitude& x) const
    {
        return Altitude( _dist - x.getMeters() );
    };

    /**
     * implements minus assignment altitude
     */
    Altitude& operator -= (const Altitude& x)
    {
         _dist -= x.getMeters();
         return *this;
    };

    /**
     * implements plus altitude
     */
    Altitude operator + (const Altitude& x) const
    {
        return Altitude( _dist + x.getMeters() );
    };

    /**
     * implements plus assignment altitude
     */
    Altitude& operator += (const Altitude& x)
    {
         _dist += x.getMeters();
         return *this;
    };

    /**
     * implements setting std altitude by given MSL altitude + QNH
     */
    void setStdAltitude(const Altitude& MSLAltitude, int QNH);

protected:

    static altitudeUnit _altitudeUnit;
};


/**
 * \struct AltitudeCollection
 *
 * @short Collection of the different expressions for the current altitude
 *
 * @author André Somers
 *
 * This structure contains different representations for the current altitude.
 * There are different ways to express the current altitude, and there are
 * some uncertainties too. All that data can be stored in this structure, so it
 * can be nicely passed as a single parameter.
 */
struct AltitudeCollection
{
    /**
     * The altitude according to the GPS given in MSL.
     */
    Altitude gpsAltitude;

    /**
     * The standard pressure altitude derived from the GPS altitude.
     * Given in STD.
     */
    Altitude stdAltitude;

    /**
     * The pressure altitude given in MSL.
     */
    Altitude pressureAltitude;

    /**
     * Average altitude above terrain (GND). Based on pressure altitude.
     * Because our terrain data does not return the exact terrain height for a given point, but
     * only a minimum and a maximum, we return the average as the gndAltitude.
     */
    Altitude gndAltitude;

    /**
     * The estimation for the error for the altitude we have above the terrain. This only includes
     * the error caused by the way the gndAltitude is calculated, not by any GPS errors.
     *
     * @see gndAltitude, @see gpsAltitudeError
     */
    Distance gndAltitudeError;
};

