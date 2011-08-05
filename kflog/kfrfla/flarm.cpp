/***********************************************************************
**
**   flarm.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2011 by Eggert Ehmke
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: flarm.cpp 1236 2011-06-27 20:05:37Z axel $
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
#include <QFileDialog>

#include "flarm.h"

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

// for the flarmcfg file we need DOS line feeds
#define ENDL "\r\n"

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

  char manufactureShortKey = 'X';
  char manufactureKey[] = "xxx";  // Let's start with an empty key. If 'xxx'
                                  // appears, then reading the 'A'-record
                                  // failed.

  unsigned char *memContents; /* buffer to hold igc contents */
  int contentSize;            /* length of igc file buffer   */


/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the program. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int /* signal*/)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
}

Flarm::Flarm( QObject *parent ) : FlightRecorderPluginBase( parent )
{
  //Set flight recorders capabilities. Defaults are 0 and false.
  //_capabilities.maxNrTasks = TASK_MAX;             //maximum number of tasks
  _capabilities.maxNrTasks = 1;                      //maximum number of tasks
  _capabilities.maxNrWaypoints = WAYPOINT_MAX;       //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = MAXTSKPNT;   //maximum number of waypoints per task
  _capabilities.maxNrPilots = 1;                     //maximum number of pilots
  _capabilities.transferSpeeds = bps02400 |          //supported transfer speeds
                                 bps04800 |
                                 bps09600 |
                                 bps19200 |
                                 bps38400;

  _capabilities.supDlWaypoint = false;       //supports downloading of waypoints?
  _capabilities.supUlWaypoint = false;       //supports uploading of waypoints?
  _capabilities.supDlFlight = false;         //supports downloading of flights?
  _capabilities.supUlFlight = false;         //supports uploading of flights?
  _capabilities.supSignedFlight = false;     //supports downloading in of signed flights?
  _capabilities.supDlTask = false;           //supports downloading of tasks?
  _capabilities.supUlTask = false;           //supports uploading of tasks?
  _capabilities.supExportDeclaration = true; //supports export of declaration?
  _capabilities.supUlDeclaration = false;    //supports uploading of declarations?
  _capabilities.supDspSerialNumber = false;
  _capabilities.supDspRecorderType = false;
  _capabilities.supDspPilotName = true;
  _capabilities.supDspGliderType = true;
  _capabilities.supDspGliderID = true;
  _capabilities.supDspCompetitionID = false;
  _capabilities.supAutoSpeed = false;       //supports automatic transfer speed detection
  //End set capabilities.

  portID = -1;
  //_da4BufferValid = false;

  _keepalive = new QTimer( this );
  connect( _keepalive, SIGNAL(timeout()), this, SLOT(slotTimeout()) );
}

Flarm::~Flarm()
{
  closeRecorder();
  qDeleteAll( flightIndex );
}

/**
  * this function will be called each second to keep the connection to FLARM device alive
  */
void Flarm::slotTimeout()
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
      qDebug( "Flarm::keepalive failed: ret = %x", ret );
    }
}

/**
 * Returns the transfer mode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Flarm::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

int Flarm::getFlightDir( QList<FRDirEntry *>* dirList )
{
  Q_UNUSED(dirList)

  qDebug ("Flarm::getFlightDir");

  return FR_NOTSUPPORTED;
}

/**
  * This function retrieves the basic recorder data from the flarm device
  * currently supported are: serial number, devive type, pilot name, glider type, glider id, competition id.
  * Written by Eggert Ehmke <eggert.ehmke@berlin.de>, <eggert@kflog.org>
  */
int Flarm::getBasicData(FR_BasicData& data)
{
  Q_UNUSED(data)

  // TODO: adapt to FLARM
  qDebug ("Flarm::getBasicData");

  return FR_NOTSUPPORTED;
}

int Flarm::getConfigData(FR_ConfigData& /*data*/)
{
  return FR_NOTSUPPORTED;
}

int Flarm::writeConfigData(FR_BasicData& /*basicdata*/, FR_ConfigData& /*configdata*/)
{
  return FR_NOTSUPPORTED;
}

int Flarm::downloadFlight(int /*flightID*/, int /*secMode*/, const QString& /*fileName*/)
{
  qDebug ("Flarm::downloadFlight");

  return FR_NOTSUPPORTED;
}

