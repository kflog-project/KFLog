/***********************************************************************
**
**   airport.h
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

#ifndef AIRPORT_H
#define AIRPORT_H

#include <radiopoint.h>
#include <runway.h>

/**
  * This class is used for handling airports. The object can be one of
  * Airport, MilAirport, CivMilAirport, Airfield, ClosedAirfield,
  * CivHeliport, MilHeliport, AmbHeliport.
  *
  * @see BaseMapElement#objectType
  *
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  */
class Airport : public RadioPoint
{
  public:
    /**
     * Creates a new Airport-object. n is the name of the airport, a is the
     * alias, t is the typeID, latPos and lonPos give the position, elev is
     * the elevation and f the frequency.
     */
    Airport(QString n, QString a, QString abbr, unsigned int t,
        QPoint pos, unsigned int e, const char* f, bool v, bool wP);
    /**
     * Destructor, does nothing special.
     */
    ~Airport();
    /**
     * Returns the frequency of the glidersite.
     */
    QString getFrequency() const;
    /**
     * Returns a runway-struct, containing the data of the given runway.
     */
    struct runway getRunway(int index = 0) const;
    /**
     * Returns the number of runways.
     */
    unsigned int getRunwayNumber() const;
    /** */
    virtual void printMapElement(QPainter* printPainter) const;
    /** */
    unsigned int getElevation() const;
    /** */
    virtual QString getInfoString() const;
    /**
     * Used for defining the surface of a runway.
     */
    enum Surface {NotSet, Grass, Asphalt, Concrete};

  private:
    /**
     * The elevation of the airport.
     */
    unsigned int elevation;
    /** */
    bool vdf;
    /**
     * Contains the runway-data.
     */
    struct runway* rwData;
    /**
     * Contains the number of runways.
     */
    unsigned int rwNum;
};

#endif
