/***********************************************************************
**
**   filser.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Christian Fughe, Harald Maier, Eggert Ehmke
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <ctype.h>
#include <cmath>
#include <string.h>

#include <QtCore>

#include "filser.h"

#define MAX_LSTRING    63

/*logger defines*/
#define NODATA         0
#define LSTRING        1 /* to 63 string*/
#define END           64        /* 40 */

#define SHVERSION    127        /* 7f */
#define START        128        /* 80 */
#define ORIGIN       160        /* a0 */

#define POSITION_OK        191  /* bf */
#define POSITION_BAD       195  /* c3 */
#define REQ_BASIC_DATA     0xc4
#define REQ_FLIGHT_DATA    0xc9


#define SECURITY_NEW       239        /* ef */ /* 28.5.2004 Fughe: Upgrading to G3 security records */
#define SECURITY           240        /* f0 */
#define COMPETITION_CLASS  241        /* f1 */
#define SAT_CON            242        /* f2 */
#define DIFFERENTIAL       243        /* f3 */
#define EVENT              244        /* f4 */
#define SECURITY_OLD       245        /* f5 */
#define SER_NR             246        /* f6 */
#define TASK               247        /* f7 */
#define UNITS              248        /* f8 */
#define FIXEXT             249        /* f9 */
#define EXTEND             250        /* fa */
#define DATUM              251        /* fb */
#define FLIGHT_INFO        253        /* fc */
#define EXTEND_INFO        254        /* fd */
#define FIXEXT_INFO        255        /* fe */

#define LOW_SECURITY        0x0d
#define MED_SECURITY        0x0e
#define HIGH_SECURITY       0x0f


/**
 * The device-name of the port.
 */
const char* portName = '\0';
int portID = -1;
const char* c36 = "0123456789abcdefghijklmnopqrstuvwxyz";

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
unsigned char STX = 0x02, /* Command prefix like AT for modems        */
  ACK = 0x06,      /* Response OK, if the crc check is ok             */
  NAK = 0x15,      /* Response not OK, if the crc check is not ok     */
  SYN = 0x16,      /* Request for CONNECT                             */
  K = 'K' | 0x80,  /* get_extra_data()   - trailing fix sized block   */
  L = 'L' | 0x80,  /* get_mem_sections() - the flight data is         */
                   /*                      retrieved in blocks        */
  M = 'M' | 0x80,  /* getFlightDir()-      table of flights           */
  N = 'N' | 0x80,  /* def_mem()          - memory range of one flight */
  Q = 'Q' | 0x80,  /* read_mem_setting()                              */
  R = 'R',         /* readWaypoints()                                 */
  W = 'W',         /* writeWaypoints()                                */
  f = 'f' | 0x80;  /* get_logger_data()  - first block                */
                   /* f++ get_logger_data()  - next block             */

#define BUFSIZE 1024          /* General buffer size                  */

  // @AP: Set a manufacture key that also Posigraph SDI logger files can be
  // converted in the right manner. They use the keyword SDI.
  //
  // 09.03.2005 Fughe: Today we know and support three manufactures:
  //
  //                          FIL   Filser
  //                          SDI   Streamline Data Instruments
  //                          LXN   LX Navigation

  char manufactureShortKey = 'X';
  char manufactureKey[] = "xxx";  // Let's start with an empty key. If 'xxx'
                                  // appears, then reading the 'A'-record
                                  // failed.

  unsigned char *memContents; /* buffer to hold igc contents */
  int contentSize;            /* length of igc file buffer   */


void debugHex (const void* buf, unsigned int size)
{
  for( unsigned int ix1 = 0; ix1 < size; ix1 += 0x10 )
    {
      QString line;
      line.sprintf( "%03X:  ", ix1 );

      for( int ix2 = 0; ix2 < 0x10; ix2++ )
        {
          QString byte;
          byte.sprintf( "%02X ", ((unsigned char*) buf)[ix1 + ix2] );
          line += byte;
        }

      line += "    ";

      for( int ix2 = 0; ix2 < 0x10; ix2++ )
        {
          if( isprint( ((unsigned char*) buf)[ix1 + ix2] ) )
            line += ((unsigned char*) buf)[ix1 + ix2];
          else
            line += ' ';
        }

      qDebug( "%s", line.toLatin1().data() );
    }
}

/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the program. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int /* signal*/)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
}

Filser::Filser( QObject *parent ) : FlightRecorderPluginBase( parent )
{
  //Set flight recorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = TASK_MAX;             //maximum number of tasks
  _capabilities.maxNrWaypoints = WAYPOINT_MAX;     //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = MAXTSKPNT; //maximum number of waypoints per task
  _capabilities.maxNrPilots = 1;             //maximum number of pilots
  _capabilities.transferSpeeds = bps02400 |  //supported transfer speeds
                                 bps04800 |
                                 bps09600 |
                                 bps19200 |
                                 bps38400;

  _capabilities.supDlWaypoint = true;        //supports downloading of waypoints?
  _capabilities.supUlWaypoint = true;        //supports uploading of waypoints?
  _capabilities.supDlFlight = true;          //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  _capabilities.supSignedFlight = true;      //supports downloading in of signed flights?
  _capabilities.supDlTask = true;            //supports downloading of tasks?
  _capabilities.supUlTask = true;            //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  _capabilities.supDspSerialNumber = true;
  _capabilities.supDspRecorderType = true;
  _capabilities.supDspPilotName = true;
  _capabilities.supDspGliderType = true;
  _capabilities.supDspGliderID = true;
  _capabilities.supDspCompetitionID = true;
  _capabilities.supAutoSpeed = true;       //supports automatic transfer speed detection
  //End set capabilities.

  portID = -1;
  _da4BufferValid = false;

  _keepalive = new QTimer( this );
  connect( _keepalive, SIGNAL(timeout()), this, SLOT(slotTimeout()) );
}

Filser::~Filser()
{
  closeRecorder();
  qDeleteAll( flightIndex );
}

/**
  * this function will be called each second to keep the connection to LX device alive
  */
void Filser::slotTimeout()
{
  tcflush(portID, TCIOFLUSH); // Make sure the next ACK comes from the
                              // following wb(SYN). And remove the
                              // position data, that might have been
                              // arrived.
  wb( SYN );
  tcdrain( portID );
  int ret = rb();

  if( ret != ACK )
    {
      qDebug( "Filser::keepalive failed: ret = %x", ret );
    }
}

