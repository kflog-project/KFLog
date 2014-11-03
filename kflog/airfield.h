/***********************************************************************
 **
 **   airfield.h
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2014 by Axel Pauli
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

#include "runway.h"
#include "singlepoint.h"

class Airfield : public SinglePoint
{
 public:

  /**
   * Airfield default constructor
   */
  Airfield() :
    SinglePoint(),
    m_frequency(0.0),
    m_atis(0.0),
    m_winch(false),
    m_towing(false),
    m_rwShift(0),
    m_landable(true)
   {
   };

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
   * @param  country The country of the airfield as two letter code
   * @param  comment An additional comment related to the airfield
   * @param  winch  "true", if winch launch is available
   * @param  towing "true", if aero towing is available
   * @param  landable "true", if airfield is landable
   * @param  atis ATIS
   */
  Airfield( const QString& name,
            const QString& icao,
            const QString& shortName,
            const BaseMapElement::objectType typeId,
            const WGSPoint& wgsPos,
            const QPoint& pos,
            const float elevation,
            const float frequency,
            const QString& country = "",
            const QString comment = "",
            bool winch = false,
            bool towing = false,
            bool landable = true,
            const float atis = 0.0 );

  /**
   * Destructor
   */
  virtual ~Airfield();

  /**
   * @return the frequency of the airfield as String.
   */
  QString frequencyAsString() const
    {
      return (m_frequency > 0) ? QString("%1").arg(m_frequency, 0, 'f', 3) : QString("");
    };

  /**
   * @return the frequency of the airfield as float value.
   */
  float getFrequency() const
    {
      return m_frequency;
    };

  /**
   * @param value The frequency of the airfield as float value.
   */
  void setFrequency( const float value )
    {
      m_frequency = value;
    };

  /**
   * @return The ATIS frequency of the airfield.
   */
  float getAtis() const
    {
      return m_atis;
    };

  /**
   * @param The ATIS frequency of the airfield.
   */
  void setAtis( const float value )
    {
      m_atis = value;
    };

  /**
   * @return ICAO name
   */
  QString getICAO() const
    {
      return m_icao;
    };

  /**
   * @param value The ICAO name of the airfield
   */
  void setICAO( const QString& value )
    {
      m_icao = value;
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
    return m_rwList.size();
  };

  /**
   * @return The runway list.
   */
  QList<Runway>& getRunwayList()
  {
    return m_rwList;
  };

  /**
   * Adds a runway to the list of runways.
   *
   * \param The new runway to be added to the runway list.
   */
  void addRunway( Runway& runway )
  {
    m_rwList.append( runway );
  };

  /**
   * @return "true", if winch launching is available.
   */
  bool hasWinch() const
    {
      return m_winch;
    };

  /**
   * \param value The winch flag of the airfield
   */
  void setWinch( const bool value )
    {
      m_winch = value;
    };

  /**
   * @return "true", if aero towing is available.
   */
  bool hasTowing() const
    {
      return m_towing;
    };

  /**
   * \param value The towing flag of the airfield
   */
  void setTowing( const bool value )
    {
      m_towing = value;
    };

  /**
   * @return "true", if it is landable
   */
  bool isLandable() const
    {
      return m_landable;
    };

  /**
   * \param value The landing flag of the airfield
   */
  void setLandable( const bool value )
    {
      m_landable = value;
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

 protected:

  /**
   * Calculates the runway shift for the icon to be drawn.
   */
  void calculateRunwayShift()
  {
    // calculate the default runway shift in 1/10 degrees.
    m_rwShift = 90/10; // default direction is 90 degrees

    // We assume, that the first runway is always the main runway.
    if( m_rwList.size() > 0 )
      {
        Runway rw = m_rwList.first();

        // calculate the real runway shift in 1/10 degrees.
        if ( rw.m_heading.first <= 36 )
          {
            m_rwShift = (rw.m_heading.first >= 18 ? (rw.m_heading.first)-18 : rw.m_heading.first);
          }
      }
  };

 private:

   /**
    * The ICAO name
    */
   QString m_icao;

   /**
    * The frequency
    */
   float m_frequency;

   /**
    * The ATIS frequency
    */
   float m_atis;

   /**
   * Contains the available runways.
   */
  QList<Runway> m_rwList;

  /**
   * The launching-type. "true" if the site has a winch.
   */
  bool m_winch;

  /**
   * The launching-type. "true" if the site has aero tow.
   */
  bool m_towing;

  /**
   * Contains the shift of the runway during drawing.
   */
  unsigned short m_rwShift;

  /**
   * Flag to indicate the landability of the airfield.
   */
  bool m_landable;
};

#endif
