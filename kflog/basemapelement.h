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

#include <mapmatrix.h>
#include <mapconfig.h>

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
    /**
     * List of all accessable element-types:
     * @see #typeID
     *
     * Typ                 ID
     * ------------------------
     * NotSelected          0
     *
     * IntAirport           1    Airport              2
     * MilAirport           3    CivMilAirport        4
     * Airfield             5    ClosedAirfield       6
     * CivHeliport          7    MilHeliport          8
     * AmbHeliport          9    Glidersite          10
     * UltraLight          11    HangGlider          12
     * Parachute           13    Ballon              14
     * Outlanding          15
     *
     * VOR                 16    VORDME              17
     * VORTAC              18    NDB                 19
     * CompPoint           20
     *
     * AirA                21    AirB                22
     * AirC                23    AirD                24
     * AirElow             25    AirEhigh            26
     * AirF                27    ControlC            28
     * ControlD            29    Danger              30
     * LowFlight           31    Restricted          32
     * TMZ                 33
     *
     * Obstacle            34    LightObstacle       35
     * ObstacleGroup       36    LightObstacleGroup  37
     * Spot                38
     *
     * Isohypse            39    Glacier             40
     * Border              41
     *
     * City                42    Village             43
     *
     * Landmark            44
     *
     * Highway             45    Road                46
     *
     * Railway             47    AerialRailway       48
     *
     * Lake                49    River               50
     * Canal               51
     *
     * Flight              52    Task                53
     */
    enum objectType {NotSelected,
        IntAirport, Airport, MilAirport, CivMilAirport, Airfield,
        ClosedAirfield, CivHeliport, MilHeliport, AmbHeliport, Glidersite,
        UltraLight, HangGlider, Parachute, Ballon, Outlanding,
        VOR, VORDME, VORTAC, NDB, CompPoint,
        AirA, AirB, AirC, AirD, AirElow, AirEhigh, AirF, ControlC,
        ControlD, Danger, LowFlight, Restricted, TMZ,
        Obstacle, LightObstacle, ObstacleGroup, LightObstacleGroup, Spot,
        Isohypse, Glacier, Border,
        City, Village,
        Landmark,
        Highway, Road,
        Railway, AerialRailway,
        Lake, River, Canal,
        Flight, Task};
    /**
     * The three types of elevation-data used in the maps.
     */
    enum elevationType {NotSet, MSL, GND, FL};
    /** */
    static void initMapElement(MapMatrix* matrix, MapConfig* config);

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
    /** */
    static MapMatrix* glMapMatrix;
    /** */
    static MapConfig* glConfig;
};

#endif
