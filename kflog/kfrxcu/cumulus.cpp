/***********************************************************************
**
**   cumulus.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "cumulus.h"

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <kio/jobclasses.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <ktempdir.h>
#include "cumuluswaypointfile.h"


Cumulus::Cumulus(){
  //Set Flightrecorders capabilities. Defaults are 0 and false.
  _capabilities.maxNrTasks = 1000;             //maximum number of tasks, limited by memory of device
  _capabilities.maxNrWaypoints = 1000;         //maximum number of waypoints, limited by memory of device
  _capabilities.maxNrWaypointsPerTask = 1000;  //maximum number of waypoints per task, limited by memory of device
  _capabilities.maxNrPilots = 2;               //maximum number of pilots

  _capabilities.supDlWaypoint = true;      //supports downloading of waypoints?
  _capabilities.supUlWaypoint = true;      //supports uploading of waypoints?
  _capabilities.supDlFlight = true;        //supports downloading of flights?
  //_capabilities.supUlFlight = true;        //supports uploading of flights?
  //_capabilities.supSignedFlight = true;    //supports downloading in of signed flights?
  _capabilities.supDlTask = true;          //supports downloading of tasks?
  //_capabilities.supUlTask = true;          //supports uploading of tasks?
  //_capabilities.supUlDeclaration = true;   //supports uploading of declarations?
  //_capabilities.supDspSerialNumber = true;
  _capabilities.supDspRecorderType = true;
  //_capabilities.supDspPilotName = true;
  //_capabilities.supDspGliderType = true;
  //_capabilities.supDspGliderID = true;
  //_capabilities.supDspCompetitionID = true;
  //End set capabilities.

  _tmpWaypointFile="";
  _home="";
  _tmpFlightDir=0;
}


Cumulus::~Cumulus(){
  if (_tmpFlightDir)
    delete _tmpFlightDir;
}

/**
 * Returns the name of the lib.
 */
QString Cumulus::getLibName() const
{
  return i18n("Cumulus/KFLog-e plugin, version %1").arg("0.2");
}


/**
 * Returns the transfermode this plugin supports.
 */
FlightRecorderPluginBase::TransferMode Cumulus::getTransferMode() const
{
  return FlightRecorderPluginBase::URL;
}


/**
 * Returns a list of recorded flights in this device.
 */
int Cumulus::getFlightDir(QPtrList<FRDirEntry>* flist){
  int res = FR_NOTSUPPORTED;
  
  if (KIO::NetAccess::exists(KURL(_URL + homePath() + "Documents/application/x-igc"), true, _parent)) {
    //directory exists
    if (!KIO::NetAccess::dircopy(KURL(_URL + homePath() + "Documents/application/x-igc"),
        KURL(getTmpFlightDir()->name()), _parent)) {
      _errorinfo=i18n("There was a problem transfering the flights from the PDA.");
      return FR_ERROR;
    }  
    
    QDir flightdir(getTmpFlightDir()->name() + "/x-igc","*.IGC",QDir::Name|QDir::IgnoreCase,QDir::Files);
    //qDebug("tmp dir name: %s", flightdir.absPath().latin1());
    if (flightdir.count()==0) {
      //the directory contains no igc files
      _errorinfo=i18n("There are no flights recorded on the PDA.");
      return FR_ERROR;
    }
    
    flightList.resize(flightdir.count());
    flist->clear();
    FRDirEntry* flightentry=0;
    
    const QFileInfoList *list = flightdir.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    int cnt=0;
    
    while ( (fi = it.current()) != 0 ) {
      flightentry = getFlightInfo(fi->filePath());
      if (flightentry) {
        flist->append(flightentry);
        flightList[cnt]= fi->filePath();
        cnt++;
      }
      ++it;
    }
    res = FR_OK;
    qDebug("returning %d flights", cnt);
        
  } else {
    //directory does not exist
    _errorinfo=i18n("There are no flights recorded on the PDA.");
    return FR_ERROR;
  }
  return res;
}


/**
 * Store file on definitive location.
 */
