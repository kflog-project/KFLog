/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of libkfrxsp.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/
#include <unistd.h>
#include <ctype.h>

#include <klocale.h>

#include "soaringpilot.h"

int breakTransfer = 0;
unsigned int maxNrTasks = (unsigned int) -1;
unsigned int maxNrWaypoints = (unsigned int) -1;
unsigned int maxNrWaypointsPerTask = 10;
unsigned int maxNrPilots = 0;

SoaringPilot sp;

extern "C"
{
  /**
   * Returns the name of the lib.
   */
  QString getLibName();
  int getFlightDir(QList<FRDirEntry>*);
  /** */
  int downloadFlight(int flightID, int secMode, char* fileName);
  /** get recorder info serial id*/
  QString getRecorderSerialNo();
  /** */
  int openRecorder(char* portName, int baud);
  /** */
  int closeRecorder();
  /** write flight declaration to recorder */
  /*  not available for SP */
  /* int writeDeclaration(FRTaskDeclaration *taskDecl, 
     QList<FRTaskPoint> *taskPoints); */
  /** read waypoint and flight declaration form from recorder into mem */
  /*  not available for SP */
  /* int readDatabase(); */
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

QString getLibName()  {  return "libkfrxsp";  }

int getFlightDir(QList<FRDirEntry>* dirList)
{
//  dirList->clear();
//  int err;
//
//  if((err = vl.read_directory()) == VLA_ERR_NOERR) {
//    tm lastDate;
//    lastDate.tm_year = 0;
//    lastDate.tm_mon = 0;
//    lastDate.tm_mday = 1;
//    int flightCount = 0;
//
//    for(int loop = 0; loop < vl.directory.nflights; loop++) {
//      DIRENTRY flight = vl.directory.flights[loop];
//
//      if(lastDate.tm_year == flight.firsttime.tm_year &&
//         lastDate.tm_mon == flight.firsttime.tm_mon &&
//         lastDate.tm_mday == flight.firsttime.tm_mday) {
//        flightCount++;
//      }
//      else {
//        flightCount = 1;
//      }
//
//      FRDirEntry* entry = new FRDirEntry;
//
//      entry->pilotName = flight.pilot;
//      entry->gliderID = flight.gliderid;
//      entry->firstTime = flight.firsttime;
//      entry->lastTime = flight.lasttime;
//      entry->duration = flight.recordingtime;
//      entry->shortFileName = flight.filename;
//      //cerr << flight.filename << endl;
//      entry->longFileName.sprintf("%d-%.2d-%.2d-GCS-%s-%.2d.igc",
//                                  flight.firsttime.tm_year + 1900,
//                                  flight.firsttime.tm_mon + 1,
//                                  flight.firsttime.tm_mday,
//                                  wordtoserno(flight.serno),
//                                  flightCount);
//
//      dirList->append(entry);
//
//      lastDate = flight.firsttime;
//    }
//  }
//
//  return err == VLA_ERR_NOERR;
  return 1;
}

int downloadFlight(int flightID, int secMode, char* fileName)
{
  //return vl.read_igcfile(fileName, flightID, secMode) == VLA_ERR_NOERR;
  return 1;
}

QString getRecorderSerialNo()
{
  return "n.a.";
}

int openRecorder(char* pName, int baud)
{
  return sp.openLogger(pName, baud);
}

int closeRecorder()
{
  return sp.closeLogger();
}

int writeDeclaration(FRTaskDeclaration* taskDecl, QList<FRTaskPoint> *taskPoints)
{
//  unsigned int loop;
//  FRTaskPoint *tp;
//
//  // Filling the strings with whitespaces
//  QString pilotA(taskDecl->pilotA.leftJustify(32, ' ', true));
//  QString pilotB(taskDecl->pilotB.leftJustify(32, ' ', true));
//  sprintf(vl.declaration.flightinfo.pilot, "%s%s", (const char*)pilotA,
//          (const char*)pilotB);
//
//  strcpy(vl.declaration.flightinfo.gliderid,
//         taskDecl->gliderID.leftJustify(7, ' ', true));
//  strcpy(vl.declaration.flightinfo.gliderty
//         taskDecl->gliderType.leftJustify(12, ' ', true));
//  strcpy(vl.declaration.flightinfo.competitionid,
//         taskDecl->compID.leftJustify(3, ' ', true));
//  strcpy(vl.declaration.flightinfo.competitionclass,
//         taskDecl->compClass.leftJustify(12, ' ', true));
//
//  // TakeOff (same ans landing ...)
//  tp = taskPoints->at(0);
//  strcpy(vl.declaration.flightinfo.homepoint.name, tp->name.left(6));
//  vl.declaration.flightinfo.homepoint.lon = tp->lonPos / 600000.0;
//  vl.declaration.flightinfo.homepoint.lat = tp->latPos / 600000.0;
//
//  // Begin of Task
//  tp = taskPoints->at(1);
//  strcpy(vl.declaration.task.startpoint.name, tp->name.left(6));
//  vl.declaration.task.startpoint.lat = tp->latPos / 600000.0;
//  vl.declaration.task.startpoint.lon = tp->lonPos / 600000.0;
//
//  for(loop = 2; loop < MIN(taskPoints->count() - 2, 12); loop++) {
//    tp = taskPoints->at(loop);
//    strcpy(vl.declaration.task.turnpoints[loop - 2].name, tp->name.left(6));
//    vl.declaration.task.turnpoints[loop - 2].lat = tp->latPos / 600000.0;
//    vl.declaration.task.turnpoints[loop - 2].lon = tp->lonPos / 600000.0;
//  }
//
//  vl.declaration.task.nturnpoints = MAX(MIN((int)taskPoints->count() - 4, 12), 0);
//
//  // End of Task
//  tp = taskPoints->at(taskPoints->count() - 2);
//  strcpy(vl.declaration.task.finishpoint.name, tp->name.left(6));
//  vl.declaration.task.finishpoint.lat = tp->latPos / 600000.0;
//  vl.declaration.task.finishpoint.lon = tp->lonPos / 600000.0;
//
//  return vl.write_db_and_declaration() == VLA_ERR_NOERR;
  return 1;
}

int readTasks(QList<FRTask> *tasks)
{
  return sp.downloadTasks(tasks);
}

int writeTasks(QList<FRTask> *tasks)
{
//  FRTask *task;
//  FRTaskPoint *tp;
//  VLAPI_DATA::ROUTE *r;
//  VLAPI_DATA::WPT *wp;
//  unsigned int taskCnt;
//  unsigned int wpCnt;
//
//  // delete old tasks
//  if(vl.database.routes != 0) {
//    delete[] vl.database.routes;
//    vl.database.routes = 0;
//  }
//  // create new, check max possible tasks
//  vl.database.nroutes = MIN(tasks->count(), maxNrTasks);
//  vl.database.routes = new VLAPI_DATA::ROUTE[vl.database.nroutes];
//
//  taskCnt = 0;
//  for (task = tasks->first(); task != 0; task = tasks->next()) {
//    // should never happen
//    if (taskCnt >= maxNrTasks) {
//      break;
//    }
//
//    r = vl.database.routes + taskCnt++;
//    strcpy(r->name, task->name.leftJustify(14, ' ', true));
//    wpCnt = 0;
//    for (tp = task->wayPoints.first(); tp != 0; tp = task->wayPoints.next()) {
//      // should never happen
//      if (wpCnt >= maxNrWaypointsPerTask) {
//        break;
//      }
//      // ignore take off and landing
//      if (tp->type == FlightTask::TakeOff || tp->type == FlightTask::Landing) {
//        continue;
//      }
//      wp = r->wpt + wpCnt++;
//      strcpy(wp->name, tp->name.leftJustify(6, ' ', true));
//      wp->lat = tp->latPos / 600000.0;
//      wp->lon = tp->lonPos / 600000.0;
//      wp->typ = 0;
//    }
//
//    // fill remaining turnpoints with '0xff'
//    while (wpCnt < maxNrWaypointsPerTask) {
//      memset(r->wpt + wpCnt++, 0xff, sizeof(VLAPI_DATA::WPT));
//    }
//  }
//
//  return vl.write_db_and_declaration() == VLA_ERR_NOERR;
  return 1;
}

int readWaypoints(QList<FRWaypoint> *waypoints)
{
  return sp.downloadWaypoints(waypoints);
}

int writeWaypoints(QList<FRWaypoint> *waypoints)
{
  return sp.uploadWaypoints(waypoints);
}
