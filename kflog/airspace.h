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

#include <lineelement.h>

/**
 * This class is used for the several airspaces. The object can be
 * one of: AirC, AirCtemp, AirD, AirDtemp, ControlD, AirElow, AirEhigh,
 *  AirF, Restricted, Danger, LowFlight
 *
 * @see BaseMapElement#objectType
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
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
    Airspace(QString n, unsigned int t, QPointArray pA);
    /**
     * Destructor, does nothing special.
     */
    ~Airspace();
    /**
     * Draws the element into the given painter.
     */
    virtual QRegion* drawRegion(QPainter* targetPainter, QPainter* maskPainter);
    /** */
    virtual void printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder);
    /** */
    virtual void printMapElement(QPainter* printPainter);
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
    /** */
    void setValues(unsigned int upper = 0, unsigned int upperType = 0,
            unsigned int lower = 0, unsigned int lowerType = 0);
    /** */
    QString getInfoString();

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

/*************************************************************************
 *
 * Die Ein- und Ausgabeoperatoren
 *
 *************************************************************************/
//QDataStream& operator<<(QDataStream& outStream, const Airspace& airspace);
//QTextStream& operator<<(QTextStream& outStream, const Airspace& airspace);
//QDataStream& operator>>(QDataStream& outStream, Airspace& airspace);

#endif
