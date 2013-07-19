/***********************************************************************
**
**   airspace.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
 * AirF, Restricted, Danger, LowFlight ...
 *
 * Due to the cross pointer reference to the air region this class do not
 * allow copies and assignments of an existing instance.
 *
 * \date 2000-2013
 *
 */

#ifndef AIRSPACE_H
#define AIRSPACE_H

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
   * Creates a new Airspace object.
   *
   * \param name The name of the airspace
   * \param oType The object type identifier.
   * \param The projected coordinates of the airspace as polygon.
   * \param upper The upper altitude limit of the airspace
   * \param upperType The upper altitude reference
   * \param lower The lower altitude limit of the airspace
   * \param lowerType The lower altitude reference
   */
  Airspace( QString name, BaseMapElement::objectType oType, QPolygon pP,
            int upper, BaseMapElement::elevationType upperType,
            int lower, BaseMapElement::elevationType lowerType);

  /**
   * Tells the caller, if the airspace is drawable or not
   */
  bool isDrawable() const;

  /**
   * Returns true, if the passed WGS coordinate point lays inside the airspace
   * polygon.
   */
  bool isWgsPointInside( QPoint& point );

  /**
   * Returns true, if the passed projected coordinate point lays inside the
   * airspace polygon.
   */
  bool isProjectedPointInside( QPoint& point );

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
   * Returns the upper limit of the airspace.
   */
  unsigned int getUpperL() const
  {
    return (unsigned int) rint(uLimit.getMeters());
  };

  /**
   * Returns the lower limit of the airspace.
   */
  unsigned int getLowerL() const
  {
    return (unsigned int) rint(lLimit.getMeters());
  };

  /**
   * Returns the type of the upper limit (MSN, GND, FL)
   * @see BaseMapElement#elevationType
   * @see #uLimitType
   */
  BaseMapElement::elevationType getUpperT() const
  {
      return uLimitType;
  };

  /**
   * Returns the type of the lower limit (MSN, GND, FL)
   * @see BaseMapElement#elevationType
   * @see #lLimitType
   */
  BaseMapElement::elevationType getLowerT() const
  {
      return lLimitType;
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

private:
  /**
   * Contains the lower limit.
   * @see #getLowerL
   */
  Altitude lLimit;
  /**
   * Contains the type of the lower limit
   * @see #lLimit
   * @see #getLowerT
   */
  BaseMapElement::elevationType lLimitType;
  /**
   * Contains the upper limit.
   * @see #getUpperL
   */
  Altitude uLimit;
  /**
   * Contains the type of the upper limit
   * @see #uLimit
   * @see #getUpperT
   */
  BaseMapElement::elevationType uLimitType;

  /**
   * The airspace data as QPainterPath object in WGS84 coordinates.
   */
  QPainterPath m_airspaceRegion;
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
    qSort( begin(), end(), CompareAirspaces() );
  };
};

#endif
