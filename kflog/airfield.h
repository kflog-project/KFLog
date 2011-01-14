/***********************************************************************
 **
 **   airfield.h
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * \class Airfield
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Class to handle different types of airfields.
 *
 * This class is used for handling of airfields. The object can be one of
 * Airport, MilAirport, CivMilAirport, Airfield, ClosedAirfield,
 * CivHeliport, MilHeliport, AmbHeliport, UltraLight, GliderSite
 *
 * \see BaseMapElement#objectType
 *
 * This class is derived from \ref SinglePoint
 *
 * \date 2000-2011
 *
 * $Id$
 *
 */

#ifndef AIRFIELD_H
#define AIRFIELD_H

#include <QList>
#include <QString>

#include "singlepoint.h"
#include "runway.h"

class Airfield : public SinglePoint
{
 public:

  /**
   * Creates a new Airfield-object.
   *
   * @param  name  The name of the airfield
   * @param  icao  The ICAO-name
   * @param  shortName  The abbreviation, used for the gps-logger
   * @param  typeId  The map element type identifier
   * @param  wgsPos The position as WGS84 datum
   * @param  pos  The position
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  comment An additional comment related to the airfield
   * @param  winch  "true", if winch launch is available
   * @param  towing "true", if aero towing is available
   * @param  landable "true", if airfield is landable
   */
  Airfield( const QString& name,
            const QString& icao,
            const QString& shortName,
            const BaseMapElement::objectType typeId,
            const WGSPoint& wgsPos,
            const QPoint& pos,
            const uint elevation,
            const QString& frequency,
            const QString comment = "",
            bool winch = false,
            bool towing = false,
            bool landable = true );

  /**
   * Destructor
   */
  virtual ~Airfield();

  /**
   * @return the frequency of the airport.
   */
  QString getFrequency() const
    {
      return frequency;
    };

  /**
   * @return ICAO name
   */
  QString getICAO() const
    {
      return icao;
    };

  /**
   * Returns the data of a runway.
   *
   * \param index The index number of the runway.
   *
   * \return A runway object, containing the data of the runway.
   */
  Runway* getRunway( int index=0 );

  /**
   * @return The number of available runways.
   */
  int getRunwayNumber()
  {
    return rwData.size();
  };

  /**
   * Adds a runway to the list of runways.
   *
   * \param The new runway to be added to the runway list.
   */
  void addRunway( Runway& runway )
  {
    rwData.append( runway );
  };

  /**
   * @return "true", if winch launching is available.
   */
  bool hasWinch() const
    {
      return winch;
    };

  /**
   * @return "true", if aero towing is available.
   */
  bool hasTowing() const
    {
      return towing;
    };

  /**
   * @return "true", if it is landable
   */
  bool isLandable() const
    {
      return landable;
    };

  /**
   * @return the comment text of the airfield
   */
  QString getComment() const
    {
      return comment;
    };

  /**
   * Return a short html-info-string about the airport, containing the
   * name, the alias, the elevation and the frequency as well as a small
   * icon of the airport type.
   *
   * Reimplemented from SinglePoint (@ref SinglePoint#getInfoString).
   * @return the info string
   */
  virtual QString getInfoString();

  /**
   * Draws the element into the given painter.
   *
   * \param targetP The painter to draw the element into.
   */
  virtual bool drawMapElement( QPainter* targetP );

  /**
   * Prints the element. Reimplemented from BaseMapElement.
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  virtual void printMapElement( QPainter* printP, bool isText );

 private:

   /**
    * The ICAO name
    */
   QString icao;

   /**
    * The frequency
    */
   QString frequency;

   /**
    * Comment related to the airfield.
    */
   QString comment;

  /**
   * Contains the available runways.
   */
  QList<Runway> rwData;

  /**
   * The launching-type. "true" if the site has a winch.
   */
  bool winch;

  /**
   * The launching-type. "true" if the site has aero tow.
   */
  bool towing;

  /**
   * Flag to indicate the landability of the airfield.
   */
  bool landable;
};

#endif