/**
 * Returns the transfer mode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Filser::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

int Filser::getFlightDir( QList<FRDirEntry *>* dirList )
{
  qDebug ("Filser::getFlightDir");

  int flightCount = 0;
  unsigned char indexByte = 1;
  int rc = FR_OK;
  unsigned char *bufP;
  unsigned char buf[BUFSIZE + 1];
  int i;

  // Remove all old entries in the list, if there are any.
  qDeleteAll( *dirList );
  dirList->clear();

  if( !readMemSetting() )
    {
      return FR_ERROR;
    }

  _errorinfo = "";

  _keepalive->blockSignals(true);

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(M);

  while (indexByte) {
    bufP = buf;
    while ((FLIGHT_INDEX_WIDTH + buf - bufP) > 0) {
      bufP = readData(bufP, (FLIGHT_INDEX_WIDTH + buf - bufP));
    }

    indexByte = buf[0];

    if ((bufP - buf) != FLIGHT_INDEX_WIDTH) {
      _errorinfo = tr("getFlightDir(): Wrong amount of bytes from LX-device");
      rc = FR_ERROR;
      break;
    }
    else if (calcCrcBuf(buf, FLIGHT_INDEX_WIDTH - 1) != buf[FLIGHT_INDEX_WIDTH - 1]) {
      _errorinfo = tr("getFlightDir(): Bad CRC");
      rc = FR_ERROR;
      break;
    }
    else if (buf[0] == 0)
    {
      break;
    }
    else {
      struct flightTable *ft = new struct flightTable;

      // uncomment this if you want to analyze the buffer
      // debugHex (buf, FLIGHT_INDEX_WIDTH);

      memcpy(ft->record, buf, bufP - buf);

      flightIndex.append(ft);

      // remove \0 between date and time
      ft->record[17] = ' ';

      struct tm startTime, stopTime;

      strptime((char *)ft->record + 9, "%d.%m.%y %T", &startTime);
      // here we might get into trouble when we have flights from australia
      stopTime = startTime;
      strptime((char *)ft->record + 0x1b, "%T", &stopTime);

      time_t startTime_t = mktime(&startTime);
      time_t stopTime_t = mktime(&stopTime);

      FRDirEntry* entry = new FRDirEntry;

      flightCount++;

      entry->pilotName = (const char *)ft->record + 40;
//      entry->gliderID = "n.a.";
      // the glider type is not contained in the flight list; we take it from basic data
      entry->gliderID = _basicData.gliderType;

      entry->duration = stopTime_t - startTime_t;
      if (entry->duration < 0)
      {
        // lets handle the aussie flights
        stopTime_t += 24*60*60;
        entry->duration += 24*60*60;
        // yes use localtime here ! The stopTime is already UTC
        localtime_r (&stopTime_t, &stopTime);
      }
      entry->firstTime = startTime;
      entry->lastTime = stopTime;
      dirList->append(entry);

      if (indexByte != 0 && indexByte != 1) {
        _errorinfo = tr("getFlightDir(): Wrong index byte");
        rc = FR_ERROR;
        break;
      }
    }
  }

  if (flightIndex.isEmpty())
    {
      _errorinfo = tr("getFlightDir(): no flights available in LX-device");
      rc = FR_ERROR;
    }
  else {
      //
      // 09.03.2005 Fughe: Use the manufacture key as well in file names.
      //

      // 09.03.2005 Fughe: We need to retrieve one flight, to set
      //                   'manufactureShortKey', and 'manufactureKey'. As soon
      //                   as anybody identifies a logger function for this
      //                   purpose, the time consuming retrieval can be
      //                   removed.

    QString tmpFile = QDir::homePath() + "/.tmpigc";

    int rc = downloadFlight( 0, 0, tmpFile );

    if( rc == FR_OK )
      {
        for( i = 0; i < flightCount; i++ )
          {
            dirList->at(i)->shortFileName.sprintf("%c%c%c%c%s%c.igc",
                                         c36[dirList->at(i)->firstTime.tm_year % 10],
                                         c36[dirList->at(i)->firstTime.tm_mon + 1],
                                         c36[dirList->at(i)->firstTime.tm_mday],
                                         manufactureShortKey,
                                         wordtoserno((flightIndex.at(i)->record[91] << 8)
                                                    + flightIndex.at(i)->record[92]),
                                         c36[flightIndex.at(i)->record[94]]); // 09.03.2005 Fughe: This is
                                                                              // the counter of the flight
                                                                              // of the day (IGC tech specs).
                                                                              // Please, keep it this way.
            dirList->at(i)->longFileName.sprintf("%d-%.2d-%.2d-%s-%s-%.2d.igc",
                                        dirList->at(i)->firstTime.tm_year + 1900,
                                        dirList->at(i)->firstTime.tm_mon + 1,
                                        dirList->at(i)->firstTime.tm_mday,
                                        manufactureKey,
                                        wordtoserno((flightIndex.at(i)->record[91] << 8)
                                                   + flightIndex.at(i)->record[92]),
                                        flightIndex.at(i)->record[94]); // 09.03.2005 Fughe: This is
                                                                        // the counter of the flight
                                                                        // of the day (IGC tech specs).
                                                                        // Please, keep it this way.
            qWarning("%s   %s", dirList->at(i)->longFileName.toLatin1().data(),
                                dirList->at(i)->shortFileName.toLatin1().data());
          }
      }

    remove( tmpFile.toAscii().data() );
  }

  _keepalive->blockSignals(false);
  return rc;
}

/**
  * This function retrieves the basic recorder data from the LX device
  * currently supported are: serial number, devive type, pilot name, glider type, glider id, competition id.
  * The structure of the data was analyzed on a Windows2000 box using the Filser LXe application.
  * It has been verified with two LX20 devices, Version 5.11 and 5.2, at speeds 9600 and 19200 bps.
  * Written by Eggert Ehmke <eggert.ehmke@berlin.de>, <eggert@kflog.org>
  */
int Filser::getBasicData(FR_BasicData& data)
{
  // if the serialNumber contains data, take them
  if (!_basicData.serialNumber.isEmpty())
  {
    data = _basicData;
    return FR_OK;
  }

  _keepalive->blockSignals(true);

  int rc = FR_OK;
  unsigned char *bufP;
  unsigned char *bufP_o = 0;
  unsigned char buf[BUFSIZE + 1];
  int buffersize = BUFSIZE;
  int min_data   = 0x80;
  // actually, depending on the version of the LX device, a different
  // amount of bytes are sent.
  // this value is close enough to the real number
  // V5.0:  0x12F bytes
  // V5.11: 0x131 bytes
  // V5.2:  0x140 bytes


  if (!check4Device()) {
    return FR_ERROR;
  }

  _errorinfo = "";
  int lc = 0 ;

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(REQ_BASIC_DATA);

  bufP = buf;
  // ( - Christian - )
  // Read until no more data is comming in, bufP == bufP_o.
  // LX20 V5.0 starts writing Position data on the serial line when leaving
  // the CONNECT mode after the timeout of 10 sec. There is less than 1
  // second between end-of-data and start of sending the position data.
  // I suggest VTIME = 1 !
  //
  // Eggert, reading 0x130 bytes into the buffer failed with my recorder,
  // because it sends 0x12F bytes only. The remaining byte was filled from
  // the position data after the 10 sec timeout of the CONNECT mode. The
  // following check4Device() below failed too of course and terminated this
  // function.
  //
  // newTermEnv.c_cc[VTIME] = 20; // wait at least 2 sec for no more data.
                               // 13.03.2005 Fughe: Maybe to long and newer
                               // recorders like the LX7000 from Markus start
                               // sending NMEA data. Keep it to the default value.
  // tcsetattr(portID, TCSANOW, &newTermEnv);
  while( (buffersize + buf - bufP) > 0 )
    {
      bufP = readData( bufP, (buffersize + buf - bufP) );

      if( bufP == bufP_o ) // No more data
        {
          break;
        }

      bufP_o = bufP;
    }
  // newTermEnv.c_cc[VTIME] = 1; // reset the wait time to 0.1 sec.
                                 // 13.03.2005 Fughe: Keep it default.
  // tcsetattr(portID, TCSANOW, &newTermEnv);

  // uncomment this if you want to analyze the buffer
  // debugHex (buf, buffersize);

  if ((bufP - buf) < min_data) {
    _errorinfo = tr("getBasicData(): Wrong amount of bytes from LX-device");
    _basicData.recorderType = QString("n.a.");
    _basicData.serialNumber = QString("n.a.");
  }
  /*
  // we cannot calculate a checksum because we ignored the rest of the data
  else if (calcCrcBuf(buf, buffersize - 1) != buf[buffersize - 1])
  {
    _errorinfo = tr("getBasicData(): Bad CRC");
    rc = FR_ERROR;
  }
  */
  else
  {
    QString data( (const char *) buf );
    QStringList list = data.split( QRegExp("[\n\r]") );

    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
    {
      // Example: Version LX20 V5.11
      if ((*it).left(7).toUpper() == "VERSION")
        _basicData.recorderType = (*it).mid(8);
      // Example: SN12969,HW3.0
      else if ((*it).left(2) == "SN")
        _basicData.serialNumber = (*it).mid(2);
    }
  }

  // during sleep, hopefully the extra bytes will arrive and can be flushed savely.
  //sleep (1);
  //tcflush(portID, TCIOFLUSH);
  //
  // 13.03.05 Fughe: Maybe it is saver to throw them away.
  //                 check4Device() is doing this now too by 'while(0xff != rb());'.
  while(0xff != rb())
    lc++;
  qWarning ("while _basicData: %d + %d (%d)", (int)(bufP - buf), lc, BUFSIZE);

  if (!check4Device()) {
    _keepalive->blockSignals(false);
    return FR_ERROR;
  }

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(REQ_FLIGHT_DATA);

  // again, there are more bytes to be expected from the device. They will
  // be flushed
  //
  // ( - Christian - )
  // The documentations says there are BASIC_LENGTH bytes. And in case of a
  // class descriptor you can get 9 bytes more with another function
  // ('I' | 0x80).
  //
  bufP = buf;
  while ((BASIC_LENGTH + buf - bufP + 1 ) > 0) {
    bufP = readData(bufP, (BASIC_LENGTH + buf - bufP +1 ));
  }
  // uncomment this if you want to analyze the buffer
  // debugHex (buf, BASIC_LENGTH);

  if (calcCrcBuf(buf, BASIC_LENGTH) != buf[BASIC_LENGTH])
  {
    _errorinfo = tr("getBasicData(): Bad CRC");
    rc = FR_ERROR;
  }
  //
  // ( - Christian - )
  //
  // I rely on the Filser structure here as documented, no integrity check.
  //
  // Today Filser recorders can exchange this data among themselves.
  //
  else
  {
    // hopefully, Filser will not change size or position of data fields ...
    _basicData.pilotName = (char*)&buf[3];
    _basicData.gliderType = (char*)&buf[0x16];
    _basicData.gliderID = (char*)&buf[0x22];
    _basicData.competitionID = (char*)&buf[0x2a];
    data = _basicData;
  }

  _keepalive->blockSignals(false);

  return rc;
}

