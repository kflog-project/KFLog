/***********************************************************************
 **
 **   soaringpilot.cpp
 **
 **   This file is part of KFLog2.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003 by Harald Maier
 **                   2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "soaringpilot.h"

#include <cstdlib>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <QtCore>

#include "../airfield.h"
#include "../mainwindow.h"

extern QSettings   _settings;
extern MainWindow *_mainWindow;

extern int breakTransfer;

static char c36[] = "0123456789abcdefghijklmnopqrstuvwxyz";

static int portID = -1;
/**
 * holds the port-settings at start of the application
 */
static struct termios oldTermEnv;
/**
 * is used to change the port-settings
 */
static struct termios newTermEnv;

/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the program. Called via signal-handler of the runtime-environment.
 */
static void releaseTTY(int /*signal*/)
{
  tcsetattr( portID, TCSANOW, &oldTermEnv );
  exit( -1 );
}

SoaringPilot::SoaringPilot( QObject *parent ) : FlightRecorderPluginBase( parent )
{
  //Set Flightrecorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = (unsigned int) -1;             //maximum number of tasks
  _capabilities.maxNrWaypoints = (unsigned int) -1;         //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = (unsigned int) -1; //maximum number of waypoints per task
  _capabilities.maxNrPilots = 1;            //maximum number of pilots

  _capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  _capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  _capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  //_capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  _capabilities.supDlTask = true;          //supports downloading of tasks?
  _capabilities.supUlTask = true;          //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  _capabilities.supDspSerialNumber = true;
  _capabilities.supDspRecorderType = true;
  //_capabilities.supDspPilotName = true;
  //_capabilities.supDspGliderType = true;
  //_capabilities.supDspGliderID = true;
  //_capabilities.supDspCompetitionID = true;
  //End set capabilities.

  portID = -1;
}

SoaringPilot::~SoaringPilot()
{
  closeRecorder();
}

/** write a file like structure to the device */
int SoaringPilot::writeFile( QStringList &file )
{
  for( int i = 0; i < file.size(); i++ )
    {
      QString line = file.at(i) + "\r\n";

      if( write( portID, line.toAscii().data(), line.size() ) != 1 )
        {
          return FR_ERROR;
        }
    }

  return FR_OK;
}

/** read a file like structure from the device */
int SoaringPilot::readFile(QStringList &file)
{
  char inbyte;
  QString s;
  time_t t1;
  int start = 0;
  _errorinfo = "";

  t1 = time( 0 );

  while( !breakTransfer )
    {
      if( read( portID, &inbyte, sizeof(inbyte) ) )
        {
          start = 1;
          t1 = time( 0 );

          switch( inbyte )
            {
              case '\n':
                file.append( s );
                s = "";
                break;

              case '\r':
                continue;

              default:
                s.append( inbyte );
                break;
            }
        }
      else if( start )
        {
          if( time( 0 ) - t1 > 2 )
            {
              break;
            }
        }
      else
        {
          // waiting 5 secs. for response
          if( time( 0 ) - t1 > 5 )
            {
              _errorinfo = tr( "No response from recorder within 5 seconds!" );
              return FR_ERROR;
            }
        }
    }

  return FR_OK;
}

/** No descriptions */
int SoaringPilot::coordToDegree(QString &s)
{
  // format 48:00.000N
  double deg = 0;
  double min = 0.0;
  double sec = 0;
  int res = 0;
  QString negChar("swSW");

  s.trimmed();

  QStringList tmp = s.left(s.length() - 1).split(":");

  if (tmp.size() == 2) {
    deg = tmp[0].toDouble();
    min = tmp[1].toDouble();
    res = (int)((600000.0 * deg) + (10000.0 * min));
  }
  else if (tmp.size() == 3) {
    deg = tmp[0].toDouble();
    min = tmp[1].toDouble();
    sec = tmp[2].toDouble();
    res = (int)((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));
  }
  // We add 1 to avoid rounding-errors ...
  res += 1;

  if (negChar.contains(s.right(1))) {
    res = -res;
  }

  return res;
}

/** No descriptions */
QString SoaringPilot::degreeToDegMin(int d, bool isLat)
{
  QString tmp;
  int tmpD = abs(d);
  int degree  = tmpD / 600000;
  double minute = (double)(tmpD - (degree * 600000)) / 10000.0;

  if (isLat) {
    tmp.sprintf("%02d:%02.3f%c", degree, minute, (d < 0 ? 'S' : 'N'));
  }
  else {
    tmp.sprintf("%03d:%02.3f%c", degree, minute, (d < 0 ? 'W' : 'E'));
  }

  return tmp;
}

