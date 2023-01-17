/***********************************************************************
 **
 **   Frequency.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2018-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtCore>

#include "filetools.h"
#include "Frequency.h"

// declare static object used for translations
QHash<int, QString> Frequency::typeTranslations;

/**
 * @return The frequency type as string.
 */
QString Frequency::typeAsString( const quint8 type )
{
  if( typeTranslations.isEmpty() == true )
    {
      loadTranslations();
    }

  if( typeTranslations.contains(type) == true )
    {
      return typeTranslations[type];
    }

  // Unknown type
  return typeTranslations[255];
}

quint8 Frequency::typeAsInt( QString& type )
{
  if( typeTranslations.isEmpty() == true )
    {
      loadTranslations();
    }

  QHashIterator<int, QString> i(typeTranslations);

  // search reverse over the hash via value
  while( i.hasNext() )
    {
      i.next();

      if( i.value() == type )
        {
          return i.key();
        }
    }

  // Unknown, if nothing has found.
  return Frequency::Unknown;
}

QString unitAsString( const quint8 unit )
{
  switch( unit )
  {
    case Frequency::kHz:
      return QString("kHz");
    case Frequency::MHz:
      return QString("MHz");
    case Frequency::Unknown:
    default:
      return QString( QObject::tr("unknown") );
  }
}

/**
 * Search and return the main frequency. If no main frequency is found,
 * return the first list element.
 *
 * Return true, if a frequency is put into the passed argument fq otherwise
 * false.
 */
bool Frequency::getMainFrequency( const QList<Frequency>& fqList, Frequency& fq )
{
  if( fqList.size() > 0.0 )
    {
      int i = 0;
      int fqlIdx = -1;

      // Only a main frequency should be shown in the display
      for( i = 0; i < fqList.size(); i++ )
        {
          quint8 type = fqList.at(i).getType();

          if( type == Frequency::Tower || type == Frequency::Info ||
              type == Frequency::Information ||
              fqList.at(i).isPrimary() == true )
            {
              fqlIdx = i;
              break;
            }
        }

      if( fqlIdx != -1  )
        {
          fq = fqList.at(i);
          return true;
        }
    }

  return false;
}

void Frequency::loadTranslations()
{
  // Load translation data
  typeTranslations.insert( Frequency::Approach,  QObject::tr( "Approach" ) );
  typeTranslations.insert( Frequency::APRON,  QObject::tr( "Apron" ) );
  typeTranslations.insert( Frequency::Arrival,  QObject::tr( "Arrival" ) );
  typeTranslations.insert( Frequency::Center,  QObject::tr( "Center" ) );
  typeTranslations.insert( Frequency::CTAF,  QObject::tr( "CTaf" ) );
  typeTranslations.insert( Frequency::Delivery,  QObject::tr( "Delivery" ) );
  typeTranslations.insert( Frequency::Departure,  QObject::tr( "Departure" ) );
  typeTranslations.insert( Frequency::FIS,  QObject::tr( "FIS" ) );
  typeTranslations.insert( Frequency::Gliding,  QObject::tr( "Gliding" ) );
  typeTranslations.insert( Frequency::Ground,  QObject::tr( "Ground" ) );
  typeTranslations.insert( Frequency::Info,  QObject::tr( "Info" ) );
  typeTranslations.insert( Frequency::Multicom,  QObject::tr( "Multicom" ) );
  typeTranslations.insert( Frequency::Radar,  QObject::tr( "Radar" ) );
  typeTranslations.insert( Frequency::Tower,  QObject::tr( "Tower" ) );
  typeTranslations.insert( Frequency::ATIS,  QObject::tr( "Atis" ) );
  typeTranslations.insert( Frequency::Radio,  QObject::tr( "Radio" ) );
  typeTranslations.insert( Frequency::Other,  QObject::tr( "Other" ) );
  typeTranslations.insert( Frequency::AIRMET,  QObject::tr( "Airmet" ) );
  typeTranslations.insert( Frequency::AWOS,  QObject::tr( "Awos" ) );
  typeTranslations.insert( Frequency::Lights,  QObject::tr( "Lights" ) );
  typeTranslations.insert( Frequency::VOLMET,  QObject::tr( "Volmet" ) );
  typeTranslations.insert( Frequency::Information,  QObject::tr( "Information" ) );
  typeTranslations.insert( Frequency::Unknown,  QObject::tr( "Unknown" ) );
}

/**
 * Save a frequency list into a data stream.
 */
void Frequency::saveFrequencies( QDataStream& out, const QList<Frequency>& fqList )
{
  int size = fqList.size();

  if( size > 255 )
    {
      qWarning() << "Frequency::saveFequencies: List is too big, cutting it to 255 elements.";
      size = 255;
    }

  // Number of list elements
  out << quint8( size );

  for( int i = 0; i < size; i++ )
   {
      const Frequency& fq = fqList.at(i);

      out << fq.m_value;
      out << quint8(fq.m_unit);
      out << quint8(fq.m_type);
      ShortSave( out, fq.m_userType.toUtf8() );
      out << fq.m_primary;
      out << fq.m_publicUse;
   }
}

/**
 * Load a frequency from a data stream.
 */
void Frequency::loadFrequencies( QDataStream& in, QList<Frequency>& fqList )
{
  quint8 size;
  in >> size;

  for( int i = 0; i < size; i++ )
    {
      float value;
      quint8 unit;
      quint8 type;
      QString userType;
      bool primary;
      bool publicUse;

      in >> value;
      in >> unit;
      in >> type;
      ShortLoad(in, userType);
      in >> primary;
      in >> publicUse;

      Frequency fq( value,
                    unit,
                    type,
                    userType,
                    primary,
                    publicUse );

      fqList.append( fq );
    }
}
