/***********************************************************************
**
**   soaringpilot.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SOARINGPILOT_H
#define SOARINGPILOT_H

#include <qstring.h>

#include <../frstructs.h>
#include <../flighttask.h>

/**Implementation for SoaringPilot.

  *@author Harald Maier
  */

class SoaringPilot
{
public: 
  SoaringPilot();
  ~SoaringPilot();
  /** No descriptions */
  int closeLogger();
  /** No descriptions */
  int openLogger(char *port, int baud);
  int downloadWaypoints(QList<Waypoint> *waypoints);
  int uploadWaypoints(QList<Waypoint> *waypoints);
  int downloadTasks(QList<FlightTask> *tasks);
  int uploadTasks(QList<FlightTask> *tasks);
private: // Private methods
  /** write a file like structure to the device */
  int writeFile(QStringList &file);
  /** read a file like structure from the device */
  int readFile(QStringList &file);
  int coordToDegree(QString &s);
  QString degreeToDegMin(int d, bool isLat);
  QString degreeToDegMinSec(int d, bool isLat);
  int feetToMeter(QString &s);
  QString meterToFeet(int m);
};

#endif
