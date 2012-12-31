/***********************************************************************
**
**   airspacewarningdistance.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): André Somers, 2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: airspacewarningdistance.h 4502 2010-12-09 22:32:02Z axel $
**
***********************************************************************/

/**
 * \class AirspaceWarningDistance
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Collection of distances to airspaces.
 *
 * This class holds a set of six distances to airspaces, used to warn the user
 * if he's getting too close to an airspace.
 *
 * \date 2009-2010
 *
 */

#ifndef AIRSPACE_WARNING_DISTANCE_H
#define AIRSPACE_WARNING_DISTANCE_H

#include "distance.h"

class AirspaceWarningDistance
{
public:

  Distance horClose;
  Distance horVeryClose;
  Distance verAboveClose;
  Distance verAboveVeryClose;
  Distance verBelowClose;
  Distance verBelowVeryClose;

  bool operator==(const AirspaceWarningDistance& x) const {
      return (
              horClose == x.horClose &&
              horVeryClose == x.horVeryClose &&
              verAboveClose == x.verAboveClose &&
              verAboveVeryClose == x.verAboveVeryClose &&
              verBelowClose == x.verBelowClose &&
              verBelowVeryClose == x.verBelowVeryClose
             );
  }

  bool operator!=(const AirspaceWarningDistance& x) const {
      return !operator==(x);
  }

};

#endif

