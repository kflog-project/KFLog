/***********************************************************************
**
**   garminfr.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Thomas Nielsen, André Somers
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <iostream>
#include <termios.h>
#include <unistd.h>

#include <QtCore>

#include "../frstructs.h"
#include "garminfr.h"


GarminFR::GarminFR( QObject *parent ) : FlightRecorderPluginBase( parent )
{
  //Set Flightrecorders capabilities. Defaults are 0 and false.

  //CAN'T FIND THIS DATA IN THE ORIGINAL PLUGIN?! WHAT ARE THE CORRECT VALUES?

  //_capabilities.maxNrTasks = 1000;             //maximum number of tasks, limited by memory of device
  //_capabilities.maxNrWaypoints = 1000;         //maximum number of waypoints, limited by memory of device
  //_capabilities.maxNrWaypointsPerTask = 1000;  //maximum number of waypoints per task, limited by memory of device
  //_capabilities.maxNrPilots = 2;               //maximum number of pilots

  //_capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  //_capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  //_capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  //_capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  //_capabilities.supDlTask = true;          //supports downloading of tasks?
  //_capabilities.supUlTask = true;          //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  //_capabilities.supDspSerialNumber = true;
  _capabilities.supDspRecorderType = true;
  //_capabilities.supDspPilotName = true;
  //_capabilities.supDspGliderType = true;
  //_capabilities.supDspGliderID = true;
  //_capabilities.supDspCompetitionID = true;
  //End set capabilities.

  portName = "\0";
  portID = -1;
}

GarminFR::~GarminFR()
{
}

/**
 * Returns the name of the lib.
 */
QString GarminFR::getLibName() const
{
  return "libkfrgmn";
}

/**
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode GarminFR::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

/**
 * Returns a list of recorded flights in this device.
 */
int GarminFR::getFlightDir(QList<FRDirEntry*>* dirList)
{
  qDeleteAll( *dirList );
  dirList->clear();

  qDebug("opening port");
  gmn.openComm(portName);

  qDebug("requesting tracklog");
  gmn.getTrackLog();

  gmn.closeComm();

  return FR_OK;
}

/**
 *
 */
int GarminFR::downloadFlight( int /*flightID*/,
                              int /*secMode*/,
                              const QString& /*fileName*/ )
{
  return FR_NOTSUPPORTED;
}


/**
  * get recorder basic data
  */
int GarminFR::getBasicData( FR_BasicData& data )
{
  _basicData.serialNumber = "???";
  _basicData.recorderType = "Garmin";
  _basicData.pilotName = "???";
  _basicData.gliderType = "???";
  _basicData.gliderID = "???";
  _basicData.competitionID = "???";

  data = _basicData;

  return FR_OK;
}

int GarminFR::getConfigData(FR_ConfigData& /*data*/)
{
  return FR_NOTSUPPORTED;
}

int GarminFR::writeConfigData( FR_BasicData& /*basicdata*/,
                               FR_ConfigData& /*configdata*/ )
{
  return FR_NOTSUPPORTED;
}

/**
 * Opens the recorder for serial communication.
 */
int GarminFR::openRecorder(const QString& portName, int /*baud*/)
{
  this->portName = portName.toLatin1().constData();

  if( !gmn.openComm( this->portName ) )
    {
      qWarning( "No logger found!" );
      return FR_ERROR;
    }

  return FR_OK;
}

/**
 * Closes the connection with the flight recorder.
 */
int GarminFR::closeRecorder()
{
  _isConnected = false;
  return FR_OK;
}

/**
 * Read tasks from recorder
 */
int GarminFR::readTasks(QList<FlightTask*> * /*tasks*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Write tasks to recorder
 */
int GarminFR::writeTasks(QList<FlightTask*> * /*tasks*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Read waypoints from recorder
 */
int GarminFR::readWaypoints(QList<Waypoint*> * /*waypoints*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Write waypoints to recorder
 */
int GarminFR::writeWaypoints(QList<Waypoint*> * /*waypoints*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Opens the recorder for other communication.
 */
int GarminFR::openRecorder(const QString& /*URL*/)
{
  return FR_NOTSUPPORTED;
}

 /**
 * Write flight declaration to recorder
 */
int GarminFR::writeDeclaration( FRTaskDeclaration * /*taskDecl*/,
                                QList<Waypoint*> * /*taskPoints*/,
                                const QString& /*name*/ )
{
  return FR_NOTSUPPORTED;
}

 /**
 * Export flight declaration to file
 */
int GarminFR::exportDeclaration( FRTaskDeclaration * /*taskDecl*/,
                                QList<Waypoint*> * /*taskPoints*/,
                                const QString& /*name*/ )
{
  return FR_NOTSUPPORTED;
}

/*************************************************************************
**
** begin of logger-specific implementation
**
*************************************************************************/


 /*

QByteArray getFlight(char* pName, int flightID, char* tmpFileName, int* ret)
{
  extern char* portName;

  cerr << "getFlight(" << pName << ", " << flightID << ", "
       << tmpFileName << ")\n";

  QByteArray bArray;

  portName = pName;

  QFile tmpFile(tmpFileName);
  QTextStream outStream(bArray, IO_WriteOnly);
  QTextStream inStream(&tmpFile);

  while(!inStream.eof())
      outStream << inStream.readLine();

  *ret = 1;
  return bArray;
}

int downloadFlight(char* pName, int flightID, int secMode, char* fileName)
{
  extern char* portName;

  portName = pName;

  return 1;
}

QString getRecorderName(char* pName)
{
  extern char* portName;
  portName = pName;
  char * strModel;

  gmn.openComm(portName);
  gmn.getModel();
  gmn.closeComm();

  return strModel;
}

int openRecorder(char* pName)
{
  extern char* portName;

  portName = pName;

  if (!gmn.openComm(portName))
    {
      warning(tr("No logger found!"));
      return -1;
    }

  return 0;
}
   */


