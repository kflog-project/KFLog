/***********************************************************************
 **
 **   isolist.h
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef ISO_LIST_H
#define ISO_LIST_H

#include <QPainterPath>
#include <QList>

/**
 * \class IsoListEntry
 *
 * \author Axel Pauli
 *
 * \brief An object to be put into the \ref IsoList.
 *
 * This class contains a QPainterPath and a height. A list of entries
 * like this is created when the map is drawn and is used to detect the
 * elevation at a given position, for instance under the mouse cursor.
 *
 * \date 2008-2010
 *
 * \version $Id$
 */
class IsoListEntry
{
 public:

  /**
   * Constructor.
   * @param path Path in coordinate system of the map-object, not in KFLog system
   * @param height the elevation of the isoline in meters
   */
  IsoListEntry( QPainterPath* path, const int height=0 );

  /**
   * Copy constructor is needed to make a deep copy of the QPainterPath pointer.
   */
  IsoListEntry( const IsoListEntry& x );

  /**
   * Assignment operator is needed to make a deep copy of the QPainterPath pointer.
   */
  IsoListEntry& operator=(const IsoListEntry& x);

  /**
   * Destructor
   */
  virtual ~IsoListEntry();

  bool operator == (const IsoListEntry& x)
  {
    return x.height==height;
  }

  bool operator >= (const IsoListEntry& x)
  {
    return x.height>=height;
  }

  bool operator <= (const IsoListEntry& x)
  {
    return x.height<=height;
  }

  bool operator < (const IsoListEntry& x)
  {
    return x.height<height;
  }

  bool operator > (const IsoListEntry& x)
  {
    return x.height>height;
  }

  bool operator()(const IsoListEntry &iso1, const IsoListEntry &iso2) const
  {
    return (iso1.height < iso2.height);
  };

  bool operator()(const IsoListEntry *iso1, const IsoListEntry *iso2) const
  {
    return (iso1->height < iso2->height);
  };

  QPainterPath* path;
  int height;

  /**
  * Compares two items, in this case, IsoListEntries.
  *
  * The items are compared by height only. The result is a reverse sorted
  * list, highest entry at lowest position.
  */
  bool operator < (const IsoListEntry& other) const
  {
    return height > other.height;
  };
};

/**
 * \class IsoList
 *
 * \author Axel Pauli
 *
 * \brief A list of objects from type \ref IsoListEntry.
 *
 * This class is a list of objects from type \ref IsoListEntry which an
 * additional sort method.
 *
 * \date 2008
 */

class IsoList : public QList<IsoListEntry>
{
 public:

  IsoList() {};

  virtual ~IsoList() {};

  void sort()
  {
    qSort( begin(), end() );
  };

};

#endif
