/***********************************************************************
**
**   basemapelement.h
**
**   This file is part of KFLog2.
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

#ifndef BASEMAPELEMENT_H
#define BASEMAPELEMENT_H

#include <resource.h>
#include <wp.h>

#include <qpainter.h>

/**
 * Baseclass for all mapelements. The class will be inherited by all
 * classes implementing the map-elements. The clas provides several
 * virtual function for writing and drawing the elements. Additionaly,
 * the cass provides two enums for the element-type and the type of
 * elevation-values.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
    class BaseMapElement
{
  public:
    /**
     * Creates a new (virtual) mapelement.
     */
    BaseMapElement(const char* name = 0, unsigned int typeID = 0);
    /**
     * Destructor, does nothing.
     */
    virtual ~BaseMapElement();
    /**
     * Virtual function for drawing the element into the given painter. The
     * QRegion-pointer is used in those mapelements, which will create a
     * QRegion when drawn (e.g. AreaElement).
     * The function must be implemented in the child-classes.
     */
    virtual void drawMapElement(QPainter* targetPainter, QPainter* maskPainter);
    /** */
    virtual QRegion* drawRegion(QPainter* targetPainter, QPainter* maskPainter);
    /** */
    virtual void printMapElement(QPainter* printPainter, const double dX,
          const double dY, const int mapCenterLon, const double scale,
          const struct elementBorder mapBorder);
    /** */
    virtual void printMapElement(QPainter* printPainter);
    /**
     * Virtual function which returns the name of the object.
     * The name is no member of BaseMapElement, but is needed in several
     * child-classes. Therefor, the memberfunction "getName" is declared
     * here.
     */
    virtual QString getName() const;
    /**
     * Returns the typeID of the element.
     */
    virtual unsigned int getTypeID() const;
    /** */
    virtual void readConfig();
    /**
     * Returns the position of the map-object in the current map.
     * The position will be set to (-1000,-1000) during contruction.
     */
//    virtual QPoint getMapPosition() const;
    /**
     * List of all accessable element-types:
     * @see #typeID
     *
     * NotSelected      :  0,   ControlD           : 24,   MidRoad          : 48,
     * IntAirport       :  1,   AirElow            : 25,   SmallRoad        : 49,
     * Airport          :  2,   AirEhigh           : 26,   RoadBridge       : 50,
     * MilAirport       :  3,   AirF               : 27,   RoadTunnel       : 51,
     * CivMilAirport    :  4,   Restricted         : 28,   Railway          : 52,
     * Airfield         :  5,   Danger             : 29,   RailwayBridge    : 53,
     * ClosedAirfield   :  6,   LowFlight          : 30,   Station          : 54,
     * CivHeliport      :  7,   Obstacle           : 31,   AerialRailway    : 55,
     * MilHeliport      :  8,   LightObstacle      : 32,   Coast            : 56,
     * AmbHeliport      :  9,   ObstacleGroup      : 33,   BigLake          : 57,
     * Glidersite       : 10,   LightObstacleGroup : 34,   MidLake          : 58,
     * UltraLight       : 11,   CompPoint          : 35,   SmallLake        : 59,
     * HangGlider       : 12,   HugeCity           : 36,   BigRiver         : 60,
     * Parachute        : 13,   BigCity            : 37,   MidRiver         : 61,
     * Ballon           : 14,   MidCity            : 38,   SmallRiver       : 62,
     * Outlanding       : 15,   SmallCity          : 39,   Dam              : 63,
     * VOR              : 16,   Village            : 40,   Lock             : 64,
     * VORDME           : 17,   Oiltank            : 41,   Spot             : 65,
     * VORTAC           : 18,   Factory            : 42,   Pass             : 66,
     * NDB              : 19,   Castle             : 43,   Glacier          : 67;
     * AirC             : 20,   Church             : 44,   WayPoint         : 68,
     * AirCtemp         : 21,   Tower              : 45,   Flight           : 69,
     * AirD             : 22,   Highway            : 46,   Isohypse         : 70,
     * AirDtemp         : 23,   HighwayEntry       : 47,   Task             : 71
     *
     */
    enum objectType {NotSelected, IntAirport, Airport, MilAirport,
        CivMilAirport, Airfield, ClosedAirfield, CivHeliport, MilHeliport,
        AmbHeliport, Glidersite, UltraLight, HangGlider, Parachute, Ballon,
        Outlanding, VOR, VORDME, VORTAC, NDB, AirC, AirCtemp, AirD, AirDtemp,
        ControlD, AirElow, AirEhigh, AirF, Restricted, Danger, LowFlight,
        Obstacle, LightObstacle, ObstacleGroup, LightObstacleGroup,
        CompPoint, HugeCity, BigCity, MidCity, SmallCity, Village, Oiltank,
        Factory, Castle, Church, Tower, Highway, HighwayEntry, MidRoad,
        SmallRoad, RoadBridge, RoadTunnel, Railway, RailwayBridge, Station,
        AerialRailway, Coast, BigLake, MidLake, SmallLake, BigRiver,
        MidRiver, SmallRiver, Dam, Lock, Spot, Pass, Glacier, WayPoint,
        Flight, Isohypse, Task};
    /**
     * The three types of elevation-data used in the maps.
     */
    enum elevationType {NotSet, MSL, GND, FL};
    /** */

  protected:
    /**
     * Prooves, if the object is in the drawing-area of the map.
     */
    virtual bool __isVisible() const;
    /**
     * The name of the mapelement.
     */
    QString name;
    /**
     * The type-id of the element. The value is stored as "unsigned int", so
     * switch will produce no warnings ..
     *
     * @see #objectType
     */
    unsigned int typeID;
    /**
     * ist das hier nötig? reicht das nicht bei SinglePoint ???
     */
    QPoint curPos;
    /**
     * The index-number of the mapsection of the element.
     */
    unsigned int section;
};

#endif
