/***********************************************************************
**
**   runway.h
**
**   This file is part of KFlog4
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
 * \date 2008-2011
 *
 * $Id$
 *
 */

#ifndef RUNWAY_H
#define RUNWAY_H

#include <QHash>
#include <QPair>
#include <QString>
#include <QStringList>

class Runway
{

public:

  /**
   * Used to define the surface of a runway.
   */
  enum SurfaceType {Unknown = 0, Grass = 1, Asphalt = 2, Concrete = 3, Sand = 4};

  Runway( const unsigned short len,
          const QPair<ushort, ushort> dir,
          const enum SurfaceType surf,
          const bool open );

  virtual ~Runway() {};

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
   * The length of the runway, given in meters.
   */
  unsigned short length;

  /**
   * The surface of the runway, one of SurfaceType, see above.
   */
  enum SurfaceType surface;

  /**
   * Flag to indicate if the runway is open or closed.
   */
  bool isOpen;

  /**
   * Contains the shift of the runway during drawing.
   */
  unsigned short rwShift;

  /**
   * \return The runway directions as pair.
   */
  QPair<ushort, ushort> getRunwayDirection()
    {
      return direction;
    }

  /**
   * Sets the runway directions.
   *
   * \param New Runway directions as pair.
   */
  void setRunwayDirections( const QPair<ushort, ushort>& rwDir );

private:

  /**
   * The direction of the runway, given in steps of 1/10 degree (1-36).
   */
  QPair<ushort, ushort> direction;

  /**
   * Static pointer to surface translations
   */
  static QHash<int, QString> surfaceTranslations;
  static QStringList sortedTranslations;

  /**
   * Static method for loading of object translations
   */
  static void loadTranslations();
};

#endif
