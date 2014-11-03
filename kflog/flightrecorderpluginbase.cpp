/***********************************************************************
**
**   flightrecorderpluginbase.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "flightrecorderpluginbase.h"

FlightRecorderPluginBase::transferStruct FlightRecorderPluginBase::transferData [] =
{
#ifndef _WIN32
    {bps00075,     75,     B75},
    {bps00150,    150,    B150},
    {pbs00200,    200,    B200},
    {bps00300,    300,    B300},
    {bps00600,    600,    B600},
    {bps01200,   1200,   B1200},
    {bps01800,   1800,   B1800},
    {bps02400,   2400,   B2400},
    {bps04800,   4800,   B4800},
    {bps09600,   9600,   B9600},
    {bps19200,  19200,  B19200},
    {bps38400,  38400,  B38400},
    {bps57600,  57600,  B57600},
    {bps115200,115200, B115200}
#endif
};

int FlightRecorderPluginBase::transferDataMax = 14;

FlightRecorderPluginBase::FlightRecorderPluginBase( QObject *parent ) :
  QObject( parent ),
  _isConnected(false),
  _errorinfo("")
{
  //initialize capabilities to none. This class is never instanciated, so this could be skipped
  _capabilities.maxNrTasks = 0;             //maximum number of tasks
  _capabilities.maxNrWaypoints = 0;         //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = 0;  //maximum number of waypoints per task
  _capabilities.maxNrPilots = 0;            //maximum number of pilots
  _capabilities.transferSpeeds = bps00000;  //supported transfer speeds (all)

  _capabilities.supDlWaypoint = false;      //supports downloading of waypoints?
  _capabilities.supUlWaypoint = false;      //supports uploading of waypoints?
  _capabilities.supDlFlight = false;        //supports downloading of flights?
  _capabilities.supUlFlight = false;        //supports uploading of flights?
  _capabilities.supSignedFlight = false;    //supports downloading in of signed flights?
  _capabilities.supDlTask = false;          //supports downloading of tasks?
  _capabilities.supUlTask = false;          //supports uploading of tasks?
  _capabilities.supExportDeclaration = false;//supports export of declaration?
  _capabilities.supUlDeclaration = false;   //supports uploading of declarations?
  _capabilities.supDspPilotName = false;    //supports display of pilot name
  _capabilities.supDspCoPilotName = false;  //supports display of copilot name
  _capabilities.supDspRecorderType = false; //supports display of recorder type
  _capabilities.supDspSerialNumber = false; //supports display of serial number
  _capabilities.supDspDvcID = false;        //supports display of device ID
  _capabilities.supDspSwVersion = false;    //supports display of software version
  _capabilities.supDspGliderID = false;     //supports display of glider ID
  _capabilities.supDspGliderType = false;   //supports display of glider type
  _capabilities.supDspCompetitionID = false;//supports display of competition ID
  _capabilities.supAutoSpeed = false;       //supports automatic transfer speed detection

  _capabilities.supEditGliderID = false;    //supports changing the glider ID
  _capabilities.supEditGliderType = false;  //supports changing the glider type
  _capabilities.supEditGliderPolar = false; //supports editing the glider polar
  _capabilities.supEditUnits = false;       //supports changing units (speed, altitude, distance, ...)
  _capabilities.supEditPilotName = false;   //supports changing the pilot name
  _capabilities.supEditCoPilotName = false; //supports changing the copilot name
  _capabilities.supEditGoalAlt = false;     //supports changing arrival altitude
  _capabilities.supEditArvRadius = false;   //supports changing arrival radius
  _capabilities.supEditAudio = false;       //supports changing vario audio settings
  _capabilities.supEditLogInterval = false; //supports changing logging intervals
}


FlightRecorderPluginBase::~FlightRecorderPluginBase()
{
}

/**
 * Returns additional info about an error that occurred (optional).
 * _errorinfo is reset afterwards.
 */
QString FlightRecorderPluginBase::lastError()
{
  QString error = _errorinfo;
  _errorinfo = "";
  return error;
}

void FlightRecorderPluginBase::setParent( QObject* parent )
{
  _parent = parent;
}
