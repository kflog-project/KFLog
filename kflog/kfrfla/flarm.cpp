/***********************************************************************
**
**   flarm.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2011 by Eggert Ehmke
**                   2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   This module implements the Flarm data port protocol as specified
**   in the document 
**   	DATA PORT SPECIFICATIONS Software version 5.00 (Mar 01, 2011)
**    or newer. This document is issued by
**   	2011 FLARM Technology GmbH
**	Baar-Switzerland
**   http://www.flarm.com/support/manual/FLARM_DataportManual_v5.00E.pdf
**
***********************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <ctype.h>
#include <cmath>
#include <cstring>

#include <QtWidgets>

#include "flarm.h"

/*logger defines*/

// for the flarmcfg file we need DOS line feeds
#define ENDL "\r\n"

/**
 * The device-name of the port.
 */
const char* portName = "\0";
int portID = -1;

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
 * of the program. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int /* signal*/)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
}

Flarm::Flarm( QObject *parent ) : FlightRecorderPluginBase( parent ),
  _speed(B0)
{
  //Set flight recorders capabilities. Defaults are 0 and false.
  //_capabilities.maxNrTasks = TASK_MAX;             //maximum number of tasks
  _capabilities.maxNrTasks = 1;                      //maximum number of tasks
  _capabilities.maxNrWaypoints = WAYPOINT_MAX;       //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = MAXTSKPNT;   //maximum number of waypoints per task
  _capabilities.maxNrPilots = 1;                     //maximum number of pilots
  _capabilities.transferSpeeds = bps04800 |          //supported transfer speeds
                                 bps09600 |
                                 bps19200 |
                                 bps38400 |
                                 bps57600;

  _capabilities.supDlWaypoint = false;       //supports downloading of waypoints?
  _capabilities.supUlWaypoint = false;       //supports uploading of waypoints?
  _capabilities.supDlFlight = false;         //supports downloading of flights?
  _capabilities.supUlFlight = false;         //supports uploading of flights?
  _capabilities.supSignedFlight = false;     //supports downloading in of signed flights?
  _capabilities.supDlTask = false;           //supports downloading of tasks? no
  _capabilities.supUlTask = false;           //supports uploading of tasks?
  _capabilities.supExportDeclaration = true; //supports export of declaration?
  _capabilities.supUlDeclaration = true;     //supports uploading of declarations?
  _capabilities.supDspSerialNumber = true;
  _capabilities.supDspRecorderType = true;
  _capabilities.supDspDvcID = true;
  _capabilities.supDspSwVersion = true;
  _capabilities.supDspPilotName = true;
  _capabilities.supDspCoPilotName = true;
  _capabilities.supDspGliderType = true;
  _capabilities.supEditPilotName = true;
  _capabilities.supEditCoPilotName = true;
  _capabilities.supDspGliderType = true;
  _capabilities.supDspGliderID = true;
  _capabilities.supDspCompetitionID = true;
  _capabilities.supAutoSpeed = true;       //supports automatic transfer speed detection
  //End set capabilities.

  portID = -1;

}

