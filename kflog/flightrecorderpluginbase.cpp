/***********************************************************************
**
**   flightrecorderpluginbase.cpp
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

#include "flightrecorderpluginbase.h"

FlightRecorderPluginBase::FlightRecorderPluginBase(){
  _isConnected=false;
  _errorinfo="";

  //initialize capabilities to none.
  _capabilities.maxNrTasks = 0;             //maximum number of tasks
  _capabilities.maxNrWaypoints = 0;         //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = 0;  //maximum number of waypoints per task
  _capabilities.maxNrPilots = 0;            //maximum number of pilots

  _capabilities.supDlWaypoint = false;      //supports downloading of waypoints?
  _capabilities.supUlWaypoint = false;      //supports uploading of waypoints?
  _capabilities.supDlFlight = false;        //supports downloading of flights?
  _capabilities.supUlFlight = false;        //supports uploading of flights?
  _capabilities.supSignedFlight = false;    //supports downloading in of signed flights?
  _capabilities.supDlTask = false;          //supports downloading of tasks?
  _capabilities.supUlTask = false;          //supports uploading of tasks?
  _capabilities.supUlDeclaration = false;   //supports uploading of declarations?

}


FlightRecorderPluginBase::~FlightRecorderPluginBase(){
}

/**
 * Returns additional info about an error that occured (optional).
 * _errorinfo is reset afterwards.
 */
QString FlightRecorderPluginBase::lastError(){
  QString ret=_errorinfo;
  _errorinfo="";
  return ret;
}