int Flarm::openRecorder(const QString& pName, int baud)
{
  //TODO: adapt to FLARM
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
//  These are the only speeds known by Flarm devices, right?
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
    //_da4BufferValid = false;

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


/**
  * this method copied from Cumulus
  * NMEA-0183 Standard
  * The optional checksum field consists of a "*" and two hex digits
  * representing the exclusive OR of all characters between, but not
  * including, the "$" and "*".  A checksum is required on some sentences.
  */
ushort Flarm::calcCheckSum (int pos, const QString& sentence)  
{
  ushort sum = 0;
  
  for( int i=1; i < pos; i++ ) {
    ushort c = (sentence[i]).toAscii();

    if( c == '$' ) // Start sign will not be considered
      continue;

    if( c == '*' ) // End of sentence reached
      break;

    sum ^= c;
  }

  return sum;
}

/** 
  * This method copied from Cumulus
  * This method checks if the checksum in the sentence matches the sentence. It retuns true if it matches, and false otherwise. 
  */
bool Flarm::checkCheckSum(int pos, const QString& sentence)
{
  ushort check = (ushort) sentence.right(2).toUShort(0, 16);
  return (check == calcCheckSum (pos, sentence));
}

/*
 * Read the memory setup from the lx device.
 *
 * The lx devices do have a memory setup described with three bytes.
 * In the future this function might be necessary to disconnect from lx
 * devices with more memory. The CRC check will tell.
 */
unsigned char *Flarm::readData(unsigned char *bufP, int count)
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

unsigned char *Flarm::writeData(unsigned char *bufP, int count)
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

/**
 * Check presence of FLARM-device and make CONNECT
 *
 * Necessary wakeup before a command
 * if the FLARM-device is in TIMEOUT countdown
 * mode waiting for CONNECT.
 */
bool Flarm::AutoBaud()
{
  //TODO: adapt to FLARM
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
  // medicine for my FLARM not to whistle. Tcflush belongs after while() and
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
        _errorinfo = tr("No response from recorder within 10 seconds!\n");
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
      }
    }

    tcsetattr(portID, TCSANOW, &newTermEnv);

  }
  return rc;
}

/**
 * Check presence of Flarm-device and make CONNECT
 *
 * Necessary wakeup before a command
 * if the FLARM-device is in TIMEOUT countdown
 * mode waiting for CONNECT.
 */
bool Flarm::check4Device()
{
  //TODO: adapt to FLARM
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
  // medicine for my FLARM not to whistle. Tcflush belongs after while() and
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
        _errorinfo = tr("No response from recorder within 10 seconds!\n");
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
int Flarm::wb(unsigned char c)
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
unsigned char Flarm::rb()
{
  unsigned char buf;

  if( read( portID, &buf, 1 ) != 1 )
    {
      return 0xff;
    }

  return buf;
}

char *Flarm::wordtoserno(unsigned int Binaer)
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

int Flarm::closeRecorder()
{
  if( portID != -1 )
    {
      _keepalive->stop();
      tcsetattr( portID, TCSANOW, &oldTermEnv );
      close( portID );
      portID = -1;
      _isConnected = false;
      //_da4BufferValid = false;
      return FR_OK;
    }
  else
    {
      return FR_ERROR;
    }
}

/**
  * create lat string as defined by flarm docu
  * @Author: eggert.ehmke@berlin.de
  */
QString Flarm::lat2flarm(int lat)
{
  QString hemisphere = (lat>=0) ? "N" : "S";
  lat = abs(lat);

  int deg, min, sec;
  WGSPoint::calcPos (lat, deg, min, sec);
  // in Flarm spec this is defined as 1/1000 minutes.
  int dec = (sec / 60.0) * 1000;
  
  QString result = QString().sprintf("%02d%02d%03d", deg, min, dec);
  result += hemisphere;
  return result;
}
               
/**
  * create lon string as defined by flarm docu
  * @Author: eggert.ehmke@berlin.de
  */
QString Flarm::lon2flarm(int lon)
{
  QString hemisphere = (lon>=0) ? "E" : "W";
  lon = abs(lon);
  int deg, min, sec;
  WGSPoint::calcPos (lon, deg, min, sec);
  // in Flarm spec this is defined as 1/1000 minutes.
  int dec = (sec / 60.0) * 1000;
  QString result = QString().sprintf("%03d%02d%03d", deg, min, dec);
  result += hemisphere;
  return result;
}

int Flarm::writeDeclaration(FRTaskDeclaration* , QList<Waypoint*>* )
{
  return FR_NOTSUPPORTED;
}

/**
  * export flight declaration to flarmcfg.txt file
  * @Author: eggert.ehmke@berlin.de
  */
int Flarm::exportDeclaration(FRTaskDeclaration* decl, QList<Waypoint*>* wpList)
{
    //TODO: reuse for upload
    qDebug ("Flarm::exportDeclaration");

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),
                            QDir::homePath() + QDir::separator() + "flarmcfg.txt",
                            tr("FlarmCfg (flarmcfg.txt)"), 0);

    QFile file (fileName);
    file.setPermissions (QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
    if(!file.open(QIODevice::WriteOnly)) {
      qDebug()<<"Error opening the file";
      return FR_ERROR;
    }
    
    QTextStream stream(&file);

    int result = sendStreamData (stream, decl, wpList, true);
    file.close();
    return result;
}

void Flarm::sendStreamComment (QTextStream& stream, const QString& comment, bool isFile) {
  if (isFile)
    stream << "// " << comment << ENDL;
}

