/***********************************************************************
**
**   volkslogger.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by Harald Maier
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "volkslogger.h"
#include "vlapi2.h"
#include "vlapihlp.h"

#include <fcntl.h>
#include <termios.h>
#include <ctime>
#include <csignal>

#include <ctype.h>

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

VLAPI vl;

Volkslogger::Volkslogger( QObject *parent ) : FlightRecorderPluginBase( parent )
{
  //Set Flight recorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = 25;             //maximum number of tasks
  _capabilities.maxNrWaypoints = 500;         //maximum number of waypoints
  _capabilities.maxNrWaypointsPerTask = 10; //maximum number of waypoints per task
  _capabilities.maxNrPilots = 25;            //maximum number of pilots

  _capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  _capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  _capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  _capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  _capabilities.supDlTask = true;          //supports downloading of tasks?
  _capabilities.supUlTask = true;          //supports uploading of tasks?
  _capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  _capabilities.supDspSerialNumber = true;
  _capabilities.supDspRecorderType = true;
  //_capabilities.supDspPilotName = true;
  //_capabilities.supDspGliderType = true;
  //_capabilities.supDspGliderID = true;
  //_capabilities.supDspCompetitionID = true;
  //End set capabilities.

  portID = -1;
  haveDatabase = false;
}

Volkslogger::~Volkslogger()
{
  closeRecorder();
}

/**
 * Returns the transfer mode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Volkslogger::getTransferMode() const
{
  return FlightRecorderPluginBase::serial;
}

QString Volkslogger::getLibName() const {  return "libkfrgcs";  }

int Volkslogger::getFlightDir(QList<FRDirEntry*>* dirList)
{
  qDeleteAll( *dirList );
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
      entry->longFileName = QString().asprintf("%d-%.2d-%.2d-GCS-%s-%.2d.igc",
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

  return err == VLA_ERR_NOERR ? FR_OK : FR_ERROR;
}

/*QByteArray getFlight(int flightID, char* tmpFileName, int* ret)
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
*/
int Volkslogger::downloadFlight(int flightID, int secMode, const QString& fileName)
{
  return (vl.read_igcfile( fileName.toLatin1().data(), flightID, secMode) == VLA_ERR_NOERR ? FR_OK : FR_ERROR);
}


/**
  * get recorder basic data
  */
int Volkslogger::getBasicData(FR_BasicData& data)
{
  vl.read_info();
  _basicData.serialNumber = wordtoserno(vl.vlinfo.vlserno);
  _basicData.recorderType = "Volkslogger";
  _basicData.pilotName = "???";
  _basicData.gliderType = "???";
  _basicData.gliderID = "???";
  _basicData.competitionID = "???";
  data = _basicData;
  return FR_OK;
}

int Volkslogger::getConfigData(FR_ConfigData& /*data*/)
{
  return FR_NOTSUPPORTED;
}

int Volkslogger::writeConfigData(FR_BasicData& /*basicdata*/, FR_ConfigData& /*configdata*/)
{
  return FR_NOTSUPPORTED;
}

int Volkslogger::openRecorder(const QString& pName, int baud)
{
  int err;
  portName = pName.toLatin1().data();

  if((err = vl.open(1, 5, 0, baud)) != VLA_ERR_NOERR) {
    qWarning() << QObject::tr("No logger found!");
    _isConnected = false;
  }
  else {
    _isConnected = true;
  }

  return err == VLA_ERR_NOERR ? FR_OK : FR_ERROR;
}

int Volkslogger::closeRecorder()
{
  _isConnected = false;
  vl.close(1);
  return FR_OK;
}

int Volkslogger::exportDeclaration(FRTaskDeclaration* taskDecl, QList<Waypoint*> *taskPoints, const QString&)
{
  Q_UNUSED(taskDecl);
  Q_UNUSED(taskPoints);
  return FR_NOTSUPPORTED;
}

