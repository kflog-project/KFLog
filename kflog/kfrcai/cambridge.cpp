/***********************************************************************
**
**   cambridge.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2007 by Hendrik Hoeth
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/*
 * FIXME:
 *    - I don't use error checking yet, but so far I simply ignore those bytes
 */

#include "cambridge.h"
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <math.h>

#define STX        0x03

// command mode
#define CMD_MODE   1
// download mode
#define DN_MODE    2
// upload mode with short replies (<=255 byte)
#define UPS_MODE   3
// upload mode with long replies (any length)
#define UPL_MODE   4

#define TIMEOUT_ERROR  -1
#define CHECKSUM_ERROR -2


// Cambridge waypoint attributes are using bit arrays
#define CAI_TURNPOINT        1
#define CAI_AIRFIELD         2
#define CAI_MARKPOINT        4 
#define CAI_LANDINGPOINT     8
#define CAI_STARTPOINT      16
#define CAI_FINISHPOINT     32
#define CAI_HOMEPOINT       64
#define CAI_THERMALPOINT   128
#define CAI_WAYPOINT       256
#define CAI_AIRSPACE       512


const char* c36 = "0123456789abcdefghijklmnopqrstuvwxyz";

/*
 * name and ID of the port
 */
const char* portName = '\0';
int portID;

/*
 * holds the port-settings at start of the application
 */
struct termios oldTermEnv;

/*
 * is used to change the port-settings
 */
struct termios newTermEnv;

/*
 * Needed to reset the serial port in any case of unexpected exiting
 * of the programm. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int /* signal*/)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  //exit(-1);
}

void debugHex (const void* buf, unsigned int size)
{
  for (unsigned int ix1=0; ix1 < size; ix1+=0x10)
  {
    QString line;
    line.sprintf ("%03X:  ", ix1);
    for (int ix2=0; ix2<0x10; ix2++)
    {
      QString byte;
      byte.sprintf("%02X ", ((unsigned char*)buf)[ix1+ix2]);
      line += byte;
    }
    line += "    ";
    for (int ix2=0; ix2<0x10; ix2++)
    {
      if (isprint (((unsigned char*)buf)[ix1+ix2]))
        line += ((unsigned char*)buf)[ix1+ix2];
      else
        line += ' ';
    }
    qDebug (line);
  }
}

QString extractString(unsigned char* buf, int start, int count)
{
  unsigned char foo[2048];
  for (int i=start ; i<start+count ; i++)
    foo[i-start]=buf[i];
  foo[count]=0x00;
  return (QString)(char *)foo;
}

int extractInteger(unsigned char* buf, int start, int count)
{
  int foo = (int)buf[start+count-1];
  if (count > 1)
    foo += (int)buf[start+count-2]*256;
  if (count > 2)
    foo += (int)buf[start+count-3]*256*256;
  if (count > 3)
    foo += (int)buf[start+count-4]*256*256*256;
  if (count > 4)
  {
    warning("extractInteger(): Not supported for more than 4 byte");
    return -1;
  }
  return foo;
}

Cambridge::Cambridge()
{
  _capabilities.transferSpeeds = bps01200 |  //supported transfer speeds
                                 bps02400 |
                                 bps04800 |
                                 bps09600 |
                                 bps19200 |
                                 bps38400 |
                                 bps57600 ; //  bps115200 doesn't work for me?!?
  _capabilities.supDlWaypoint = true;        //supports downloading of waypoints?
  _capabilities.supUlWaypoint = true;        //supports uploading of waypoints?
  _capabilities.supDspRecorderType = true;   //supports display of recorder type
  _capabilities.supDspSerialNumber = true;   //supports display of serial number
  _capabilities.supDspPilotName = true;
  _capabilities.supDspGliderType = true;
  _capabilities.supDspGliderID = true;
  _capabilities.supDlFlight = true;          //supports downloading of flights?
  _capabilities.supSignedFlight = true;      //supports downloading in of signed flights?
  portID = -1;
}

Cambridge::~Cambridge()
{
}

FlightRecorderPluginBase::TransferMode Cambridge::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

QString Cambridge::getLibName() const
{
  return "libkfrcai";
}