/** No descriptions */
QString SoaringPilot::degreeToDegMinSec(int d, bool isLat)
{
  QString tmp;
  int tmpD = abs(d);
  int degree  = tmpD / 600000;
  int minute = (tmpD - (degree * 600000)) / 10000;
  int seconds = ((tmpD - (degree * 600000) - (10000 * minute)) * 60) / 10000;

  if (isLat) {
    tmp.sprintf("%02d:%02d:%02d.00%c", degree, minute, seconds, (d < 0 ? 'S' : 'N'));
  }
  else {
    tmp.sprintf("%03d:%02d:%02d.00%c", degree, minute, seconds, (d < 0 ? 'W' : 'E'));
  }

  return tmp;
}

int SoaringPilot::feetToMeter(QString &s)
{
  int meter = 0;
  s.trimmed();
  if (s.right(1) == "F") {
    QString tmp;
    tmp.sprintf("%.0f", (s.left(s.length() - 1).toDouble() * 0.3048));
    meter = tmp.toInt();
  }
  return meter;
}

QString SoaringPilot::meterToFeet(int m)
{
  QString feet;
  feet.sprintf("%.0fF", m / 0.3048);
  return feet;
}


/** New access methods - embedded in FlightRecorderPluginBase
    ========================================================= */


/**
 * Returns the name of the lib.
 */
QString SoaringPilot::getLibName() const
{
  return "libkfrxsp";
}

/**
 * Returns the transfe rmode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode SoaringPilot::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

/**
 * Returns a list of recorded flights in this device.
 */

int SoaringPilot::getFlightDir(QList<FRDirEntry*> *dirList)
{
  /* André: I don't quite get this one. Shouldn't this return some FRDirEntries? */
  // SoaringPilot is something "special". It doesn't provide a flight directory
  // You have to select the flight in SP, can be one or all !!!3
  time_t startTime_t;
  struct tm startTime;

  qDeleteAll( *dirList );
  dirList->clear();

  FRDirEntry* entry = new FRDirEntry;

  startTime_t = 0;
  startTime = *gmtime(&startTime_t);

  entry->pilotName = tr("Please select flight from SoaringPilot and start transfer");
  entry->gliderID = "";
  entry->firstTime = startTime;
  entry->lastTime = startTime;
  entry->duration = 0;
  entry->shortFileName = "short.igc";
  entry->longFileName = "long.igc";

  dirList->append(entry);

  return FR_OK;
}

/**
 * This method does something other as normally expected by a FlightRecorder
 * Plugin. The special handling must be adapted by the calling method.
 */
int SoaringPilot::downloadFlight(int /*flightID*/, int /*secMode*/, const QString& fileName)
{
  QStringList file;
  QString A;
  QString tmp;
  QString dir;
  QString key;
  QHash<QString, int> flightCount;
  int fc;
  int ret;
  QFile f;
  int day, month, year;
  QString _fileName = fileName;

  // If no DefaultFlightDirectory is configured, we must use $HOME instead of the root-directory
  dir = _settings.value( "/Path/DefaultFlightDirectory",
                         _mainWindow->getApplicationDataDirectory() ).toString();

  dir += "/";

  bool shortName = fileName.toUpper().contains("SHORT.IGC");

  // IGC File structure
  ret = readFile( file );

  if (ret == FR_OK) {
    for ( int i = 0; i < file.size(); i++ ) {
      tmp = file.at(i);

      if (tmp.startsWith("A")) {
        // new flight
        if (f.isOpen()) {
          f.close();
        }

        A = tmp;

        if (tmp.startsWith("HFDTE")) {
          if (tmp.length() >= 11) {
            day = tmp.mid(5, 2).toInt();
            month = tmp.mid(7, 2).toInt();
            year = tmp.mid(9, 2).toInt();
          }
          else {
            day = month = year = 0;
          }

          key.sprintf("%02d%02d%02d", day, month, year);

          if ( flightCount.contains(key) ) {
            fc = flightCount.value(key);
            fc++;
            flightCount.insert( key, fc );
          }
          else {
            flightCount.insert( key, 1 );
          }

          if (shortName) {
            _fileName.sprintf("%d%c%cX%s%c.IGC", year, c36[month], c36[day],
                             _basicData.serialNumber.toLatin1().data(), c36[fc]);
          }
          else {
            _fileName.sprintf("20%.2d-%.2d-%.2d-XSP-%s-%.2d.IGC",
                             year, month, day,
                             _basicData.serialNumber.toLatin1().data(), fc);
          }
        }
        else {
          _errorinfo = tr("invalid file structure\n\nHFTDE record expected");
          ret = FR_ERROR;
          break;
        }

        f.setFileName(dir + _fileName);

        if (!f.open(QIODevice::WriteOnly)) {
          _errorinfo = tr("IO error while saving file ") + _fileName;
          ret = FR_ERROR;
          break;
        }

        f.write( A.toAscii().data(), A.length() );
      }
      // write data to file
      f.write(tmp.toAscii().data(), tmp.length() );
    }
  }
  return ret;
}

