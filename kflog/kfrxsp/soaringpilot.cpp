/***********************************************************************
 **
 **   soaringpilot.cpp
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

#include "soaringpilot.h"

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <signal.h>

#include <qfile.h>
#include <qstringlist.h>
#include <qdict.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>

#include "../airport.h"

extern int breakTransfer;
char c36[] = "0123456789abcdefghijklmnopqrstuvwxyz";

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
 * of the programm. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int /*signal*/)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  exit(-1);
}

SoaringPilot::SoaringPilot()
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
}

/** write a file like structure to the device */
int SoaringPilot::writeFile(QStringList &file)
{
  QStringList::Iterator line;
  const char *p;

  for (line = file.begin(); line != file.end(); ++line) {
    *line += "\r\n";
    p = *line;
    for (unsigned int len = 0; len < (*line).length(); len++) {
      if (write(portID, p + len, sizeof(char)) != 1) {
        return FR_ERROR;
      }
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

  t1 = time(NULL);
  while (!breakTransfer) {
    if (read(portID, &inbyte, sizeof(inbyte))) {
      start = 1;
      t1 = time(NULL);
      switch(inbyte) {
      case '\n':
        file.append(s);
        s = "";
        break;
      case '\r':
        continue;
      default:
        s.append(inbyte);
      }
    }
    else if (start) {
      if (time(NULL) - t1 > 2) {
        break;
      }
    }
    else {
      // waiting 5 secs. for response
      if (time(NULL) - t1 > 5) {
        _errorinfo = i18n("No response from recorder within 5 seconds!");
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

  s.stripWhiteSpace();

  QStringList tmp = QStringList::split(":", s.left(s.length() - 1));
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
  s.stripWhiteSpace();
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
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode SoaringPilot::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

/**
 * Returns a list of recorded flights in this device.
 */

int SoaringPilot::getFlightDir(QPtrList<FRDirEntry> *dirList)
{
  /* André: I don't quite get this one. Shouldn't this return some FRDirEntries? */
  // SearingPilot is something "special". It doesn't provide a flight directory
  // You have to select the flight in SP, can be one or all !!!3
  time_t startTime_t;
  struct tm startTime;

  dirList->clear();
  FRDirEntry* entry = new FRDirEntry;

  startTime_t = 0;
  startTime = *gmtime(&startTime_t);

  entry->pilotName = i18n("Please select flight from SoaringPilot and start transfer");
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
 *
 */
int SoaringPilot::downloadFlight(int /*flightID*/, int /*secMode*/, const QString& fileName)
{
  QStringList file;
  QStringList::iterator line;
  QString A;
  QString tmp;
  QString dir;
  QString key;
  QDict<int> flightCount;
  int *fc;
  int ret;
  QFile f;
  int day, month, year;
  QString _fileName = fileName;

  flightCount.setAutoDelete(true);

  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  dir = config->readEntry("DefaultFlightDirectory") + "/";
  config->setGroup(0);

  bool shortName = (_fileName.upper().find("SHORT.IGC") != -1);

  // IGC File structure
  ret = readFile(file);
  if (ret == FR_OK) {
    for (line = file.begin(); line != file.end(); ++line) {
      tmp = *line;
      if (tmp.left(1) == "A") {
        // new flight
        if (f.isOpen()) {
          f.close();
        }

        A = *line;
        ++line;
        tmp = *line;

        if (tmp.left(5) == "HFDTE") {
          if (tmp.length() >= 11) {
            day = tmp.mid(5, 2).toInt();
            month = tmp.mid(7, 2).toInt();
            year = tmp.mid(9, 2).toInt();
          }
          else {
            day = month = year = 0;
          }

          key.sprintf("%02d%02d%02d", day, month, year);
          if ((fc = flightCount.find(key)) != 0) {
            (*fc)++;
          }
          else {
            fc = new int(1);
            flightCount.insert(key, fc);
          }

          if (shortName) {
            _fileName.sprintf("%d%c%cX%s%c.IGC", year, c36[month], c36[day],
                             (const char *)_basicData.serialNumber.latin1(), c36[*fc]);
          }
          else {
            _fileName.sprintf("20%.2d-%.2d-%.2d-XSP-%s-%.2d.IGC",
                             year, month, day, 
                             (const char *)_basicData.serialNumber.latin1(), *fc);
          }
        }
        else {
          _errorinfo = i18n("invalid file structure\n\nHFTDE record expexted");
          ret = FR_ERROR;
          break;
        }

        f.setName(dir + _fileName);
        if (!f.open(IO_WriteOnly)) {
          _errorinfo = i18n("IO error while saving file ") + _fileName;
          ret = FR_ERROR;
          break;
        }
        f.writeBlock(A + "\n", A.length() + 1);
      }
      // write data to file
      f.writeBlock(tmp + "\n", tmp.length() + 1);
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

/**
 * Opens the recorder for serial communication.
 */
int SoaringPilot::openRecorder(const QString& portName, int baud)
{
  speed_t speed;

  /* eventuell als Mode zusätzlich O_NONBLOCK ??? */
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
 * Closes the connection with the flightrecorder.
 */
int SoaringPilot::closeRecorder() {
  if (portID != -1) {
    tcsetattr(portID, TCSANOW, &oldTermEnv);
    close(portID);
    _isConnected=false;
    return FR_OK;
  }

  else {
    return FR_ERROR;
  }
}

/**
 * Read tasks from recorder
 */
int SoaringPilot::readTasks(QPtrList<FlightTask> *tasks)
{
  QStringList file;
  QStringList::iterator line;
  QStringList tokens;
  QString tmp;
  QString nam;
  int ret;
  Waypoint *wp;
  QPtrList <Waypoint> wpList;
  unsigned int nrPoints;
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
      tokens = QStringList::split(",", *line, true);

      if (tokens.size() >= 3 && tokens[0] == "TS") {
        wpList.clear();
        nam = tokens[1];
        nrPoints = tokens[2].toInt();
        // check for takeoff and landing
        takeoff = (tokens.size() >= 4 && tokens[3].contains("T"));
        landing = (tokens.size() >= 4 && tokens[3].contains("L"));

        while (++line != file.end()) {
          tokens = QStringList::split(",", *line, true);
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
              _errorinfo = i18n("invalid task definition in task ") +
                nam + "\n" + i18n("Nr of waypoints differ from task header");
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
int SoaringPilot::writeTasks(QPtrList<FlightTask> *tasks)
{
  QStringList file;
  QString tmp, typ;
  FlightTask *task;
  Waypoint *wp;
  QPtrList <Waypoint> wpList;
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

  for (task = tasks->first(); task != 0; task = tasks->next()) {
    wpList = task->getWPList();
    nrPoints = wpList.count();
    if (nrPoints >= 4) {
      // complete task with takeoff and landing
      typ = "TL";
    }
    else {
      typ = "";
    }
    tmp.sprintf("TS,%s,%d,%s\r\n", task->getFileName().latin1(), nrPoints, typ.latin1());
    file.append(tmp);
    for (wp = wpList.first(); wp != 0; wp = wpList.next()) {
      tmp.sprintf("TW,%s,%s,%s,%s\r\n",
                  degreeToDegMinSec(wp->origP.lat(), true).latin1(),
                  degreeToDegMinSec(wp->origP.lon(), false).latin1(),
                  meterToFeet(wp->elevation).latin1(),
                  wp->name.latin1());
      file.append(tmp);
    }
    file.append("TE\r\n");
  }

  return writeFile(file);
}

/**
 * Read waypoints from recorder
 */
int SoaringPilot::readWaypoints(QPtrList<Waypoint> *waypoints)
{
  QStringList file;
  QStringList::iterator line;
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
    for (line = file.begin(); line != file.end(); ++line) {
      tokens = QStringList::split(",", *line, true);
      if (tokens.size() >= 6) {
        frWp = new Waypoint(tokens[5].stripWhiteSpace());
        frWp->origP.setPos(coordToDegree(tokens[1]), coordToDegree(tokens[2]));
        frWp->elevation = feetToMeter(tokens[3]);

        tmp = tokens[4];
        frWp->isLandable = (tmp.contains('A') > 0) || (tmp.contains('L') > 0);
        if (frWp->isLandable) {
          frWp->surface = tmp.contains('A') > 0 ? Airport::Asphalt : Airport::Grass;
          frWp->type = tmp.contains('A') > 0 ? BaseMapElement::Airfield : BaseMapElement::Glidersite;
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
int SoaringPilot::writeWaypoints(QPtrList<Waypoint> *waypoints)
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
  for (frWp = waypoints->first(); frWp != 0; frWp = waypoints->next()) {
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
        case BaseMapElement::Glidersite:
          typ += "L";
          break;
      }
    }
    tmp.sprintf("%d,%s,%s,%s,%s,%s,%s\r\n",
                line++,
                degreeToDegMin(frWp->origP.lat(), true).latin1(),
                degreeToDegMin(frWp->origP.lon(), false).latin1(),
                meterToFeet(frWp->elevation).latin1(),
                typ.latin1(),
                frWp->name.latin1(),
                frWp->comment.latin1());
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
int SoaringPilot::writeDeclaration(FRTaskDeclaration * /*taskDecl*/, QPtrList<Waypoint> * /*taskPoints*/)
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
