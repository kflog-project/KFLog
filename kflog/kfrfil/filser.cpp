/***********************************************************************
**
**   filser.cpp
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

#include <../airport.h>

#include "filser.h"

#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include <klocale.h>
#include <ctype.h>

/**
 * The device-name of the port.
 */
char* portName = '\0';
int portID;

extern int breakTransfer;

/**
 * holds the port-settings at start of the application
 */
struct termios oldTermEnv;

/**
 * is used to change the port-settings
 */
struct termios newTermEnv;

/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the programm. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int signal)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  exit(-1);
}

Filser::Filser()
{
  //Set Flightrecorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = 100;             //maximum number of tasks
  _capabilities.maxNrWaypoints = 600;         //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = 10; //maximum number of waypoints per task
  _capabilities.maxNrPilots = 1;            //maximum number of pilots

  //_capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  //_capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  _capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  _capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  //_capabilities.supDlTask = true;          //supports downloading of tasks?
  //_capabilities.supUlTask = true;          //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  //End set capabilities.

  portID = -1;
}

Filser::~Filser()
{
}

/**
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Filser::getTransferMode()
{
  return FlightRecorderPluginBase::serial;
}

QString Filser::getLibName()  {  return "libkfrfil";  }

int Filser::getFlightDir(QList<FRDirEntry>* dirList)
{
  return FR_OK;
}

int Filser::downloadFlight(int flightID, int secMode, QString fileName)
{
  return FR_OK;
}


QString Filser::getRecorderSerialNo()
{
  return "???";
}

int Filser::openRecorder(const QString pName, int baud)
{
  speed_t speed;
  portName = (char *)pName.latin1();

  portID = open(portName, O_RDWR | O_NOCTTY);
  if(portID != -1) {
    //
    // Before we change any port-settings, we must establish a
    // signal-handler, which is used to restore the port-settings
    // after terminating the programm.
    //    Because a SIGKILL-signal removes the programm immediately,
    // the status of the port will be undefined.
    //
    struct sigaction sact;

    sact.sa_handler = releaseTTY;
    sigaction(SIGHUP, &sact, NULL);
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGPIPE, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);

    /*
     * Set the terminal mode of the serial line
     */

    // reading the current port-settings
    tcgetattr(portID, &newTermEnv);

    // storing the port-settings to restore them ...
    oldTermEnv = newTermEnv;


    /*
     * Do some common settup
     */
    cfmakeraw (&newTermEnv);
    newTermEnv.c_iflag |= (IGNBRK | IGNPAR);
    /*
     * No flow control at all :-(
     */
    newTermEnv.c_cflag &= ~(CRTSCTS | IXON | IXOFF);
    newTermEnv.c_cflag |= CLOCAL;

    if(baud >= 115200) speed = B115200;
    else if(baud >= 57600) speed = B57600;
    else if(baud >= 38400) speed = B38400;
    else if(baud >= 19200) speed = B19200;
    else if(baud >=  9600) speed = B9600;
    else if(baud >=  4800) speed = B4800;
    else if(baud >=  2400) speed = B2400;
    else if(baud >=  1800) speed = B1800;
    else if(baud >=  1200) speed = B1200;
    else if(baud >=   600) speed = B600;
    else if(baud >=   300) speed = B300;
    else if(baud >=   200) speed = B200;
    else if(baud >=   150) speed = B150;
    else if(baud >=   110) speed = B110;
    else speed = B75;

    cfsetospeed(&newTermEnv, speed);
    cfsetispeed(&newTermEnv, speed);

    _isConnected = true;
    return FR_OK;
    }
  else {
    warning(i18n("No logger found!"));
    _isConnected = false;
    return FR_ERROR;
  }
}

int Filser::closeRecorder()
{
  if (portID != -1) {
    tcsetattr(portID, TCSANOW, &oldTermEnv);
    close(portID);
    _isConnected = false;
    return FR_OK;
  }
  else {
    return FR_ERROR;
  }
}

/** NOT IMLEMENTED
    ============================================*/

int Filser::writeDeclaration(FRTaskDeclaration* taskDecl, QList<Waypoint> *taskPoints)
{
  return FR_NOTSUPPORTED;
}

int Filser::readDatabase()
{
  return FR_NOTSUPPORTED;
}

int Filser::readTasks(QList<FlightTask> *tasks)
{
  return FR_NOTSUPPORTED;
}

int Filser::writeTasks(QList<FlightTask> *tasks)
{
  return FR_NOTSUPPORTED;
}

int Filser::readWaypoints(QList<Waypoint> *waypoints)
{
  return FR_NOTSUPPORTED;
}

int Filser::writeWaypoints(QList<Waypoint> *waypoints)
{
  return FR_NOTSUPPORTED;
}

/**
 * Opens the recorder for other communication.
 */
int Filser::openRecorder(QString URL)
{
  return FR_NOTSUPPORTED;
}