/**
  * get recorder basic data
  */
int SoaringPilot::getBasicData(FR_BasicData& data)
{
  _basicData.serialNumber = "000";
  _basicData.recorderType = "SoaringPilot";
  _basicData.pilotName = "???";
  _basicData.gliderType = "???";
  _basicData.gliderID = "???";
  _basicData.competitionID = "???";
  data = _basicData;
  return FR_OK;
}

int SoaringPilot::getConfigData(FR_ConfigData& /*data*/)
{
  return FR_NOTSUPPORTED;
}

int SoaringPilot::writeConfigData(FR_BasicData& /*basicdata*/, FR_ConfigData& /*configdata*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Opens the recorder for serial communication.
 */
int SoaringPilot::openRecorder(const QString& portName, int baud)
{
  speed_t speed;

  /* eventuell als Mode zusätzlich O_NONBLOCK ??? */
  portID = open(portName.toAscii().data(), O_RDWR | O_NOCTTY);

  if(portID != -1) {
    //
    // Before we change any port-settings, we must establish a
    // signal-handler, which is used to restore the port-settings
    // after terminating the program.
    //    Because a SIGKILL-signal removes the program immediately,
    // the status of the port will be undefined.
    //
    struct sigaction sact;

    sact.sa_handler = releaseTTY;
    sigaction(SIGHUP, &sact, NULL);
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGPIPE, &sact, NULL);
    sigaction(SIGTERM, &sact, NULL);

    ////////////////////////////////////////////////////////////////////////
    //
    // port-configuration
    //

    // reading the current port-settings
    tcgetattr(portID, &newTermEnv);

    // storing the port-settings to restore them ...
    oldTermEnv = newTermEnv;

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

    // input flags
    newTermEnv.c_iflag |= CREAD;   // (wichtig!)
    newTermEnv.c_iflag |= CLOCAL;  // (wichtig!)
    newTermEnv.c_iflag &= ~IGNCR;  // (wichtig!)
    newTermEnv.c_iflag &= ~ISTRIP; // dont strip
    newTermEnv.c_iflag &= ~ICRNL; // don't change newline (\n) in carriage-return (\r)
    newTermEnv.c_iflag &= ~INLCR; // don't change carriage-return (\r) in newline (\n)
    newTermEnv.c_iflag &= ~IXON; // disable XON flow-control on output
    newTermEnv.c_iflag &= ~IXOFF; // disable XON flow-control on intput
    newTermEnv.c_iflag |= IGNBRK; // ignore break
    newTermEnv.c_iflag &= ~BRKINT; // dont signal break

    // control flags
    newTermEnv.c_cflag |= CRTSCTS; // Hardware control on output
    newTermEnv.c_cflag |= CS8; // Bytesize = 8
    newTermEnv.c_cflag &= ~CSTOPB; // 1 stop-bit
    newTermEnv.c_cflag &= ~PARENB; // no parity

    // control characters
    newTermEnv.c_cc[VMIN] = 0; // don't wait for a character
    newTermEnv.c_cc[VTIME] = 1; // wait at least 1 msec.

    // output flags
    newTermEnv.c_oflag &= ~OPOST; // no post processing
    newTermEnv.c_oflag &= ~ONLCR; // don't change newline (\n) in carriage-return (\r)
    newTermEnv.c_oflag &= ~OCRNL; // don't change carriage-return (\r) in newline (\n)

    // line flags
    newTermEnv.c_lflag &= ~ICANON; // enable raw-mode (diable canonical-mode)
    newTermEnv.c_lflag &= ~IEXTEN; // no extended processing
    newTermEnv.c_lflag &= ~ISIG; // don't interpret INTR, QUIT, SUSP or DSUSP
    newTermEnv.c_lflag &= ~ECHO;// echo input characters

    // Activating the port-settings
    tcsetattr(portID, TCSANOW, &newTermEnv);
    _isConnected=true;
    return FR_OK;
  }
  else {
    _isConnected=false;
    return FR_ERROR;
  }
}