int Cumulus::downloadFlight(int flightID, int /*secMode*/, const QString& fileName){
  qDebug("copying flight %d (%s) to %s", flightID, flightList[flightID].latin1(), fileName.latin1());
  if (KIO::NetAccess::copy(flightList[flightID], fileName, _parent))
    return FR_OK;
  
  _errorinfo=i18n("There was a problem copying the flight from the temporary to the new location.");
  return FR_ERROR;
}


/**
  * get recorder basic data
  */
int Cumulus::getBasicData(FR_BasicData& data)
{
  _basicData.serialNumber = "???";
  _basicData.recorderType = "Cumulus";
  _basicData.pilotName = "???";
  _basicData.gliderType = "???";
  _basicData.gliderID = "???";
  _basicData.competitionID = "???";
  data = _basicData;
  return FR_OK;
}

/**
 * Opens the recorder for other communication.
 */
int Cumulus::openRecorder(const QString& URL){
  //Don't forget to set _isConnected to true if succeeded.
  KURL kurl(URL);
  QString path;
  if (kurl.hasPath()) {
    path=kurl.path(1);
  }
  //basicly, we check if cumulus is installed.
  kurl.setPath("/opt/QtPalmtop/bin/cumulus");
  if (KIO::NetAccess::exists(kurl, true, _parent)) {
    kurl.setPath(""); //clear path from URL
    _URL=kurl.url();
    _isConnected=true;
    _home = path;
    return FR_OK;
  } else {
    _isConnected=false;
    _errorinfo=i18n("<qt>Could not connect to PDA, or the following file did not exist:<br><br>%1</qt>").arg(kurl.prettyURL());
    return FR_ERROR;
  }
}


/**
 * Closes the connection with the flightrecorder.
 */
int Cumulus::closeRecorder(){
  _isConnected=false;
  return FR_OK;
  //Don't forget to set _isConnected to false if succeeded.
}


/**
 * Read tasks from recorder
 */
int Cumulus::readTasks(QPtrList<FlightTask> *tasks){
  int res=FR_NOTSUPPORTED;
  qDebug("CuFR line %d", __LINE__);
  if (!_isConnected) {                  //check if we are connected
    _errorinfo=i18n("Not connected to PDA!");
    return FR_ERROR;
  }

  if (KIO::NetAccess::exists(KURL(_URL + homePath() + QString("tasks.tsk")), true, _parent)) {
    if (!KIO::NetAccess::download(KURL(_URL + homePath() + QString("tasks.tsk")), _tmpTasksFile, _parent)) {
      warning("Something wend wrong trying to download the tasks file!");
      _errorinfo=i18n("Could not download tasks file from PDA.");
      return FR_ERROR;
    }
      
    tasks->clear();
    QFile f( _tmpTasksFile );
  
    if( !f.open( IO_ReadOnly ) )
    {
      // could not read file ...
      _errorinfo=i18n("Could not read temporary task file downloaded from PDA.");
      return FR_ERROR;
    }

   QTextStream stream( &f );
    QString line;
    bool isTask( false );
    QString numTask, taskName;
    QStringList tmpList;
    QStringList::Iterator it;
    Waypoint* wp;
    QList<Waypoint> wpList;
    //wpList.setAutoDelete(true);
  
    while( !stream.eof() )
      {
        line = stream.readLine();
  
        if( line.mid( 0, 1 ) == "#" ) continue;
  
        if( line.mid( 0, 2 ) == "TS" )
          {
            // new task ...
            isTask = true;
            tmpList = QStringList::split( ",", line, true );
            it = tmpList.at( 1 );
            taskName = (*it).latin1();
          }
        else if( line.mid( 0, 2 ) == "TW" && isTask )
          {
            // new wp ...
            wpList.append( new Waypoint );
            wp = wpList.current();
  
            tmpList = QStringList::split( ",", line, true );
            wp->origP.setLat( ( (QString)( *tmpList.at( 1 ) ).latin1() ).toInt() );
            wp->origP.setLon( ( (QString)( *tmpList.at( 2 ) ).latin1() ).toInt() );
            //wp->projP = _globalMapMatrix->wgsToMap( wp->origP );
            wp->elevation = ( (QString)( *tmpList.at( 3 ) ).latin1() ).toInt();
            wp->name = ( *tmpList.at( 4 ) ).latin1();
            wp->icao = ( *tmpList.at( 5 ) ).latin1();
            wp->description = ( *tmpList.at( 6 ) ).latin1();
            wp->frequency = ( (QString)( *tmpList.at( 7 ) ).latin1() ).toDouble();
          }
        else if( line.mid( 0, 2 ) == "TE" && isTask )
          {
           // task complete
            isTask = false;
            tasks->append( new FlightTask( wpList, true, taskName ) );
            numTask.sprintf( "%d", tasks->count() );
      
            wpList.clear();
          }
      }
  
    f.close();
 
    if (tasks->count()==0)
      {
        _errorinfo=i18n("No tasks defined on PDA.");
        return FR_ERROR;
      }
    else 
      {
  qDebug("CuFR taskcount: %d", tasks->count());
        return FR_OK;
      }  
    
  } else {
    _errorinfo=i18n("Tasks not found on PDA.");
    return FR_ERROR;
  }
  return res;
}


