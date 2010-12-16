/***********************************************************************
**
**   basemapelement.h
**
**   This file is part of KFLog4
**
************************************************************************
**
**   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class BaseMapElement
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Base class for all map elements
 *
 * This is the base class for all map elements. The class will be inherited
 * by all classes implementing the map-elements. The class provides several
 * virtual function for writing and drawing the elements. Additionally,
 * the class provides two enumerations for the element-type and the type of
 * elevation-values.
 *
 * \date 2000-2010
 */

#ifndef BASE_MAP_ELEMENT_H
#define BASE_MAP_ELEMENT_H

#include "resource.h"

#include <QPainter>
#include <QString>
#include <QHash>
#include <QStringList>

#include "mapmatrix.h"
#include "mapconfig.h"

class BaseMapElement
{
public:
  /**
   * List of all accessible element-types:
   * @see #typeID
   */
  enum objectType {
    NotSelected = NOT_SELECTED,
    IntAirport = INT_AIRPORT, Airport = AIRPORT, MilAirport = MIL_AIRPORT, CivMilAirport = CIVMIL_AIRPORT,
    Airfield = AIRFIELD, ClosedAirfield = CLOSED_AIRFIELD, CivHeliport = CIV_HELIPORT,
    MilHeliport = MIL_HELIPORT, AmbHeliport = AMB_HELIPORT, Glidersite = GLIDERSITE, UltraLight = ULTRALIGHT,
    HangGlider = HANGGLIDER, Parachute = PARACHUTE, Balloon = BALLOON, Outlanding = OUTLANDING, Vor = VOR,
    VorDme = VORDME, VorTac = VORTAC, Ndb = NDB, CompPoint = COMPPOINT,
    AirA = AIR_A, AirB = AIR_B, AirC = AIR_C, AirD = AIR_D, AirElow = AIR_E_LOW, AirEhigh = AIR_E_HIGH, WaveWindow = WAVE_WINDOW,
    AirF = AIR_F, ControlC = CONTROL_C, ControlD = CONTROL_D, Danger = DANGER,
    LowFlight = LOW_FLIGHT, Restricted = RESTRICTED, Prohibited = PROHIBITED, Tmz = TMZ, GliderSector = GLIDER_SECTOR, Obstacle = OBSTACLE,
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
   * The five types of elevation-data used in the maps.
   */
  enum elevationType { NotSet, MSL, GND, FL, STD, UNLTD };

  /**
   * Creates a new (virtual) map element.
   *
   * @param  name  The name of the element.
   * @param  typeID  The type id of the element.
   * @param  secID The number of the map segment.
   */
  BaseMapElement( const QString& name,
                  const objectType typeID = NotSelected,
                  const unsigned short secID=0 );

  /**
   * Destructor
   */
  virtual ~BaseMapElement();

  /**
   * virtual function for drawing the element into the given painter.
   *
   * The function must be implemented in the child-classes.
   * @param  targetP  The painter to draw the element into.
   * @param  maskP    The maskpainter for targetP
   * @return true, if element was drawn otherwise false.
   */
  virtual bool drawMapElement(QPainter* targetP, QPainter* maskP) = 0;

  /**
   * Virtual function for printing the element.
   *
   * The function must be implemented in the child-classes.
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some map elements should be printed.
   */
  virtual void printMapElement(QPainter* printP, bool isText);

  /**
   * @return the name of the element.
   */
  virtual const QString getName() const
  {
    return name;
  };

  /**
   * @return the object type of the element.
   */
  virtual objectType getObjectType() const
  {
    return typeID;
  };

  /**
   * Used to return a info string about the element.
   * Should be reimplemented in subclasses.
   */
  virtual QString getInfoString() const
    {
      return QString( "" );
    };

  /**
   * Initializes the static members of BaseMapelement.
   * @see glMapMatrix
   * @see glConfig
   */
  static void initMapElement(MapMatrix* matrix, MapConfig* config);

  /**
   * Get translation string for BaseMapelement object type.
   */
  static QString item2Text( const int objectType, QString defaultValue=QString("") );

  /**
   * Get BaseMapelement objectType for translation string.
   */
  static int text2Item( const QString& text );

  /**
   * Get sorted translations
   */
  static QStringList& getSortedTranslationList();

  /**
   * Write property of MapSegment.
   */
  virtual void setMapSegment( const unsigned short _newVal )
    {
      MapSegment = _newVal;
    };

  /**
   * Read property of MapSegment.
   */
  virtual unsigned short getMapSegment() const
    {
      return MapSegment;
    };

  /**
   * Compare two map elements by their names
   */
  bool operator < (const BaseMapElement& other) const
    {
      return getName() < other.getName();
    };

  /**
   * Proofs, if the object is in the drawing area of the map.
   *
   * The function must be implemented in the derived classes.
   * @return "true/false"
   */
  virtual bool isVisible() { return true; };

protected:

  /**
   * The name of the map element.
   */
  QString name;

  /**
   * The type-id of the element. The value is stored as "unsigned int", so
   * switch will produce no warnings ..
   * @see #objectType
   */
  objectType typeID;

  /**
   * Static pointer to object translation relations
   */
  static QHash<int, QString> objectTranslations;
  static QStringList sortedTranslations;

  /**
   * Static method for loading of object translations
   */
  static void loadTranslations();

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

  /**
   * Holds the id of the map segment this map element belongs to.
   * Storing this will enable unloading elements that are no longer needed.
   */
  unsigned short MapSegment;
};

#endif
