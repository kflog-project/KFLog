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
  * This struct is used for defining a runway. It is used for the
  * international airports. All other airports use the struct
  * "runway", because they don't need the coordinates of the runway.
  *
  * @author Heiner Lamprecht
  * @version $Id$
  */
struct intrunway
{
  /**
    * The length of the runway, given in meters.
    */
  unsigned int length;
  /**
    * The direction of the runway, given in steps of 10 degree.
    * NOTE: Only the first two digits are given. Thus "25" meens "250".
    */
  unsigned int direction;
  /**
    * The surface of the runway.
    *
    * @see Airport#Surface
    */
  unsigned int surface;
  /**
    * The coordinates of the two edged of the runway.
    */
  int latitudeA;
  /**
    * The coordinates of the two edged of the runway.
    */
  int longitudeA;
  /**
    * The coordinates of the two edged of the runway.
    */
  int latitudeB;
  /**
    * The coordinates of the two edged of the runway.
    */
  int longitudeB;
};

/**
  * This struct is used for defining a runway. It is used for all "small"
  * airports, not for the international airports, they use the struct
  * "intrunway", because they need two points per runway.
  *
  * @author Heiner Lamprecht
  * @version 0.1
  */
struct runway
{
  /**
    * The length of the runway, given in meters.
    */
  unsigned int length;
  /**
    * The direction of the runway, given in steps of 10 degree.
    * NOTE: Only the first two digits are given. Thus "25" meens "250".
    */
  unsigned int direction;
  /**
    * The surface of the runway.
    *
    * @see Airport#Surface
    */
  unsigned int surface;
};

#endif
