/***********************************************************************
**
**   waypointelement.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINTELEMENT_H
#define WAYPOINTELEMENT_H

#include <qstring.h>
#include <qpoint.h>

/**
  *@author Harald Maier
  */

class WaypointElement {
public: 
	WaypointElement();
	~WaypointElement();
public: // Public attributes
  /** the name seen by the logger */
  QString name;
  /** long name or description (internal only) */
  QString description;
  /** ICAO name */
  QString icao;
  /** */
  QString comment;
  /** internal type id */
  int type;
  /** internal surface id */
  int surface;
  /** */
  int runway;
  /** */
  int length;
  /** */
  int elevation;
  /** */
  double frequency;
  /** LAT/LONG */
  QPoint pos;
  /** flag for landable*/
  bool isLandable;
};

#endif
