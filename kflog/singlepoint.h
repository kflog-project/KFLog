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

#include <basemapelement.h>

/**
 * Mapelement used for small objects. The object can be one of:
 * UltraLight, HangGlider, Parachute, Ballon, Village
 * or Landmark. Consists only of a name and a position.
 *
 * @see BaseMapElement#objectType
 * @see Airport
 * @see ElevPoint
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
	   * @param  alias  An alias-name, used for the gps-logger
	   * @param  typeID  The typeid
	   * @param  pos  The position
	   * @param  wP  "true", if the element is a waypoint
     */
    SinglePoint(QString name, QString alias, unsigned int typeID, QPoint pos,
        bool wP = false);
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
     */
    virtual void printMapElement(QPainter* printP) const;
    /**
     * @return the position of the element.
     */
    virtual QPoint getPosition() const;
    /**
     * Sets <TT>isWaypoint</TT> to <TT>isW</TT>.
     */
    virtual void setWaypoint(bool isW);
    /**
     * @return <TT>true</TT>, if the element is a waypoint, otherwise
     * <TT>false</TT>.
     */
    virtual bool isWayPoint() const;
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

  protected:
    /**
     * Proofes, if the object is in the drawing-area of the map.
     *
     * @return "true", if the element is in the drawing-area of the map.
     */
    virtual bool __isVisible() const;
    /**
     * The lat/lon-position of the element
     */
    QPoint position;
    /** <TT>true</TT>, if the element is used as a waypoint. */
    bool isWaypoint;
    /**
     * The abbreviation used for the GPS-logger.
     */
    QString abbrev;
    /**
     * The current draw-position of the element.
     */
    QPoint curPos;
};

#endif
