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

#include <qstringlist.h>

extern int breakTransfer;

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
void releaseTTY(int signal)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  exit(-1);
}

SoaringPilot::SoaringPilot()
{
  portID = -1;
}

SoaringPilot::~SoaringPilot()
{
}

/** No descriptions */
int SoaringPilot::openLogger(char *port, int baud)
{
  speed_t speed;

  /* eventuell als Mode zusätzlich O_NONBLOCK ??? */
  portID = open(port, O_RDWR | O_NOCTTY);

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
    return 1;
  }
  else {
    return 0;
  }
}

/** No descriptions */
int SoaringPilot::closeLogger()
{
  if (portID != -1) {
    tcsetattr(portID, TCSANOW, &oldTermEnv);  
    return 1;
  }
  else {
    return 0;
  }
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
        return 0;
      }
    }
  }
  return 1;
}

/** read a file like structure from the device */
int SoaringPilot::readFile(QStringList &file)
{
  char inbyte;
  QString s;
  time_t t1;
  int start = 0;

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
  }
  return 1;
}

/** No descriptions */
int SoaringPilot::downloadWaypoints(QList<FRWaypoint> *waypoints)
{
  QStringList file;
  QStringList::iterator line;
  QStringList tokens;
  QString tmp;
  int ret;
  FRWaypoint *frWp;
  FRTaskPoint *tp;

  //** -------------------------------------------------------------
  //**      SOARINGPILOT Version 1.8.8 Waypoints
  //**      Date: 20 Feb 2003
  //** -------------------------------------------------------------
  //1,48:00.000N,009:00.000E,590F,ATLSFMH,KFLOG,Remark,000000000000000000
  ret = readFile(file);
  if (ret) {
    for (line = file.begin(); line != file.end(); ++line) {
      tokens = QStringList::split(",", *line, true);
      if (tokens.size() >= 6) {
        frWp = new FRWaypoint;
        tp = &frWp->point;

        tp->name = tokens[5];
        tp->name = tp->name.stripWhiteSpace();

        tp->latPos = coordToDegree(tokens[1]);
        tp->lonPos = coordToDegree(tokens[2]);
        tp->elevation = feetToMeter(tokens[3]);

        tmp = tokens[4];
        frWp->isLandable = (tmp.contains('A') > 0) || (tmp.contains('L') > 0);
        frWp->isHardSurface = (tmp.contains('A') > 0);
        frWp->isAirport = (tmp.contains('A') > 0);
        frWp->isCheckpoint = 0;

        frWp->comment = tokens[6];

        waypoints->append(frWp);
      }
    }
  }
  return ret;
}

/** No descriptions */
int SoaringPilot::uploadWaypoints(QList<FRWaypoint> *waypoints)
{
  QStringList file;
  QString tmp, typ;
  FRWaypoint *frWp;
  FRTaskPoint *tp;
  int line = 1;
  //** -------------------------------------------------------------
  //**      SOARINGPILOT Version 1.8.8 Waypoints
  //**      Date: 20 Feb 2003
  //** -------------------------------------------------------------
  //1,48:00.000N,009:00.000E,590F,ATLSFMH,KFLOG,Remark,000000000000000000
  for (frWp = waypoints->first(); frWp != 0; frWp = waypoints->next()) {
    tp = &frWp->point;
    typ = "";
    if (frWp->isLandable) {
      if (frWp->isHardSurface) {
	typ += "AL";
      }
      else {
	typ += "L";
      }
    }
    tmp.sprintf("%d,%s,%s,%s,%s,%s,%s\r\n", 
		line++, 
		degreeToDegMin(tp->latPos, true).latin1(),
		degreeToDegMin(tp->lonPos, false).latin1(),
		meterToFeet(tp->elevation).latin1(),
		typ.latin1(),
		tp->name.latin1(),
		frWp->comment.latin1());
    file.append(tmp);
  }
  return writeFile(file);
}

int SoaringPilot::downloadTasks(QList<FRTask> *tasks)
{
  QStringList file;
  QStringList::iterator line;
  QStringList tokens;
  QString tmp;
  int ret;
  FRTask *task;
  FRTaskPoint *tp;
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
  ret = readFile(file);
  if (ret) {
    for (line = file.begin(); line != file.end(); ++line) {
      tokens = QStringList::split(",", *line, true);
      warning(*line);
      if (tokens.size() >= 4 && tokens[0] == "TS") {
	task = new FRTask;
	task->name = tokens[1];
	nrPoints = tokens[2].toInt();
	while (line != file.end()) {
	  warning(*line);
	  tokens = QStringList::split(",", *line, true);
	  if (tokens.size() >= 5 && tokens[0] == "TW") {
	    tp = new FRTaskPoint;
	    tp->name = tokens[4];
	    tp->latPos = coordToDegree(tokens[1]);
	    tp->lonPos = coordToDegree(tokens[2]);
	    tp->elevation = feetToMeter(tokens[3]);
	    task->wayPoints.append(tp);
	  }
	  else if (tokens.size() >= 1 && tokens[0] == "TE") {
	    // check with declaration
	    if (nrPoints != task->wayPoints.count()) {
	      return 0;
	    }
	    break;
	  }
	  ++line;
	}
// 	if (task->wayPoints.count() == 0) {
// 	  // append take off
// 	  tp->type = FlightTask::TakeOff;
// 	}
// 	if (task->wayPoints.count() == 0) {
// 	  tp->type = FlightTask::Begin;
// 	}
// 	// modify last for end of task
// 	task->wayPoints.last()->type = FlightTask::End;
// 	// make copy for landing
// 	tp = new FRTaskPoint;
// 	*tp = *(task->wayPoints.last());
// 	tp->type = FlightTask::Landing;
// 	task->wayPoints.append(tp);
	
	tasks->append(task);
      }
    }
  }
  return ret;
}

int SoaringPilot::uploadTasks(QList<FRTask> *tasks)
{
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
  int tmpD = abs(d - 1); // substract 1 from rounding errors
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
  int tmpD = abs(d - 1); // substract 1 from rounding errors
  int degree  = tmpD / 600000;
  double minute = (double)(tmpD - (degree * 600000)) / 10000.0;
  double seconds = (double)(tmpD - (degree * 600000) - (10000.0 * minute));
  seconds = (seconds * 60.0) / 1000;

  if (isLat) {
    tmp.sprintf("%02d:%02f:%02.2f%c", degree, minute, seconds, (d < 0 ? 'S' : 'N'));
  }
  else {
    tmp.sprintf("%02d:%02f:%02.2f%c", degree, minute, seconds, (d < 0 ? 'W' : 'E'));
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