/**
 * Closes the connection with the flight recorder.
 */
int SoaringPilot::closeRecorder()
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
 * Read tasks from recorder
 */
int SoaringPilot::readTasks(QList<FlightTask*> *tasks)
{
  QStringList file;
  QStringList::iterator line;
  QStringList tokens;
  QString tmp;
  QString nam;
  int ret;
  Waypoint *wp;
  QList<Waypoint*> wpList;
  int nrPoints;
  bool takeoff, landing;
  _errorinfo = "";

  // ** -------------------------------------------------------------
  // **      SOARINGPILOT Version 1.8.8 Tasks
  // **      Date: 22 Feb 2003
  // ** -------------------------------------------------------------
  // TS,NewTsk1,6,TL
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TW,48:06:43.20N,009:45:49.80E,1903F,BIBERA,
  // TW,48:17:16.02N,009:27:40.98E,2323F,HAYING,
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TE
  ret = readFile(file);
  if (ret == FR_OK) {
    for (line = file.begin(); line != file.end(); ++line) {
        QString s = *line;
        tokens = s.split(",");

      if (tokens.size() >= 3 && tokens[0] == "TS") {
        wpList.clear();
        nam = tokens[1];
        nrPoints = tokens[2].toInt();
        // check for takeoff and landing
        takeoff = (tokens.size() >= 4 && tokens[3].contains("T"));
        landing = (tokens.size() >= 4 && tokens[3].contains("L"));

        while (++line != file.end()) {
          QString s = *line;
          tokens = s.split(",");
          if (tokens.size() >= 5 && tokens[0] == "TW") {
            wp = new Waypoint;
            wp->name = tokens[4];
            wp->origP.setPos(coordToDegree(tokens[1]), coordToDegree(tokens[2]));
            wp->elevation = feetToMeter(tokens[3]);
            wp->type = FlightTask::RouteP;
            wpList.append(wp);
          }
          else if (tokens.size() >= 1 && tokens[0] == "TE") {
            // check with declaration
            if (nrPoints != wpList.count()) {
              _errorinfo = tr("invalid task definition in task ") +
                nam + "\n" + tr("Nr of waypoints differ from task header");
              return FR_ERROR;
            }
            break;
          }
        }

        if (!takeoff) {// append takeoff, copy of first point
          wpList.prepend(new Waypoint(wpList.first()));
        }
        if (!landing) {// append landing, copy of last point
          wpList.append(new Waypoint(wpList.last()));
        }

        nrPoints = wpList.count();
        if (nrPoints >= 2) {
          wpList.at(nrPoints - 1)->type = FlightTask::End;
          wpList.at(nrPoints - 2)->type = FlightTask::Landing;
          wpList.at(1)->type = FlightTask::Begin;
          wpList.at(0)->type = FlightTask::TakeOff;
        }

        tasks->append(new FlightTask(wpList, true, nam));
      }
    }
  }
  return ret;
}

/**
 * Write tasks to recorder
 */
int SoaringPilot::writeTasks(QList<FlightTask*> *tasks)
{
  QStringList file;
  QString tmp, typ;
  FlightTask *task;
  Waypoint *wp;
  QList <Waypoint*> wpList;
  int nrPoints;
  // ** -------------------------------------------------------------
  // **      SOARINGPILOT Version 1.8.8 Tasks
  // **      Date: 22 Feb 2003
  // ** -------------------------------------------------------------
  // TS,NewTsk1,6,TL
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TW,48:06:43.20N,009:45:49.80E,1903F,BIBERA,
  // TW,48:17:16.02N,009:27:40.98E,2323F,HAYING,
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TW,48:13:19.98N,009:54:25.02E,1765F,LAUPHE,
  // TE

  foreach(task, *tasks) {
    wpList = task->getWPList();
    nrPoints = wpList.count();
    if (nrPoints >= 4) {
      // complete task with takeoff and landing
      typ = "TL";
    }
    else {
      typ = "";
    }
    tmp.sprintf("TS,%s,%d,%s\r\n", task->getFileName().toLatin1().data(), nrPoints, typ.toLatin1().data());
    file.append(tmp);
    foreach(wp, wpList) {
      tmp.sprintf("TW,%s,%s,%s,%s\r\n",
                  degreeToDegMinSec(wp->origP.lat(), true).toLatin1().data(),
                  degreeToDegMinSec(wp->origP.lon(), false).toLatin1().data(),
                  meterToFeet(wp->elevation).toLatin1().data(),
                  wp->name.toLatin1().data());
      file.append(tmp);
    }
    file.append("TE\r\n");
  }

  return writeFile(file);
}

