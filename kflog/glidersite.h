/***********************************************************************
**
**   glidersite.h
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

#ifndef GLIDERSITE_H
#define GLIDERSITE_H

#include <radiopoint.h>
#include <runway.h>

/**
 * This class provides handling the glider-sites. This class inherites
 * "ElevPoint".
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class GliderSite : public RadioPoint
{
  public:
    /**
     * Creates a new GliderSite-object. n is the name of the site,
     * latPos and lonPos give the position, elev is the elevation and f the
	   * frequency, winch indicates the type of launching available.
     */
    GliderSite(QString n, QString abbr, QPoint pos, unsigned int elev,
        const char* f, bool w, bool wP);
    /**
     * Destructor, does nothing special.
     */
    ~GliderSite();
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
    /**
     * true, if only winch launching is available.
     */
    bool isWinch() const;
    /** */
    virtual void printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder);
    /** */
    virtual void printMapElement(QPainter* printPainter);
    /**
     * Returns the elevation of the element.
     */
    unsigned int getElevation() const;

  private:
    /**
     * Contains the elevation.
     */
    unsigned int elevation;
    /**
     * The launching-type. "true" if the site only has a winch, "false",
     * if aero tow is also available.
     */
    bool winch;
    /** Contains the runway-data. */
    struct runway* rwData;
    /** Contains the number of runways. */
    unsigned int rwNum;
};

/*************************************************************************
 *
 * Die Ein- und Ausgabeoperatoren
 *
 *************************************************************************/
//QDataStream& operator<<(QDataStream& outStream, const GliderSite& site);
//QTextStream& operator<<(QTextStream& outStream, const GliderSite& site);
//QDataStream& operator>>(QDataStream& outStream, GliderSite& site);

#endif