int Cambridge::openRecorder(const QString& pName, int baud)
{
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
    newTermEnv.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    newTermEnv.c_oflag &= ~OPOST;
    newTermEnv.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    /*
     * No flow control at all :-(
     */
    newTermEnv.c_cflag &= ~(CSIZE | PARENB | CRTSCTS | IXON | IXOFF);
    newTermEnv.c_cflag |= (CS8 | CLOCAL);

    // control characters
    newTermEnv.c_cc[VMIN] = 0; // don't wait for a character
    newTermEnv.c_cc[VTIME] = 1; // wait at least 1 msec.

    cfsetospeed(&newTermEnv, B4800);
    cfsetispeed(&newTermEnv, B4800);

    // Activating the port-settings
    tcsetattr(portID, TCSANOW, &newTermEnv);

    // now change baud rate to user setting
    wb(STX);
    wait_ms(100);
    switch (baud) {
      case 1200:
        sendCommand("baud 4");
        qDebug("baud 4");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B1200);
        cfsetispeed(&newTermEnv, B1200);
        break;
      case 2400:
        sendCommand("baud 5");
        qDebug("baud 5");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B2400);
        cfsetispeed(&newTermEnv, B2400);
        break;
      case 4800:
        sendCommand("baud 6");
        qDebug("baud 6");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B4800);
        cfsetispeed(&newTermEnv, B4800);
        break;
      case 9600:
        sendCommand("baud 7");
        qDebug("baud 7");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B9600);
        cfsetispeed(&newTermEnv, B9600);
        break;
      case 19200:
        sendCommand("baud 8");
        qDebug("baud 8");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B19200);
        cfsetispeed(&newTermEnv, B19200);
        break;
      case 38400:
        sendCommand("baud 9");
        qDebug("baud 9");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B38400);
        cfsetispeed(&newTermEnv, B38400);
        break;
      case 57600:
        sendCommand("baud 10");
        qDebug("baud 10");
        wait_ms(50);
        cfsetospeed(&newTermEnv, B57600);
        cfsetispeed(&newTermEnv, B57600);
        break;
//      case 115200:
//        sendCommand("baud 11");
//        qDebug("baud 11");
//        wait_ms(50);
//        cfsetospeed(&newTermEnv, B115200);
//        cfsetispeed(&newTermEnv, B115200);
//        break;
    }

    // Activating the port-settings
    qDebug("activating port setting ...");
    tcsetattr(portID, TCSANOW, &newTermEnv);
    qDebug("Done.");
    wait_ms(50);

    _isConnected = true;
    return FR_OK;
    }
  else {
    warning(i18n("No logger found!"));
    _isConnected = false;
    return FR_ERROR;
  }
}

int Cambridge::openRecorder(const QString& URL)
{
  return FR_NOTSUPPORTED;
}

int Cambridge::closeRecorder()
{
  if (portID != -1) {
    // before we close the connection, set the baud rate
    // of the CAI 302 back to its default value and let
    // it provide nmea output.
    if (_isConnected)
    {
      wb(STX);
      wait_ms(100);
      sendCommand("baud 6");
      wait_ms(50);
      cfsetospeed(&newTermEnv, B4800);
      cfsetispeed(&newTermEnv, B4800);
      tcsetattr(portID, TCSANOW, &newTermEnv);
      wait_ms(50);
      sendCommand("pnp");
    }

    tcsetattr(portID, TCSANOW, &oldTermEnv);
    close(portID);
    _isConnected = false;
    return FR_OK;
  }
  else {
    return FR_ERROR;
  }
}

int Cambridge::getBasicData(FR_BasicData& data)
{
  unsigned char reply[2048];
  int replysize = 0;

  // go into command mode, then switch to upload mode
  wb(STX);
  wait_ms(100);
  sendCommand("upload");
  wait_ms(100);

  replysize = readReply("w", UPS_MODE, reply);
  if (replysize==TIMEOUT_ERROR) return FR_ERROR;
  _basicData.serialNumber = extractString(reply,15,3);
  _basicData.recorderType = QString("Cambridge 300");

  replysize = readReply("o 0", UPS_MODE, reply);
  if (replysize==TIMEOUT_ERROR) return FR_ERROR;
  _basicData.pilotName = extractString(reply,0,24);

  replysize = readReply("g 0", UPS_MODE, reply);
  if (replysize==TIMEOUT_ERROR) return FR_ERROR;
  _basicData.gliderType = extractString(reply,0,12);
  _basicData.gliderID = extractString(reply,12,12);

  _basicData.competitionID = QString("???");
  data = _basicData;
  return FR_OK;
}

