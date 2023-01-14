/***********************************************************************
**
**   runway.h
**
**   This file is part of KFLog
**
************************************************************************
**
**   Copyright (c): 2008-2023 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Runway
 *
 * \author Axel Pauli
 *
 * \brief Runway data class.
 *
 * This class is used for defining a runway together with its surface and the
 * translation types.
 *
 * \date 2008-2023
 *
 */

#pragma once

#include <QDataStream>
#include <QList>
#include <QHash>
#include <QString>
#include <QStringList>

class Runway
{

public:

  /**
   * Used to define the surface of a runway.
   */
  enum SurfaceType { Unknown = 0,
                     Grass = 1,
                     Asphalt = 2,
                     Concrete = 3,
                     Sand = 4,
                     Water = 5,
                     Gravel = 6,
                     Ice = 7,
                     Snow = 8,
                     Clay = 9,
                     Stone = 10,
                     Metal = 11,
                     Rubber = 12,
                     Wood = 13,
                     Earth = 14
   };

  /**
   * Used to define the type of runway operation.
   */
  enum operations { Active = 0,
                    TemporarilyClosed = 1,
                    Closed = 2
  };

  /**
   * Allowed take-off/landing turn directions for this runway.
   */
  enum turnDirections { Right = 0,
                        Left = 1,
                        Both = 2,
                        OneWay = 3
  };

  Runway() :
    m_name(),
    m_length(0.0),
    m_width(0.0),
    m_heading(0),
    m_alignedTrueNorth(false),
    m_operations(Active),
    m_turnDirection(Both),
    m_mainRunway(false),
    m_takeOffOnly(false),
    m_landingOnly(false),
    m_surface(Unknown)
    {
    };

  Runway( const QString& name,
          const float length,
          const float width,
          const quint16 heading,
          const bool alignedTrueNorth,
          const quint8 operations,
          const quint8 turnDirection,
          const bool mainRunway,
          const bool takeOffOnly,
          const bool landingOnly,
          const quint8 surface ) :
    m_name( name ),
    m_length( length ),
    m_width( width ),
    m_heading( heading ),
    m_alignedTrueNorth( alignedTrueNorth ),
    m_operations( operations ),
    m_turnDirection( turnDirection ),
    m_mainRunway( mainRunway ),
    m_takeOffOnly( takeOffOnly ),
    m_landingOnly( landingOnly ),
    m_surface( surface )
    {
    };

  virtual ~Runway() {};

  /**
   * Prints out all runway data.
   */
  void printData();

  /**
   * Get translation string for surface type.
   */
  static QString item2Text( const int surfaceType, QString defaultValue=QString("") );

  /**
   * Get surface type for translation string.
   */
  static int text2Item( const QString& text );

  /**
   * Get sorted translations
   */
  static QStringList& getSortedTranslationList();

  /**
   * Returns the designator of the runway.
   */
  QString getName() const
  {
    return m_name;
  }

  /**
   * Sets the designator of the runway.
   */
  void setName( const QString& designator )
  {
    m_name = designator;
  }

  unsigned short getHeading() const
  {
    return m_heading;
  }

  void setHeading( unsigned short heading )
  {
    m_heading = heading;
  }

  bool isAlignedTrueNorth() const
  {
    return m_alignedTrueNorth;
  }

  void setAlignedTrueNorth( bool alignedTrueNorth )
  {
    m_alignedTrueNorth = alignedTrueNorth;
  }

  quint8 getOperations() const
  {
    return m_operations;
  }

  void setOperations( quint8 operations )
  {
    m_operations = operations;
  }

  quint8 getTurnDirection() const
  {
    return m_turnDirection;
  }

  void setTurnDirection( quint8 turnDirection )
  {
    m_turnDirection = turnDirection;
  }

  bool isLandingOnly() const
  {
    return m_landingOnly;
  }

  void setLandingOnly( bool landingOnly )
  {
    m_landingOnly = landingOnly;
  }

  bool isTakeOffOnly() const
  {
    return m_takeOffOnly;
  }

  void setTakeOffOnly( bool takeOffOnly )
  {
    m_takeOffOnly = takeOffOnly;
  }

  float getLength() const
  {
    return m_length;
  }

  void setLength( const float length )
  {
    m_length = length; // meters expected
  }

  float getWidth() const
  {
    return m_width;
  }

  void setWidth( float width )
  {
    m_width = width; // meters expected
  }

  bool isMainRunway() const
  {
    return m_mainRunway;
  }

  void setMainRunway( bool mainRunway )
  {
    m_mainRunway = mainRunway;
  }

  bool isOpen() const
  {
    if( m_operations == Active )
      {
        return true;
      }

    return false;
  }

  bool isBidirectional() const
  {
    if( m_turnDirection == Both )
      {
        return true;
      }

    return false;
  }

  quint8 getSurface() const
  {
    return m_surface;
  }

  void setSurface( quint8 surface )
  {
    m_surface = surface;
  }

  /**
   * Calculate the runway shift for the drawing of the first found main runway.
   * If no main runway exists, the first runway from the list is taken.
   * Default shift is 90 degrees, if no runways are defined.
   */
  static quint16 calculateRunwayShift( const QList<Runway>& rwList );

  /**
   * Get the first found main runway. If no main runway exists, the first
   * runway from the list is taken.
   */
  static const Runway& getMainRunway( const QList<Runway> &rwList );

  /**
   * Save a runway list into a data stream.
   */
  static void saveRunways( QDataStream& out, const QList<Runway>& rwyList );

  /**
   * Load a runwayList from a data stream.
   */
  static void loadRunways( QDataStream& in, QList<Runway>& rwyList );

 protected:

  /**
   * Static method for loading of object translations
   */
  static void loadTranslations();

  /**
   * The designator of the runway.
   */
  QString m_name;

  /**
   * The length of the runway, given in meters.
   */
  float m_length;

  /**
   * The width of the runway, given in meters.
   */
  float m_width;

  /**
   * The true heading of the runway, given in steps of 0...360
   */
  quint16 m_heading;

  /**
   * Aligned true north
   */
  bool m_alignedTrueNorth;

  /**
   * The type of the operations. Possible values: 0, 1, 2
   */
  quint8 m_operations;

  /**
   * Allowed take-off/landing turn directions for this runway. Possible values:
   *
   * 0, 1, 2
   */
  quint8 m_turnDirection;

  /**
   * Marks this runway as a runway on which most operations take place. Multiple
   * runways can be marked as main runway. For map drawing, the primary runway
   * with the best pavement and highest length is used. This logic also applies
   * if no primary main runway is specified.
   */
  bool m_mainRunway;

  /**
   * Restricted to take off only.
   */
  bool m_takeOffOnly;

  /**
   * Restricted to landing only.
   */
  bool m_landingOnly;

  /**
   * The surface of the runway, one of SurfaceType, see above.
   */
  quint8 m_surface;

  /**
   * Static pointer to surface translations
   */
  static QHash<int, QString> surfaceTranslations;
  static QStringList sortedTranslations;
};
