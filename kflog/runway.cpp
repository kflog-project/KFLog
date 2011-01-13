/***********************************************************************
**
**   runway.cpp
**
**   This file is part of KFLog4
**
************************************************************************
**
**   Copyright (c): 2008-2011 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "runway.h"

// declare static objects used for translations
QHash<int, QString> Runway::surfaceTranslations;
QStringList Runway::sortedTranslations;

Runway::Runway( const unsigned short len,
                const QPair<ushort, ushort> dir,
                const enum SurfaceType surf,
                const bool open ) :
  length(len),
  surface(surf),
  isOpen(open),
  rwShift(9),
  direction(dir)
{
  // Calculate the real runway shift in 1/10 degrees. Used by drawing to find
  // the right icon.
  if( direction.first <= 36 )
    {
      rwShift = ( direction.first >= 18 ? direction.first-18 : direction.first );
    }
}

void Runway::setRunwayDirections( const QPair<ushort, ushort>& newRwDir )
{
  // Calculate the real runway shift in 1/10 degrees. Used by drawing to find
  // the right icon.
  if( newRwDir.first <= 36 )
    {
      rwShift = ( direction.first >= 18 ? direction.first-18 : direction.first );
    }

  direction = newRwDir;
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
  surfaceTranslations.insert( Runway::Unknown,  QObject::tr( "Unknown" ) );
  surfaceTranslations.insert( Runway::Grass,    QObject::tr( "Grass" ) );
  surfaceTranslations.insert( Runway::Asphalt,  QObject::tr( "Asphalt" ) );
  surfaceTranslations.insert( Runway::Concrete, QObject::tr( "Concrete" ) );
  surfaceTranslations.insert( Runway::Sand,     QObject::tr( "Sand" ) );

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
  if( surfaceTranslations.isEmpty() )
    {
      // Load surface - translation data
      loadTranslations();
    }

  // qDebug("Runway::getSortedTranslationList: size: %d", sortedTranslations.size());

  return sortedTranslations;
}
