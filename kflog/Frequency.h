/***********************************************************************
 **
 **   Frequency.h
 **
 **   This file is part of KFlog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2018-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class Frequency
 *
 * \author Axel Pauli
 *
 * \brief Class to handle frequency of an radiopoint and an airfield.
 *
 * This class contains a frequency and its related type.
 *
 * \date 2018-2023
 *
 */

#pragma once

#include <QDataStream>
#include <QHash>
#include <QList>
#include <QString>

class Frequency
{
 public:

  /**
   * Frequency units
   */
  enum unit {
    unknown = 0,
    kHz = 1,
    MHz = 2
  };

  /**
   * Frequency types according to openAIP
   */
  enum type
  {
    Approach = 0,
    APRON = 1,
    Arrival = 2,
    Center = 3,
    CTAF = 4,
    Delivery = 5,
    Departure = 6,
    FIS = 7,
    Gliding = 8,
    Ground = 9,
    Info = 10,
    Multicom = 11,
    Unicom = 12,
    Radar = 13,
    Tower = 14,
    ATIS = 15,
    Radio = 16,
    Other = 17,
    AIRMET = 18,
    AWOS = 19,
    Lights = 20,
    VOLMET = 21,
    Information = 22,
    Unknown = 255
  };

  /**
   * Static hash to frequency type translations
   */
  static QHash<int, QString> typeTranslations;

  Frequency() :
    m_value(0.0),
    m_unit(unknown),
    m_type(Unknown),
    m_primary(true),
    m_publicUse(true)
  {
  }

  Frequency( float value,
             quint8 unit,
             quint8 type,
             QString userType,
             bool primary,
             bool publicUse ) :
    m_value(value),
    m_unit(unit),
    m_type(type),
    m_userType(userType),
    m_primary(primary),
    m_publicUse(publicUse)
  {
  }

  virtual ~Frequency()
  {
  }

  void setFrequencyAndType( float value, quint8 type )
  {
    m_value = value;
    m_type = type;
  }

  quint8 getType() const
  {
    return m_type;
  }

  void setType( const quint8 type)
  {
    m_type = type;
  }

  float getValue() const
  {
    return m_value;
  }

  void setValue( const float value )
  {
    m_value = value;
  }

  /**
   * @return The frequency type as string.
   */
  static QString typeAsString( const quint8 type );

  /**
   * @return The frequency type as string.
   */
  QString typeAsString() const
    {
      return typeAsString( m_type );
    }

  /**
   * @return The frequency type as integer.
   */
  static quint8 typeAsInt( QString& type );

  /**
   * @return The frequency unit as string.
   */
  static QString unitAsString( const quint8 unit );

  /**
   * @return The frequency with unit as string.
   */
  static QString frequencyAsString( Frequency frequency, bool withUnit=true )
    {
      QString fs;

      if( frequency.getValue() > 0 )
        {
          if( frequency.getUnit() == kHz )
            {
              fs = QString("%1").arg(frequency.getValue(), 0, 'f', 0);
            }
          else
            {
              fs = QString("%1").arg(frequency.getValue(), 0, 'f', 3);
            }

          if( withUnit == true )
            {
               quint8 unit = frequency.getUnit();

              if( unit == kHz || unit == MHz )
                {
                  QString us = ( unit == kHz ) ? "kHz" : "MHz";
                  fs += " (" + us + ")";
                }
            }
        }

      return fs;
    };

  /**
   * Search and return the main frequency. If no main frequency is found,
   * return the first list element.
   *
   * Return true, if a frequency is put into the passed argument fq otherwise
   * false.
   */
  static bool getMainFrequency( const QList<Frequency>& fqList, Frequency& fq );

  /**
   * @return The frequency with unit as string.
   */
  QString frequencyAsString( bool withUnit=true ) const
  {
    return frequencyAsString( *this, withUnit );
  }

  bool isPrimary() const
  {
    return m_primary;
  }

  void setPrimary( bool primary )
  {
    m_primary = primary;
  }

  bool isPublicUse() const
  {
    return m_publicUse;
  }

  void setPublicUse( bool publicUse )
  {
    m_publicUse = publicUse;
  }

  quint8 getUnit() const
  {
    return m_unit;
  }

  void setUnit( const quint8 unit )
  {
    m_unit = unit;
  }

  const QString& getName() const
  {
    return m_name;
  }

  void setName( const QString &name )
  {
    m_name = name;
  }

  const QString& getUserType() const
  {
    return m_userType;
  }

  void setUserType( const QString &type )
  {
    m_userType = type;
  }

  /**
   * Save a frequency list into a data stream.
   */
  static void saveFrequencies( QDataStream& out, const QList<Frequency>& fqList );

  /**
   * Load a frequency from a data stream.
   */
  static void loadFrequencies( QDataStream& in, QList<Frequency>& fqList );

 protected:

  /**
   * Static method for loading of object translations
   */
  static void loadTranslations();

  /**
   * An optional frequency name and/or callsign, like 'Herrenteich Info',
   * that may help to clarify the intended purpose of the frequency.
   * The frequency name should not contain any additional frequencies.
   */
  QString m_name;

  /**
   * Frequency value
   */
  float m_value;

  /**
   * Frequency unit kHz or MHz
   */
  quint8 m_unit;

  /**
   * The frequency type
   */
  quint8 m_type;

  /**
   * A frequency type, defined by the user. It cannot be an integer.
   */
  QString m_userType;

  /**
   * Mark as primary frequency
   */
  bool m_primary;

  /**
   * Mark as public use
   */

  bool m_publicUse;
};