/**
 * Read waypoints from recorder
 */
int SoaringPilot::readWaypoints(QList<Waypoint*> *waypoints)
{
  QStringList file;
  QStringList tokens;
  QString tmp;
  int ret;
  Waypoint *frWp;

  //** -------------------------------------------------------------
  //**      SOARINGPILOT Version 1.8.8 Waypoints
  //**      Date: 20 Feb 2003
  //** -------------------------------------------------------------
  //1,48:00.000N,009:00.000E,590F,ATLSFMH,KFLOG,Remark,000000000000000000
  ret = readFile(file);

  if (ret == FR_OK) {
    for ( int i = 0; i < file.size(); i++ ) {

        tokens = file.at(i).split(",");

      if (tokens.size() >= 6) {
        frWp = new Waypoint(tokens[5].trimmed());
        frWp->origP.setPos(coordToDegree(tokens[1]), coordToDegree(tokens[2]));
        frWp->elevation = feetToMeter(tokens[3]);

        tmp = tokens[4];
        frWp->isLandable = (tmp.contains('A') > 0) || (tmp.contains('L') > 0);
        if (frWp->isLandable) {
          frWp->surface = tmp.contains('A') > 0 ? Runway::Asphalt : Runway::Grass;
          frWp->type = tmp.contains('A') > 0 ? BaseMapElement::Airfield : BaseMapElement::Gliderfield;
        }

        frWp->comment = tokens[6];

        waypoints->append(frWp);
      }
    }
  }
  return ret;
}

/**
 * Write waypoints to recorder
 */
int SoaringPilot::writeWaypoints(QList<Waypoint*> *waypoints)
{
  QStringList file;
  QString tmp, typ;
  Waypoint *frWp;
  int line = 1;
  //** -------------------------------------------------------------
  //**      SOARINGPILOT Version 1.8.8 Waypoints
  //**      Date: 20 Feb 2003
  //** -------------------------------------------------------------
  //1,48:00.000N,009:00.000E,590F,ATLSFMH,KFLOG,Remark,000000000000000000
  foreach(frWp, *waypoints) {
    typ = "";
    if (frWp->isLandable) {
      switch(frWp->type) {
        case BaseMapElement::Airfield:
        case BaseMapElement::Airport:
        case BaseMapElement::IntAirport:
        case BaseMapElement::MilAirport:
        case BaseMapElement::CivMilAirport:
          typ += "AL";
          break;
        case BaseMapElement::Gliderfield:
          typ += "L";
          break;
      }
    }
    tmp.sprintf("%d,%s,%s,%s,%s,%s,%s\r\n",
                line++,
                degreeToDegMin(frWp->origP.lat(), true).toLatin1().data(),
                degreeToDegMin(frWp->origP.lon(), false).toLatin1().data(),
                meterToFeet(frWp->elevation).toLatin1().data(),
                typ.toLatin1().data(),
                frWp->name.toLatin1().data(),
                frWp->comment.toLatin1().data());
    file.append(tmp);
  }
  return writeFile(file);
}

/** NOT IMLEMENTED
    ============================================*/

/**
 * Opens the recorder for other communication.
 */
int SoaringPilot::openRecorder(const QString& /*URL*/)
{
  return FR_NOTSUPPORTED;
}

 /**
 * Write flight declaration to recorder
 */
int SoaringPilot::writeDeclaration(FRTaskDeclaration * /*taskDecl*/, QList<Waypoint*> * /*taskPoints*/, const QString& /*name*/)
{
  return FR_NOTSUPPORTED;
}

 /**
 * Export flight declaration to file
 */
int SoaringPilot::exportDeclaration(FRTaskDeclaration * /*taskDecl*/, QList<Waypoint*> * /*taskPoints*/, const QString& /*name*/)
{
  return FR_NOTSUPPORTED;
}

/**
 * Read waypoint and flight declaration form from recorder into mem
 */
int SoaringPilot::readDatabase()
{
  return FR_NOTSUPPORTED;
}