int Cambridge::getFlightDir(QPtrList<FRDirEntry>* dirList)
{
  unsigned char reply[2048];
  int replysize = 0;

  // clear current flight list -- we don't want duplicates
  if (dirList->count()>0) dirList->clear();

  // go into command mode, then switch to upload mode
  wb(STX);
  wait_ms(100);
  sendCommand("upload");
  wait_ms(100);

  // loop over all flights in the recorder. Flight information
  // is returned in blocks of 8 flights, so we need to keep asking
  // for new blocks until we got the last flight.
  int done = 0;
  int offset = 0;
  while (!done)
  {
    QString foo;
    foo.setNum(196+offset);
    foo.prepend("b ");
    replysize=0;
    while (replysize<=0) {
      replysize = readReply(foo, UPL_MODE, reply);
      if (replysize==TIMEOUT_ERROR) return FR_ERROR;
    }

    const int Nflights = extractInteger(reply,0,1);
    if (offset==0)
      qDebug("There are %d flights on the recorder", Nflights);

    int maxflights = Nflights-8*offset;
    if (maxflights>8) maxflights=8;
    for (int i=0 ; i<maxflights ; i++)
    {
      FRDirEntry* entry = new FRDirEntry;

      struct tm startTime, stopTime;
      startTime.tm_year = extractInteger(reply,  1+i*36, 1)+100;
      startTime.tm_mon  = extractInteger(reply,  2+i*36, 1)-1;
      startTime.tm_mday = extractInteger(reply,  3+i*36, 1);
      startTime.tm_hour = extractInteger(reply,  4+i*36, 1);
      startTime.tm_min  = extractInteger(reply,  5+i*36, 1);
      startTime.tm_sec  = extractInteger(reply,  6+i*36, 1);
      stopTime.tm_year  = extractInteger(reply,  7+i*36, 1)+100;
      stopTime.tm_mon   = extractInteger(reply,  8+i*36, 1)-1;
      stopTime.tm_mday  = extractInteger(reply,  9+i*36, 1);
      stopTime.tm_hour  = extractInteger(reply, 10+i*36, 1);
      stopTime.tm_min   = extractInteger(reply, 11+i*36, 1);
      stopTime.tm_sec   = extractInteger(reply, 12+i*36, 1);
      entry->pilotName  = extractString (reply, 13+i*36, 24);

      entry->firstTime = startTime;
      entry->lastTime = stopTime;

      time_t startTime_t = mktime(&startTime);
      time_t stopTime_t = mktime(&stopTime);
      entry->duration = stopTime_t - startTime_t;

      // the glider type is not contained in the flight list; we take it from basic data
      entry->gliderID = _basicData.gliderID;
      entry->gliderType = _basicData.gliderType;

      dirList->append(entry);
    }
    if (Nflights>dirList->count())
      offset++;
    else
      done = 1;
  }

  // Now that we have a list of all flights we can give them
  // filenames:
  for (size_t i=0; i<dirList->count(); i++) {
    // Count flights that occurred on the same day
    int y = dirList->at(i)->firstTime.tm_year;
    int m = dirList->at(i)->firstTime.tm_mon;
    int d = dirList->at(i)->firstTime.tm_mday;
    int dayflightcounter = 1;
    for (size_t j=i+1 ; j<dirList->count(); j++)
      if (y == dirList->at(j)->firstTime.tm_year &&
          m == dirList->at(j)->firstTime.tm_mon  &&
          d == dirList->at(j)->firstTime.tm_mday) dayflightcounter++;
    // create igc filenames
    dirList->at(i)->shortFileName.sprintf("%c%c%c%c%s%c.igc",
                                 c36[dirList->at(i)->firstTime.tm_year % 10],
                                 c36[dirList->at(i)->firstTime.tm_mon + 1],
                                 c36[dirList->at(i)->firstTime.tm_mday],
                                 'c',
                                 (const char*)_basicData.serialNumber,
                                 c36[dayflightcounter]);
    dirList->at(i)->longFileName.sprintf("%d-%.2d-%.2d-%s-%s-%.2d.igc",
                                 dirList->at(i)->firstTime.tm_year + 1900,
                                 dirList->at(i)->firstTime.tm_mon + 1,
                                 dirList->at(i)->firstTime.tm_mday,
                                 "cam",
                                 (const char*)_basicData.serialNumber,
                                 c36[dayflightcounter]);
    warning(dirList->at(i)->longFileName + "   " + dirList->at(i)->shortFileName);
  }
}