void Flarm::sendStreamData (QTextStream& stream, const QString& sentence, bool isFile) {
  if (isFile)
    stream << sentence << ENDL;
  else {
    ushort cs = calcCheckSum (sentence.length(), sentence);
    // qDebug () << "cs: " << cs << endl;
    QString str = sentence + "*";
    QString ccr = QString ("%1").arg (cs, 2, 16, QChar('0'));
    stream << str << ccr << ENDL;
  }
}

int Flarm::sendStreamData (QTextStream& stream, FRTaskDeclaration* decl, QList<Waypoint*>* wpList, bool isFile) {

    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString ();

    sendStreamComment (stream, "FLARM configuration file has been created by KFlog", isFile);
    sendStreamComment (stream, timestamp, isFile);

    sendStreamComment (stream, "activated competition mode", isFile);
    sendStreamData (stream, "$PFLAC,S,CFLAGS,2", isFile);

    sendStreamComment (stream, "deaktivated Stealth mode", isFile);
    sendStreamData (stream, "$PFLAC,S,PRIV,0", isFile);

    sendStreamComment (stream, "aircraft type;  1 = glider", isFile);
    sendStreamData (stream, "$PFLAC,S,ACFT,1", isFile);

    sendStreamComment (stream, "Pilot name", isFile);
    sendStreamData (stream, "$PFLAC,S,PILOT," + decl->pilotA, isFile);

    if (!decl->pilotB.isEmpty()) {
      sendStreamComment (stream, "Copilot name", isFile);
      sendStreamData (stream, "$PFLAC,S,COPIL," + decl->pilotB, isFile);
    }

    sendStreamComment (stream, "Glider type", isFile);
    sendStreamData (stream, "$PFLAC,S,GLIDERTYPE," + decl->gliderType, isFile);

    sendStreamComment (stream, "Aircraft registration", isFile);
    sendStreamData (stream, "$PFLAC,S,GLIDERID," + decl->gliderID, isFile);

    sendStreamComment (stream, "Competition ID", isFile);
    sendStreamData (stream, "$PFLAC,S,COMPID," + decl->compID, isFile);

    sendStreamComment (stream, "Competition Class", isFile);
    sendStreamData (stream, "$PFLAC,S,COMPCLASS," + decl->compClass, isFile);

    //TODO: make configurable?
    sendStreamComment (stream, "Logger interval", isFile);
    sendStreamData (stream, "$PFLAC,S,LOGINT,4", isFile);

    //TODO: use task name?
    sendStreamComment (stream, "Task declaration", isFile);
    sendStreamData (stream, "$PFLAC,S,NEWTASK,new task", isFile);

    int wpCnt = 0;
    Waypoint *wp; 

    foreach(wp, *wpList)
    {
        // should never happen
        if (wpCnt >= (int)_capabilities.maxNrWaypointsPerTask)
            break;

        // ignore take off and landing
        //if (wp->type == FlightTask::TakeOff || wp->type == FlightTask::Landing)
        //    continue;

        //int index = findWaypoint (wp);
        // qDebug ("wp: %s", wp->name.toLatin1().constData());
        sendStreamData (stream, "$PFLAC,S,ADDWP," + lat2flarm(wp->origP.lat()) + "," + lon2flarm(wp->origP.lon()) + "," + wp->name, isFile);
    }

    return FR_OK;
}

int Flarm::readDatabase()
{
  return FR_NOTSUPPORTED;
}

/**
  * read the tasks from the lx recorder
  * read the da4 buffer and select tasks
  * tasks are constructed from waypoints in the same buffer !
  */
int Flarm::readTasks(QList<FlightTask*> * /*tasks*/)
{
  qDebug ("Flarm::readTasks");
  return FR_NOTSUPPORTED;
}

/**
  * write the tasks to the flarm recorder
  * read the da4 buffer
  * write tasks; if the proper waypoints are not in the buffer,
  * they are constructed on the fly
  * write the buffer back to recorder
  */
int Flarm::writeTasks(QList<FlightTask*>* /*tasks*/)
{
  qDebug ("Flarm::writeTasks");

  return FR_NOTSUPPORTED;
}

/**
  * read the waypoints from the lx recorder
  * read the da4 buffer and select waypoints
  */
int Flarm::readWaypoints(QList<Waypoint*>* /*wpList*/)
{
  qDebug ("Flarm::readWaypoints");

  return FR_NOTSUPPORTED;
}

/**
  * write the waypoints to the flarm recorder
  * read the da4 buffer
  * write waypoints
  * write the buffer back to recorder
  */
int Flarm::writeWaypoints(QList<Waypoint*>* /*wpList*/)
{
  qDebug( "Flarm::writeWaypoints" );

  return FR_NOTSUPPORTED;
}

/**
 * Opens the recorder for other communication.
 */
int Flarm::openRecorder(const QString& /*URL*/)
{
  return FR_NOTSUPPORTED;
}