/**
 * Write tasks to recorder
 */
int Cumulus::writeTasks(QPtrList<FlightTask> *tasks){
  return FR_NOTSUPPORTED;
}


/**
 * Read waypoints from recorder
 */
int Cumulus::readWaypoints(QPtrList<Waypoint> *waypoints){
  int ret=0;

  if (!_isConnected) {                  //check if we are connected
    _errorinfo=i18n("Not connected to PDA!");
    return FR_ERROR;
  }

  //get the waypointfile from the device using a KIO slave
  _tmpWaypointFile="";
  //qDebug("Trying to access %s",QString(_URL + "/home/root/cumulus.kwp").latin1());
  if (!KIO::NetAccess::download(KURL(_URL + homePath() + "Applications/cumulus/cumulus.kwp"),_tmpWaypointFile, _parent)) {
    warning("Something wend wrong trying to download the waypointfile!");
    _errorinfo=i18n("Could not download waypointfile from PDA.");
    return FR_ERROR;
  }

  //_tmpWaypointFile now contains the name of our temporary local waypointfile.
  //qDebug("Local waypointfile: %s",_tmpWaypointFile.latin1());

  //create a cumulus waypoint file object, use it to load the file, and return the result.
  CumulusWaypointFile* wpfile=new CumulusWaypointFile;
  ret=wpfile->load(_tmpWaypointFile, waypoints);
  //qDebug("number of waypoints read: %d",ret);
  delete wpfile;
  KIO::NetAccess::removeTempFile(_tmpWaypointFile);
  _tmpWaypointFile="";
  return ret;
}


/**
 * Write waypoints to recorder
 */
int Cumulus::writeWaypoints(QPtrList<Waypoint> *waypoints){
  int ret=FR_NOTSUPPORTED;
  if (!_isConnected) {                  //check if we are connected
    _errorinfo=i18n("Not connected to PDA!");
    return FR_ERROR;
  }

  KTempFile f;                          //create a temporary file,
  f.setAutoDelete(true);                //  and set autodelete to true.
  if (f.status()!=0) {                  //check to see if a temporary file could be created.
    _errorinfo=i18n("Could not create temporary file. Please check your writepermissions.");
    return FR_ERROR;
  }
  CumulusWaypointFile* wpfile=new CumulusWaypointFile;  //create a new CumulusWaypointFile,
  ret=wpfile->save(f.name(), waypoints);                //   and use it to write our waypoints.
  if (ret>0) {                                          //writing to tempfile wend well
    if (KIO::NetAccess::upload(f.name(),KURL(_URL + homePath() + "Applications/cumulus/cumulus.kwp"), _parent)) { //try to upload the file
      ret=FR_OK;
    } else {
      _errorinfo=i18n("Upload of temporary file to PDA failed.");
      ret=FR_ERROR;
    }
  }

  return ret;
}


QString Cumulus::homePath() {
  if (!_home.isEmpty()) {
    return _home;
  } 
     
  if (KIO::NetAccess::exists(KURL(_URL + "/home/zaurus/Settings/Cumulus.conf"), true, _parent)) {
    _home="/home/zaurus/";
    return _home;
  }
  if (KIO::NetAccess::exists(KURL(_URL + "/home/root/Settings/Cumulus.conf"), true, _parent)) {
    _home="/home/root/";
    return _home;
  }
  if (KIO::NetAccess::exists(KURL(_URL + "/root/Settings/Cumulus.conf"), true, _parent)) {
    _home="/root/";
    return _home;
  }
  qDebug("CuFR line %d", __LINE__);
}