Flarm::~Flarm()
{
  closeRecorder();
  qDeleteAll( flightIndex );
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
  * Warning: this method must be adapted to future Flarm versions.
  * The Flarm documentation does not recommend to parse this information.
  * However, this is the only means to read serial # and device type
  */
QString Flarm::getFlarmDebug (QFile& file) {
  QString str = "$PFLAS,R*";
  ushort cs = calcCheckSum (str.length(), str);
  QString ccs = QString ("%1").arg (cs, 2, 16, QChar('0'));
  QString sentence = str + ccs + ENDL;
  qDebug () << "getFlarmDebug cmd: " << sentence << endl;
  file.write (sentence.toLatin1().constData(), sentence.length());
  file.flush();
    
  for (int i=0; i<10;i++){
    QString bytes = file.readLine();
    qDebug () << "debug: " << bytes;
    if (bytes.contains ("Build"))
      return bytes;
  }
  return "";
}


QString Flarm::getFlarmData (QFile& file, const QString& cmd, const QString& key) {
  QString str = cmd + ",R," + key + "*";
  ushort cs = calcCheckSum (str.length(), str);
  QString ccs = QString ("%1").arg (cs, 2, 16, QChar('0'));
  QString sentence = str + ccs + ENDL;
  qDebug () << "getFlarmData cmd: " << sentence << endl;
  file.write (sentence.toLatin1().constData(), sentence.length());
  file.flush();
    
  QString bytes = file.readLine();
  //sometimes some other sentences come inbetween
  QTime t1 = QTime::currentTime ();
  while (!bytes.startsWith (cmd + ",A,")) {
    if (t1.secsTo (QTime::currentTime ()) > 10) {
      qDebug () << "No response from recorder within 10 seconds!" << endl;
      return "";
    }
    qDebug () << "ignored bytes: " << bytes << endl;
    bytes = file.readLine();
  }
  qDebug () << "answer: " << bytes;

  QStringList list = bytes.split("*");
  QString answer = list[0];
  QString checksum = list[1];
  bool ok;
  cs = checksum.toShort (&ok, 16);
  if (!ok) {
    qDebug () << "checksum not readable: " << checksum << endl;
    return "";
  }
  // qDebug () << "checksum valid" << endl;
  if (cs == calcCheckSum (answer.length(), answer)) {
    // qDebug () << "checksum ok" << endl;
    list = answer.split(",");
    return list[3];
  } else {
    qDebug () << "bad Checksum: " << bytes << "; " << checksum << endl;
    return "";
  }
}

bool Flarm::putFlarmData (QFile& file, const QString& cmd, const QString& key, const QString& data1, const QString& data2, const QString& data3) {
  QString str = cmd + ",S," + key;
  if (data1 != NULL)
    str += "," + data1;
  if (data2 != NULL)
    str += "," + data2; 
  if (data3 != NULL)
    str += "," + data3; 
  str += "*";
  ushort cs = calcCheckSum (str.length(), str);
  QString ccs = QString ("%1").arg (cs, 2, 16, QChar('0'));
  QString sentence = str + ccs + ENDL;
  qDebug () << "putFlarmData cmd: " << sentence << endl;
  file.write (sentence.toLatin1().constData(), sentence.length());
  file.flush();

  QString bytes = file.readLine();
  //sometimes some other sentences come inbetween
  QTime t1 = QTime::currentTime();
  while (!bytes.startsWith (cmd + ",A,")) {
    if (t1.secsTo (QTime::currentTime()) > 10) {
      // qDebug () << "No response from recorder within 10 seconds!" << endl;
      _errorinfo = tr("No response from recorder within 10 seconds!\n");
      return false;
    }
    qDebug () << "ignored bytes: " << bytes << endl;
    bytes = file.readLine();
  }
  qDebug () << "putFlarmData answer: " << bytes << endl;

  QStringList list = bytes.split("*");
  QString answer = list[0];
  QString checksum = list[1];
  bool ok;
  cs = checksum.toInt (&ok, 16);
  if (!ok) {
    qDebug () << "checksum not readable: " << checksum << endl;
    return "";
  }
  // qDebug () << "checksum valid" << endl;
  if (cs == calcCheckSum (answer.length(), answer)) {
    return true;
  } else {
    qDebug () << "bad Checksum: " << bytes << "; " << checksum << endl;
    return false;
  }
}

/**
  * This function retrieves the basic recorder data from the flarm device
  * currently supported are: devive id, pilot name, copilot name, glider type, glider id, competition id.
  * Written by Eggert Ehmke <eggert.ehmke@berlin.de>, <eggert@kflog.org>
  */
int Flarm::getBasicData(FR_BasicData& data)
{
  qDebug ("Flarm::getBasicData");
  
  if (!check4Device()) {
    return FR_ERROR;
  }

  QFile file;
  file.open (portID, QIODevice::ReadWrite);
  
  data.pilotName     = getFlarmData (file, "$PFLAC","PILOT");
  data.copilotName   = getFlarmData (file, "$PFLAC","COPIL");
  data.gliderType    = getFlarmData (file, "$PFLAC","GLIDERTYPE");
  data.gliderID      = getFlarmData (file, "$PFLAC","GLIDERID");
  data.competitionID = getFlarmData (file, "$PFLAC","COMPID");
  // this delivers always 0xFFFFFF; we get device id from debug info
  // data.devID  = getFlarmData (file, "$PFLAC","ID");
  data.swVersion     = getFlarmData (file, "$PFLAV","");
  qDebug () << "Version: " << data.swVersion << endl;
  
  QStringList debug  = getFlarmDebug (file).split (",");
  data.recorderType  = debug[0];
  data.serialNumber  = debug[1];
  data.dvcID         = debug[2];

  return FR_OK;
}

int Flarm::getConfigData(FR_ConfigData& /*data*/)
{
  return FR_NOTSUPPORTED;
}

/**
  * This function sends the basic recorder data to the flarm device
  * currently supported are: pilot name, copilot name, glider type, glider id, competition id.
  * Written by Eggert Ehmke <eggert.ehmke@berlin.de>, <eggert@kflog.org>
  */
int Flarm::writeConfigData(FR_BasicData& data, FR_ConfigData& /*configdata*/)
{
  qDebug ("Flarm::writeConfigData");
  
  if (!check4Device()) {
    return FR_ERROR;
  }
  
  QFile file;
  file.open (portID, QIODevice::ReadWrite);

  if (!putFlarmData (file, "$PFLAC", "PILOT", data.pilotName))
    return FR_ERROR;
  if (!putFlarmData (file, "$PFLAC", "COPIL", data.copilotName))
    return FR_ERROR;
  if (!putFlarmData (file, "$PFLAC", "GLIDERTYPE", data.gliderType))
    return FR_ERROR;
  if (!putFlarmData (file, "$PFLAC", "GLIDERID", data.gliderID))
    return FR_ERROR;
  if (!putFlarmData (file, "$PFLAC", "COMPID", data.competitionID))
    return FR_ERROR;

  return FR_OK;
}

int Flarm::downloadFlight(int /*flightID*/, int /*secMode*/, const QString& /*fileName*/)
{
  qDebug ("Flarm::downloadFlight");

  return FR_NOTSUPPORTED;
}

int Flarm::openRecorder(const QString& pName, int baud)
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

//  4800 - 57600 bps
//  These are the only speeds known by Flarm devices
//  Taken from Data port Specification
    if (baud >= 57600)     _speed = B57600;
    else if(baud >= 38400) _speed = B38400;
    else if(baud >= 19200) _speed = B19200;
    else if(baud >=  9600) _speed = B9600;
    else                   _speed = B4800;

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

    return FR_OK;
  } else {
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
    ushort c = (sentence[i]).toLatin1();

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
  * This method checks if the checksum in the sentence matches the sentence.
  * It retuns true if it matches, and false otherwise.
  */
bool Flarm::checkCheckSum(int pos, const QString& sentence)
{
  ushort check = (ushort) sentence.right(2).toUShort(0, 16);
  return (check == calcCheckSum (pos, sentence));
}

/**
 * Check baud rate of FLARM-device
 *
 */
bool Flarm::AutoBaud()
{
  speed_t autospeed;
  int     autobaud = 57600;
  _errorinfo = "";

  QTime t1 = QTime::currentTime();
  while (true) {
    tcflush(portID, TCIOFLUSH);

    QFile file;
    file.open (portID, QIODevice::ReadOnly);
    
    QString bytes = file.readLine();
    qDebug () << "bytes: " << bytes;

    // check for some typical sentences
    if (bytes.contains (QRegExp("^\\$PFLAU|^\\$GPGGA|^\\$PGRMZ|^\\$GPRMC"))) {
      break;
    }
    else {
      // waiting 10 secs. for response
      // qDebug ("ret = %x", ret);
      if (t1.secsTo (QTime::currentTime()) > 10) {
        _errorinfo = tr("No response from recorder within 10 seconds!\n");
        return false;
      }
    }

    //
    // ( - Christian - )
    //
    // Autobauding :-)
    //
    // this way we do autobauding each time this function is called.
    // Shouldn't we do it in OpenRecorder?
    if     (autobaud >= 57600) { autobaud = 38400; autospeed = B57600; }
    else if(autobaud >= 38400) { autobaud = 19200; autospeed = B38400; }
    else if(autobaud >= 19200) { autobaud =  9600; autospeed = B19200; }
    else if(autobaud >=  9600) { autobaud =  4800; autospeed = B9600; }
    else                       { autobaud = 57600; autospeed = B4800; }

    cfsetospeed(&newTermEnv, autospeed);
    cfsetispeed(&newTermEnv, autospeed);
    if (_speed != autospeed)
    {
      _speed = autospeed;
      switch (_speed)
      {
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
        case B57600:
          emit newSpeed (57600);
          qDebug ("autospeed: %d", 57600);
          break;
        default:
          qDebug ("autospeed: illegal value");
      }
    }

    tcsetattr(portID, TCSANOW, &newTermEnv);

  }
  return true;
}

/**
 * Check presence of Flarm-device
 *
 */
bool Flarm::check4Device()
{
  _errorinfo = "";
  
  QFile file;
  file.open (portID, QIODevice::ReadWrite);

  // self test
  QString result = getFlarmData (file, "$PFLAE","");
  if (result.isEmpty()) {
    _errorinfo = tr("No response from flarm device!\n");
    return false;
  }
  if (result.compare ("0") == 0)
    return true;
  else {
    _errorinfo = tr("device failure");
    qDebug () << "device failure: " << result << endl;
    return false;
  }
}

int Flarm::closeRecorder()
{
  if( portID != -1 )
    {
      tcsetattr( portID, TCSANOW, &oldTermEnv );
      close( portID );
      portID = -1;
      _isConnected = false;
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

  QString result = QString().asprintf("%02d%02d%03d", deg, min, dec);
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
  QString result = QString().asprintf("%03d%02d%03d", deg, min, dec);
  result += hemisphere;
  return result;
}

int Flarm::writeDeclaration(FRTaskDeclaration* decl, QList<Waypoint*>* wpList, const QString& name)
{
    qDebug() << "Flarm::writeDeclaration" << endl;
    if (!check4Device())
      return FR_ERROR;
      
    QFile file;
    file.open (portID, QIODevice::ReadWrite);

    // deactivated competition mode
    if (!putFlarmData (file, "$PFLAC", "CFLAGS", "0"))
      return FR_ERROR;

    // deaktivated Stealth mode"
    if (!putFlarmData (file, "$PFLAC", "PRIV", "0"))
      return FR_ERROR;

    // aircraft type;  1 = glider
    if (!putFlarmData (file, "$PFLAC", "ACFT", "1"))
      return FR_ERROR;

    // Pilot name
    if (!putFlarmData (file, "$PFLAC", "PILOT", decl->pilotA))
      return FR_ERROR;

    // Copilot name
    if (!putFlarmData (file, "$PFLAC", "COPIL", decl->pilotB))
      return FR_ERROR;

    // Glider type
    if (!putFlarmData (file, "$PFLAC", "GLIDERTYPE", decl->gliderType))
      return FR_ERROR;

    // Aircraft registration
    if (!putFlarmData (file, "$PFLAC", "GLIDERID", decl->gliderID))
      return FR_ERROR;

    // Competition ID
    if (!putFlarmData (file, "$PFLAC", "COMPID", decl->compID))
      return FR_ERROR;

    // Competition Class
    if (!putFlarmData (file, "$PFLAC", "COMPCLASS", decl->compClass))
      return FR_ERROR;

    //TODO: make configurable?
    // Logger interval
    if (!putFlarmData (file, "$PFLAC", "LOGINT", "3"))
      return FR_ERROR;

    // Task declaration
    if (!putFlarmData (file, "$PFLAC","NEWTASK", name))
      return FR_ERROR;

    int wpCnt = 0;
    Waypoint *wp;

    foreach(wp, *wpList)
    {
        // should never happen
        if (wpCnt >= (int)_capabilities.maxNrWaypointsPerTask)
            break;

        // qDebug ("wp: %s", wp->name.toLatin1().constData());
        if (!putFlarmData (file, "$PFLAC", "ADDWP", lat2flarm(wp->origP.lat()), lon2flarm(wp->origP.lon()), wp->name))
          return FR_ERROR;
    }

    return FR_OK;
}

/**
  * export flight declaration to flarmcfg.txt file
  * @Author: eggert.ehmke@berlin.de
  */
int Flarm::exportDeclaration(FRTaskDeclaration* decl, QList<Waypoint*>* wpList, const QString& name)
{
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

    int result = sendStreamData (stream, decl, wpList, name);
    file.close();
    return result;
}

void Flarm::sendStreamComment (QTextStream& stream, const QString& comment) {
    stream << "// " << comment << ENDL;
}

void Flarm::sendStreamData (QTextStream& stream, const QString& sentence) {
  stream << sentence << ENDL;
}

int Flarm::sendStreamData (QTextStream& stream, FRTaskDeclaration* decl, QList<Waypoint*>* wpList, const QString& name) {

    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString ();

    sendStreamComment (stream, "FLARM configuration file has been created by KFlog");
    sendStreamComment (stream, timestamp);

    sendStreamComment (stream, "deactivated competition mode");
    sendStreamData (stream, "$PFLAC,S,CFLAGS,0");

    sendStreamComment (stream, "deactivated Stealth mode");
    sendStreamData (stream, "$PFLAC,S,PRIV,0");

    sendStreamComment (stream, "aircraft type;  1 = glider");
    sendStreamData (stream, "$PFLAC,S,ACFT,1");

    sendStreamComment (stream, "Pilot name");
    sendStreamData (stream, "$PFLAC,S,PILOT," + decl->pilotA);

    sendStreamComment (stream, "Copilot name");
    sendStreamData (stream, "$PFLAC,S,COPIL," + decl->pilotB);

    sendStreamComment (stream, "Glider type");
    sendStreamData (stream, "$PFLAC,S,GLIDERTYPE," + decl->gliderType);

    sendStreamComment (stream, "Aircraft registration");
    sendStreamData (stream, "$PFLAC,S,GLIDERID," + decl->gliderID);

    sendStreamComment (stream, "Competition ID");
    sendStreamData (stream, "$PFLAC,S,COMPID," + decl->compID);

    sendStreamComment (stream, "Competition Class");
    sendStreamData (stream, "$PFLAC,S,COMPCLASS," + decl->compClass);

    //TODO: make configurable?
    sendStreamComment (stream, "Logger interval");
    sendStreamData (stream, "$PFLAC,S,LOGINT,3");

    //TODO: use task name?
    sendStreamComment (stream, "Task declaration");
    sendStreamData (stream, "$PFLAC,S,NEWTASK," + name);

    int wpCnt = 0;
    Waypoint *wp;

    foreach(wp, *wpList)
    {
        // should never happen
        if (wpCnt >= (int)_capabilities.maxNrWaypointsPerTask)
            break;

        // qDebug ("wp: %s", wp->name.toLatin1().constData());
        sendStreamData (stream, "$PFLAC,S,ADDWP," + lat2flarm(wp->origP.lat()) + "," + lon2flarm(wp->origP.lon()) + "," + wp->name);
    }

    return FR_OK;
}

int Flarm::readDatabase()
{
  return FR_NOTSUPPORTED;
}

/**
  * read the tasks from the Flarm device
  */
int Flarm::readTasks(QList<FlightTask*> * /*tasks*/)
{
  qDebug ("Flarm::readTasks");

  return FR_NOTSUPPORTED;
}

/**
  * write the tasks to the flarm device
  */
int Flarm::writeTasks(QList<FlightTask*>* /*tasks*/)
{
  qDebug ("Flarm::writeTasks");

  return FR_NOTSUPPORTED;
}

/**
  * read the waypoints from the flarm device
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
