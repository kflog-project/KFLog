/***********************************************************************
**
**   flighttask.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FLIGHTTASK_H
#define FLIGHTTASK_H

#include <basemapelement.h>
#include <wp.h>

#include <qlist.h>

/**
 *
 * @author Heiner Lamprecht
 * @version
 */
class FlightTask : public BaseMapElement
{
  public:
    /** */
    FlightTask();
    /** */
    ~FlightTask();
    /** */
    void appendWaypoint(struct wayPoint* newPoint);
    /**
     * The waypoint-types.
     */
    enum WaypointType {NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                     End = 8, FreeP = 16, Landing = 32};

  private:
    /**
     * true, if the task is the original task of a flight as read from
     * the igc-file.
     */
    bool isOrig;
    QList<struct wayPoint> wpList;
    unsigned int task_end;
    unsigned int task_begin;
};

#endif
