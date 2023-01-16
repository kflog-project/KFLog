/***********************************************************************
 **
 **   airfield.h
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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
 * CivHeliport, MilHeliport, AmbHeliport, UltraLight, Gliderfield
 *
 * \see BaseMapElement#objectType
 *
 * This class is derived from \ref SinglePoint
 *
 * \date 2000-2023
 *
 */

#pragma once

#include <QList>
#include <QMutex>
#include <QString>

#include "Frequency.h"
#include "runway.h"
#include "singlepoint.h"

class Airfield : public SinglePoint
{
 public:

  /**
   * Airfield default constructor
   */
  Airfield();

  /**
   * Creates a new Airfield-object.
   *
   * @param  name  The name of the airfield
   * @param  icao  The ICAO-name
   * @param  shortName  The abbreviation, used for the gps-logger
   * @param  typeId  The map element type identifier
   * @param  wgsPos The position as WGS84 datum
   * @param  pos  The position
   * @param  rwList A list with runway objects
   * @param  elevation  The elevation
   * @param  frequencyList A list with the frequency objects
   * @param  country The country of the airfield as two letter code
   * @param  comment An additional comment related to the airfield
   * @param  hasWinch "true", if winch launch is available
   * @param  hasTowing "true", if aero towing is available
   * @param  isPPR "true", if airfield is PPR
   * @param  isPrivate "true", if airfield is private
   * @param  hasSkyDiving "true", if airfield has sky diving
   * @param  isLandable "true", if airfield is landable
   */
  Airfield( const QString& name,
            const QString& icao,
            const QString& shortName,
            const BaseMapElement::objectType typeId,
            const WGSPoint& wgsPos,
            const QPoint& pos,
            const float elevation,
            const QList<Runway>& rwyList,
            const QList<Frequency> frequencyList,
            const QString country,
            const QString comment,
            bool hasWinch,
            bool hasTowing,
            bool isPPR,
            bool isPrivate,
            bool hasSkyDiving,
            bool isLandable,
            const float atis = 0.0 );

  /**
   * Destructor
   */
  virtual ~Airfield();

  /**
   * @return The frequency as String.
   */
  QString frequencyAsString( const float frequency ) const
    {
      return (frequency > 0) ? QString("%1").arg(frequency, 0, 'f', 3) : QString("");
    };

  /**
   * @return The frequency list of the airfield.
   */
  QList<Frequency>& getFrequencyList()
    {
      return m_frequencyList;
    };

  /**
   * @param freq The frequency and its type of the airfield.
   */
  void addFrequency( Frequency freqencyAndType)
    {
      m_frequencyList.append( freqencyAndType );
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
   * @return The runway list, containing the data of all runways.
   */
  QList<Runway>& getRunwayList()
    {
      return m_rwyList;
    };

  /**
   * Adds a runway object to the runway list.
   *
   * @param value The runway object, containing the data of the runway.
   */
   void addRunway( const Runway& value )
     {
       m_rwyList.append( value );
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

  bool isPPR() const
  {
    return m_ppr;
  }

  void setPPR( bool attribute )
  {
    m_ppr = attribute;
  }

  bool isPrivate() const
  {
    return m_private;
  }

  void setPrivate( bool attribute )
  {
    m_private = attribute;
  }

  bool hasSkyDiving() const
  {
    return m_skyDiving;
  }

  void setSkyDiving( bool attribute )
  {
    m_skyDiving = attribute;
  }

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
   */
  virtual bool drawMapElement( QPainter* targetP );

  quint16 getRwShift() const
  {
    return m_rwShift;
  }

  /**
   * Prints the element. Reimplemented from BaseMapElement.
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  virtual void printMapElement( QPainter* printP, bool isText );

  void setRwShift( quint16 mRwShift )
  {
    m_rwShift = mRwShift;
  }

  /**
   * Calculates the runway shift for the icon to be drawn.
   */
  void calculateRunwayShift()
    {
      m_rwShift = Runway::calculateRunwayShift( m_rwyList );
    }

 protected:

  /**
  * The ICAO name
  */
  QString m_icao;

  /**
  * All speech frequencies with type of the airfield.
  */
  QList<Frequency> m_frequencyList;

  /**
   * The ATIS frequency
   */
  float m_atis;

  /**
   * Contains all runways.
   */
  QList<Runway> m_rwyList;

  /**
   * The launching-type. "true" if the site has a m_winch.
   */
  bool m_winch;

  /**
   * The launching-type. "true" if the site has aero tow.
   */
  bool m_towing;

  /**
   * PPR flag
   */
  bool m_ppr;

  /**
   * Private flag
   */
  bool m_private;

  /**
   * sky diving flag
   */
   bool m_skyDiving;

   /**
    * Flag to indicate the land ability of the airfield.
    */
   bool m_landable;

   /**
   * Contains the shift of the runway during drawing in 1/10 degrees.
   */
  quint16 m_rwShift;

  /**
   * Mutex to protect pixmap creation.
   */
  static QMutex mutex;
};
