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

/*
 * Command bytes for communication with the lx device
 */
unsigned char STX = 0x02, /* Command prefix like AT for modems      */
  ACK = 0x06,      /* Response OK, if the crc check is ok           */
  SYN = 0x16,      /* Request for CONNECT                           */
  K = 'K' | 0x80,  /* get_extra_data()   - trailing fix sized block */
  L = 'L' | 0x80,  /* get_mem_sections() - the flight data is       */
                   /*                      retrieved in blocks      */
  M = 'M' | 0x80,  /* read_flight_index()- table of flights           */
  N = 'N' | 0x80,  /* def_mem()          - memory range of one flight */
  Q = 'Q' | 0x80,  /* read_mem_setting()                       */
  f = 'f' | 0x80;  /* get_logger_data()  - first block         */
                   /* f++ get_logger_data()  - next block          */

#define BUFSIZE 1024          /* Genaral buffer size             */
#define FLIGHT_INDEX_WIDTH 96 /* Bytes per record of the flight  */
                              /* index.                          */
#define LX_MEM_RET 7          /* Number of bytes returned by     */
                              /* wb(STX), wb(Q).                 */

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
    // control characters
    newTermEnv.c_cc[VMIN] = 0; // don't wait for a character
    newTermEnv.c_cc[VTIME] = 1; // wait at least 1 msec.
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

    // Activating the port-settings
    tcsetattr(portID, TCSANOW, &newTermEnv);
    
    _isConnected = true;
    return FR_OK;
    }
  else {
    warning(i18n("No logger found!"));
    _isConnected = false;
    return FR_ERROR;
  }
}

/**
 * Calculate the check sum
 */
char Filser::calc_crc(char d, char crc)
{
  char tmp, count, crcpoly=0x69; /* Static value for the calculation of the checksum. */

  for(count = 8; --count >= 0; d <<= 1) {
    tmp = crc ^ d;
    crc <<= 1;
    if(tmp < 0) {
      crc ^= crcpoly;
    }
  }
  return crc;
}

/**
 * Calculate the check sum on a buffer of bytes
 */
char Filser::calc_crc_buf(char *buf, unsigned int count)
{
  unsigned int i;
  char crc = 0xff;
  for(i = 0; i < count; i++) {
    crc = calc_crc(buf[i], crc);
  }
  return crc;
}

/*
 * Read the memory setup from the lx device.
 *
 * The lx devices do have a memory setup described with three bytes.
 * In the future this function might be necessary to disconnect from lx
 * devices with more memory. The CRC check will tell.
 */

char *Filser::read_data(char *buf_p, int count)
{
  int rc;
  switch (rc = read(portID, buf_p, count)) {
  case 0:
    warning("read_data(): EOF\n");
    break;
  case -1:
    warning("read_data(): ERROR\n");
    break;
  default:
    buf_p += rc;
    break;
  }
  return buf_p;  
}

int Filser::read_mem_setting()
{
  char *buf_p;
  char buf[BUFSIZE + 1];

  check4Device();
  wb(STX);
  wb(Q);

  buf_p = buf;
  while ((LX_MEM_RET + buf - buf_p) > 0) {
    buf_p = read_data(buf_p, LX_MEM_RET + buf - buf_p);
  }

  if(calc_crc_buf(buf, 6) != buf[6]) {
    warning("read_mem_setting(): Bad CRC\n");
    return 0;
  }  

  /*
   * Byte description and the values from my logger
   *
   *     (buf[0] == 0x0)       position on the start page, MS byte
   *     (buf[1] == 0x0)       position on the start page, LS byte
   *     (buf[2] == 0x6)       start page
   *     (buf[3] == 0x80)      position on the stop page, MS byte
   *     (buf[4] == 0x0)       position on the stop page, LS byte
   *     (buf[5] == 0x0b)      stop page
   *     (buf[6] == 0x41)      CRC value
   */
  warning("read_mem_setting(): all fine!!\n");

  return 1;
}

/*
 * Check presence of LX-device and make CONNECT
 *
 * Necessary wakeup before a command
 * if the LX-device is in TIMEOUT countdown
 * mode waiting for CONNECT.
 */
bool Filser::check4Device()
{
  bool rc = false;
  int ret;
  time_t t1;
  _errorinfo = "";

  t1 = time(NULL);
  while (!breakTransfer) {
    wb(SYN);
    ret = rb();
    if (ret == ACK) {
      warning("connected");
      rc = true;
      break;
    }
    else {
      // waiting 5 secs. for response
      if (time(NULL) - t1 > 5) {
        _errorinfo = i18n("No response from recorder within 5 seconds!\nDid you press READ/WRITE?");
        rc = false;
        break;
      }      
    }
  }
  return rc;
}

/*
 * write byte
 */
int Filser::wb(char c)
{
  if (write(portID, &c, 1) != 1) {
    return -1;
  }
  return 1;
}


/*
 * read byte
 */
int Filser::rb()
{
  int i;
  char buf;

  if (read(portID, &buf, 1) != 1) {
    return -1;
  }
  return buf;
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
