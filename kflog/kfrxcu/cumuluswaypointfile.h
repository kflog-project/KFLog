/***********************************************************************
**
**   cumuluswaypointfile.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CUMULUSWAYPOINTFILE_H
#define CUMULUSWAYPOINTFILE_H

#define FO_NOTSUPPORTED -2
#define FO_ERROR -1
#define FO_OK 1

#include <qobject.h>
#include "../waypoint.h"
#include <qlist.h>
#include <qstring.h>

/**This class contains all there is to know about loading and saving waypoint files in Cumulus format.
  *@author André Somers
  */

class CumulusWaypointFile : public QObject  {
public: 
	CumulusWaypointFile();
	~CumulusWaypointFile();
  /** Tries to save the waypoints in the list, and returns NOTIMPLEMENTED (-2), ERROR (-1) or OK (1). */
  int save(QString file, QList<Waypoint> *waypoints);
  /** loads the waypoints into the QList, and returns the number of loaded waypoints. */
  int load(QString file, QList<Waypoint> *waypoints);
};

#endif