int Cambridge::downloadFlight(int flightID, int secMode, const QString& fileName)
{
  unsigned char reply[2048];
  int replysize = 0;

  // go into command mode, then switch to upload mode
  wb(STX);
  wait_ms(100);
  sendCommand("upload");
  wait_ms(100);

  // initiate file transfer
  QString foo;
  foo.setNum(64+flightID);
  foo.prepend("b ");
  replysize = readReply(foo, UPL_MODE, reply);
  if (replysize==TIMEOUT_ERROR) return FR_ERROR;
  if (extractString(reply,0,1)!="Y")
  {
    warning("downloadFlight(): Flight %d not available", flightID);
    return FR_ERROR;
  }
  int bpp = extractInteger(reply,1,2);

  // get flight data
  int done = 0;
  QString igcdata;
  while (!done)
  {
    replysize = readReply("b n", UPL_MODE, reply);
    if (replysize==TIMEOUT_ERROR) return FR_ERROR;
    int validbytes = extractInteger(reply,0,2);
    if (validbytes!=bpp) done=1;
    igcdata.append(extractString(reply,2,validbytes));
  }

  // get signature
  replysize = readReply("b s", UPL_MODE, reply);
  if (replysize==TIMEOUT_ERROR) return FR_ERROR;
  int validbytes = extractInteger(reply,0,2);
  igcdata.append(extractString(reply,2,validbytes));

  // write file
  QFile f(fileName);
  if (f.open(IO_WriteOnly))
  {
    f.writeBlock((const char *)igcdata, igcdata.length());
    f.close();
    return FR_OK;
  }
  else
  {
    warning(i18n("cannot open igc file ") + fileName);
    return FR_ERROR;
  }
}

int Cambridge::writeDeclaration(FRTaskDeclaration *taskDecl, QPtrList<Waypoint> *taskPoints)
{
  return FR_NOTSUPPORTED;
}

int Cambridge::readDatabase()
{
  return FR_NOTSUPPORTED;
}

int Cambridge::readTasks(QPtrList<FlightTask> *tasks)
{
  return FR_NOTSUPPORTED;
}

int Cambridge::writeTasks(QPtrList<FlightTask> *tasks)
{
  return FR_NOTSUPPORTED;
}

int Cambridge::readWaypoints(QPtrList<Waypoint> *waypoints)
{
  unsigned char reply[2048];
  int replysize = 0;

  // go into command mode, then switch to upload mode
  wb(STX);
  wait_ms(100);
  sendCommand("upload");
  wait_ms(100);

  replysize = readReply("c", UPS_MODE, reply);
  if (replysize==TIMEOUT_ERROR) return FR_ERROR;
  unsigned int Npoints = extractInteger(reply,0,2);
  qDebug("There are %d waypoints on the recorder", Npoints);

  int Wsize = extractInteger(reply,2,1);
  if (Wsize != 38) {
    qDebug("Waypoint size is %d bytes, should be 38!", Wsize);
    return FR_ERROR;
  }

  Waypoint * frWp;
  for (size_t i=0; i<Npoints; i++) {
    QString cmd;
    cmd.sprintf ("c %d", i);
    replysize = readReply(cmd, UPS_MODE, reply);
    if (replysize==TIMEOUT_ERROR) return FR_ERROR;
    int lat = extractInteger(reply,  0,  4) -  54000000; // Equator is at 54000000 TTOM
    int lon = extractInteger(reply,  4,  4) - 108000000; // Greenwich is at 108000000 TTOM
    int elv = extractInteger(reply,  8,  2);
    int  id = extractInteger(reply, 10,  2);
    int att = extractInteger(reply, 12,  2);
    QString name   = extractString(reply, 14, 12).stripWhiteSpace();
    QString remark = extractString(reply, 26, 12).stripWhiteSpace();
    // debugHex (reply,64);
    // qDebug ("lat = %d", lat);
    // qDebug ("lon = %d", lon);
    // qDebug ("elv = %d", elv);
    // qDebug (" id = %d",  id);
    // qDebug ("att = %d", att);
    // qDebug ("name = "+name);
    // qDebug ("remark = "+remark);
    int type = BaseMapElement::NotSelected;
    bool landable = false;
    if (att & CAI_AIRFIELD) {
      type = BaseMapElement::Airfield;
      landable = true;
    } else {
      type = BaseMapElement::Landmark;
    }
    frWp = new Waypoint;
    frWp->name = name;
    frWp->description = name;
    frWp->comment = remark;
    frWp->origP.setPos(lat, lon);
    frWp->elevation = elv;
    frWp->type = type;
    frWp->isLandable = landable;
    waypoints->append(frWp);
  }
  return FR_OK;
}

int Cambridge::writeWaypoints(QPtrList<Waypoint> *waypoints)
{
  return FR_NOTSUPPORTED;
}


/*
 * write byte
 */
int Cambridge::wb(unsigned char c)
{
  // qDebug ("wb (%x)", c);
  if (write(portID, &c, 1) != 1) {
    return -1;
  }
  return 1;
}

void Cambridge::wait_ms(const int t)
{
  usleep(t * 1000);
}