int Filser::getConfigData(FR_ConfigData& /*data*/)
{
  return FR_NOTSUPPORTED;
}

int Filser::writeConfigData(FR_BasicData& /*basicdata*/, FR_ConfigData& /*configdata*/)
{
  return FR_NOTSUPPORTED;
}

int Filser::downloadFlight(int flightID, int /*secMode*/, const QString& fileName)
{
  int rc;
  unsigned char memSection[0x21];/* Information received from the   */
                                 /* logger about the memory         */
                                 /* blocks of a specific flight     */
                                 /* for download. The table is      */
                                 /* 0x20 bytes long. Two bytes for  */
                                 /* a block. Plus one byte for CRC. */
  FILE *f;

  _errorinfo = "";

  struct flightTable *ft = flightIndex.at(flightID);

  if (!check4Device() || !defMem(ft) ||
      !getMemSection(memSection, sizeof(memSection)) ||
      !getLoggerData(memSection, sizeof(memSection)))
    {
      rc = FR_ERROR;
    }
  else
    {
      if( (f = fopen( fileName.toLatin1().data(), "w" )) != 0 )
        {
          if( convFil2Igc( f, memContents, memContents + contentSize ) )
            {
              rc = FR_OK;
            }
          else
            {
              _errorinfo += tr( "\nCheck igc file for further info." );
              rc = FR_ERROR;
            }

          fclose( f );
        }
      else
        {
          _errorinfo = tr( "\nCannot open temporary file: " ) + fileName;
          rc = FR_ERROR;
        }
    }

  if( memContents )
    {
      delete memContents;
      memContents = 0;
      contentSize = 0;
    }

  return rc;
}

int Filser::openRecorder(const QString& pName, int baud)
{
  portName = pName.toLatin1().data();

  portID = open(portName, O_RDWR | O_NOCTTY);

  if(portID != -1) {
    //
    // Before we change any port-settings, we must establish a
    // signal-handler, which is used to restore the port-settings
    // after terminating the program.
    // Because a SIGKILL-signal removes the program immediately,
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
    newTermEnv.c_iflag = IGNPAR;
    newTermEnv.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    newTermEnv.c_oflag &= ~OPOST;
    newTermEnv.c_oflag |= ONLCR; 
    newTermEnv.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    /*
     * No flow control at all :-(
     */
    newTermEnv.c_cflag &= ~(CSIZE | PARENB | CSTOPB | CRTSCTS | IXON | IXOFF);
    newTermEnv.c_cflag |= (CS8 | CLOCAL | CREAD);
    
    // control characters
    newTermEnv.c_cc[VMIN] = 0; // don't wait for a character
    newTermEnv.c_cc[VTIME] = 1; // wait at least 1 msec.

//    if(baud >= 115200) speed = B115200;
//    else if(baud >= 57600) speed = B57600;
//    else
//
//  ( - Christian - )
//  2400 - 38400 bps
//  These are the only speeds known by Filser devices, right?
//  Does anybody have different experiences?
    if(baud >= 38400) _speed = B38400;
    else if(baud >= 19200) _speed = B19200;
    else if(baud >=  9600) _speed = B9600;
    else if(baud >=  4800) _speed = B4800;
    else                   _speed = B2400;

    cfsetospeed(&newTermEnv, _speed);
    cfsetispeed(&newTermEnv, _speed);

    // flush the device
    tcflush (portID, TCIOFLUSH);
    // Activating the port-settings
    tcsetattr(portID, TCSANOW, &newTermEnv);

    _isConnected = true;
    _da4BufferValid = false;

    if(!AutoBaud()){
      qWarning() << QObject::tr("No baudrate found!");
      _isConnected = false;
      return FR_ERROR;
    };

    _keepalive->start (1000); // one second timer
    return FR_OK;
    }
  else {
    qWarning() << QObject::tr("No logger found!");
    _isConnected = false;
    return FR_ERROR;
  }
}

bool Filser::defMem(struct flightTable *ft)
{
  unsigned char        address_buf[7];
  int flight_start_adr, flight_end_adr;

  /* Flight_table->record[3] is the 4-th and highest address byte,    */
  /* but wb(STX), wb(N) takes only three bytes. Neither this software */
  /* nor the software from Filser is prepared for a value of the 4-th */
  /* address byte different from 0.                                   */
  flight_start_adr = (ft->record[4] << 16) + (ft->record[1] << 8) + (ft->record[2]);

  if(ft->record[3]) {
    _errorinfo = tr("Invalid memory size in the flight table from the LX-device.");
    return false;
  }

  /* The discussion about flight_table->record[3] holds as well for   */
  /* flight_table->record[7].                                         */
  flight_end_adr = (ft->record[4+4] << 16) +(ft->record[1+4]<< 8) + (ft->record[2+4]);

  if(ft->record[7]) {
    _errorinfo = tr("Invalid memory size in the flight table from the LX-device.");
    return false;
  }

  memcpy(address_buf, &flight_start_adr, 3);
  memcpy(address_buf + 3, &flight_end_adr, 3);
  address_buf[6] = calcCrcBuf(address_buf, 6);

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(N);
  for(unsigned int i = 0; i < sizeof(address_buf); i++) {
    wb(address_buf[i]);
  }
  tcdrain (portID);
  if (rb() != ACK) {
    _errorinfo = tr("Invalid response from LX-device.");
    return false;
  }
  return true;
}

bool Filser::getMemSection(unsigned char *memSection, int size)
{
  int i;

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(L);
  tcdrain (portID);
  for(i = 0; i < size; i++) {
    memSection[i] = rb();
  }

  if(calcCrcBuf(memSection, size-1) != memSection[size-1]) {
    _errorinfo = tr("get_mem_sections(): Bad CRC");
    return false;
  }
  return true;
}

bool Filser::getLoggerData(unsigned char *memSection, int sectionSize)
{
  unsigned char *bufP, *bufP2;
  /*
   * Calculate the size the of the memory buffer
   */
  contentSize = 0;

  for(int i = 0; i < ((sectionSize - 1) / 2); i++) {
    if(!(memSection[2 * i] | memSection[(2 * i) + 1])) {
      break;
    }
    contentSize += (memSection[2 * i] << 8) + memSection[(2 * i) + 1];
  }
  /*
   * plus 0x100 bytes for the extra data of get_extra_data()
   * we ignore this, required only for .fil files
   */
  //fil_file_length+=0x100;
  /*
   * Allocate fil_file_length bytes
   * plus one byte for the CRC byte.
   */

  memContents = new unsigned char [(contentSize) + 1]; // for CRC
  bufP = bufP2 = memContents;

  // read each memory section
  for(int i = 0; i < ((sectionSize - 1) / 2); i++) {
    if(!(memSection[2 * i] | memSection[(2 * i) + 1])) {
      break;
    }
    int count = ((unsigned char)memSection[2 * i] << 8) + (unsigned char)memSection[(2 * i) + 1];

    tcflush(portID, TCIOFLUSH);
    wb(STX);
    wb(f + i);
    while ((bufP - bufP2) < (count + 1)) {
      bufP = readData(bufP, (bufP2 + count + 1 - bufP));
    }
    if (calcCrcBuf(bufP2, count) != bufP2[count]) {
      _errorinfo = tr("get_logger_data(): Bad CRC");
      delete memContents;
      memContents = 0;
      contentSize = 0;
      return false;
    }
    bufP2 += count;
    bufP = bufP2;
  }

  return true;
}

/*
 * This function translates a .fil-file to an .igc-file.
 *
 * The .fil-file is expected in a buffer beginning at *fil_p and ending
 * at *fil_p_last.
 *
 * The resulting .igc-file is written to the open FILE pointer *figc.
 */