int Volkslogger::writeDeclaration(FRTaskDeclaration* taskDecl, QList<Waypoint*> *taskPoints, const QString&)
{
  Waypoint *tp;
  int loop;

  if (!haveDatabase) {
    if (readDatabase() == FR_ERROR) {
      return FR_ERROR;
    }
  }
  // Filling the strings with white spaces
  QString pilotA(taskDecl->pilotA.leftJustified(32, ' ', true));
  QString pilotB(taskDecl->pilotB.leftJustified(32, ' ', true));
  sprintf( vl.declaration.flightinfo.pilot, "%s%s",
           pilotA.toLatin1().data(),
           pilotB.toLatin1().data() );

  strcpy(vl.declaration.flightinfo.gliderid,
         taskDecl->gliderID.leftJustified(7, ' ', true).toLatin1().data());
  strcpy(vl.declaration.flightinfo.glidertype,
         taskDecl->gliderType.leftJustified(12, ' ', true).toLatin1().data());
  strcpy(vl.declaration.flightinfo.competitionid,
         taskDecl->compID.leftJustified(3, ' ', true).toLatin1().data());
  strcpy(vl.declaration.flightinfo.competitionclass,
         taskDecl->compClass.leftJustified(12, ' ', true).toLatin1().data());

  // TakeOff (same ans landing ...)
  tp = taskPoints->at(0);
  strcpy(vl.declaration.flightinfo.homepoint.name, tp->name.left(6).toLatin1().data());
  vl.declaration.flightinfo.homepoint.lon = tp->origP.lon() / 600000.0;
  vl.declaration.flightinfo.homepoint.lat = tp->origP.lat() / 600000.0;

  // Begin of Task
  tp = taskPoints->at(1);
  strcpy(vl.declaration.task.startpoint.name, tp->name.left(6).toLatin1().data());
  vl.declaration.task.startpoint.lat = tp->origP.lat() / 600000.0;
  vl.declaration.task.startpoint.lon = tp->origP.lon() / 600000.0;

  for(loop = 2; loop < std::min(int(taskPoints->count()) - 2, 12); loop++) {
    tp = taskPoints->at(loop);
    strcpy(vl.declaration.task.turnpoints[loop - 2].name, tp->name.left(6).toLatin1().data());
    vl.declaration.task.turnpoints[loop - 2].lat = tp->origP.lat() / 600000.0;
    vl.declaration.task.turnpoints[loop - 2].lon = tp->origP.lon() / 600000.0;
  }

  vl.declaration.task.nturnpoints = std::max(std::min((int)taskPoints->count() - 4, 12), 0);

  // End of Task
  tp = taskPoints->at(taskPoints->count() - 2);
  strcpy(vl.declaration.task.finishpoint.name, tp->name.left(6).toLatin1().data());
  vl.declaration.task.finishpoint.lat = tp->origP.lat() / 600000.0;
  vl.declaration.task.finishpoint.lon = tp->origP.lon() / 600000.0;

  return vl.write_db_and_declaration() == VLA_ERR_NOERR ? FR_OK : FR_ERROR;
}

int Volkslogger::readDatabase()
{
  if (vl.read_db_and_declaration() == VLA_ERR_NOERR) {
    haveDatabase = true;
    return FR_OK;
  }
  else {
    haveDatabase = false;
    return FR_ERROR;
  }
}

int Volkslogger::readTasks(QList<FlightTask*> *tasks)
{
  QList<Waypoint*> taskPoints;
  Waypoint *tp;
  VLAPI_DATA::ROUTE *r;
  VLAPI_DATA::WPT *wp;
  int taskCnt;
  int wpCnt;

  if (!haveDatabase) {
    if (readDatabase() == FR_ERROR) {
      return FR_ERROR;
    }
  }

  for (taskCnt = 0; taskCnt < vl.database.nroutes; taskCnt++) {
    r = &(vl.database.routes[taskCnt]);
    taskPoints.clear();
    for (wpCnt = 0; wpCnt < _capabilities.maxNrWaypointsPerTask; wpCnt++) {
      wp = &(r->wpt[wpCnt]);
      if (isalnum(wp->name[0])) {
        tp = new Waypoint;
        tp->name = wp->name;
        tp->origP.setPos((int)(wp->lat * 600000.0), (int)(wp->lon * 600000.0));
        tp->type = FlightTask::RouteP;

        if (taskPoints.count() == 0) {
          // append take off
          tp->type = FlightTask::TakeOff;
          taskPoints.append(tp);
          // make copy for begin
          tp = new Waypoint(taskPoints.first());
          tp->type = FlightTask::Begin;
        }
        taskPoints.append(tp);
      }
    }
    // modify last for end of task
    taskPoints.last()->type = FlightTask::End;
    // make copy for landing
    tp = new Waypoint(taskPoints.last());
    tp->type = FlightTask::Landing;
    taskPoints.append(tp);

    tasks->append(new FlightTask(taskPoints, true, r->name));
  }
  return FR_OK;
}

