/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of libkfrgcs.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <iostream.h>

#include <unistd.h>
#include <ctype.h>

#include <vlapi2.h>
#include <vlapihlp.h>

#include <klocale.h>
#include <qarray.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <../frstructs.h>
#include <../flighttask.h>

#include <termios.h>

#define MAX(a,b)   ( ( a > b ) ? a : b )
#define MIN(a,b)   ( ( a < b ) ? a : b )

int breakTransfer = 0;
unsigned int maxNrTasks = 25;
unsigned int maxNrWaypoints = 500;
unsigned int maxNrWaypointsPerTask = 10;
unsigned int maxNrPilots = 25;

extern "C"
{
  /**
   * Returns the name of the lib.
   */
  QString getLibName();
  /**
   * Reads the list of flight in the logger. The lib has to take care
   * that all fields of the FRDirEntry are filled with valuable content.
   *
   * Return -1, if there is any problem with the logger, -2, if the
   * method is not implemented, 1, if reading was okay.
   */
  int getFlightDir(QList<FRDirEntry>*);
  /**
   * Used to download a flight from the FR. The flight is returned as a
   * QByteArray.
   */
  QByteArray getFlight(int flightID, char* tmpFileName, int* ret);
  /** */
  int downloadFlight(int flightID, int secMode, char* fileName);
  /** get recorder info serial id*/
  QString getRecorderSerialNo();
  /** */
  int openRecorder(char* portName, int baud);
  /** */
  int closeRecorder();
  /** write flight declaration to recorder */
  int writeDeclaration(FRTaskDeclaration *taskDecl, 
                       QList<FRTaskPoint> *taskPoints);
  /** read waypoint and flight declaration form from recorder into mem */
  int readDatabase();
  /** read tasks from recorder */
  int readTasks(QList<FRTask> *tasks);
  /** write tasks to recorder */
  int writeTasks(QList<FRTask> *tasks);
  /** read waypoints from recorder */
  int readWaypoints(QList<FRWaypoint> *waypoints);
  /** write waypoints to recorder */
  int writeWaypoints(QList<FRWaypoint> *waypoints);
}

/*************************************************************************
**
** begin of logger-specific implementation
**
*************************************************************************/

/**
 * The device-name of the port.
 */
char* portName = '\0';
/**
 * The file-handle
 */
int portID = -1;

/**
 * holds the port-settings at start of the application
 */
struct termios oldTermEnv;

/**
 * is used to change the port-settings
 */
struct termios newTermEnv;

VLAPI vl;

QString getLibName()  {  return "libkfrgcs";  }

int getFlightDir(QList<FRDirEntry>* dirList)
{
  dirList->clear();
  int err;

  if((err = vl.read_directory()) == VLA_ERR_NOERR) {
    tm lastDate;
    lastDate.tm_year = 0;
    lastDate.tm_mon = 0;
    lastDate.tm_mday = 1;
    int flightCount = 0;

    for(int loop = 0; loop < vl.directory.nflights; loop++) {
      DIRENTRY flight = vl.directory.flights[loop];
      
      if(lastDate.tm_year == flight.firsttime.tm_year &&
         lastDate.tm_mon == flight.firsttime.tm_mon &&
         lastDate.tm_mday == flight.firsttime.tm_mday) {
        flightCount++;
      }
      else {
        flightCount = 1;
      }

      FRDirEntry* entry = new FRDirEntry;

      entry->pilotName = flight.pilot;
      entry->gliderID = flight.gliderid;
      entry->firstTime = flight.firsttime;
      entry->lastTime = flight.lasttime;
      entry->duration = flight.recordingtime;
      entry->shortFileName = flight.filename;
      //cerr << flight.filename << endl;
      entry->longFileName.sprintf("%d-%.2d-%.2d-GCS-%s-%.2d.igc",
                                  flight.firsttime.tm_year + 1900, 
                                  flight.firsttime.tm_mon + 1,
                                  flight.firsttime.tm_mday, 
                                  wordtoserno(flight.serno),
                                  flightCount);

      dirList->append(entry);
      
      lastDate = flight.firsttime;
      //      		  vl.read_igcfile(fileName, loop, 0);
    }
  }

  return err == VLA_ERR_NOERR;
}

QByteArray getFlight(int flightID, char* tmpFileName, int* ret)
{
  QByteArray bArray;
  int err;

  if ((err = vl.read_igcfile(tmpFileName, 2, 0)) == VLA_ERR_NOERR) {
    QFile tmpFile(tmpFileName);
    QTextStream outStream(bArray, IO_WriteOnly);
    QTextStream inStream(&tmpFile);

    while(!inStream.eof()) {
      outStream << inStream.readLine();
    }
  }

  *ret = (err == VLA_ERR_NOERR);
  return bArray;
}

int downloadFlight(int flightID, int secMode, char* fileName)
{
  return vl.read_igcfile(fileName, flightID, secMode) == VLA_ERR_NOERR;
}

