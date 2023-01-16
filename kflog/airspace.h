/***********************************************************************
**
**   airspace.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
**                   2009-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Airspace
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Class to handle airspaces.
 *
 * This class is used for the several airspaces. The object can be
 * one of: AirC, AirCtemp, AirD, AirDtemp, ControlD, AirElow, WaveWindow,
 * AirF, Restricted, Danger, SUA ...
 *
 * Due to the cross pointer reference to the air region this class do not
 * allow copies and assignments of an existing instance.
 *
 * \date 2000-2023
 *
 * \version 1.4
 *
 */

#pragma once

#include <algorithm>
#include <QDateTime>
#include <QPolygon>
#include <QPainter>
#include <QPainterPath>
#include <QRect>

#include "altitude.h"
#include "airspacewarningdistance.h"
#include "lineelement.h"
#include "wgspoint.h"

class Airspace : public LineElement
{
public:

  enum ConflictType { None, NearAbove, NearBelow, VeryNearAbove, VeryNearBelow, Inside };

  /**
   * ICAO airspace classes of openAIP.
   */
  enum icaoClass {
    AS_A=0,
    AS_B=1,
    AS_C=2,
    AS_D=3,
    AS_E=4,
    AS_F=5,
    AS_G=6,
    AS_SUA=8,
    AS_Unkown=255
  };

  /**
   * Kind of activity used by openAIP
   */
  enum activity {
    No=0, // No specific activity (default)
    Parachuting=1,
    Aerobatics=2,
    AeroclubAndArialWorkArea=3,
    UltraLightMachines=4, // (ULM) Activity
    HangGlidingAndParagliding=5
  };

  Airspace();

  /**
   * Creates a new Airspace object.
   *
   * \param name The name of the airspace
   * \param oType The object type identifier.
   * \param The projected coordinates of the airspace as polygon.
   * \param upper The upper altitude limit of the airspace
   * \param upperType The upper altitude reference
   * \param lower The lower altitude limit of the airspace
   * \param lowerType The lower altitude reference
   * \param identifier An airspace identifier
   * \param country The country as two letter code, where the airspace is located
   */
  Airspace( QString name,
            BaseMapElement::objectType oType,
            QPolygon pP,
            const float upper,
            const BaseMapElement::elevationType upperType,
            const float lower,
            const BaseMapElement::elevationType lowerType,
            const int icaoClass=AS_Unkown,
            QString country="",
            quint8 activity=0,
            bool byNotam=false );

  Airspace(const Airspace& a) = default;
  Airspace& operator=(const Airspace& a) = default;

  /**
   * Destructor
   */
  virtual ~Airspace()
  {
  }

  /**
   * Creates a new airspace object using the current set airspace data.
   */
  Airspace createAirspaceObject();

  /**
   * Tells the caller, if the airspace is drawable or not
   */
  bool isDrawable() const
  {
    return ( glConfig->isBorder(typeID) && isVisible() );
  };

  /**
   * Returns true, if the passed WGS84 coordinate point lays inside the airspace
   * polygon.
   */
  bool isWgsPointInside( const QPoint& point )
  {
    if( m_airspaceRegion.isEmpty() )
      {
        return false;
      }

    QPoint pp = glMapMatrix->wgsToMap( point );
    return m_airspaceRegion.contains( pp );
  };

  /**
   * Returns true, if the passed projected coordinate point lays inside the
   * airspace polygon.
   */
  bool isProjectedPointInside( const QPoint& point )
  {
    if( m_airspaceRegion.isEmpty() )
      {
        return false;
      }

    return m_airspaceRegion.contains( point );
  };

  /**
   * Draws the airspace into the given painter.
   * Return a pointer to the drawn region or 0.
   *
   * @param targetP The painter to draw the element into.
   *
   * @param viewRect The view bounding rectangle.
   */
  void drawRegion( QPainter* targetP, const QRect &viewRect );

  /**
   * Return a painter path to the mapped airspace region data.
   */
  QPainterPath createRegion();

  /**
   * Sets the upper limit of the airspace.
   */
  void setUpperL( const Altitude& alt )
  {
    m_uLimit = alt;
  };

  /**
   * Returns the upper limit of the airspace in meters.
   */
  unsigned int getUpperL() const
  {
    return (unsigned int) rint(m_uLimit.getMeters());
  };

  /**
   * Returns the upper limit of the airspace.
   */
  const Altitude& getUpperAltitude() const
  {
    return m_uLimit;
  };

  /**
   * Sets the lower limit of the airspace.
   */
  void setLowerL( const Altitude& alt )
  {
    m_lLimit = alt;
  };

  /**
   * Returns the lower limit of the airspace in meters.
   */
  unsigned int getLowerL() const
  {
    return (unsigned int) rint(m_lLimit.getMeters());
  };

  /**
   * Returns the lower limit of the airspace.
   */
  const Altitude& getLowerAltitude() const
  {
    return m_lLimit;
  };