int Volkslogger::writeTasks(QList<FlightTask*> *tasks)
{
  FlightTask *task;
  QList<Waypoint*> taskPoints;
  Waypoint *tp;
  VLAPI_DATA::ROUTE *r;
  VLAPI_DATA::WPT *wp;
  int taskCnt = 0;
  int wpCnt;

  if (!haveDatabase) {
    if (readDatabase() == FR_ERROR) {
      return FR_ERROR;
    }
  }

  // delete old tasks
  if(vl.database.routes != 0) {
    delete[] vl.database.routes;
    vl.database.routes = 0;
  }
  // create new, check max possible tasks
  vl.database.nroutes = std::min(tasks->count(), _capabilities.maxNrTasks);
  vl.database.routes = new VLAPI_DATA::ROUTE[vl.database.nroutes];

  foreach(task, *tasks) {
    // should never happen
    if (taskCnt >= _capabilities.maxNrTasks) {
      break;
    }

    r = vl.database.routes + taskCnt++;
    strcpy(r->name, task->getFileName().leftJustified(14, ' ', true).toLatin1().data());
    wpCnt = 0;
    taskPoints = task->getWPList();
    foreach(tp, taskPoints) {
      // should never happen
      if (wpCnt >= _capabilities.maxNrWaypointsPerTask) {
        break;
      }
      // ignore take off and landing
      if (tp->type == FlightTask::TakeOff || tp->type == FlightTask::Landing) {
        continue;
      }
      wp = r->wpt + wpCnt++;
      strcpy(wp->name, tp->name.leftJustified(6, ' ', true).toLatin1().data());
      wp->lat = tp->origP.lat() / 600000.0;
      wp->lon = tp->origP.lon() / 600000.0;
      wp->typ = 0;
    }

    // fill remaining turn points with '0xff'
    while (wpCnt < _capabilities.maxNrWaypointsPerTask) {
      memset(r->wpt + wpCnt++, 0xff, sizeof(VLAPI_DATA::WPT));
    }
  }

  return vl.write_db_and_declaration() == VLA_ERR_NOERR ? FR_OK : FR_ERROR;
}

int Volkslogger::readWaypoints(QList<Waypoint*> *waypoints)
{
  int n;
  Waypoint *frWp;
  VLAPI_DATA::WPT *wp;

  if (!haveDatabase) {
    if (readDatabase() == FR_ERROR) {
      return FR_ERROR;
    }
  }

  for (n = 0; n < vl.database.nwpts; n++) {
    wp = &(vl.database.wpts[n]);
    Runway rwy;
    frWp = new Waypoint;
    frWp->rwyList.append(rwy);
    frWp->name = wp->name;
    frWp->name = frWp->name.trimmed();

    frWp->origP.setPos((int)(wp->lat * 600000.0), (int)(wp->lon * 600000.0));

    bool isLandable = (wp->typ & VLAPI_DATA::WPT::WPTTYP_L) > 0;

    if (isLandable) {
      rwy.setOperations( Runway::Active );
      rwy.setSurface( (wp->typ & VLAPI_DATA::WPT::WPTTYP_H) > 0 ? Runway::Asphalt : Runway::Grass );
    }

    frWp->type = (wp->typ & VLAPI_DATA::WPT::WPTTYP_A) > 0 ? BaseMapElement::Airfield : -1;

    waypoints->append(frWp);
  }
  return FR_OK;
}

int Volkslogger::writeWaypoints(QList<Waypoint*> *waypoints)
{
  Waypoint *frWp;
  VLAPI_DATA::WPT *wp;
  int wpCnt;

  if (!haveDatabase) {
    if (readDatabase() == FR_ERROR) {
      return FR_ERROR;
    }
  }

  // delete old waypoints
  if(vl.database.wpts != 0) {
    delete[] vl.database.wpts;
    vl.database.wpts = 0;
  }
  // create new, check max possible wapoints
  vl.database.nwpts = std::min(waypoints->count(), _capabilities.maxNrWaypoints);
  vl.database.wpts = new VLAPI_DATA::WPT[vl.database.nwpts];

  wpCnt = 0;

  foreach(frWp, *waypoints) {
    // should never happen
    if (wpCnt >= _capabilities.maxNrWaypoints) {
      break;
    }
    wp = &(vl.database.wpts[wpCnt++]);
    strcpy(wp->name, frWp->name.leftJustified(6, ' ', true).toLatin1().data());
    wp->lat = frWp->origP.lat() / 600000.0;
    wp->lon = frWp->origP.lon() / 600000.0;

    Runway rwy;

    if( frWp->rwyList.size() > 0 )
      {
        rwy = frWp->rwyList[0];
      }

    // TODO
//    wp->typ =
//      (rwy.m_isOpen ? VLAPI_DATA::WPT::WPTTYP_L : 0) |
//      (rwy.m_surface == Runway::Asphalt || rwy.m_surface == Runway::Concrete ? VLAPI_DATA::WPT::WPTTYP_H : 0) |
//      (frWp->type == BaseMapElement::Airfield || frWp->type == BaseMapElement::Gliderfield ||
//       frWp->type == BaseMapElement::Airport || frWp->type == BaseMapElement::IntAirport ||
//       frWp->type == BaseMapElement::MilAirport || frWp->type == BaseMapElement::CivMilAirport ? VLAPI_DATA::WPT::WPTTYP_A : 0);
  }

  return vl.write_db_and_declaration() == VLA_ERR_NOERR ? FR_OK : FR_ERROR;
}

/** NOT IMLEMENTED
    ============================================*/

/**
 * Opens the recorder for other communication.
 */
int Volkslogger::openRecorder(const QString& /*URL*/)
{
  return FR_NOTSUPPORTED;
}
