/***********************************************************************
**
**   garminfr.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Thomas Nielsen, André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "garminfr.h"
#include <iostream>

#include <unistd.h>

#include <klocale.h>
#include <qarray.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include "../frstructs.h"
#include <termios.h>


GarminFR::GarminFR(){
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
  //End set capabilities.

  portName = '\0';
  portID = -1;

}

GarminFR::~GarminFR(){
}

/**
 * Returns the name of the lib.
 */
QString GarminFR::getLibName(){
  return "libkfrgmn";
}


/**
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode GarminFR::getTransferMode(){
  return FlightRecorderPluginBase::serial;
}


/**
 * Returns a list of recorded flights in this device.
 */
int GarminFR::getFlightDir(QList<FRDirEntry>* dirList){
  dirList->clear();


  warning(i18n("opening port"));
  gmn.openComm(portName);

  warning(i18n("requesting tracklog"));
  gmn.getTrackLog();

  gmn.closeComm();

  return FR_OK;
  
}


/**
 *
 */
int GarminFR::downloadFlight(int /*flightID*/, int /*secMode*/, const QString& /*fileName*/){
  return FR_NOTSUPPORTED;
}


/**
 * get recorder info serial id
 */
QString GarminFR::getRecorderSerialNo(){
  return QString("");      //
}


/**
 * Opens the recorder for serial communication.
 */
int GarminFR::openRecorder(const QString& portName, int baud) {

  this->portName = (char*)portName.latin1();

  if (!gmn.openComm(this->portName))
    {
      warning(i18n("No logger found!"));
      return FR_ERROR;
    }

  return FR_OK;

}



/**
 * Closes the connection with the flightrecorder.
 */
int GarminFR::closeRecorder(){
  _isConnected=false;
  return FR_OK;
  //Don't forget to set _isConnected to false if succeeded.
}


/**
 * Read tasks from recorder
 */
int GarminFR::readTasks(QList<FlightTask> * /*tasks*/){
  return FR_NOTSUPPORTED;
}


/**
 * Write tasks to recorder
 */
int GarminFR::writeTasks(QList<FlightTask> * /*tasks*/){
  return FR_NOTSUPPORTED;
}


/**
 * Read waypoints from recorder
 */
int GarminFR::readWaypoints(QList<Waypoint> * /*waypoints*/){
  return FR_NOTSUPPORTED;
}


/**
 * Write waypoints to recorder
 */
int GarminFR::writeWaypoints(QList<Waypoint> * /*waypoints*/){
  return FR_NOTSUPPORTED;
}


/** NOT IMLEMENTED
    ============================================*/

/**
 * Opens the recorder for other communication.
 */
int GarminFR::openRecorder(const QString& /*URL*/){
  return FR_NOTSUPPORTED;
}

 /**
 * Write flight declaration to recorder
 */
int GarminFR::writeDeclaration(FRTaskDeclaration * /*taskDecl*/, QList<Waypoint> * /*taskPoints*/) {
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
      warning(i18n("No logger found!"));
      return -1;
    }

  return 0;
}
   */

   