unsigned char *Cambridge::readData(unsigned char *bufP, int count)
{
  int rc;
  switch (rc = read(portID, bufP, count)) {
  case -1:
    warning("readData(): ERROR");
    break;
  default:
    // qDebug ("readData: %x(%x)", rc, count);
    bufP += rc;
    break;
  }
  return bufP;
}

int Cambridge::calcChecksum8(unsigned char *buf, int count)
{
  unsigned char foo = 0;
  for (int i=0 ; i<count ; i++)
  {
    foo = foo^buf[i];
  }
  return foo;
}

int Cambridge::calcChecksum16(unsigned char *buf, int count)
{
  int foo=0;
  for (int i=0 ; i<count ; i++)
    foo=(foo+buf[i])&0xffff;
  return foo;
}

int Cambridge::sendCommand(QString cmd)
{
  // flush the buffer and send the command
  tcflush(portID, TCIOFLUSH);
  write(portID, (const char *)cmd, cmd.length());
  wb('\r');
  return FR_OK;
}

int Cambridge::readReply(QString cmd, int mode, unsigned char *reply)
{
  sendCommand(cmd);
  unsigned char *bufP;
  unsigned char buf[2048];
  int buffersize = 2047;
  bufP = buf;

  // We might have to implement this later. Maybe.
  if (mode==CMD_MODE)
  {
    warning("readReply(): CMD_MODE not yet supported!");
    return 0;
  }

  // initialize the output array. You never know ...
  for (int i=0 ; i <2048 ; i++) reply[i]=0x00;

  // initialize the buffer ... better safe than sorry.
  for (int i=0 ; i <2048 ; i++) buf[i]=0x00;

  // Never mind the names ... I took them from Cambridge's User Guide:
  int XX = -1;  // length of the reply, including the prompt
  int YY = -1;  // checksum of the user request, as provided by the recorder
  int ZZ = -1;  // checksum of the reply

  int t1 = time(NULL);
  int done = 0;
  while (!done)
  {
    bufP = readData(bufP, (buffersize + buf - bufP));
    // in command and download mode we could rely on the prompt,
    // but in upload mode binary data is transmitted and the
    // null strings give trouble in the usual string types.
    // So we need to check the length and stop reading when
    // we see the ">" of the prompt at the proper position.
    if (mode==UPS_MODE || mode==DN_MODE)
    {
      XX = extractInteger(buf, cmd.length(), 1);
      YY = extractInteger(buf, cmd.length()+1, 1);
      ZZ = extractInteger(buf, cmd.length()+2, 1);
    }
    else if (mode==UPL_MODE)
    {
      XX = extractInteger(buf, cmd.length(), 2);
      YY = extractInteger(buf, cmd.length()+2, 1);
      ZZ = extractInteger(buf, cmd.length()+3, 2);
    }

    // qDebug("%d", XX);
    if (buf[XX+cmd.length()-1]==0x3e) done=1;

    if (time(NULL)>(t1+10))
    {
      warning("readReply(): timeout in logger communication");
      return TIMEOUT_ERROR;
    }
  }

  // uncomment this line if you want a hex dump of the buffer
  // debugHex (buf,32);

  // calculate start position of data
  int start = 0;
  if (mode==UPS_MODE || mode==DN_MODE)
    start = cmd.length()+3;
  else if (mode==UPL_MODE)
    start = cmd.length()+5;

  // the prompt at the end of the reply is included in the checksum,
  // so we keep it now and delete it later.
  for (size_t i=start ; i<XX+cmd.length() ; i++)
    reply[i-start]=buf[i];

  int cmd_checksum = calcChecksum8((unsigned char *)(const char *)cmd, cmd.length());
  int reply_checksum = 0;
  if (mode==UPS_MODE || mode==DN_MODE)
    reply_checksum = calcChecksum8(reply, XX);
  else if (mode==UPL_MODE)
    reply_checksum = calcChecksum16(reply, XX);

  if ((cmd_checksum!=YY) || (reply_checksum!=ZZ))
  {
    warning("readReply(): got wrong checksum");
    return CHECKSUM_ERROR;
  }

  // uncomment this line if you want a hex dump of the buffer
  // debugHex (reply,100);

  // now delete the prompt at the end of the reply. The prompts are:
  // command mode:  "\r\n\ncmd>"
  // download mode: "\r\n\ndn>"
  // upload mode:   "\r\n\nup>"
  for (size_t i=XX+cmd.length()-6 ; i<XX+cmd.length() ; i++)  // "-6" for the prompt
    reply[i-start]=0x00;

  return XX-(start-1)-6;
}


