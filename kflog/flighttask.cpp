/***********************************************************************
**
**   flighttask.cpp
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

#include "flighttask.h"

FlightTask::FlightTask()
: BaseMapElement("task", BaseMapElement::Task)
{

}

FlightTask::~FlightTask()
{

}

void FlightTask::appendWaypoint(struct wayPoint* newPoint)
{
  wpList.append(newPoint);

  if(task_end == 0 && wpList.count() > 2) {
    int loop = 0;

    for(int n = wpList.count() - 3; n >= 0; n--) {
      loop++;
      if(newPoint->latitude == wpList.at(n)->latitude &&
              newPoint->longitude == wpList.at(n)->longitude) {
        bool noTask = false;

        task_end = wpList.count() - 1;
        task_begin = n;

        wpList.at(task_end)->type = FlightTask::End;
        wpList.at(task_begin)->type = FlightTask::Begin;

        ///// Müll rausfiltern
        if(task_end - task_begin == 2) {
          if(wpList.at(task_begin)->longitude ==
                  wpList.at(task_begin + 1)->longitude &&
             wpList.at(task_begin)->latitude ==
                  wpList.at(task_begin + 1)->latitude) {
            noTask = true;
          }
        } else {
          for(unsigned int loop = task_begin + 2; loop < task_end; loop++) {
            if(wpList.at(loop - 1)->longitude == wpList.at(loop)->longitude &&
                 wpList.at(loop - 1)->latitude == wpList.at(loop)->latitude) {
               noTask = true;
            }
          }
        }

        if(noTask) {
          for(unsigned int m = 0; m < task_end; m++) {
            wpList.at(m)->type = FlightTask::FreeP;
          }
//          flightType = FlightTask::Abgebrochen;
//          KMsgBox errorBox(0,i18n("Task not valid"),
//               i18n("The task is not valid!\n"
//                    "Possibly it´s always the same waypoint."), 2, i18n("OK"));
//          errorBox.exec();
        } else {
          for(unsigned int m = 0; m < task_begin; m++) {
            wpList.at(m)->type = FlightTask::FreeP;
          }
          for(unsigned int m = task_begin + 1; m < task_end; m++) {
            wpList.at(m)->type = FlightTask::RouteP;
          }
        }
        break;
      }
    }
  } else {
    newPoint->type = FlightTask::FreeP;
  }
}
