/***********************************************************************
**
**   cumulus.cpp
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

#include "cumulus.h"

/*
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
*/
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include "cumuluswaypointfile.h"

Cumulus::Cumulus(){
  //Set Flightrecorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = 1000;             //maximum number of tasks, limited by memory of device
  _capabilities.maxNrWaypoints = 1000;         //maximum number of waypoints, limited by memory of device
  _capabilities.maxNrWaypointsPerTask = 1000;  //maximum number of waypoints per task, limited by memory of device
  _capabilities.maxNrPilots = 2;               //maximum number of pilots

  _capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  _capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  //_capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  //_capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  _capabilities.supDlTask = true;          //supports downloading of tasks?
  //_capabilities.supUlTask = true;          //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  //End set capabilities.

  _tmpWaypointFile="";
}


Cumulus::~Cumulus(){
}

/**
 * Returns the name of the lib.
 */
QString Cumulus::getLibName() const
{
  return i18n("Cumulus/KFLog-e plugin, version %1").arg("0.1");  
}


/**
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Cumulus::getTransferMode() const
{
  return FlightRecorderPluginBase::URL;  
}


/**
 * Returns a list of recorded flights in this device.
 */
int Cumulus::getFlightDir(QList<FRDirEntry>*){
  return FR_NOTSUPPORTED;  
}


/**
 *
 */
int Cumulus::downloadFlight(int /*flightID*/, int /*secMode*/, const QString& /*fileName*/){
  return FR_NOTSUPPORTED;  
}


/**
 * get recorder info serial id
 */
QString Cumulus::getRecorderSerialNo(){
  return QString("Cumulus");       //TODO: replace with something more sensible, like Cumulus version?
}


/**
 * Opens the recorder for other communication.
 */
int Cumulus::openRecorder(const QString& URL){
  //Don't forget to set _isConnected to true if succeeded.

  //basicly, we check if cumulus is installed.
  if (KIO::NetAccess::exists(KURL(URL + "/opt/QtPalmtop/bin/cumulus"))) {
    _URL=URL;
    _isConnected=true;
    return FR_OK;
  } else {
    _isConnected=false;
    _errorinfo=i18n("<qt>Could not connect to PDA, or the following file did not exist:<br><br>%1</qt>").arg(URL + "/opt/QtPalmtop/bin/cumulus");
    return FR_ERROR;
  }  
}


/**
 * Closes the connection with the flightrecorder.
 */
int Cumulus::closeRecorder(){
  _isConnected=false;
  return FR_OK;
  //Don't forget to set _isConnected to false if succeeded.
}


/**
 * Read tasks from recorder
 */
int Cumulus::readTasks(QList<FlightTask> *tasks){
  int res=FR_NOTSUPPORTED;
  if (!_isConnected) {                  //check if we are connected
    _errorinfo=i18n("Not connected to PDA!");
    return FR_ERROR;
  }



  return res;
}


/**
 * Write tasks to recorder
 */
int Cumulus::writeTasks(QList<FlightTask> *tasks){
  return FR_NOTSUPPORTED;
}


/**
 * Read waypoints from recorder
 */
int Cumulus::readWaypoints(QList<Waypoint> *waypoints){
  int ret=0;
  
  if (!_isConnected) {                  //check if we are connected
    _errorinfo=i18n("Not connected to PDA!");
    return FR_ERROR;
  }

  //get the waypointfile from the device using a KIO slave
  _tmpWaypointFile="";
  //qDebug("Trying to access %s",QString(_URL + "/home/root/cumulus.kwp").latin1());
  if (!KIO::NetAccess::download(KURL(_URL + "/home/root/cumulus.kwp"),_tmpWaypointFile)) {
    warning("Something wend wrong trying to download the waypointfile!");
    _errorinfo=i18n("Could not download waypointfile from PDA.");
    return FR_ERROR;
  }
    
  //_tmpWaypointFile now contains the name of our temporary local waypointfile.
  //qDebug("Local waypointfile: %s",_tmpWaypointFile.latin1());

  //create a cumulus waypoint file object, use it to load the file, and return the result.
  CumulusWaypointFile* wpfile=new CumulusWaypointFile;
  ret=wpfile->load(_tmpWaypointFile, waypoints);
  //qDebug("number of waypoints read: %d",ret);
  delete wpfile;
  KIO::NetAccess::removeTempFile(_tmpWaypointFile);
  _tmpWaypointFile="";
  return ret;
}


/**
 * Write waypoints to recorder
 */
int Cumulus::writeWaypoints(QList<Waypoint> *waypoints){
  int ret=FR_NOTSUPPORTED;
  if (!_isConnected) {                  //check if we are connected
    _errorinfo=i18n("Not connected to PDA!");
    return FR_ERROR;
  }
  
  KTempFile f;                          //create a temporary file,
  f.setAutoDelete(true);                //  and set autodelete to true.
  if (f.status()!=0) {                  //check to see if a temporary file could be created.
    _errorinfo=i18n("Could not create temporary file. Please check your writepermissions.");
    return FR_ERROR;
  }
  CumulusWaypointFile* wpfile=new CumulusWaypointFile;  //create a new CumulusWaypointFile,
  ret=wpfile->save(f.name(), waypoints);                //   and use it to write our waypoints.
  if (ret>0) {                                          //writing to tempfile wend well
    if (KIO::NetAccess::upload(f.name(),KURL(_URL + "/home/root/cumulus.kwp"))) { //try to upload the file
      ret=FR_OK;
    } else {
      _errorinfo=i18n("Upload of temporary file to PDA failed.");
      ret=FR_ERROR;
    }
  }
  
  return ret;
}


/** NOT IMLEMENTED
    ============================================*/

/**
 * Opens the recorder for serial communication.
 */
int Cumulus::openRecorder(const QString& portName, int baud) {
  return FR_NOTSUPPORTED;
}


 /**
 * Write flight declaration to recorder
 */
int Cumulus::writeDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint> *taskPoints) {
  return FR_NOTSUPPORTED;
}

