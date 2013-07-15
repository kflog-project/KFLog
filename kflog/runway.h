/***********************************************************************
**
**   runway.h
**
**   This file is part of KFlog4
**
************************************************************************
**
**   Copyright (c): 2008-2013 Axel Pauli
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
 * \date 2008-2013
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

  /**
   * Default constructor.
   */
  Runway() :
    length(0.0),
    width(0.0),
    surface(Unknown),
    isOpen(false),
    rwShift(9),
    headings(QPair<ushort, ushort> (0, 0))
    {
    };

  Runway( const float length,
          const QPair<ushort, ushort> headings,
          const enum SurfaceType surf,
          const bool open=false,
          const float width=0.0 );

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
  float length;

  /**
   * The width of the runway, given in meters.
   */
  float width;

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
   * \return The runway headings as pair.
   */
  QPair<ushort, ushort> getRunwayHeadings()
    {
      return headings;
    }

  /**
   * Sets the runway headings.
   *
   * \param New Runway headings as pair.
   */
  void setRunwayHeadings( const QPair<ushort, ushort>& rwyHeadings );

  /**
   * The headings of the runway, given in steps of 1/10 degree (1-36).
   */
  QPair<ushort, ushort> headings;

private:

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
