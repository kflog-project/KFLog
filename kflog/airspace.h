/***********************************************************************
**
**   airspace.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef AIRSPACE_H
#define AIRSPACE_H

#include "lineelement.h"

/**
 * This class is used for the several airspaces. The object can be
 * one of: AirC, AirCtemp, AirD, AirDtemp, ControlD, AirElow, AirEhigh,
 * AirF, Restricted, Danger, LowFlight
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 * @see BaseMapElement#objectType
 */
class Airspace : public LineElement
{
  public:
    /**
     * Creates a new Airspace-object. n is the name, t the typeID. length
     * is the number of coordinates. upper and upperType give the upper limit
     * of the airspace and the type of value (MSL, GND, FL); lower and
     * lowerType give the value for the lower limit.
     */
    Airspace(QString n, unsigned int t, QPointArray pA,
            unsigned int upper, unsigned int upperType,
            unsigned int lower, unsigned int lowerType);
    /**
     * Destructor, does nothing special.
     */
    ~Airspace();
    /**
     * Draws the airspace into the given painter.
     * Return a pointer to the drawn region.
     * @param  targetP  The painter to draw the element into.
     * @param  maskP  The maskpainter for targetP
     */
    QRegion* drawRegion(QPainter* targetP, QPainter* maskP);
    /**
     * Returns the upper limit of the airspace.
     */
    unsigned int getUpperL() const;
    /**
     * Returns the lower limit of the airspace.
     */
    unsigned int getLowerL() const;
    /**
     * Returns the type of the upper limit (MSN, GND, FL)
     * @see BaseMapElement#elevationType
     * @see #uLimitType
     */
    unsigned int getUpperT() const;
    /**
     * Returns the type of the lower limit (MSN, GND, FL)
     * @see BaseMapElement#elevationType
     * @see #lLimitType
     */
    unsigned int getLowerT() const;
    /**
     * The three types of elevation-data used in the maps.
     */
    enum LimitType {NotSet, MSL, GND, FL};
    /**
     * Returns a html-text-string about the airspace containing the name,
     * the type and the borders.
     * @return the infostring
     */
    QString getInfoString() const;

  private:
    /**
     * Contains the lower limit.
     * @see #getLowerL
     */
    unsigned int lLimit;
    /**
     * Contains the type of the lower limit
     * @see #lLimit
     * @see #getLowerT
     */
    unsigned int lLimitType;
    /**
     * Contains the upper limit.
     * @see #getUpperL
     */
    unsigned int uLimit;
    /**
     * Contains the type of the upper limit
     * @see #uLimit
     * @see #getUpperT
     */
    unsigned int uLimitType;
};

#endif
