/***********************************************************************
**
**   singlepoint.h
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

#ifndef SINGLEPOINT_H
#define SINGLEPOINT_H

#include "basemapelement.h"

/**
 * Mapelement used for small objects. The object can be one of:
 * UltraLight, HangGlider, Parachute, Balloon, PopulationPlace
 * or Landmark. Consists only of a name and a position.
 *
 * @see BaseMapElement#objectType
 * @see Airport
 * @see GliderSite
 * @see RadioPoint
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class SinglePoint : public BaseMapElement
{
  public:
    /**
     * Creates a new "SinglePoint".
     *
     * @param  name  The name
     * @param  gps  An alias-name, used for the gps-logger
     * @param  typeID  The typeid
     * @param  pos  The projected position
     * @param  wgsPos  The original WGS-position
     * @param  elevation The elevation of the point when avaible
     * @param  lmtyp Additional field (eg. for the population index for cities or the lm_typ)
     */
    SinglePoint(const QString& name, const QString& gps, unsigned int typeID,
        WGSPoint wgsPos, QPoint pos, unsigned int elevation = 0,
        unsigned int lmtyp = 0);
    /**
     * Destructor
     */
    ~SinglePoint();
    /**
     * Draws the element into the given painter. Reimplemented from
     * BaseMapElement.
     *
     * @param  targetP  The painter to draw the element into.
     * @param  maskP  The maskpainter for targetP
     */
    virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
    /**
     * Prints the element. Reimplemented from BaseMapElement.
     *
     * @param  printP  The painter to draw the element into.
     *
     * @param  isText  Shows, if the text of some mapelements should
     *                 be printed.
     */
    virtual void printMapElement(QPainter* printP, bool isText);
    /**
     * @return the projected position of the element.
     */
    virtual QPoint getPosition() const;
    /**
     * @return the WGSposition of the element. (normales Lat/Lon System)
     */
    virtual WGSPoint getWGSPosition() const;
    /**
     * @return the gps-name of the element.
     */
    virtual QString getWPName() const;
    /**
     * @return the position in the current map.
     */
    virtual QPoint getMapPosition() const;
    /**
     * Used to return a little info-string about the element.
     * Should be reimplemented in subclasses.
     */
    virtual QString getInfoString() const;
    /**
     * @return the elevation of the element.
     */
    unsigned int getElevation() const;

  protected:
    /**
     * Proofes, if the object is in the drawing-area of the map.
     *
     * @return "true", if the element is in the drawing-area of the map.
     */
    virtual bool __isVisible() const;
    /**
     */
    WGSPoint wgsPosition;
    /**
     * The projected lat/lon-position of the element
     */
    QPoint position;
    /**
     * The abbreviation used for the GPS-logger.
     */
    QString gpsName;
    /**
     * The current draw-position of the element.
     */
    QPoint curPos;
    /**
     * The elevation.
     */
    unsigned int elevation;
    /**
     * Additional field
     */
    unsigned int lm_typ;
};

#endif
