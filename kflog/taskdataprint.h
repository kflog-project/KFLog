/***********************************************************************
**
**   taskdataprint.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASK_DATA_PRINT_H
#define TASK_DATA_PRINT_H

#include "flighttask.h"

/**
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Prints out the current task data.
 *
 * Prints out the current task data.
 *
 * \data 2002-2011
 *
 * \version $Id$
 */
class TaskDataPrint
{
  public:

  TaskDataPrint(FlightTask* task);

  virtual ~TaskDataPrint();
};

#endif