QString getRecorderSerialNo()
{
  vl.read_info();

	// Aufbau der Versions- und sonstigen Nummern
//	vlinfo.sessionid = 256*buffer[0] + buffer[1];
//	vlinfo.vlserno = 256*buffer[2] + buffer[3];
//	vlinfo.fwmajor = buffer[4] / 16;
//	vlinfo.fwminor = buffer[4] % 16;
//	vlinfo.fwbuild = buffer[7];

  return wordtoserno(vl.vlinfo.vlserno);
}

int openRecorder(char* pName, int baud)
{
  extern char* portName;
  int err;

  portName = pName;

  if((err = vl.open(1, 5, 0, baud)) != VLA_ERR_NOERR) {
    warning(i18n("No logger found!"));
  }

  return err == VLA_ERR_NOERR;
}

int closeRecorder()
{
  vl.close(1);
  return 1;
}

int writeDeclaration(FRTaskDeclaration* taskDecl, QList<FRTaskPoint> *taskPoints)
{
//  vl.read_db_and_declaration();
//
  unsigned int loop;
  FRTaskPoint *tp;

  // Filling the strings with whitespaces
  QString pilotA(taskDecl->pilotA.leftJustify(32, ' ', true));
  QString pilotB(taskDecl->pilotB.leftJustify(32, ' ', true));
  sprintf(vl.declaration.flightinfo.pilot, "%s%s", (const char*)pilotA, 
          (const char*)pilotB);

  strcpy(vl.declaration.flightinfo.gliderid, 
         taskDecl->gliderID.leftJustify(7, ' ', true));
  strcpy(vl.declaration.flightinfo.glidertype, 
         taskDecl->gliderType.leftJustify(12, ' ', true));
  strcpy(vl.declaration.flightinfo.competitionid, 
         taskDecl->compID.leftJustify(3, ' ', true));
  strcpy(vl.declaration.flightinfo.competitionclass, 
         taskDecl->compClass.leftJustify(12, ' ', true));

  // TakeOff (same ans landing ...)
  tp = taskPoints->at(0);
  strcpy(vl.declaration.flightinfo.homepoint.name, tp->name.left(6));
  vl.declaration.flightinfo.homepoint.lon = tp->lonPos / 600000.0;
  vl.declaration.flightinfo.homepoint.lat = tp->latPos / 600000.0;

  // Begin of Task
  tp = taskPoints->at(1);
  strcpy(vl.declaration.task.startpoint.name, tp->name.left(6));
  vl.declaration.task.startpoint.lat = tp->latPos / 600000.0;
  vl.declaration.task.startpoint.lon = tp->lonPos / 600000.0;

  for(loop = 2; loop < MIN(taskPoints->count() - 2, 12); loop++) {
    tp = taskPoints->at(loop);
    strcpy(vl.declaration.task.turnpoints[loop - 2].name, tp->name.left(6));
    vl.declaration.task.turnpoints[loop - 2].lat = tp->latPos / 600000.0;
    vl.declaration.task.turnpoints[loop - 2].lon = tp->lonPos / 600000.0;
  }

  vl.declaration.task.nturnpoints = MAX(MIN((int)taskPoints->count() - 4, 12), 0);

  // End of Task
  tp = taskPoints->at(taskPoints->count() - 2);
  strcpy(vl.declaration.task.finishpoint.name, tp->name.left(6));
  vl.declaration.task.finishpoint.lat = tp->latPos / 600000.0;
  vl.declaration.task.finishpoint.lon = tp->lonPos / 600000.0;

  return vl.write_db_and_declaration() == VLA_ERR_NOERR;
}

int readDatabase()
{
  return vl.read_db_and_declaration() == VLA_ERR_NOERR;
}

int readTasks(QList<FRTask> *tasks)
{
  FRTask *task;
  FRTaskPoint *tp;
  VLAPI_DATA::ROUTE *r;
  VLAPI_DATA::WPT *wp;
  int taskCnt;
  unsigned int wpCnt;

  for (taskCnt = 0; taskCnt < vl.database.nroutes; taskCnt++) {
    r = &(vl.database.routes[taskCnt]);
    task = new FRTask;
    task->name = r->name;
    for (wpCnt = 0; wpCnt < maxNrWaypointsPerTask; wpCnt++) {
      wp = &(r->wpt[wpCnt]);
      if (isalnum(wp->name[0])) {
        tp = new FRTaskPoint;
        tp->name = wp->name;
        tp->latPos = (int)(wp->lat * 600000.0);
        tp->lonPos = (int)(wp->lon * 600000.0);
        tp->type = FlightTask::RouteP;

        if (task->wayPoints.count() == 0) {
          // append take off
          tp->type = FlightTask::TakeOff;
          task->wayPoints.append(tp);
          // make copy for begin
          tp = new FRTaskPoint;
          *tp = *(task->wayPoints.first());
          tp->type = FlightTask::Begin;
        }
        task->wayPoints.append(tp);
      }
    }
    // modify last for end of task
    task->wayPoints.last()->type = FlightTask::End;
    // make copy for landing
    tp = new FRTaskPoint;
    *tp = *(task->wayPoints.last());
    tp->type = FlightTask::Landing;
    task->wayPoints.append(tp);

    tasks->append(task);
  }
  return 1;
}

