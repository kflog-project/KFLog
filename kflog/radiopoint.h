/***********************************************************************
 **
 **   radiopoint.h
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

#pragma once

#include <climits>

#include <QList>
#include <QPoint>
#include <QString>

#include "Frequency.h"
#include "singlepoint.h"

/**
 * \class RadioPoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief This class provides a map element for radio-navigation-facilities.
 *
 * This class provides a map element for radio navigation facilities. It is
 * derived from \ref SinglePoint. This class is used for: VOR, VORDME, VORTAC,
 * NDB and CompPoint.
 *
 * @see BaseMapElement#objectType
 *
 * \date 2000-2023
 */

class RadioPoint : public SinglePoint
{
  public:

  enum RadioType {
    // The navaid types. Possible values:
    dme=0,
    tacan=1,
    ndb=2,
    vor=3,
    vor_dme=4,
    vortac=5,
    dvor=6,
    dvor_dme=7,
    dvortac=8
  };

  /**
   * Default constructor
   */
  RadioPoint() :
    SinglePoint(),
    m_range(0.0),
    m_declination(SHRT_MIN),
    m_aligned2TrueNorth(false)
   {
   };

  /**
   * Creates a new radio-point.
   *
   * @param  name  The name.
   * @param  icao  The ICAO name.
   * @param  shortName The abbreviation, used for the GPS logger.
   * @param  typeID The type identifier.
   * @param  wgsPos The original WGS84 position.
   * @param  pos    The projected position.
   * @param  frequencyList A list with the frequency objects
   * @param  channel The channel.
   * @param  elevation The elevation.
   * @param  country The country location.
   * @param  range The service range in meters.
   * @param  declination The declination
   * @param  aligned2TrueNorth Alignment to true north
   */
  RadioPoint( const QString& name,
              const QString& icao,
              const QString& shortName,
              BaseMapElement::objectType typeID,
              const WGSPoint& wgsPos,
              const QPoint& pos,
              const QList<Frequency> frequencyList,
              const QString channel = "",
              const float elevation = 0.0,
              const QString country = "",
              const float range = 0.0,
              const float declination = SHRT_MIN,
              const bool aligned2TrueNorth = false );

  /**
   * Destructor
   */
  virtual ~RadioPoint();

  /**
    * Prints the element. Reimplemented from BaseMapElement.
    *
    * @param  printPainter The painter to draw the element into.
    *
    * @param  isText  Shows, if the text of some mapelements should
    *                 be printed.
    */
   virtual void printMapElement( QPainter* printPainter, bool isText );

  /**
  * Return a short html-info-string about the navaid, containing the
  * name, the alias, the elevation and the frequency as well as a small
  * icon of the navaid type.
  *
  * Reimplemented from SinglePoint (@ref SinglePoint#getInfoString).
  * @return the info string
  */
  QString getInfoString();

  /**
   * @return The frequency as string.
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
   * @param freq The frequency and its type.
   */
  void addFrequency( Frequency freqencyAndType )
    {
      m_frequencyList.append( freqencyAndType );
    };

  QString getChannel() const
    {
      return m_channel;
    };

  void setChannel( const QString& value )
    {
      m_channel = value;
    };

  /**
   * @return ICAO name
   */
  QString getICAO() const
    {
      return m_icao;
    };

  /**
   * @param value ICAO name
   */
  void setICAO( const QString& value )
    {
      m_icao = value;
    }

  bool isAligned2TrueNorth () const
    {
      return m_aligned2TrueNorth;
    }

  void setAligned2TrueNorth (bool aligned2TrueNorth)
    {
      m_aligned2TrueNorth = aligned2TrueNorth;
    }

  float getDeclination () const
    {
      return m_declination;
    }

  void setDeclination (float declination)
    {
      m_declination = declination;
    }

  float getRange () const
    {
      return m_range;
    }

  void setRange (float range)
    {
      m_range = range;
    }

  /**
   * Combines channel, range, declination and north alignment as text string.
   *
   * \return additional items of radio point as text.
   */
  QString getAdditionalText() const;

 protected:

  /**
  * All frequencies of the radio point.
  */
  QList<Frequency> m_frequencyList;

  /**
   * The channel.
   */
  QString m_channel;

  /**
   * The icao name
   */
  QString m_icao;

  /**
   * Range of service in meters. 0 means unknown.
   */
  float m_range;

  /**
   * Declination, SHRT_MIN means undefined.
   */
  float m_declination;

  /**
   * Aligned to true north.
   */
  bool m_aligned2TrueNorth;
};
