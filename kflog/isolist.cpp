/***********************************************************************
 **
 **   isolist.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "isolist.h"

/**
 * Constructor.
 * @param path Path in coordinate system of the map object, not in KFLog system
 * @param height the elevation of the isoline in meters
 */
IsoListEntry::IsoListEntry( QPainterPath* path, const int height )
{
  this->path = path;
  this->height = height;
};

/**
 * Copy constructor is needed to make a deep copy of the QPainterPath pointer.
 */
IsoListEntry::IsoListEntry( const IsoListEntry& x )
{
  height = x.height;
  path = x.path;

  // Make a deep copy of the QPainterPath object, if it exists.
  if( x.path )
    {
      path = new QPainterPath( *x.path );
    }
}

/**
 * Assignment operator is needed to make a deep copy of the QPainterPath pointer.
 */
IsoListEntry& IsoListEntry::operator=(const IsoListEntry& x)
{
  if( this == &x )
    {
      // Same object, nothing to do.
      return *this;
    }

  // overtake height
  height = x.height;

  // The QPainterPath object of the left side must be deleted, if it exists.
  if( path )
    {
      delete path;
      path = static_cast<QPainterPath *> (0);
    }

  // Make a deep copy of the QPainterPath object from the right side, if it exists.
  if( x.path )
    {
      path = new QPainterPath( *x.path );
    }

  return *this;
}

/**
 * Destructor
 */
IsoListEntry::~IsoListEntry()
{
  // QPainterPath must be deleted, if it was allocated.
  if( path )
    {
      delete path;
    }
}
