/***********************************************************************
**
**   runway.cpp
**
**   This file is part of KFLog
**
************************************************************************
**
**   Copyright (c): 2008-2022 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "filetools.h"
#include "runway.h"

// declare static objects used for translations
QHash<int, QString> Runway::surfaceTranslations;
QStringList Runway::sortedTranslations;

void Runway::printData()
{
  qDebug() << "RWY-Name=" << m_name
           << "RWY-Heading=" << m_heading
           << "Length=" << m_length
           << "Width=" << m_width
           << "Sfc=" << m_surface
           << "MainRunway" << m_mainRunway
           << "TakeOffOnly" << m_takeOffOnly
           << "LandingOnly" << m_landingOnly;
}

/**
 * Get translation string for surface type.
 */
QString Runway::item2Text( const int surfaceType, QString defaultValue )
{
  if( surfaceTranslations.isEmpty() )
    {
      loadTranslations();
    }

  return surfaceTranslations.value( surfaceType, defaultValue );
}

/**
 * Get surface type for translation string.
 */
int Runway::text2Item( const QString& text )
{
  if( surfaceTranslations.isEmpty() )
    {
      // Load object - translation data
      loadTranslations();
    }

  return surfaceTranslations.key( text );
}

void Runway::loadTranslations()
{
  // Load translation data
  surfaceTranslations.insert( Runway::Unknown,  QObject::tr( "unknown" ) );
  surfaceTranslations.insert( Runway::Grass,    QObject::tr( "Grass" ) );
  surfaceTranslations.insert( Runway::Asphalt,  QObject::tr( "Asphalt" ) );
  surfaceTranslations.insert( Runway::Concrete, QObject::tr( "Concrete" ) );
  surfaceTranslations.insert( Runway::Sand,     QObject::tr( "Sand" ) );
  surfaceTranslations.insert( Runway::Water,    QObject::tr( "Water" ) );
  surfaceTranslations.insert( Runway::Gravel,   QObject::tr( "Gravel" ) );
  surfaceTranslations.insert( Runway::Ice,      QObject::tr( "Ice" ) );
  surfaceTranslations.insert( Runway::Snow,     QObject::tr( "Snow" ) );
  surfaceTranslations.insert( Runway::Clay,     QObject::tr( "Clay" ) );
  surfaceTranslations.insert( Runway::Stone,    QObject::tr( "Stone" ) );
  surfaceTranslations.insert( Runway::Metal,    QObject::tr( "Metal" ) );
  surfaceTranslations.insert( Runway::Rubber,   QObject::tr( "Rubber" ) );
  surfaceTranslations.insert( Runway::Wood,     QObject::tr( "Wood" ) );
  surfaceTranslations.insert( Runway::Earth,    QObject::tr( "Earth" ) );

  // load sorted translation strings
  QHashIterator<int, QString> it(surfaceTranslations);

  while( it.hasNext() )
    {
      it.next();
      sortedTranslations.append( it.value() );
    }

  sortedTranslations.sort();
}

/**
 * Get sorted translations
 */
QStringList& Runway::getSortedTranslationList()
{
  if( surfaceTranslations.isEmpty() ) {
    // Load surface - translation data
    loadTranslations();
  }

  // qDebug("Runway::getSortedTranslationList: size: %d", sortedTranslations.size());

  return sortedTranslations;
}

/**
 * Calculate the runway shift for the drawing of the first found main runway.
 * If no main runway exists, the first runway from the list is taken.
 * Default shift is 90 degrees, if no runways are defined.
 */
quint16 Runway::calculateRunwayShift( const QList<Runway>& rwList )
{
  // calculate the default runway shift in 1/10 degrees.
  quint16 rwShift = 90/10; // default direction is 90 degrees

  if( rwList.size() > 0 )
    {
      // search the first main runway
      bool found = false;

      for( int i = 0; i < rwList.size(); i ++ )
        {
          if( rwList.at(i).isMainRunway() )
            {
              rwShift = ( rwList.at(i).getHeading() + 5 ) / 10;
              found = true;
            }
        }

      // No main runway found, take first list element.
      if( found == false )
        {
          rwShift = ( rwList.at(0).getHeading() + 5 ) / 10;
        }

      // calculate the real runway shift in 1/10 degrees.
      rwShift = (rwShift >= 18) ? rwShift - 18 : rwShift;
    }

  return rwShift;
}

/**
 * Get the first found main runway. If no main runway exists, the first
 * runway from the list is taken.
 */
const Runway& Runway::getMainRunway( const QList<Runway>& rwList )
{
  static Runway rw;

  if( rwList.size() > 0 )
    {
      // search the first main runway
      for( int i = 0; i < rwList.size(); i ++ )
        {
          if( rwList.at(i).isMainRunway() )
            {
              return rwList.at(i);
            }
        }

      // return first none main runway
      return rwList.at(0);
    }

  // return empty runway, if list is empty.
  return rw;
}

/**
 * Save a runway list into a data stream.
 */
void Runway::saveRunways( QDataStream& out, const QList<Runway>& rwyList )
{
  int size = rwyList.size();

  if( size > 255 )
    {
      qWarning() << "Runway::saveRunways(): List is too big, cutting it to 255 elements.";
      size = 255;
    }

  // Number of list elements
  out << quint8( size );

  for( int i = 0; i < size; i++ )
   {
     const Runway& rwy = rwyList.at(i);

     ShortSave( out, rwy.getName() );
     out << rwy.m_length;
     out << rwy.m_width;
     out << quint16( rwy.m_heading );
     out << rwy.m_alignedTrueNorth;
     out << quint8( rwy.m_operations );
     out << quint8( rwy.m_turnDirection );
     out << rwy.m_mainRunway;
     out << rwy.m_takeOffOnly;
     out << rwy.m_landingOnly;
     out << quint8( rwy.m_surface );
  }
}

/**
 * Load a runwayList from a data stream.
 */
void Runway::loadRunways( QDataStream& in, QList<Runway>& rwyList )
{
  quint8 size;
  in >> size;

  for( int i = 0; i < size; i++ )
    {
      QString name;
      float length;
      float width;
      quint16 heading;
      bool alignedTrueNorth;
      quint8 operations;
      quint8 turnDirection;
      bool mainRunway;
      bool takeOffOnly;
      bool landingOnly;
      quint8 surface;

      // read runway data
      ShortLoad(in, name);
      in >> length;
      in >> width;
      in >> heading;
      in >> alignedTrueNorth;
      in >> operations;
      in >> turnDirection;
      in >> mainRunway;
      in >> takeOffOnly;
      in >> landingOnly;
      in >> surface;

      Runway rwy( name,
                  length,
                  width,
                  heading,
                  alignedTrueNorth,
                  operations,
                  turnDirection,
                  mainRunway,
                  takeOffOnly,
                  landingOnly,
                  surface );

      rwyList.append( rwy );
    }
}
