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
#include <wp.h>

/**
 * Mapelement used for small objects. The object can be one of:
 * ClosedAirfield, UltraLight, HangGlider, Parachute, Ballon, Village,
 * Oiltank, Factory, Castle, Church, Tower, HighwayEntry, RailwayBridge,
 * Station, Dam, Lock.
 * Consists only of a name and a position.
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
	   * Creates a new "SinglePoint". n is the name of the element, t is the
     * typeID, latPos and lonPos give the position of the element.
     */
    SinglePoint(QString n, QString alias, unsigned int t, QPoint pos,
        bool wP = false);
    /**
	   * Destructor, does nothing special.
	   */
    ~SinglePoint();
    /**
     * Draws the element into the given painter.
     */
    virtual void drawMapElement(QPainter* targetPainter, QPainter* maskPainter);
    /** */
    virtual void printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder);
    /** */
    virtual void printMapElement(QPainter* printPainter);
    /**
     * Returns the position of the element.
     */
    virtual QPoint getPosition() const;
    /**
     * Sets <TT>isWaypoint</TT> to <TT>isW</TT>.
     */
    virtual void setWaypoint(bool isW);
    /**
     * Returns <TT>true</TT>, if the element is a waypoint.
     */
    virtual bool isWayPoint() const;
    /**
     * Returns the gps-name of the element.
     */
    virtual QString getWPName() const;
    /** */
    virtual QPoint getMapPosition() const;
    /** */
    virtual QString getInfoString() const;

  protected:
    /** */
    virtual bool __isVisible() const;
    /** */
    QPoint position;
    /** <TT>true</TT>, if the element is used as a waypoint. */
    bool isWaypoint;
    /** The abbreviation used for the GPS-logger. */
    QString abbrev;
    /** */
    QString iconName;
    /** */
    QPoint curPos;
};

/*************************************************************************
 *
 * Die Ein- und Ausgabeoperatoren
 *
 *************************************************************************/
//QDataStream& operator<<(QDataStream& outStream, const SinglePoint& sPoint);
//QTextStream& operator<<(QTextStream& outStream, const SinglePoint& sPoint);
//QDataStream& operator>>(QDataStream& outStream, SinglePoint& sPoint);

#endif
