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

#include "radiopoint.h"
#include "runway.h"

/**
  * This class is used for handling airports. The object can be one of
  * Airport, MilAirport, CivMilAirport, Airfield, ClosedAirfield,
  * CivHeliport, MilHeliport, AmbHeliport.
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  * @see BaseMapElement#objectType
  */
class Airport : public RadioPoint
{
  public:
    /**
     * Creates a new Airport-object.
     * @param  name  The name
     * @param  icao  The icao-name
     * @param  abbr  The abbreviation, used for the gps-logger
     * @param  typeID  The typeid
     * @param  pos  The position
     * @param  elevation  The elevation
     * @param  frequency  The frequency
     * @param  vdf  "true",
     */
    Airport(QString name, QString icao, QString abbr, unsigned int typeID,
        WGSPoint wgsPos, QPoint pos, unsigned int elevation,
        const char* frequency, bool vdf);
    /**
     * Destructor
     */
    ~Airport();
    /**
     * @return the frequency of the airport.
     */
    QString getFrequency() const;
    /**
     * @return a runway-struct, containing the data of the given runway.
     */
    runway getRunway(int index = 0) const;
    /**
     * @return the number of runways.
     */
    unsigned int getRunwayNumber() const;
    /**
     * Prints the element. Reimplemented from BaseMapElement.
     * @param  printP  The painter to draw the element into.
     *
     * @param  isText  Shows, if the text of some mapelements should
     *                 be printed.
     */
    virtual void printMapElement(QPainter* printP, bool isText) const;
    /**
     * Return a short html-info-string about the airport, containg the
     * name, the alias, the elevation and the frequency as well as a small
     * icon of the airporttype.
     *
     * Reimplemented from SinglePoint (@ref SinglePoint#getInfoString).
     * @return the infostring
     */
    virtual QString getInfoString() const;
    /**
     * Used to define the surface of a runway.
     */
    enum SurfaceType {Unknown = 0, Grass = 1, Asphalt = 2, Concrete = 3};

  private:
    /**
     */
    bool vdf;
    /**
     * Contains the runway-data.
     */
    runway* rwData;
    /**
     * Contains the number of runways.
     */
    unsigned int rwNum;
};

#endif