bool Filser::convFil2Igc(FILE *figc,  unsigned char *fil_p, unsigned char *fil_p_last)
{
  int i, j, l, ftab[16], etab[16], time = 0, time_orig = 0, fix_lat, fix_lat_orig = 0, fix_lon, fix_lon_orig = 0, tp;
  unsigned char flight_no = 0, *fil_p_ev = NULL;

  unsigned int ext_dat;
  char HFDTE[256], fix_ext_num = 0, ext_num = 0, ev = 0, fix_stat;
  unsigned int flt_id;
  char *flt_pilot = NULL, *flt_glider = NULL, *flt_reg = NULL, *flt_comp = NULL, *flt_observer = NULL, *flt_gps = NULL;
  unsigned char flt_class_id = 0, flt_gps_datum = 0, flt_fix_accuracy = 0;

  struct task {
    int usage;
    int lat;
    int lon;
    char T[256];
  };

  struct task TP[12];

  struct  three_letter_s {
    char *name;
    int count;
  };

  struct three_letter_s   three_letter_tab[16] = {
    { (char *)"FXA", 2 },
    { (char *)"VXA", 2 },
    { (char *)"RPM", 4 },
    { (char *)"GSP", 4 },
    { (char *)"IAS", 4 },
    { (char *)"TAS", 4 },
    { (char *)"HDM", 2 },
    { (char *)"HDT", 2 },
    { (char *)"TRM", 2 },
    { (char *)"TRT", 2 },
    { (char *)"TEN", 4 },
    { (char *)"WDI", 2 },
    { (char *)"WVE", 4 },
    { (char *)"ENL", 2 },
    { (char *)"VAR", 2 },
    { (char *)"XX3", 2 }
  };

  char *competition_class[] = {
    (char *)"STANDARD",
    (char *)"15-METER",
    (char *)"OPEN",
    (char *)"18-METER",
    (char *)"WORLD",
    (char *)"DOUBLE",
    (char *)"MOTOR_GL",
    (char *)""
  };



  char *gps_datum_tab[] = {
    (char *)"ADINDAN        ",
    (char *)"AFGOOYE        ",
    (char *)"AIN EL ABD 1970",
    (char *)"COCOS ISLAND   ",
    (char *)"ARC 1950       ",
    (char *)"ARC 1960       ",
    (char *)"ASCENSION 1958 ",
    (char *)"ASTRO BEACON E ",
    (char *)"AUSTRALIAN 1966",
    (char *)"AUSTRALIAN 1984",
    (char *)"ASTRO DOS 7/14 ",
    (char *)"MARCUS ISLAND  ",
    (char *)"TERN ISLAND    ",
    (char *)"BELLEVUE (IGN) ",
    (char *)"BERMUDA 1957   ",
    (char *)"COLOMBIA       ",
    (char *)"CAMPO INCHAUSPE",
    (char *)"CANTON ASTRO   ",
    (char *)"CAPE CANAVERAL ",
    (char *)"CAPE (AFRICA)  ",
    (char *)"CARTHAGE       ",
    (char *)"CHATHAM 1971   ",
    (char *)"CHUA ASTRO     ",
    (char *)"CORREGO ALEGRE ",
    (char *)"DJAKARTA       ",
    (char *)"DOS 1968       ",
    (char *)"EASTER ISLAND  ",
    (char *)"EUROPEAN 1950  ",
    (char *)"EUROPEAN 1979  ",
    (char *)"FINLAND 1910   ",
    (char *)"GANDAJIKA BASE ",
    (char *)"NEW ZEALAND '49",
    (char *)"OSGB 1936      ",
    (char *)"GUAM 1963      ",
    (char *)"GUX 1 ASTRO    ",
    (char *)"HJOESEY 1955   ",
    (char *)"HONG KONG 1962 ",
    (char *)"INDIAN/NEPAL   ",
    (char *)"INDIAN/VIETNAM ",
    (char *)"IRELAND 1965   ",
    (char *)"DIEGO GARCIA   ",
    (char *)"JOHNSTON 1961  ",
    (char *)"KANDAWALA      ",
    (char *)"KERGUELEN ISL. ",
    (char *)"KERTAU 1948    ",
    (char *)"CAYMAN BRAC    ",
    (char *)"LIBERIA 1964   ",
    (char *)"LUZON/MINDANAO ",
    (char *)"LUZON PHILIPPI.",
    (char *)"MAHE 1971      ",
    (char *)"MARCO ASTRO    ",
    (char *)"MASSAWA        ",
    (char *)"MERCHICH       ",
    (char *)"MIDWAY ASTRO'61",
    (char *)"MINNA (NIGERIA)",
    (char *)"NAD-1927 ALASKA",
    (char *)"NAD-1927 BAHAM.",
    (char *)"NAD-1927 CENTR.",
    (char *)"NAD-1927 CANAL ",
    (char *)"NAD-1927 CANADA",
    (char *)"NAD-1927 CARIB.",
    (char *)"NAD-1927 CONUS ",
    (char *)"NAD-1927 CUBA  ",
    (char *)"NAD-1927 GREEN.",
    (char *)"NAD-1927 MEXICO",
    (char *)"NAD-1927 SALVA.",
    (char *)"NAD-1983       ",
    (char *)"NAPARIMA       ",
    (char *)"MASIRAH ISLAND ",
    (char *)"SAUDI ARABIA   ",
    (char *)"ARAB EMIRATES  ",
    (char *)"OBSERVATORIO'66",
    (char *)"OLD EGYIPTIAN  ",
    (char *)"OLD HAWAIIAN   ",
    (char *)"OMAN           ",
    (char *)"CANARY ISLAND  ",
    (char *)"PICAIRN 1967   ",
    (char *)"PUERTO RICO    ",
    (char *)"QATAR NATIONAL ",
    (char *)"QORNOQ         ",
    (char *)"REUNION        ",
    (char *)"ROME 1940      ",
    (char *)"RT-90 SWEDEN   ",
    (char *)"S.AMERICA  1956",
    (char *)"S.AMERICA  1956",
    (char *)"SOUTH ASIA     ",
    (char *)"CHILEAN 1963   ",
    (char *)"SANTO(DOS)     ",
    (char *)"SAO BRAZ       ",
    (char *)"SAPPER HILL    ",
    (char *)"SCHWARZECK     ",
    (char *)"SOUTHEAST BASE ",
    (char *)"FAIAL          ",
    (char *)"TIMBALI 1948   ",
    (char *)"TOKYO          ",
    (char *)"TRISTAN ASTRO  ",
    (char *)"RESERVED       ",
    (char *)"VITI LEVU 1916 ",
    (char *)"WAKE-ENIWETOK  ",
    (char *)"WGS-1972       ",
    (char *)"WGS-1984       ",
    (char *)"ZANDERIJ       ",
    (char *)"CH-1903        "
  };

  while(fil_p < fil_p_last) {
    if(!fil_p[0]) {
      i = 0;
      while(fil_p[0] == 0) {
        i++;
        fil_p++;
        if(fil_p > fil_p_last) {
          _errorinfo = tr("unexpected end of '.fil'-file");
          return false;
        }
      }
      fprintf(figc, "L%dEMPTY%s\r\n", i, manufactureKey );
    }
    else if((fil_p[0] <= MAX_LSTRING) && (fil_p[0] > 0)) {
      fprintf(figc, "%.*s\r\n", fil_p[0], fil_p + 1);
      fil_p += fil_p[0] + 1;
    }
    /*
     * Identify the records of a .fil-file and translate them
     * to the .igc-file format.
     */
    else switch (fil_p[0]) {
    case START:
      /* 80h                         */
      /* 8   byte: "STReRAZ\0"       */
      /* 1   byte: flight of the day */
      fil_p++;
      flight_no = fil_p[8];
      fil_p += 9;
      break;

    case DATUM:
      /* fbh                         */
      /* 1   byte: day               */
      /* 1   byte: month             */
      /* 2   byte: year              */
      fil_p++;
      sprintf(HFDTE, "%02d%02d%02d", fil_p[0], fil_p[1], (fil_p[2] << 8) + fil_p[3]);
      fil_p += 4;
      break;

    case SER_NR:
      /* f6h                         */
      /* 8   byte, "FIL_S/N\0"       */
      /* 1   byte, flight of the day */
      fil_p++;
      fprintf(figc, "A%s", fil_p);

      // @AP: save the right manufacture key from the A record
      strncpy(manufactureKey, (const char*) fil_p, 3);
      manufactureKey[3] = '\0';

      // 09.03.2005 Fughe: Define the short manufacture key for use with file names.
      //                   New manufactures have to be added here!
      if      (!strcmp( manufactureKey, "SDI")) {
        manufactureShortKey = 'S';
      }
      else if (!strcmp( manufactureKey, "LXN")) {
        manufactureShortKey = 'L';
      }
      else if (!strcmp( manufactureKey, "FIL")) {
        manufactureShortKey = 'F';
      }
      else {
        manufactureShortKey = 'X';
      }
      qDebug("Manufacture Key Code is %s   %c", manufactureKey, manufactureShortKey);

      fil_p += 9;
      fprintf(figc, "FLIGHT:%d\r\n", flight_no);
      fprintf(figc, "HFDTE%s\r\n", HFDTE);
      break;

    case FLIGHT_INFO:
      /* fch                         */
      /* 2   byte, id                */
      /* 19  byte, pilot             */
      /* 12  byte, glider            */
      /* 8   byte, registration      */
      /* 4   byte, competition       */
      /* 1   byte, class id          */
      /* 10  byte, observer          */
      /* 1   byte, gps datum         */
      /* 1   byte, fix accuracy      */
      /* 60  byte, gps               */
      fil_p++;
      flt_id = (fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      flt_pilot = (char *)fil_p;
      fil_p += 19;
      flt_glider = (char *)fil_p;
      fil_p += 12;
      flt_reg = (char *)fil_p;
      fil_p += 8;
      flt_comp = (char *)fil_p;
      fil_p += 4;
      flt_class_id = fil_p[0];
      fil_p++;
      flt_observer = (char *)fil_p;
      fil_p += 10;
      flt_gps_datum = fil_p[0];
      fil_p++;
      flt_fix_accuracy = fil_p[0];
      fil_p++;
      flt_gps = (char *)fil_p;
      fil_p += 60;
      if(flt_class_id == 7) {
        break;
      }
      fprintf(figc, "HFFXA%03d\r\n", flt_fix_accuracy);
      fprintf(figc, "HFPLTPILOT:%s\r\n", flt_pilot);
      fprintf(figc, "HFGTYGLIDERTYPE:%s\r\n", flt_glider);
      fprintf(figc, "HFGIDGLIDERID:%s\r\n", flt_reg);
      if(flt_gps_datum != 255) {
        fprintf(figc, "HFDTM%03dGPSDATUM:%s\r\n", flt_gps_datum,
                gps_datum_tab[flt_gps_datum]);
      }
      else {
        fprintf(figc, "HFDTM%03dGPSDATUM:UNKNOWN\r\n", flt_gps_datum);
      }
      fprintf(figc, "HFCIDCOMPETITIONID:%s\r\n", flt_comp);
      fprintf(figc, "HFCCLCOMPETITIONCLASS:%s\r\n", competition_class[flt_class_id]);
      fprintf(figc, "HFGPSGPS:%s\r\n", flt_gps);
      break;

    case COMPETITION_CLASS:
      /* f1h                         */
      /* 9   byte, class id          */
      fil_p++;
      if(flt_class_id != 7) {
        break;
      }
      fprintf(figc, "HFFXA%03d\r\n", flt_fix_accuracy);
      fprintf(figc, "HFPLTPILOT:%s\r\n", flt_pilot);
      fprintf(figc, "HFGTYGLIDERTYPE:%s\r\n", flt_glider);
      fprintf(figc, "HFGIDGLIDERID:%s\r\n", flt_reg);
      if(flt_gps_datum != 255) {
        fprintf(figc, "HFDTM%03dGPSDATUM:%s\r\n", flt_gps_datum, gps_datum_tab[flt_gps_datum]);
      }
      else {
        fprintf(figc, "HFDTM%03dGPSDATUM:UNKNOWN\r\n", flt_gps_datum);
      }
      fprintf(figc, "HFCIDCOMPETITIONID:%s\r\n", flt_comp);
      fprintf(figc, "HFCCLCOMPETITIONCLASS:%s\r\n", fil_p);
      fil_p += 9;
      fprintf(figc, "HFGPSGPS:%s\r\n", flt_gps);
      break;

    case SHVERSION:
      /* 7fh                         */
      /* 1   byte, hardware          */
      /* 1   byte, software          */
      fil_p++;
      fprintf(figc, "HFRFWFIRMWAREVERSION:%3.1f\r\n", fil_p[1]/10.);
      fprintf(figc, "HFRHWHARDWAREVERSION:%3.1f\r\n", fil_p[0]/10.);
      fil_p += 2;
      break;

    case FIXEXT_INFO:
      /* feh                         */
      /* 2   byte, time              */
      /* 2   byte, dat               */
      fil_p++;
      fil_p += 2;
      ext_dat = (fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      fix_ext_num = 0;
      for (l = 1; l < 65535; l *= 2) {
        if (ext_dat & l) {
          fix_ext_num++;
        }
      }
      if (fix_ext_num > 0) {
        fprintf(figc, "I%02d", fix_ext_num);
      }
      else {
        break;
      }

      i = 36;
      j = 0;
      for (l = 0; l < 16; l++) {
        if(ext_dat & (1 << l)) {
          fprintf(figc, "%02d%02d%s", i, i+three_letter_tab[l].count, three_letter_tab[l].name);
          i += three_letter_tab[l].count + 1;
          ftab[j] = three_letter_tab[l].count + 1;
          j++;
        }
      }
      fprintf(figc, "\r\n");
      break;

    case EXTEND_INFO:
      /* fdh                         */
      /* 2   byte, time              */
      /* 2   byte, dat               */
      fil_p++;
      fil_p += 2;
      ext_dat = (fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      ext_num = 0;
      for (l = 1; l < 65535; l *= 2) {
        if (ext_dat & l) {
          ext_num++;
        }
      }
      if(ext_num > 0) {
        fprintf(figc, "J%02d", ext_num);
      }
      else {
        break;
      }

      i = 8;
      j = 0;
      for (l = 0; l < 16; l++) {
        if(ext_dat & (1 << l)) {
          fprintf(figc, "%02d%02d%s", i, three_letter_tab[l].count+i, three_letter_tab[l].name);
          i += three_letter_tab[l].count + 1;
          etab[j] = three_letter_tab[l].count + 1;
          j++;
        }
      }
      fprintf(figc, "\r\n");
      break;

    case FIXEXT:
      /* f9h                         */
      fil_p++;
      for(i = 0;i < fix_ext_num; i++) {
        fprintf(figc, "%0*u", ftab[i], (fil_p[0] << 8) + fil_p[1]);
        fil_p += 2;
      }
      fprintf(figc, "\r\n");
      break;

    case EXTEND:
      /* fah                         */
      fil_p++;
      l = fil_p[0] + time;
      fil_p++;
      fprintf(figc, "K%02d%02d%02d", l / 3600, l % 3600 / 60, l % 60 );
      for(i = 0; i < ext_num; i++) {
        fprintf(figc, "%0*u", etab[i], (fil_p[0] << 8) + fil_p[1]);
        fil_p += 2;
      }
      fprintf(figc, "\r\n");
      break;

    case TASK:
      /* f7h                         */
      /* 4   byte, time              */
      /* 1   byte, day               */
      /* 1   byte, month             */
      /* 1   byte, year              */
      /* 1   byte, day   (user)      */
      /* 1   byte, month (user)      */
      /* 1   byte, year  (user)      */
      /* 2   byte, taskid            */
      /* 1   byte, num_of_tp         */
      /* 12x1 byte, usage    (12 tp) */
      /* 12x4 byte, long     (12 tp) */
      /* 12x4 byte, lat      (12 tp) */
      /* 12x9 byte, name     (12 tp) */
      fil_p++;
      time = (fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
      fil_p += 4;
      fprintf(figc, "C%02d%02d%02d%02d%02d%02d", fil_p[0], fil_p[1], fil_p[2], time / 3600, time % 3600 / 60, time % 60 );
      fil_p += 3;
      fprintf(figc, "%02d%02d%02d", fil_p[0], fil_p[1], fil_p[2]);
      fil_p += 3;

      // @AP: num_of_tp seems to be a signed char
      //
      // 09.03.2005 Fughe: Reenabling the '(signed char)' patch. It violates
      //                   the tech spec of IGC-files from 12 December 2004,
      //                   but LXe is doing so and the patch is necessary for
      //                   validation of a flight from Markus for OLC. The
      //                   recorder has no task defined and is a LX7000,
      //                   FW: 2.0, HW: 1.0 from LX Navigation. The patch
      //                   does not interfere with normal behaviour where a
      //                   number of turn points from 0 to 8 is expected.
      //
      //fprintf(figc, "%04d%02d\r\n", (fil_p[0] << 8) + fil_p[1], fil_p[2]);
      //
      fprintf(figc, "%04d%02d\r\n", (fil_p[0] << 8) + fil_p[1], (signed char) fil_p[2]);
      fil_p += 3;

      // get usage
      for(tp = 0; tp < 12; tp++) {
        TP[tp].usage = fil_p[0];
        fil_p++;
      }
      // get lon
      for(tp = 0; tp < 12; tp++) {
        TP[tp].lon = ((signed char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
        fil_p += 4;
      }
      // get lat
      for(tp = 0; tp < 12; tp++) {
        TP[tp].lat = ((signed char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
        fil_p += 4;
      }
      // get name
      for(tp = 0; tp < 12; tp++) {
        sprintf(TP[tp].T, "%s", fil_p);
        /*
         * the TP-names are 8-bytes long plus one '\0'
         * the space of shorter names is filled with '\0'
         */
        fil_p += 9;
      }
      for(tp = 0; tp < 12; tp++) {
        if(TP[tp].usage) {
          fprintf(figc, "C%02d%05d%c", TP[tp].lat / 60000, TP[tp].lat % 60000, TP[tp].lat >= 0 ?  'N' : 'S');
          fprintf(figc, "%03d%05d%c", TP[tp].lon / 60000, TP[tp].lon % 60000, TP[tp].lon >= 0 ? 'E' : 'W');
          fprintf(figc,  "%s\r\n", TP[tp].T);
        }
      }
      break;

    case EVENT:
      fil_p++;
      fil_p_ev = fil_p;
      fil_p += 9;
      ev = 1;
      break;

    case POSITION_OK:
    case POSITION_BAD:
      /* bfh                         */
      /* 2   byte, time              */
      /* 2   byte, lat               */
      /* 2   byte, lon               */
      /* 2   byte, aalt              */
      /* 2   byte, galt              */
      fix_stat = fil_p[0] == POSITION_OK ? 'A' : 'V';
      fil_p++;
      // on the arm architecture, char is unsigned ! Force it signed to make it run correctly on the Zaurus
      time = time_orig + ((signed char) fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      fix_lat = fix_lat_orig + ((signed char) fil_p[0] << 8) + fil_p[1];
      fil_p += 2;
      fix_lon = fix_lon_orig + ((signed char) fil_p[0] << 8) + fil_p[1];
      fil_p += 2;

      if(ev) {
        fprintf(figc,"E%02d%02d%02d%s\r\n", time / 3600, time % 3600 / 60, time % 60, fil_p_ev);
        ev = 0;
      }

      if (time < 0) {
        ev=0;
      }
      fprintf(figc, "B%02d%02d%02d", time / 3600, time % 3600 / 60, time % 60);
      fprintf(figc, "%02d%05d%c", abs( fix_lat / 60000), abs(fix_lat % 60000), fix_lat >= 0 ? 'N' : 'S');
      fprintf(figc, "%03d%05d%c", abs(fix_lon / 60000), abs(fix_lon % 60000), fix_lon >= 0 ? 'E' : 'W'); fprintf(figc, "%c", fix_stat);

      // @AP: we have to cast the output data to short that negative
      // altitude values are considered. In this case the altitude
      // must be given out as -1234. The problem happened by me due to
      // a forgotten altitude calibration. I started with -15m. But
      // the download routine converted all to 65xxx and that was not
      // useable for the flight duration calculation. Download with
      // SeeYou showed the problem.
      //
      // 09.03.2005 Fughe: The pressure altitude may have a dash, '-' for
      //                   negative hights according the tech specs of IGC-files
      //                   from 31 December 2004.

      fprintf(figc, "%05d", (short) (fil_p[0] << 8) + fil_p[1]);
      fil_p += 2;
      fprintf(figc, "%05d", (fil_p[0] << 8) + fil_p[1]);
      fil_p += 2;
      if (!fix_ext_num) {
        fprintf(figc,"\r\n");
      }
      break;

    case ORIGIN:
      /* a0h                         */
      /* 4   byte, time              */
      /* 4   byte, lat               */
      /* 4   byte, lon               */
      fil_p++;
      time = (fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) + fil_p[3];
      fil_p += 4;
      fprintf(figc, "L%sORIGIN%02d%02d%02d", manufactureKey, time / 3600, time % 3600 / 60, time % 60 );

      fix_lat= ((char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] <<8 ) + fil_p[3];
      fil_p += 4;
      fprintf(figc, "%02d%05d%c", abs(fix_lat / 60000), abs(fix_lat % 60000), fix_lat >= 0 ? 'N' : 'S');

      fix_lon = ((char) fil_p[0] << 24) + (fil_p[1] << 16) + (fil_p[2] << 8) +fil_p[3];
      fil_p += 4;
      fprintf(figc, "%03d%05d%c\r\n", abs(fix_lon / 60000), abs(fix_lon % 60000), fix_lon >= 0 ? 'E' : 'W');
      time_orig = time;
      fix_lat_orig = fix_lat;
      fix_lon_orig = fix_lon;
      break;

    case END:
      /* 40h                         */
      fil_p = fil_p_last;   /* end of while loop */
      break;

    case SECURITY_OLD:
      /* f5h                         */
      fil_p++;
      fprintf(figc, "G%22.22s\r\n", fil_p);
      fil_p += 22;
      break;
    case SECURITY:
      /* f0h                         */
      /* 1   byte, len               */
      /* 1   byte, type              */
      /* 64  byte, char              */
      fil_p++;

      switch(fil_p[1]) {
      case HIGH_SECURITY:
        fprintf(figc, "G2");
        break;
      case MED_SECURITY:
        fprintf(figc, "G1");
        break;
      case LOW_SECURITY:
        fprintf(figc, "G0");
        break;
      }

      for(i = 0; i < fil_p[0]; i++) {
        fprintf(figc, "%02X", fil_p[i + 2]);
      }
      fprintf(figc, "\r\n");
      fil_p += 66;
      break;
    case SECURITY_NEW:
                /* efh                         */
                /* 1   byte, len or type       */
                /* 64  byte, char              */
      fil_p++;

      fprintf(figc, "G3");

      for(i=0;i<(2+fil_p[0]);i++)
      {
        if(!((i-31)%32))
        {
          fprintf(figc, "\r\nG");
        }
        fprintf(figc, "%02X", fil_p[i+1]);
      }
      fprintf(figc, "\r\n");
      fil_p+=0x83;
      break;
    default:        /* ???? */
      fprintf(figc, "L%sUNKNOWN%#x\r\n", manufactureKey, fil_p[0]);
      fil_p++;
      _errorinfo = tr("unexpected record id in '.fil'-file");
      return false;
      break;
    }
  }

  return true;
}

/**
 * Calculate the check sum
 */

// use unsigned char instead of char ! On arm architecture, char is unsigned, on desktop it is signed !!!
unsigned char Filser::calcCrc(unsigned char d, unsigned char crc)
{
  unsigned char tmp;
  static const unsigned char crcpoly = 0x69; /* Static value for the calculation of the checksum. */

  for(int count = 8; --count >= 0; d <<= 1) {
    tmp = crc ^ d;
    crc <<= 1;
    if(tmp & 0x80) {
      crc ^= crcpoly;
    }
  }
  return crc;
}

/**
 * Calculate the check sum on a buffer of bytes
 */
unsigned char Filser::calcCrcBuf(const void *buf, unsigned int count)
{
  unsigned int i;
  unsigned char crc = 0xff;
  for(i = 0; i < count; i++) {
    crc = calcCrc(((unsigned char*)buf)[i], crc);
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
unsigned char *Filser::readData(unsigned char *bufP, int count)
{
  int rc;
  switch (rc = read(portID, bufP, count)) {
  case -1:
    qWarning("read_data(): ERROR");
    break;
  default:
    // qDebug ("readData: %x(%x)", rc, count);
    bufP += rc;
    break;
  }
  return bufP;
}

unsigned char *Filser::writeData(unsigned char *bufP, int count)
{
  int rc;
  switch (rc = write(portID, bufP, count))
  {
  case -1:
    qWarning("write_data(): ERROR");
    break;
  default:
    // qDebug ("writeData: %x(%x)", rc, count);
    bufP += rc;
    break;
  }
  return bufP;
}

bool Filser::readMemSetting()
{
  unsigned char *bufP;
  unsigned char buf[BUFSIZE + 1];

  memset(buf, '\0', sizeof(buf));

  if (!check4Device()) {
    return false;
  }

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(Q);

  bufP = buf;
  while ((LX_MEM_RET + buf - bufP) > 0) {
    bufP = readData(bufP, LX_MEM_RET + buf - bufP);
  }
  // uncomment the next statement to analyze the buffer
  // debugHex (buf, LX_MEM_RET);

  if(calcCrcBuf(buf, LX_MEM_RET-1) != buf[LX_MEM_RET-1])
  {
    qDebug("read_mem_setting(): Bad CRC");
    return false;
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
  qWarning("read_mem_setting(): all fine!!");

  return true;
}

/**
 * Check presence of LX-device and make CONNECT
 *
 * Necessary wakeup before a command
 * if the LX-device is in TIMEOUT countdown
 * mode waiting for CONNECT.
 */
bool Filser::AutoBaud()
{
  speed_t autospeed;
  int     autobaud = 38400;
  bool rc = false;
  time_t t1;
  _errorinfo = "";
  int lc = 0 ;

  // ( - Christian - )
  // Give the recorder the time of 1 sec to answer.
  //
  // It is not the delay, it is the position of tcflush, that is good
  // medicine for my LX20 not to whistle. Tcflush belongs after while() and
  // before wb(SYN).

  t1 = time(NULL);
  while (!breakTransfer) {
    tcflush(portID, TCIOFLUSH); // Make sure the next ACK comes from the
                                // following wb(SYN). And remove the
                                // position data, that might have been
                                // arrived.
    wb(SYN);
    tcdrain (portID);

    while(0xff != rb())       // 12.03.2005 Fughe: Make the stream really
      lc++;                   //                   empty!
    qWarning ("while _AB: %d", lc);
    wb(SYN);
    tcdrain (portID);

    int ret = rb();
    if (ret == ACK) {
      rc = true;
      break;
    }
    else {
      // waiting 10 secs. for response
//      qDebug ("ret = %x", ret);
      if (time(NULL) - t1 > 10) {
        _errorinfo = tr("No response from recorder within 10 seconds!\nDid you press WRITE/RTE?");
        rc = false;
        break;
      }
    }

    //
    // ( - Christian - )
    //
    // Autobauding :-)
    //
    // this way we do autobauding each time this function is called.
    // Shouldn't we do it in OpenRecorder?
    if     (autobaud >= 38400) { autobaud = 19200; autospeed = B38400; }
    else if(autobaud >= 19200) { autobaud =  9600; autospeed = B19200; }
    else if(autobaud >=  9600) { autobaud =  4800; autospeed = B9600; }
    else if(autobaud >=  4800) { autobaud =  2400; autospeed = B4800; }
    else                       { autobaud = 38400; autospeed = B2400; }

    cfsetospeed(&newTermEnv, autospeed);
    cfsetispeed(&newTermEnv, autospeed);
    if (_speed != autospeed)
    {
      _speed = autospeed;
      switch (_speed)
      {
        case B2400:
          emit newSpeed (2400);
          qDebug ("autospeed: %d", 2400);
          break;
        case B4800:
          emit newSpeed (4800);
          qDebug ("autospeed: %d", 4800);
          break;
        case B9600:
          emit newSpeed (9600);
          qDebug ("autospeed: %d", 9600);
          break;
        case B19200:
          emit newSpeed (19200);
          qDebug ("autospeed: %d", 19200);
          break;
        case B38400:
          emit newSpeed (38400);
          qDebug ("autospeed: %d", 38400);
          break;
        default:
          qDebug ("autospeed: illegal value");
          break;
      }
    }

    tcsetattr(portID, TCSANOW, &newTermEnv);

  }
  return rc;
}

/**
 * Check presence of LX-device and make CONNECT
 *
 * Necessary wakeup before a command
 * if the LX-device is in TIMEOUT countdown
 * mode waiting for CONNECT.
 */
bool Filser::check4Device()
{
  //speed_t autospeed;
  //int     autobaud = 38400;
  bool rc = false;
  time_t t1;
  _errorinfo = "";
  int lc = 0 ;

  // ( - Christian - )
  // Give the recorder the time of 1 sec to answer.
  //
  // It is not the delay, it is the position of tcflush, that is good
  // medicine for my LX20 not to whistle. Tcflush belongs after while() and
  // before wb(SYN).

  t1 = time(NULL);
  while (!breakTransfer) {
    tcflush(portID, TCIOFLUSH); // Make sure the next ACK comes from the
                                // following wb(SYN). And remove the
                                // position data, that might have been
                                // arrived.
    wb(SYN);
    tcdrain (portID);

    while(0xff != rb())         // 12.03.2005 Fughe: Make the stream really
      lc++;                     //                   empty!
    qWarning ("while c4d: %d", lc);
    wb(SYN);
    tcdrain (portID);

    int ret = rb();
    if (ret == ACK) {
      rc = true;
      break;
    }
    else {
      // waiting 10 secs. for response
//      qDebug ("ret = %x", ret);
      if (time(NULL) - t1 > 10) {
        _errorinfo = tr("No response from recorder within 10 seconds!\nDid you press WRITE/RTE?");
        rc = false;
        break;
      }
    }

//    //
//    // ( - Christian - )
//    //
//    // Autobauding :-)
//    //
//    // this way we do autobauding each time this function is called.
//    // Shouldn't we do it in OpenRecorder?
//    if     (autobaud >= 38400) { autobaud = 19200; autospeed = B38400; }
//    else if(autobaud >= 19200) { autobaud =  9600; autospeed = B19200; }
//    else if(autobaud >=  9600) { autobaud =  4800; autospeed = B9600; }
//    else if(autobaud >=  4800) { autobaud =  2400; autospeed = B4800; }
//    else                       { autobaud = 38400; autospeed = B2400; }
//
//    cfsetospeed(&newTermEnv, autospeed);
//    cfsetispeed(&newTermEnv, autospeed);
//    if (_speed != autospeed)
//    {
//      _speed = autospeed;
//      switch (_speed)
//      {
//        case B2400:
//          emit newSpeed (2400);
//          qDebug ("autospeed: %d", 2400);
//          break;
//        case B4800:
//          emit newSpeed (4800);
//          qDebug ("autospeed: %d", 4800);
//          break;
//        case B9600:
//          emit newSpeed (9600);
//          qDebug ("autospeed: %d", 9600);
//          break;
//        case B19200:
//          emit newSpeed (19200);
//          qDebug ("autospeed: %d", 19200);
//          break;
//        case B38400:
//          emit newSpeed (38400);
//          qDebug ("autospeed: %d", 38400);
//          break;
//        default:
//          qDebug ("autospeed: illegal value");
//      }
//    }
//
//    tcsetattr(portID, TCSANOW, &newTermEnv);

  }

  return rc;
}

/*
 * write byte
 */
int Filser::wb(unsigned char c)
{
  // qDebug ("wb (%x)", c);
  if( write( portID, &c, 1 ) != 1 )
    {
      return -1;
    }
  return 1;
}

/*
 * read byte
 */
// use unsigned char instead of char!
// On the arm architecture, char is unsigned!
// On desktop, it is signed !!!
unsigned char Filser::rb()
{
  unsigned char buf;

  if( read( portID, &buf, 1 ) != 1 )
    {
      return 0xff;
    }

  return buf;
}

char *Filser::wordtoserno(unsigned int Binaer)
{
  static char Seriennummer[4];
  // limitation
  if (Binaer > 46655L) {
    Binaer = 46655L;
  }

  Seriennummer[0]=c36[Binaer / 36 / 36];
  Seriennummer[1]=c36[Binaer / 36 % 36];
  Seriennummer[2]=c36[Binaer % 36];
  Seriennummer[3] = '\0';

  return Seriennummer;
}

int Filser::closeRecorder()
{
  if( portID != -1 )
    {
      _keepalive->stop();
      tcsetattr( portID, TCSANOW, &oldTermEnv );
      close( portID );
      portID = -1;
      _isConnected = false;
      _da4BufferValid = false;
      return FR_OK;
    }
  else
    {
      return FR_ERROR;
    }
}

/** NOT IMLEMENTED
    ============================================*/


int Filser::exportDeclaration(FRTaskDeclaration* , QList<Waypoint*>*, const QString& )
{
  return FR_NOTSUPPORTED;
}

int Filser::writeDeclaration(FRTaskDeclaration* , QList<Waypoint*>*, const QString& )
{
  return FR_NOTSUPPORTED;
}

int Filser::readDatabase()
{
  return FR_NOTSUPPORTED;
}

/**
  * read the tasks from the lx recorder
  * read the da4 buffer and select tasks
  * tasks are constructed from waypoints in the same buffer !
  */
int Filser::readTasks(QList<FlightTask*> * tasks)
{
  qDebug ("Filser::readTasks");

  int result = readDA4Buffer();
  if (result != FR_OK)
    return result;

  for (int RecordNumber = 0; RecordNumber < _capabilities.maxNrTasks; RecordNumber++)
  {
    emit progress (false, RecordNumber, _capabilities.maxNrTasks);
    if (_da4Buffer.tasks[RecordNumber].prg)
    {
      DA4TaskRecord record (&_da4Buffer.tasks[RecordNumber]);
      QList<Waypoint*> wplist;
      Waypoint* wp;
      for (int i = 0; i < _capabilities.maxNrWaypointsPerTask; i++)
      {
        if (record.pnttype(i) != 0)
        {
          DA4WPRecord wprecord (&_da4Buffer.waypoints[record.pntind(i)]);
          wp = wprecord.newWaypoint();
          wp->type = FlightTask::RouteP;
          if (wplist.count() == 0)
          {
            // append take off
            wp->type = FlightTask::TakeOff;
            wplist.append(wp);
            // make copy for begin
            wp = new Waypoint(wplist.first());
            wp->type = FlightTask::Begin;
          }
          wplist.append (wp);
        }
      }
      // modify last for end of task
      wplist.last()->type = FlightTask::End;
      // make copy for landing
      wp = new Waypoint(wplist.last());
      wp->type = FlightTask::Landing;
      wplist.append (wp);

      tasks->append (new FlightTask (wplist, true, QString("TSK%1").arg(RecordNumber)));
    }
  }
  emit progress (true, 100, 100);

  return FR_OK;
}

/**
  * find a waypoint in the recorder database.
  * if not found, create a new one
  */
int Filser::findWaypoint (Waypoint* wp)
{
  qDebug ("Filser::findWaypoint");
  int freeRecord = -1;
  for (int RecordNumber = 0; RecordNumber < (int)_capabilities.maxNrWaypoints; RecordNumber++)
  {
    DA4WPRecord wprecord (&_da4Buffer.waypoints[RecordNumber]);
    QString str1 = wprecord.name();
    QString str2 = wp->name;
    if (str1.trimmed().toUpper() == str2.trimmed().toUpper())
    {
      // make sure the waypoint contains the data we want
      wprecord.setWaypoint(wp);
      qDebug ("waypoint %s found at %d", wp->name.toLatin1().data(), RecordNumber);
      return RecordNumber;
    }
    else if ((wprecord.type() == BaseMapElement::NotSelected) && (freeRecord == -1))
      freeRecord = RecordNumber;
  }
  // if none was found, use a free entry
  if (freeRecord >= 0)
  {
    DA4WPRecord wprecord (&_da4Buffer.waypoints[freeRecord]);
    wprecord.setWaypoint(wp);
    qDebug ("waypoint %s not found. created at %d", wp->name.toLatin1().data(), freeRecord);
    return freeRecord;
  }
  qDebug ("waypoint not found");
  return -1;
}

/**
  * write the tasks to the lx recorder
  * read the da4 buffer
  * write tasks; if the proper waypoints are not in the buffer,
  * they are constructed on the fly
  * write the buffer back to recorder
  */
int Filser::writeTasks(QList<FlightTask*>* tasks)
{
  qDebug ("Filser::writeTasks");

  int result = readDA4Buffer ();
  if (result != FR_OK)
    return result;

  int RecordNumber = 0;
  FlightTask *task;

  foreach(task, *tasks)
  {
    DA4TaskRecord taskrecord (&_da4Buffer.tasks[RecordNumber++]);
    taskrecord.clear();
    // should never happen
    if (RecordNumber >= (int)_capabilities.maxNrTasks)
      break;
    taskrecord.setPrg (1);
    QList<Waypoint*> wplist = task->getWPList();
    int wpCnt = 0;
    Waypoint *wp;

    foreach(wp, wplist)
    {
      // should never happen
      if (wpCnt >= (int)_capabilities.maxNrWaypointsPerTask)
        break;

      // ignore take off and landing
      if (wp->type == FlightTask::TakeOff || wp->type == FlightTask::Landing)
        continue;

      int index = findWaypoint (wp);

      if (index >= 0)
        taskrecord.setInd (wpCnt, index);

      wpCnt++;
    }
  }

  while( RecordNumber < (int) _capabilities.maxNrTasks )
    {
      DA4TaskRecord taskrecord( &_da4Buffer.tasks[RecordNumber++] );
      taskrecord.clear();
    }

  _da4BufferValid = false;

  return writeDA4Buffer();
}

/**
  * read the da4 buffer from the lx recorder
  * it contains both waypoints and tasks
  */
int Filser::readDA4Buffer()
{
  if( _da4BufferValid )
    {
      return FR_OK;
    }

  if( !readMemSetting() )
    {
      return FR_ERROR;
    }

  _errorinfo = "";

  tcflush( portID, TCIOFLUSH );

  wb( STX );
  wb( R );

  unsigned char* bufP = (unsigned char*) &_da4Buffer;

  while ((bufP - (unsigned char*)&_da4Buffer) < (int)sizeof (DA4Buffer))
  {
    bufP = readData(bufP, (sizeof (DA4Buffer) + (unsigned char*)&_da4Buffer - bufP));
  }

  if( rb() != calcCrcBuf( &_da4Buffer, sizeof(DA4Buffer) ) )
    {
      _errorinfo = tr( "Filser::readWaypoints(): Bad CRC" );
      qDebug( "%s", _errorinfo.toLatin1().data() );
      return FR_ERROR;
    }

  _da4BufferValid = true;
  return FR_OK;
}

/**
  * read the waypoints from the lx recorder
  * read the da4 buffer and select waypoints
  */
int Filser::readWaypoints(QList<Waypoint*>* wpList)
{
  qDebug ("Filser::readWaypoints");

  int result = readDA4Buffer();

  if( result != FR_OK )
    {
      return result;
    }

  for (int RecordNumber = 0; RecordNumber < (int)_capabilities.maxNrWaypoints; RecordNumber++)
    {
      DA4WPRecord record( &_da4Buffer.waypoints[RecordNumber] );

      // debugHex (buffer.waypoints[RecordNumber], WAYPOINT_WIDTH);
      if( record.type() == 0 && record.name().isEmpty() )
        {
          qDebug( "this should never happen !!?" );
          break;
        }

      if( record.type() != BaseMapElement::NotSelected )
        {
          if( record.name().trimmed().isEmpty() )
            {
              continue;
            }

          Waypoint *w = record.newWaypoint();
          wpList->append( w );
        }
    }

  return FR_OK;
}

/**
  * write the da4 buffer to the lx recorder
  * it contains both waypoints and tasks
  */
int Filser::writeDA4Buffer()
{
  if( _da4BufferValid )
    {
      return FR_OK;
    }

  if( !readMemSetting() )
    {
      return FR_ERROR;
    }

  _errorinfo = "";

  tcflush(portID, TCIOFLUSH);

  wb(STX);
  wb(W);

  // transfer data to logger
  unsigned char *bufP = (unsigned char*)&_da4Buffer;

  while ((bufP - (unsigned char*)&_da4Buffer) < (int)sizeof (DA4Buffer))
  {
    bufP = writeData(bufP, (sizeof (DA4Buffer) + (unsigned char*)&_da4Buffer - bufP));
  }

  unsigned char crc = calcCrcBuf (&_da4Buffer, sizeof (DA4Buffer));
  wb (crc);

  // wait until all output has been written
  tcdrain(portID);
  int result = rb();

  if( result == ACK )
    {
      _da4BufferValid = true;
      return FR_OK;
    }
  else if( result == NAK )
    {
      _errorinfo = tr( "Filser::writeDA4Buffer: Bad CRC" );
      qDebug( "%s", _errorinfo.toLatin1().data() );
      return FR_ERROR;
    }
  else
    {
      _errorinfo = tr( "Filser::writeDA4Buffer: transfer failed" );
      qDebug( "%s", _errorinfo.toLatin1().data() );
      return FR_ERROR;
    }
}

/**
  * write the waypoints to the lx recorder
  * read the da4 buffer
  * write waypoints
  * write the buffer back to recorder
  */
int Filser::writeWaypoints(QList<Waypoint*>* wpList)
{
  qDebug( "Filser::writeWaypoints" );

  int result = readDA4Buffer();

  if( result != FR_OK )
    {
      return result;
    }

  int RecordNumber = 0;
  Waypoint *wp;

  foreach(wp, *wpList)
    {
      DA4WPRecord record (&_da4Buffer.waypoints[RecordNumber++]);

      record.setWaypoint (wp);
    }

  // fill rest of waypoints
  while( RecordNumber < _capabilities.maxNrWaypoints )
    {
      DA4WPRecord record( &_da4Buffer.waypoints[RecordNumber++] );
      record.clear();
    }

  _da4BufferValid = false;

  return writeDA4Buffer();
}

/**
 * Opens the recorder for other communication.
 */
int Filser::openRecorder(const QString& /*URL*/)
{
  return FR_NOTSUPPORTED;
}