  /**
   * Returns the type of the upper limit (MSL, GND, FL)
   * @see BaseMapElement#elevationType
   * @see #uLimitType
   */
  BaseMapElement::elevationType getUpperT() const
  {
      return m_uLimitType;
  };

  void setUpperT( const BaseMapElement::elevationType ut )
  {
    m_uLimitType = ut;
  };

  /**
   * Returns the type of the lower limit (MSL, GND, FL)
   * @see BaseMapElement#elevationType
   * @see #lLimitType
   */
  BaseMapElement::elevationType getLowerT() const
  {
      return m_lLimitType;
  };

  void setLowerT( const BaseMapElement::elevationType lt )
  {
    m_lLimitType = lt;
  };

  /**
   * Returns a html-text-string about the airspace containing the name,
   * the type and the borders.
   * @return the infostring
   */
  QString getInfoString(bool ExtendedHTMLFormat = true) const;

  /**
   * Returns a text representing the type of the airspace
   */
  static QString getTypeName (objectType);

  /**
   * Returns true if the given altitude conflicts with the airspace properties
   */
  ConflictType conflicts (const AltitudeCollection& alt,
                          const AirspaceWarningDistance& dist) const;

  /*
   * Compares two items, in this case, Airspaces.
   * The items are compared on their levels. Because kflog provides a view
   * where the user looks down on the map, the first airspace you'll see is the
   * one with the highest ceiling. So, an item with a higher ceiling is
   * bigger than an item with a lower ceiling. In case these are the same,
   * the item with the bigger floor will be the bigger item.
   *
   * The airspaces in the list are sorted in this way to make sure they are
   * stacked correctly. This has become important with the introduction of
   * transparent airspaces. By sorting the airspaces like this, the lower ones
   * will be drawn first, and the higher ones on top of them.
   */
  bool operator == (const Airspace& other) const;

  bool operator < (const Airspace& other) const;

  /**
   * Get icao airspace identifier.
   *
   * \return airspace identifier
   */
  quint8 getIcaoClass() const
  {
    return m_icaoClass;
  }

  /**
   * Set icao airspace identifier
   *
   * \param id airspace identifier
   */
  void setIcaoClass( int icaoClass)
  {
    m_icaoClass = icaoClass;
  }

  /**
   * Print out degug info.
   */
  void debug();

  /**
   * Kind of activity used by openAip.
   *
   * @return
   */
  quint8 getActivity() const
  {
    return m_activity;
  }

  /**
   * Kind of activity used by openAip.
   *
   * @param activity enumeration
   */
  void setActivity( quint8 activity )
  {
    m_activity = activity;
  }

  bool isByNotam() const
  {
    return m_byNotam;
  }

  void setByNotam( bool byNotam )
  {
    m_byNotam = byNotam;
  }

private:
  /**
   * Contains the lower limit.
   * @see #getLowerL
   */
  Altitude m_lLimit;

  /**
   * Contains the type of the lower limit
   * @see #lLimit
   * @see #getLowerT
   */
  BaseMapElement::elevationType m_lLimitType;

  /**
   * Contains the upper limit.
   * @see #getUpperL
   */
  Altitude m_uLimit;
  /**
   * Contains the type of the upper limit
   * @see #uLimit
   * @see #getUpperT
   */
  BaseMapElement::elevationType m_uLimitType;

  /**
   * The airspace data as QPainterPath object in WGS84 coordinates.
   */
  QPainterPath m_airspaceRegion;

  /**
   * ICAO identifier used by openAip.
   */
  quint8 m_icaoClass;

  /**
   * Kind of activity used by openAip.
   */
  quint8 m_activity;

  /**
   * Activation by NOTAM
   */
  bool m_byNotam;
};

/**
 * \struct CompareAirspaces
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Compare method for two \ref Airspace items.
 *
 * \see Airspace
 *
 * \date 2002-2010
 */
struct CompareAirspaces
{
  // The operator sorts the airspaces in the expected order
  bool operator()(const Airspace& as1, const Airspace& as2) const
  {
    int a1C = as1.getUpperL(), a2C = as2.getUpperL();

    if (a1C > a2C)
      {
        return false;
      }

    if (a1C < a2C)
      {
        return true;
      }

    // equal
    int a1F = as1.getLowerL();
    int a2F = as2.getLowerL();
    return (a1F < a2F);
  };
};

/**
 * \class SortableAirspaceList
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Specialized QList for Airspaces.
 *
 * \see Airspace
 *
 * Specialized QList for \ref Airspace elements. The sort member function
 * has been re-implemented to make it possible to sort items based on their
 * levels.
 *
 * \date 2002-2010
 */

class SortableAirspaceList : public QList<Airspace>
{
public:

  void sort ()
  {
    std::sort( begin(), end(), CompareAirspaces() );
  }
};