int writeTasks(QList<FRTask> *tasks)
{
  FRTask *task;
  FRTaskPoint *tp;
  VLAPI_DATA::ROUTE *r;
  VLAPI_DATA::WPT *wp;
  unsigned int taskCnt;
  unsigned int wpCnt;

  // delete old tasks
  if(vl.database.routes != 0) {
    delete[] vl.database.routes;
    vl.database.routes = 0;
  }
  // create new, check max possible tasks
  vl.database.nroutes = MIN(tasks->count(), maxNrTasks);
  vl.database.routes = new VLAPI_DATA::ROUTE[vl.database.nroutes];

  taskCnt = 0;
  for (task = tasks->first(); task != 0; task = tasks->next()) {
    // should never happen
    if (taskCnt >= maxNrTasks) {
      break;
    }

    r = vl.database.routes + taskCnt++;
    strcpy(r->name, task->name.leftJustify(14, ' ', true));
    wpCnt = 0;
    for (tp = task->wayPoints.first(); tp != 0; tp = task->wayPoints.next()) {
      // should never happen
      if (wpCnt >= maxNrWaypointsPerTask) {
        break;
      }
      // ignore take off and landing
      if (tp->type == FlightTask::TakeOff || tp->type == FlightTask::Landing) {
        continue;
      }
      wp = r->wpt + wpCnt++;
      strcpy(wp->name, tp->name.leftJustify(6, ' ', true));
      wp->lat = tp->latPos / 600000.0;
      wp->lon = tp->lonPos / 600000.0;
      wp->typ = 0;
    }

    // fill remaining turnpoints with '0xff'
    while (wpCnt < maxNrWaypointsPerTask) {
      memset(r->wpt + wpCnt++, 0xff, sizeof(VLAPI_DATA::WPT));
    }
  }

  return vl.write_db_and_declaration() == VLA_ERR_NOERR;
}

int readWaypoints(QList<FRWaypoint> *waypoints)
{
  int n;
  FRWaypoint *frWp;
  FRTaskPoint *tp;
  VLAPI_DATA::WPT *wp;

  for (n = 0; n < vl.database.nwpts; n++) {
    wp = &(vl.database.wpts[n]);
    frWp = new FRWaypoint;
    tp = &frWp->point;

    tp->name = wp->name;
    tp->name = tp->name.stripWhiteSpace();

    tp->latPos = (int)(wp->lat * 600000.0);
    tp->lonPos = (int)(wp->lon * 600000.0);
    frWp->isLandable = (wp->typ & VLAPI_DATA::WPT::WPTTYP_L) > 0;
    frWp->isHardSurface = (wp->typ & VLAPI_DATA::WPT::WPTTYP_H) > 0;
    frWp->isAirport = (wp->typ & VLAPI_DATA::WPT::WPTTYP_A) > 0;
    frWp->isCheckpoint = (wp->typ & VLAPI_DATA::WPT::WPTTYP_C) > 0;

    waypoints->append(frWp);
  }
  return 1;
}

int writeWaypoints(QList<FRWaypoint> *waypoints)
{
  FRWaypoint *frWp;
  FRTaskPoint *tp;
  VLAPI_DATA::WPT *wp;
  unsigned int wpCnt;

  // delete old waypoints
  if(vl.database.wpts != 0) {
    delete[] vl.database.wpts;
    vl.database.wpts = 0;
  }
  // create new, check max possible wapoints
  vl.database.nwpts = MIN(waypoints->count(), maxNrWaypoints);
  vl.database.wpts = new VLAPI_DATA::WPT[vl.database.nwpts];

  wpCnt = 0;
  for (frWp = waypoints->first(); frWp != 0; frWp = waypoints->next()) {
    // should never happen
    if (wpCnt >= maxNrWaypoints) {
      break;
    }
    wp = &(vl.database.wpts[wpCnt++]);
    tp = &frWp->point;
    strcpy(wp->name, tp->name.leftJustify(6, ' ', true));
    wp->lat = tp->latPos / 600000.0;
    wp->lon = tp->lonPos / 600000.0;
    wp->typ = 
      (frWp->isLandable ? VLAPI_DATA::WPT::WPTTYP_L : 0) | 
      (frWp->isHardSurface ? VLAPI_DATA::WPT::WPTTYP_H : 0) | 
      (frWp->isAirport ? VLAPI_DATA::WPT::WPTTYP_A : 0) |
      (frWp->isCheckpoint ? VLAPI_DATA::WPT::WPTTYP_C : 0);
  }

  return vl.write_db_and_declaration() == VLA_ERR_NOERR;
}
