/***********************************************************************
**
**   taskdataprint.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASKDATAPRINT_H
#define TASKDATAPRINT_H

class FlightTask;

/**
 * @author Heiner Lamprecht
 * @version $Id$
 */
class TaskDataPrint
{
  public:
    /** */
    TaskDataPrint(FlightTask* task);
    /** */
    ~TaskDataPrint();

//  private:
//  /** */
//  void __printPositionData(QPainter* painter, struct wayPoint* cPoint,
//          int yPos);
};

#endif
