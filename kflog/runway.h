/***********************************************************************
**
**   runway.h
**
**   This file is part of KFLog
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RUNWAY_H
#define RUNWAY_H

/**
 * This struct is used for defining a runway. It is used for all "small"
 * airports, not for the international airports, they use the struct
 * "intrunway", because they need two points per runway.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
struct runway
{
  /**
   * The length of the runway, given in meters.
   */
  unsigned int length;
  /**
   * The direction of the runway, given in steps of 10 degree.
   */
  unsigned int direction;
  /**
   * The surface of the runway.
   *
   * @see Airport#SurfaceType
   */
  unsigned int surface;
  /**
   * Asings, if the runway is open or closed.
   */
  bool isOpen;
};

#endif
