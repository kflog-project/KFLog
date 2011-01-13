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

#include <QObject>

#include "runway.h"

// declare static objects used for translations
QHash<int, QString> Runway::surfaceTranslations;
QStringList Runway::sortedTranslations;

Runway::Runway( const unsigned short len,
                const unsigned short dir,
                const unsigned short surf,
                const bool open,
                const unsigned short shift) :
  length(len),
  direction(dir),
  surface(surf),
  isOpen(open),
  rwShift(shift)
{
  // Calculate the real runway shift in 1/10 degrees. Used by drawing to find
  // the right icon.
  if ( direction/256 <= 36 )
    {
      rwShift = (direction/256 >= 18 ? (direction/256)-18 : direction/256);
    }
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
