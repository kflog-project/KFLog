/***********************************************************************
 **
 **   radiopoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef RADIO_POINT_H
#define RADIO_POINT_H

#include "singlepoint.h"

/**
 * \struct radioContact
 *
 * \brief This structure contains the data of one frequency;
 *
 * \date 2000-2008
 */
struct radioContact
{
  /** Frequency as string. */
  QString frequency;

  /** Call sign as string. */
  QString callSign;

  unsigned int type;
};

/**
 * \class RadioPoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief This class provides a map element for radio-navigation-facilities.
 *
 * This class provides a map element for radio navigation facilities. It is
 * derived from \ref SinglePoint. This class is used for: VOR, VORDME, VORTAC,
 * NDB and CompPoint.
 *
 * \see BaseMapElement#objectType
 *
 * \date 2000-2010
 *
 * \version $Id$
 */

class RadioPoint : public SinglePoint
{
 public:
  /**
   * Creates a new radio-point.
   *
   * @param  name  The name.
   * @param  icao  The ICAO name.
   * @param  shortName The abbreviation, used for the GPS logger.
   * @param  typeID The type identifier.
   * @param  wgsPos The original WGS84 position.
   * @param  pos    The projected position.
   * @param  frequency  The frequency.
   * @param  elevation The elevation.
   */
  RadioPoint( const QString& name,
              const QString& icao,
              const QString& shortName,
              BaseMapElement::objectType typeID,
              const WGSPoint& wgsPos,
              const QPoint& pos,
              const QString& frequency,
              int elevation = 0);

  /**
   * Destructor
   */
  virtual ~RadioPoint();

  /**
    * Prints the element. Reimplemented from BaseMapElement.
    *
    * @param  printPainter The painter to draw the element into.
    *
    * @param  isText  Shows, if the text of some mapelements should
    *                 be printed.
    */
   virtual void printMapElement( QPainter* printPainter, bool isText );

  /**
   * @return The frequency
   */
  virtual QString getFrequency() const
    {
      return frequency;
    };

  /**
   * @return ICAO name
   */
  virtual QString getICAO() const
    {
      return icao;
    };

 protected:
  /**
   * The frequency
   */
  QString frequency;

  /**
   * The icao name
   */
  QString icao;
};

#endif