KTempDir* Cumulus::getTmpFlightDir() {
  if (_tmpFlightDir)
    return _tmpFlightDir;
  
  _tmpFlightDir=new KTempDir();
  _tmpFlightDir->setAutoDelete(true);
  if (_tmpFlightDir->status()!=0)
    qFatal("error creating temporary directory!");
  return _tmpFlightDir;
}


/**
 * Returns an FRDirEntry struct for the igc file
 */
FRDirEntry* Cumulus::getFlightInfo(QString filename) {
  QString fName = filename;

  QFileInfo fInfo(fName);
  if(!fInfo.exists() || !fInfo.size() ||
      (((QString)fInfo.extension()).lower() != "igc"))  return 0;

  QFile igcFile(fName);
  if(!igcFile.open(IO_ReadOnly))  return 0;

  QString s, pilotName, gliderType, gliderID, lastB;
  QTextStream stream(&igcFile);
  tm firstfix, lastfix, date;
  int duration;
  
  bool isFirst = true;
  bool hasFirstB = false;

  QString previewText, waypointText("<EM>" + i18n("no task given") + "</EM>");

  while (!stream.eof())
    {
      s = stream.readLine();

      if(s.mid(0,1) == "H")
        {
          // We have a headerline
          if(s.mid(1,4).upper() == "FPLT")
              pilotName = s.mid(s.find(':')+1,100);
          else if(s.mid(1,4).upper() == "FGTY")
              gliderType = s.mid(s.find(':')+1,100);
          else if(s.mid(1,4).upper() == "FGID")
              gliderID = s.mid(s.find(':')+1,100);
          else if(s.mid(1,4).upper() == "FDTE") {
              date.tm_mday = s.mid(5,2).toInt();
              date.tm_mon = s.mid(7,2).toInt() -1;
              date.tm_year = s.mid(9,2).toInt() + 100; //only works for flights recorded after the year 2000, which is reasonable given the age of Cumulus :-)
              firstfix=date;
              lastfix=date;
              firstfix.tm_hour=0; firstfix.tm_min=0; firstfix.tm_sec=0;
              lastfix.tm_hour=0; lastfix.tm_min=0; lastfix.tm_sec=0;
            }
              
        }
      else if(s.mid(0,1) == "B")
        {
          if (!hasFirstB) {
            hasFirstB = true;
            firstfix=date;
            firstfix.tm_hour = s.mid(1,2).toInt();
            firstfix.tm_min = s.mid(3,2).toInt();
            firstfix.tm_sec = s.mid(5,2).toInt();
            //FIXME: we should try if we can advance the filepointer to nearly the end afterwards...
          } 
          lastB=s;
        }
    }
  lastfix=date;
  lastfix.tm_hour = lastB.mid(1,2).toInt();
  lastfix.tm_min = lastB.mid(3,2).toInt();
  lastfix.tm_sec = lastB.mid(5,2).toInt();
  
  FRDirEntry* result = new FRDirEntry;
  result->pilotName = pilotName;
  result->gliderType = gliderType;
  result->gliderID = gliderID;
  result->firstTime = firstfix;
  result->lastTime = lastfix;
  result->shortFileName = filename;
  result->longFileName = filename;
  result->duration = lastfix.tm_sec-firstfix.tm_sec + 
                    (lastfix.tm_min-firstfix.tm_min) * 60 +
                    (lastfix.tm_hour-firstfix.tm_hour) * 3600;
                    //FIXME: this does not take flights across the GMT date border into account.
  return result;
}


/** NOT IMLEMENTED
    ============================================*/

/**
 * Opens the recorder for serial communication.
 */
int Cumulus::openRecorder(const QString& portName, int baud) {
  return FR_NOTSUPPORTED;
}


 /**
 * Write flight declaration to recorder
 */
int Cumulus::writeDeclaration(FRTaskDeclaration *taskDecl, QPtrList<Waypoint> *taskPoints) {
  return FR_NOTSUPPORTED;
}

