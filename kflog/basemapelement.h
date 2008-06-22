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

/**
 * @short Baseclass for all mapelements
 *
 * This is the baseclass for all mapelements. The class will be inherited
 * by all classes implementing the map-elements. The class provides several
 * virtual function for writing and drawing the elements. Additionaly,
 * the cass provides two enums for the element-type and the type of
 * elevation-values.
 *
 */

#ifndef BASEMAPELEMENT_H
#define BASEMAPELEMENT_H

#include "resource.h"

#include <qpainter.h>

#include "mapmatrix.h"
#include "mapconfig.h"

class BaseMapElement
{
public:
  /**
   * List of all accessable element-types:
   * @see #typeID
   */
  enum objectType {
    NotSelected = NOT_SELECTED,
    IntAirport = INT_AIRPORT, Airport = AIRPORT, MilAirport = MIL_AIRPORT, CivMilAirport = CIVMIL_AIRPORT,
    Airfield = AIRFIELD, ClosedAirfield = CLOSED_AIRFIELD, CivHeliport = CIV_HELIPORT,
    MilHeliport = MIL_HELIPORT, AmbHeliport = AMB_HELIPORT, Glidersite = GLIDERSITE, UltraLight = ULTRALIGHT,
    HangGlider = HANGGLIDER, Parachute = PARACHUTE, Balloon = BALLOON, Outlanding = OUTLANDING, Vor = VOR,
    VorDme = VORDME, VorTac = VORTAC, Ndb = NDB, CompPoint = COMPPOINT,
    AirA = AIR_A, AirB = AIR_B, AirC = AIR_C, AirD = AIR_D, AirElow = AIR_E_LOW, AirEhigh = AIR_E_HIGH,
    AirF = AIR_F, ControlC = CONTROL_C, ControlD = CONTROL_D, Danger = DANGER,
    LowFlight = LOW_FLIGHT, Restricted = RESTRICTED, Prohibited = PROHIBITED, Tmz = TMZ, SuSector = SU_SECTOR, Obstacle = OBSTACLE,
    LightObstacle = LIGHT_OBSTACLE, ObstacleGroup = OBSTACLE_GROUP, LightObstacleGroup = LIGHT_OBSTACLE_GROUP,
    Spot = SPOT, Isohypse = ISOHYPSE, Glacier = GLACIER, PackIce = PACK_ICE, Border = BORDER, City = CITY,
    Village = VILLAGE, Landmark = LANDMARK, Highway = HIGHWAY, Road = ROAD, Railway = RAILWAY,
    AerialRailway = AERIAL_CABLE, Lake = LAKE, River = RIVER, Canal = CANAL, Flight = FLIGHT, Task = FLIGHT_TASK,
    Trail = TRAIL, Railway_D = RAILWAY_D, Aerial_Cable = AERIAL_CABLE, River_T = RIVER_T, Lake_T = LAKE_T,
    Forest = FOREST, Turnpoint = TURNPOINT, Thermal = THERMAL,
    FlightGroup = FLIGHT_GROUP, FAIAreaLow500 = FAI_AREA_LOW, FAIAreaHigh500 = FAI_AREA_HIGH,
    EmptyPoint = EMPTY_POINT,  // new type for nothing to draw
    objectTypeSize /* leave this at the end */};

  /**
   * The three types of elevation-data used in the maps.
   */
  enum elevationType {NotSet, MSL, GND, FL, STD, UNLTD};

  /**
   * Creates a new (virtual) mapelement.
   * @param  name  The name of the element.
   * @param  typeID  The typeid of the element.
   */
  BaseMapElement(const QString& name = QString::null, unsigned int typeID = 0);

  /**
   * Destructor
   */
  virtual ~BaseMapElement();

  /**
   * virtual function for drawing the element into the given painter.
   *
   * The function must be implemented in the child-classes.
   * @param  targetP  The painter to draw the element into.
   * @param  maskP  The maskpainter for targetP
   */
  virtual void drawMapElement(QPainter* targetP, QPainter* maskP) = 0;

  /**
   * Virtual function for printing the element.
   *
   * The function must be implemented in the child-classes.
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  virtual void printMapElement(QPainter* printP, bool isText) = 0;

  /**
   * @return the name of the element.
   */
  virtual QString getName() const;
  /**
   * @return the typeID of the element.
   */
  virtual unsigned int getTypeID() const;
  /**
   * Initializes the static members of BaseMapelement.
   * @see glMapMatrix
   * @see glConfig
   */
  static void initMapElement(MapMatrix* matrix, MapConfig* config);

  /**
   * List of all accessable element-types.
   *
   * NOTE:  All changes shall be made in resource.h, because that file is shared with our
   * map-conferting tools.  Otherwise it could lead to mapfiles not loadable anymore.
   *
   * @see #typeID
   */

protected:
  /**
   * Proofes, if the object is in the drawing-area of the map.
   *
   * The function must be implemented in the child-classes.
   * @return "true"
   */
  virtual bool __isVisible() const;

  /**
   * The name of the mapelement.
   */
  QString name;

  /**
   * The type-id of the element. The value is stored as "unsigned int", so
   * switch will produce no warnings ..
   * @see #objectType
   */
  unsigned int typeID;

  /**
   * Static pointer to _globalMapMatrix
   * @see initMapElement
   */
  static MapMatrix* glMapMatrix;

  /**
   * Static pointer to _globalMapConfig
   * @see initMapElement
   */
  static MapConfig* glConfig;

};

#endif
