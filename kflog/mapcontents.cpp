/**********************************************************************
 **
 **   mapcontents.cpp
 **
 **   This file is part of KFLog2.
 **
 **   $Id$
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **
 ***********************************************************************/

#include <cmath>
#include <iostream>
#include <stdlib.h>

#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>

#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qdom.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstring.h>
#include <qtextstream.h>

#include "mapcontents.h"
#include "mapmatrix.h"
#include "mapcalc.h"
#include "airport.h"
#include "airspace.h"
#include "basemapelement.h"
#include "downloadlist.h"
#include "flight.h"
#include "flightgroup.h"
#include "flightselectiondialog.h"
#include "glidersite.h"
#include "isohypse.h"
#include "kflog.h"
#include "lineelement.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "elevationfinder.h"
#include "openairparser.h"
#include "welt2000.h"

/*
 * Used as bit-masks to determine, if we must display
 * messageboxes on missing map-directories.
 */
#define MAP_LOADED 8

// number of last map tile, possible range goes 0...16200
#define MAX_TILE_NUMBER 16200

// number of different isoline levels
#define ISO_LINE_NUM 50

// general KFLOG file token: @KFL
#define KFLOG_FILE_MAGIC    0x404b464c

// uncompiled map file types
#define FILE_TYPE_AERO        0x41
#define FILE_TYPE_GROUND      0x47
#define FILE_TYPE_TERRAIN     0x54
#define FILE_TYPE_MAP         0x4d
#define FILE_TYPE_LM          0x4c

// versions
// The *_OLD files are based on GTOPO30 data. Support for this will disappear in the QT4 release.
// The newer files are based on SRTM3 data.
#define FILE_VERSION_GROUND_OLD   100
#define FILE_VERSION_GROUND       102
#define FILE_VERSION_TERRAIN_OLD  100
#define FILE_VERSION_TERRAIN      102
#define FILE_VERSION_MAP          101

#define CHECK_BORDER if(i == 0) {                       \
    border.north = lat_temp;   border.south = lat_temp; \
    border.east = lon_temp;    border.west = lon_temp;  \
  } else {                                              \
    border.north = std::max(border.north, lat_temp);         \
    border.south = std::min(border.south, lat_temp);         \
    border.east = std::max(border.east, lon_temp);           \
    border.west = std::min(border.west, lon_temp);           \
  }

#define READ_POINT_LIST  in >> locLength; \
  tA.resize(locLength); \
  if (locLength == 0) { \
    qDebug("zero length pointlist! (type %d)", typeIn); \
  } else { \
    for(unsigned int i = 0; i < locLength; i++) { \
      in >> lat_temp;          in >> lon_temp; \
      tA.setPoint(i, _globalMapMatrix.wgsToMap(lat_temp, lon_temp)); \
    } \
  }

#define READ_CONTACT_DATA in >> contactCount;                           \
  for(unsigned int loop = 0; loop < contactCount; loop++) \
    { \
      in >> frequency; \
      in >> contactType; \
      in >> callSign; \
    }

#define READ_RUNWAY_DATA(site) in >> rwCount; \
  for(unsigned int loop = 0; loop < rwCount; loop++) \
    { \
      in >> rwDirection; \
      in >> rwLength; \
      in >> rwMaterial; \
      in >> rwOpen; \
      runway* rw = new runway( rwLength, rwDirection, rwMaterial, rwOpen); \
      site->addRunway(rw); \
    }

// List of elevation levels (50 in total):
const int MapContents::isoLines[] =
{
  0, 10, 25, 50, 75, 100, 150, 200, 250,
  300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750,
  2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000, 4250, 4500,
  4750, 5000, 5250, 5500, 5750, 6000, 6250, 6500, 6750, 7000, 7250,
  7500, 7750, 8000, 8250, 8500, 8750
};

MapContents::MapContents()
  : isFirstLoad(0)
{
  // Setup a hash used as reverse mapping from isoLine value to array index to
  // speed up loading of ground and terrain files.
  for ( int i = 0; i < ISO_LINE_NUM; i++ ) {
    isoHash.insert( std::pair<int, int>(isoLines[i],i) );
  }


  sectionArray.resize(MAX_TILE_NUMBER);
  sectionArray.fill(false);

  for(unsigned int loop = 0; loop < ISO_LINE_NUM; loop++)
      isoList.append(new QPtrList<Isohypse>);

  airportList.setAutoDelete(true);
  airspaceList.setAutoDelete(true);
  gliderSiteList.setAutoDelete(true);
  flightList.setAutoDelete(true);
  hydroList.setAutoDelete(true);
  landmarkList.setAutoDelete(true);
  navList.setAutoDelete(true);
  obstacleList.setAutoDelete(true);
  outList.setAutoDelete(true);
  railList.setAutoDelete(true);
  reportList.setAutoDelete(true);
  roadList.setAutoDelete(true);
//  stationList.setAutoDelete(true);
  topoList.setAutoDelete(true);
  wpList.setAutoDelete(false);
  regIsoLines.setAutoDelete(true);
  regIsoLinesWorld.setAutoDelete(true);
  downloadList = new DownloadList();

  connect(downloadList,SIGNAL(allDownloadsFinished()),this,SLOT(slotDownloadFinished()));
}

MapContents::~MapContents()
{
  // Hier müssen ALLE Listen gelöscht werden!!!
/*  airportList.~QPtrList();
  airspaceList.~QPtrList();
  cityList.~QPtrList();
  gliderSiteList.~QPtrList();
  flightList.~QPtrList();
  hydroList.~QPtrList();
  landmarkList.~QPtrList();
  navList.~QPtrList();
  obstacleList.~QPtrList();
  outList.~QPtrList();
  railList.~QPtrList();
  reportList.~QPtrList();
  roadList.~QPtrList();
  stationList.~QPtrList();
  topoList.~QPtrList();
*/
}

int MapContents::degreeToNum(QString inDegree)
{
  /*
   * needed formats:
   *
   *  [g]gg° mm' ss"
   *  dddddddddd
   */
  QRegExp degree("^[0-9]?[0-9][0-9]°[ ]*[0-9][0-9]'[ ]*[0-9][0-9]\"");
  QRegExp number("^-?[0-9]+$");

  if(number.match(inDegree) != -1)
      return inDegree.toInt();
  else if(degree.match(inDegree) != -1)
    {
      int deg = 0, min = 0, sec = 0, result = 0;

      QRegExp deg1("°");
      deg = inDegree.mid(0, deg1.match(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.match(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.match(inDegree)).toInt();
      inDegree = inDegree.mid(deg2.match(inDegree) + 1, inDegree.length());

      QRegExp deg3("\"");
      sec = inDegree.mid(0, deg3.match(inDegree)).toInt();

      result = (int)((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));

      // We add 1 to avoid rounding-errors ...
      result += 1;

      QRegExp dir("[swSW]$");
      if(dir.match(inDegree) >= 0) return -result;

      return result;
    }

  // Wrong format!!!
  return 0;
}

void MapContents::closeFlight()
{
  /*
   * close current flight
   */
  BaseFlightElement *f = flightList.current();
  if(f != 0)
    {
      emit closingFlight(f);
      for (unsigned int i = 0; i < flightList.count(); i++)
        {
          FlightGroup *fg = (FlightGroup *) flightList.at(i);
          if(fg->getTypeID() == BaseMapElement::FlightGroup)
              fg->removeFlight(f);

        }

      flightList.remove(f);
      if (flightList.current() == 0) flightList.last();

      emit currentFlightChanged();
    }
}

void MapContents::__downloadFile(QString fileName, QString destString, bool wait){
  extern QSettings _settings;

  if (_settings.readNumEntry("/GeneralOptions/AutomaticMapDownload")==Inhibited)
      return;

  QUrl src = QUrl(_settings.readEntry("/GeneralOptions/Mapserver","http://maproom.kflog.org:80/mapdata/data/landscape/"));
  QUrl dest = QUrl("file:/" + destString);
  src.addPath(fileName);
  dest.addPath(fileName);

  if (wait)
    {
      KIO::NetAccess::copy(src, dest, 0); // waits until file is transmitted
      QString errorString = KIO::NetAccess::lastErrorString();
      if (errorString!="")
          QMessageBox::warning(0, "Error", errorString, QMessageBox::Ok, 0);
    }
  else
    {
      downloadList->copyKURL(&src,&dest);
    }
}

bool MapContents::__readTerrainFile( const int fileSecID,
                                     const int fileTypeID)
{
  extern const MapMatrix _globalMapMatrix;

  QString pathName;
  pathName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
  pathName = mapDir + "/landscape/" + pathName;

  if(pathName == 0)
    // Data does not exist ...
    return false;

  QFile mapfile(pathName);
  if(!mapfile.open(IO_ReadOnly)) {
    // Data exists, but can't be read:
    // We need a messagebox
    warning("KFLog: Can not open terrainfile %s", (const char*)pathName);

    QString fileName;
    fileName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
    __downloadFile(fileName,mapDir + "/landscape");

    return false;
  }

  QDataStream in(&mapfile);
  in.setVersion(6);  // QDataStream::Qt_3_3

  Q_INT8 loadTypeID;
  Q_UINT16 loadSecID, formatID;
  Q_UINT32 magic;
  QDateTime createDateTime;

  in >> magic;
  if(magic != KFLOG_FILE_MAGIC) {
    // not a .kfl file
    warning("KFLog: Trying to open old or invalid map-file; aborting!");
    warning("%s", (const char*)pathName);
    return false;
  }

  in >> loadTypeID;
  if(loadTypeID != fileTypeID) {
    // wrong data type
    return false;
  }

  in >> formatID;
  // Determine, which file format id is expected
  int expFormatID, expOldFormatID;

  if ( fileTypeID == FILE_TYPE_TERRAIN ) {
    expFormatID = FILE_VERSION_TERRAIN;
    expOldFormatID = FILE_VERSION_TERRAIN_OLD;
  } else {
    expFormatID = FILE_VERSION_GROUND;
    expOldFormatID = FILE_VERSION_GROUND_OLD;
  }

  if (formatID < expFormatID && formatID != expOldFormatID) {
    warning("KFLog: File format too old! (version %d, expecting: %d)",
        formatID, expFormatID );
    return false;
  } else if (formatID > expFormatID) {
    warning("KFLog: File format too new! (version %d, expecting: %d)",
        formatID, expFormatID );
    return false;
  }

  if (formatID == expOldFormatID) {   // this is for old terrain and ground files
    warning("KFLog: You are using old map files. Please consider re-installing\n"
            "       the terrain and ground files. Support for the old file format\n"
            "       will cease in the next major KFLog release.");
  }

  in >> loadSecID;
  if(loadSecID != fileSecID) {
    // wrong tile number.
    return false;
  }
  in >> createDateTime;

  while(!in.eof())
    {
      Q_UINT8 type;
      Q_INT16 elevation;
      Q_INT8 valley, sort;
      Q_INT32 locLength, latList_temp, lonList_temp;

      if (formatID == expOldFormatID) {   // this is for old terrain and ground files
        in >> type;
        in >> elevation;
        in >> valley;
        in >> sort;
        in >> locLength;
      } else {
        in >> elevation;
        in >> locLength;
      }

      if (elevation < 0) {
        elevation = 0;
      }

      QPointArray tA(locLength);

      for(int i = 0; i < locLength; i++) {
        in >> latList_temp;
        in >> lonList_temp;

        tA.setPoint(i, _globalMapMatrix.wgsToMap(latList_temp, lonList_temp));
      }

      sort = 0;
      valley = 0;

      Isohypse* newItem = new Isohypse(tA, elevation, valley);
      isoList.at(isoHash[elevation])->append(newItem);

    }

  return true;
}

bool MapContents::__readBinaryFile(const int fileSecID,
                                   const char fileTypeID)
{
  extern const MapMatrix _globalMapMatrix;

  QString pathName;
  pathName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
  pathName = mapDir + "/landscape/" + pathName;
  if(pathName == 0)
      // File does not exist ...
      return false;

  QFile mapfile(pathName);
  if(!mapfile.open(IO_ReadOnly))
    {
      // Data exists, but can't be read:
      // We need a messagebox
      warning("KFLog: Can not open mapfile %s", (const char*)pathName);

      QString fileName;
      fileName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
      __downloadFile(fileName,mapDir + "/landscape");

      return false;
    }

  QDataStream in(&mapfile);
  in.setVersion(6);  // QDataStream::Qt_3_3

  Q_UINT8 typeIn, lm_typ, index;
  Q_INT8 loadTypeID, sort, elev;
  Q_UINT16 loadSecID, formatID;
  Q_INT32 lat_temp, lon_temp;
  Q_UINT32 magic, locLength = 0;
  QDateTime createDateTime;
  QString name = "";

  in >> magic;
  if(magic != KFLOG_FILE_MAGIC)  return false;

  in >> loadTypeID;
  if(loadTypeID != fileTypeID)  return false;

  in >> formatID;
  if(formatID < FILE_VERSION_MAP) {
    warning("KFLog: File format too old! (version %d, expecting: %d)",
        formatID, FILE_VERSION_MAP );
    return false;
  } else if(formatID > FILE_VERSION_MAP) {
    warning("KFLog: File format too new! (version %d, expecting: %d)",
        formatID, FILE_VERSION_MAP );
    return false;
  }

  in >> loadSecID;
  if(loadSecID != fileSecID)  return false;

  in >> createDateTime;

  unsigned int gesamt_elemente = 0;
  while(!in.eof()) {
    in >> typeIn;
    locLength = 0;
    name = "";

    QPointArray tA;

    gesamt_elemente++;

    switch (typeIn) {
      case BaseMapElement::Highway:
      case BaseMapElement::Road:
      case BaseMapElement::Trail:
      case BaseMapElement::Railway:
      case BaseMapElement::Railway_D:
      case BaseMapElement::Aerial_Cable:
        READ_POINT_LIST
        roadList.append(new LineElement("", typeIn, tA));
        break;
      case BaseMapElement::Canal:
      case BaseMapElement::River:
      case BaseMapElement::River_T:
        if(formatID >= FILE_VERSION_MAP) in >> name;
        READ_POINT_LIST
        hydroList.append(new LineElement(name, typeIn, tA));
        break;
      case BaseMapElement::City:
        in >> sort;
        if(formatID >= FILE_VERSION_MAP) in >> name;
        READ_POINT_LIST
        cityList.append(new LineElement(name, typeIn, tA, sort));
        break;
      case BaseMapElement::Lake:
      case BaseMapElement::Lake_T:
        in >> sort;
        if(formatID >= FILE_VERSION_MAP) in >> name;
        READ_POINT_LIST
        hydroList.append(new LineElement(name, typeIn, tA, sort));
        break;
      case BaseMapElement::PackIce:
        // is currently not being used
        // stays anyway because of errors in the MapBin in the Data
        //qDebug("filepointer: %d", mapfile.at());
        READ_POINT_LIST
        if(formatID >= FILE_VERSION_MAP) in >> name;
        break;
      case BaseMapElement::Forest:
      case BaseMapElement::Glacier:
        in >> sort;
        if(formatID >= FILE_VERSION_MAP) in >> name;
        READ_POINT_LIST
        topoList.append(new LineElement(name, typeIn, tA, sort));
        break;
      case BaseMapElement::Village:
      // Maybe there is a problem because of the new field index for singlepoints
        if(formatID >= FILE_VERSION_MAP) in >> name;
        in >> lat_temp;
        in >> lon_temp;
        villageList.append(new SinglePoint(name, "", typeIn,
            WGSPoint(lat_temp, lon_temp),
            _globalMapMatrix.wgsToMap(lat_temp, lon_temp)));
        break;
      case BaseMapElement::Spot:
        if(formatID >= FILE_VERSION_MAP) in >> elev;
        in >> lat_temp;
        in >> lon_temp;
        obstacleList.append(new SinglePoint("Spot", "", typeIn,
            WGSPoint(lat_temp, lon_temp),
            _globalMapMatrix.wgsToMap(lat_temp, lon_temp), 0, index));
        break;
      case BaseMapElement::Landmark:
        if(formatID >= FILE_VERSION_MAP) {
          in >> lm_typ;
          in >> name;
        }
        in >> lat_temp;
        in >> lon_temp;
        landmarkList.append(new SinglePoint(name, "", typeIn,
          WGSPoint(lat_temp, lon_temp),
          _globalMapMatrix.wgsToMap(lat_temp, lon_temp),0,lm_typ));
        break;
    }
  }
  return true;
}

BaseFlightElement* MapContents::getFlight()
{
  // if list is empty, NULL will be returned
  return flightList.current();
}

QPtrList<BaseFlightElement>* MapContents::getFlightList()
{
  return &flightList;
}


void MapContents::appendFlight(Flight* flight)
{
  flightList.append(flight);

  emit newFlightAdded((Flight*)flightList.last());

  emit currentFlightChanged();
}

void MapContents::__askForDownload()
{
  extern QSettings _settings;
  int ret=0;

  switch (_settings.readNumEntry("/GeneralOptions/AutomaticMapDownload",ADT_NotSet))
    {
      case (ADT_NotSet):
        _settings.writeEntry("/GeneralOptions/AutomaticMapDownload",Inhibited); //this is temporary, will be overwritten later
        ret = QMessageBox::question(0, tr("Automatic map download"),
                  tr("<qt>There are no map-files in the directory<br><b>%1"
            "</b><br>yet. Do you want to download the data automatically?<br>"
            "(You need to have write permissions. If you want to change the directory, "
            "press \"Cancel\" and change it in the Settings menu.)</qt>").arg(mapDir + "/landscape"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        switch (ret)
          {
            case QMessageBox::Yes:
              _settings.writeEntry("/GeneralOptions/AutomaticMapDownload",Automatic); //this is temporary, will be overwritten later
              __downloadFile("G_03694.kfl",mapDir + "/landscape",true);
              if(QFile(mapDir+"/landscape/G_03694.kfl").exists())
                {
                  _settings.writeEntry("/GeneralOptions/AutomaticMapDownload",Automatic);
                }
              else
                {
                  QMessageBox::information(0, tr("No write access"),
                    tr("<qt>The directory <b>%1</b> is either not writeable<br>"
                    "or the server <b>%2</b> is not reachable.<br>"
                    "Please specify the correct path in the Settings dialog and check the internet connection!<br>"
                    " Restart KFLog afterwards.</qt>").arg(mapDir + "/landscape").arg (_settings.readEntry("/GeneralOptions/Mapserver","http://maproom.kflog.org:80/mapdata/data/landscape/")), QMessageBox::Ok);
                }
            break;
          case QMessageBox::No:
            _settings.writeEntry("/GeneralOptions/AutomaticMapDownload",Inhibited);
            break;
          }
        break;
      case (Inhibited):
        QMessageBox::information(0, tr("Directory empty"),
            tr("<qt>The directory for the map-files is empty.<br>"
                  "To download the files, please visit our homepage:<br>"
                  "<b>http://www.kflog.org/maproom/</b></qt>"), QMessageBox::Ok);
        break;
      case (Automatic):
        break;
    }
}

void MapContents::proofeSection(bool isPrint)
{
  extern MapMatrix _globalMapMatrix;
  extern QSettings _settings;
  QRect mapBorder;

  if(isPrint)
      mapBorder = _globalMapMatrix.getPrintBorder();
  else
      mapBorder = _globalMapMatrix.getViewBorder();

  int westCorner = ( ( mapBorder.left() / 600000 / 2 ) * 2 + 180 ) / 2;
  int eastCorner = ( ( mapBorder.right() / 600000 / 2 ) * 2 + 180 ) / 2;
  int northCorner = ( ( mapBorder.top() / 600000 / 2 ) * 2 - 88 ) / -2;
  int southCorner = ( ( mapBorder.bottom() / 600000 / 2 ) * 2 - 88 ) / -2;

  if(mapBorder.left() < 0)  westCorner -= 1;
  if(mapBorder.right() < 0)  eastCorner -= 1;
  if(mapBorder.top() < 0) northCorner += 1;
  if(mapBorder.bottom() < 0) southCorner += 1;

  KStandardDirs* globalDirs = KGlobal::dirs();
  mapDir = _settings.readEntry("/Path/DefaultMapDirectory", globalDirs->findResource("data", "kflog/mapdata/"));

  // Checking for the MapFiles
  if(mapDir.isNull() && !(isFirstLoad & MAP_LOADED))
    {
      /* The mapdirectory does not exist. Ask the user */
      QMessageBox::warning(0, tr("Directory not found"),
        "<qt>" +
        tr("The directory for the map-files does not exist.") + "<br>" +
        tr("Please select the directory in which the files are located.") +
        "</qt>", QMessageBox::Ok, 0);

      mapDir = QFileDialog::getExistingDirectory(QString::null, 0, 0, tr("Select map directory...") );

      _settings.writeEntry("/Path/DefaultMapDirectory", mapDir);

      isFirstLoad |= MAP_LOADED;

      if(QDir(mapDir + "/landscape").entryList("*.kfl").isEmpty())
          __askForDownload();

      emit errorOnMapLoading();
    }
  else if(QDir(mapDir + "/landscape").entryList("*.kfl").isEmpty())
    {
      emit errorOnMapLoading();
      __askForDownload();
    }
  else
    {
      emit loadingMessage(tr("Loading mapdata ..."));

      for(int row = northCorner; row <= southCorner; row++)
        {
          for(int col = westCorner; col <= eastCorner; col++)
            {
              if( !sectionArray.testBit( row + ( col + ( row * 179 ) ) ) )
                {
                  // Kachel fehlt!
                  int secID = row + ( col + ( row * 179 ) );

                  // Nun müssen die korrekten Dateien geladen werden ...
                  __readTerrainFile(secID, FILE_TYPE_GROUND);
                  __readTerrainFile(secID, FILE_TYPE_TERRAIN);
                  __readBinaryFile(secID, FILE_TYPE_MAP);
                  // Let's not plot all those circles on the map ...
                  // __readBinaryFile(secID, FILE_TYPE_LM);
                  sectionArray.setBit( secID, true );
                }
            }
        }
    }

  // Checking for Airspaces
  if (airspaceList.isEmpty()) {
    OpenAirParser oap;
    oap.load( airspaceList );
  }

  // Checking for Airfields
  if (airportList.isEmpty()) {
    Welt2000 welt2000;
    welt2000.load( airportList, gliderSiteList);
  }
}


unsigned int MapContents::getListLength(int listIndex) const
{
  switch(listIndex) {
  case AirportList:
    return airportList.count();
  case GliderSiteList:
    return gliderSiteList.count();
  case OutList:
    return outList.count();
  case NavList:
    return navList.count();
  case AirspaceList:
    return airspaceList.count();
  case ObstacleList:
    return obstacleList.count();
  case ReportList:
    return reportList.count();
  case CityList:
    return cityList.count();
  case VillageList:
    return villageList.count();
  case LandmarkList:
    return landmarkList.count();
  case RoadList:
    return roadList.count();
  case RailList:
    return railList.count();
  //case StationList:
  //  return stationList.count();
  case HydroList:
    return hydroList.count();
  case TopoList:
    return topoList.count();
  //    case IsohypseList:
  //      //warning("Anzahl der Höhenlinien: %d", isohypseList.count());
  //      return isohypseList.count();
  default:
    return 0;
  }
}


Airspace* MapContents::getAirspace(unsigned int index)
{
  return airspaceList.at(index);
}


Airport* MapContents::getAirport(unsigned int index)
{
  return airportList.at(index);
}


GliderSite* MapContents::getGlidersite(unsigned int index)
{
  return gliderSiteList.at(index);
}


BaseMapElement* MapContents::getElement(int listIndex, unsigned int index)
{
  switch(listIndex) {
  case AirportList:
    return airportList.at(index);
  case GliderSiteList:
    return gliderSiteList.at(index);
  case OutList:
    return outList.at(index);
  case NavList:
    return navList.at(index);
  case AirspaceList:
    return airspaceList.at(index);
  case ObstacleList:
    return obstacleList.at(index);
  case ReportList:
    return reportList.at(index);
  case CityList:
    return cityList.at(index);
  case VillageList:
    return villageList.at(index);
  case LandmarkList:
    return landmarkList.at(index);
  case RoadList:
    return roadList.at(index);
  case RailList:
    return railList.at(index);
  //case StationList:
  //  return stationList.at(index);
  case HydroList:
    return hydroList.at(index);
  case TopoList:
    return topoList.at(index);
  default:
    // Should never happen!
    fatal("KFLog: trying to access unknown map element list");
    return 0;
  }
}


SinglePoint* MapContents::getSinglePoint(int listIndex, unsigned int index)
{
  switch(listIndex) {
  case AirportList:
    return airportList.at(index);
  case GliderSiteList:
    return gliderSiteList.at(index);
  case OutList:
    return outList.at(index);
  case NavList:
    return navList.at(index);
  case ObstacleList:
    return obstacleList.at(index);
  case ReportList:
    return reportList.at(index);
  case VillageList:
    return villageList.at(index);
  case LandmarkList:
    return landmarkList.at(index);
  //case StationList:
  //  return stationList.at(index);
  default:
    return 0;
  }
}

void MapContents::slotDownloadFinished()
{
  qWarning("slotDownloadFinished()");
  slotReloadMapData();
}

void MapContents::slotReloadMapData()
{
  airportList.clear();
  gliderSiteList.clear();
  addSitesList.clear();
  outList.clear();
  navList.clear();
  airspaceList.clear();
  obstacleList.clear();
  reportList.clear();
  cityList.clear();
  villageList.clear();
  landmarkList.clear();
  roadList.clear();
  railList.clear();
//  stationList.clear();
  hydroList.clear();
  topoList.clear();
  isoList.clear();

  for(unsigned int loop = 0; loop < ISO_LINE_NUM; loop++)
      isoList.append(new QPtrList<Isohypse>);

  sectionArray.fill(false);
  emit contentsChanged();
}


void MapContents::printContents(QPainter* targetPainter, bool isText)
{
  proofeSection(true);

  for(BaseMapElement* topo = topoList.first(); topo; topo = topoList.next())
    topo->printMapElement(targetPainter, isText);

  for(BaseMapElement* hydro = hydroList.first(); hydro; hydro = hydroList.next())
    hydro->printMapElement(targetPainter, isText);

  for(BaseMapElement* rail = railList.first(); rail; rail = railList.next())
    rail->printMapElement(targetPainter, isText);

  for(BaseMapElement* road = roadList.first(); road; road = roadList.next())
    road->printMapElement(targetPainter, isText);

  for(BaseMapElement* city = cityList.first(); city; city = cityList.next())
    city->printMapElement(targetPainter, isText);

  for(BaseMapElement* village = villageList.first(); village; village = villageList.next())
    village->printMapElement(targetPainter, isText);

  for(BaseMapElement* nav = navList.first(); nav; nav = navList.next())
    nav->printMapElement(targetPainter, isText);

  for(BaseMapElement* airspace = airspaceList.first(); airspace; airspace = airspaceList.next())
    airspace->printMapElement(targetPainter, isText);

  for(BaseMapElement* obstacle = obstacleList.first(); obstacle; obstacle = obstacleList.next())
    obstacle->printMapElement(targetPainter, isText);

  for(BaseMapElement* report = reportList.first(); report; report = reportList.next())
    report->printMapElement(targetPainter, isText);

  for(BaseMapElement* landmark = landmarkList.first(); landmark; landmark = landmarkList.next())
    landmark->printMapElement(targetPainter, isText);

  for(BaseMapElement* airport = airportList.first(); airport; airport = airportList.next())
    airport->printMapElement(targetPainter, isText);

  for(BaseMapElement* glider = gliderSiteList.first(); glider; glider = gliderSiteList.next())
    glider->printMapElement(targetPainter, isText);

  for(BaseMapElement* out = outList.first(); out; out = outList.next())
    out->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < flightList.count(); loop++)
    flightList.at(loop)->printMapElement(targetPainter, isText);
}


void MapContents::drawList(QPainter* targetPainter, QPainter* maskPainter,
    unsigned int listID)
{
  switch(listID)
    {
      case AirportList:
        for(BaseMapElement* airport = airportList.first(); airport; airport = airportList.next())
            airport->drawMapElement(targetPainter, maskPainter);
        break;
      case GliderSiteList:
        for(BaseMapElement* glider = gliderSiteList.first(); glider; glider = gliderSiteList.next())
            glider->drawMapElement(targetPainter, maskPainter);
        break;
      case OutList:
        for(BaseMapElement* out = outList.first(); out; out = outList.next())
            out->drawMapElement(targetPainter, maskPainter);
        break;
        for(BaseMapElement* nav = navList.first(); nav; nav = navList.next())
            nav->drawMapElement(targetPainter, maskPainter);
        break;
      case AirspaceList:
        for(BaseMapElement* airspace = airspaceList.first(); airspace; airspace = airspaceList.next())
            airspace->drawMapElement(targetPainter, maskPainter);
        break;
      case ObstacleList:
        for(BaseMapElement* obstacle = obstacleList.first(); obstacle; obstacle = obstacleList.next())
            obstacle->drawMapElement(targetPainter, maskPainter);
        break;
      case ReportList:
        for(BaseMapElement* report = reportList.first(); report; report = reportList.next())
            report->drawMapElement(targetPainter, maskPainter);
        break;
      case CityList:
        for(BaseMapElement* city = cityList.first(); city; city = cityList.next())
            city->drawMapElement(targetPainter, maskPainter);
        break;
      case VillageList:
        for(BaseMapElement* village = villageList.first(); village; village = villageList.next())
            village->drawMapElement(targetPainter, maskPainter);
        break;
      case LandmarkList:
        for(BaseMapElement* landmark = landmarkList.first(); landmark; landmark = landmarkList.next())
            landmark->drawMapElement(targetPainter, maskPainter);
        break;
      case RoadList:
        for(BaseMapElement* road = roadList.first(); road; road = roadList.next())
            road->drawMapElement(targetPainter, maskPainter);
        break;
      case RailList:
        for(BaseMapElement* rail = railList.first(); rail; rail = railList.next())
            rail->drawMapElement(targetPainter, maskPainter);
        break;
      case HydroList:
        for(BaseMapElement* hydro = hydroList.first(); hydro; hydro = hydroList.next())
            hydro->drawMapElement(targetPainter, maskPainter);
        break;
      case TopoList:
        for(BaseMapElement* topo = topoList.first(); topo; topo = topoList.next())
            topo->drawMapElement(targetPainter, maskPainter);
        break;
      case FlightList:
        // In some cases, getFlightIndex returns a non-valid index :-(
        if (flightList.count() > 0 && getFlightIndex() >= 0 &&
              getFlightIndex() < (int)flightList.count())
            flightList.at(getFlightIndex())->drawMapElement(targetPainter, maskPainter);
        break;
      default:
        return;
    }
}

void MapContents::drawIsoList(QPainter* targetP, QPainter* maskP)
{
  int height = 0;
  bool useIsohypseElevationList = ElevationFinder::instance()->useIsohypseForElevation();

  extern MapConfig _globalMapConfig;

  regIsoLines.clear();

  for(QPtrList<Isohypse>* iso = isoList.first(); iso; iso = isoList.next())
    {
      if(iso->count() == 0) continue;

      for(unsigned int pos = 0; pos < ISO_LINE_NUM; pos++)
        {
          if(isoLines[pos] == iso->getFirst()->getElevation())
            {
              if(iso->getFirst()->isValley())
                  height = pos + 1;
              else
                  height = pos + 2;

              break;
            }
        }

//      targetP->setPen(QPen(_globalMapConfig.getIsoPenColor(), 1,
//        _globalMapConfig.getIsoPenStyle(iso->getFirst()->getElevation())));  // make configurable

//      targetP->setPen(QPen(_globalMapConfig.getIsoColor(height), 1, Qt::NoPen));
      targetP->setPen(QPen(_globalMapConfig.getIsoColor(height), 1, Qt::SolidLine));
      targetP->setBrush(QBrush(_globalMapConfig.getIsoColor(height),
          QBrush::SolidPattern));

      for(Isohypse* iso2 = iso->first(); iso2; iso2 = iso->next())
        {
          QRegion * reg = iso2->drawRegion(targetP, maskP);
          if(reg)
            {
              isoListEntry* entry=new isoListEntry(reg, height);
              regIsoLines.append(entry);
            }
          if( !(iso2 -> regionStored) && useIsohypseElevationList)
            {
              iso2->regionStored = true;
              // AS: There is a problem with creating regions with large coordinates, or so it seems.
              isoListEntry* entry=new isoListEntry( iso2->getRegion(), height );
              regIsoLinesWorld.append( entry );
            }
        }
    }
}

void MapContents::addDir (QStringList& list, const QString& _path, const QString& filter)
{
  //  qDebug ("addDir (%s, %s)", _path.toLatin1().data(), filter.toLatin1().data());
  QDir path (_path, filter);

  //JD was a bit annoyed by many notifications about nonexisting dirs
  if ( ! path.exists() )
    return;

  QStringList entries (path.entryList());

  for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) {
    bool found = false;
    // look for other entries with same filename
    for (QStringList::Iterator it2 =  list.begin(); it2 != list.end(); ++it2) {
      QFileInfo path2 (*it2);
      if (path2.fileName() == *it)
        found = true;
    }
    if (!found)
      list += path.absFilePath (*it);
  }
  //  qDebug ("entries: %s", list.join(";").toLatin1().data());
}

/**
 * Compares two projection objects for equality.
 * @Returns true if equal; otherwise false
 */
bool MapContents::compareProjections(ProjectionBase* p1, ProjectionBase* p2)
{
  if( p1->projectionType() != p2->projectionType() ) {
    return false;
  }

  if( p1->projectionType() == ProjectionBase::Lambert ) {
    ProjectionLambert* l1 = (ProjectionLambert *) p1;
    ProjectionLambert* l2 = (ProjectionLambert *) p2;

    if( l1->getStandardParallel1() != l2->getStandardParallel1() ||
        l1->getStandardParallel2() != l2->getStandardParallel2() ||
        l1->getOrigin() != l2->getOrigin() ) {
      return false;
    }

    return true;
  }

  if( p1->projectionType() == ProjectionBase::Cylindric ) {
    ProjectionCylindric* c1 = (ProjectionCylindric*) p1;
    ProjectionCylindric* c2 = (ProjectionCylindric*) p2;

    if( c1->getStandardParallel() != c2->getStandardParallel() ) {
      return false;
    }

    return true;
  }

  // What's that? Det kennen wir noch nicht :( Rejection!

  return false;
}
/** create a new, empty task */
void MapContents::slotNewTask()
{
  FlightTask *f = new FlightTask(genTaskName());
  flightList.append(f);
  emit newTaskAdded(f);

  QString helpText = "";

  helpText = tr(
                  "You can select waypoints with the left mouse button."
                  "You can also select free waypoints by clicking anywhere in the map."
                  "<br><br>"
                  "When you press &lt;CTRL&gt; and click with the left mouse button on a taskpoint, "
                  "it will be deleted.<br>"
                  "You can compute the task up to your current mouse position by pressing &lt;SHIFT&gt;."
                  "<br>"
                  "To finish the task, press &lt;CTRL&gt; and click the right mouse button.<br>"
                  "It's possible to move and delete taskpoints from the finished task."
                  );

  emit taskHelp(helpText);

  emit currentFlightChanged();
//  emit activatePlanning();
}

void MapContents::slotAppendTask(FlightTask *f)
{
  flightList.append(f);
  emit newTaskAdded(f);
}

/** create a new, empty flight group */
void MapContents::slotNewFlightGroup()
{
  static int gCount = 1;
  QPtrList <Flight> fl;
  BaseFlightElement *f;
  unsigned int i;
  QString tmp;

  FlightSelectionDialog *fsd = new FlightSelectionDialog(0, "flight selection dialog");

  for (i = 0; i < flightList.count(); i++)
    {
      f = flightList.at(i);
      if (f->getTypeID() == BaseMapElement::Flight)
        {
          fsd->availableFlights.append(f);
        }
    }

  if (fsd->exec() == QDialog::Accepted)
    {
      for (i = 0; i < fsd->selectedFlights.count(); i++)
        {
          fl.append((Flight *)fsd->selectedFlights.at(i));
        }

      tmp.sprintf("GROUP%03d", gCount++);

      FlightGroup * flightGroup = new FlightGroup(fl, tmp);
      flightList.append(flightGroup);
      emit newFlightGroupAdded(flightGroup);
      emit currentFlightChanged();
    }
  delete fsd;
}
/** No descriptions */
void MapContents::slotSetFlight(int id)
{
  if (id >= 0 && id < (int)flightList.count())
    {
      flightList.at(id);
      emit currentFlightChanged();
    }
}

void MapContents::slotSetFlight(BaseFlightElement *f)
{
  if (flightList.containsRef(f))
    {
      flightList.findRef(f);
      emit currentFlightChanged();
    }
}

/** No descriptions */
void MapContents::slotEditFlightGroup()
{
  QPtrList <Flight> fl;
  BaseFlightElement *f;
  BaseFlightElement *fg;
  unsigned int i;
  QString tmp;

  FlightSelectionDialog *fsd = new FlightSelectionDialog(0, "flight selection dialog");
  fg = getFlight();

  if (fg->getTypeID() == BaseMapElement::FlightGroup)
    {
      fl = ((FlightGroup *)fg)->getFlightList();
      for (i = 0; i < flightList.count(); i++)
        {
          f = flightList.at(i);
          if (f->getTypeID() == BaseMapElement::Flight)
            {
              if (fl.containsRef((Flight *)f))
                {
                  fsd->selectedFlights.append(f);
                }
              else
                {
                  fsd->availableFlights.append(f);
                }
            }
        }

      if (fsd->exec() == QDialog::Accepted)
        {
          fl.clear();
          for (i = 0; i < fsd->selectedFlights.count(); i++)
            {
              fl.append((Flight *)fsd->selectedFlights.at(i));
            }
          ((FlightGroup *)fg)->setFlightList(fl);
        }
    }

  emit currentFlightChanged();
  delete fsd;
}

/** read a task file and append all tasks to flight list
switch to first task in file */
bool MapContents::loadTask(QFile& path)
{
  QFileInfo fInfo(path);

  extern const MapMatrix _globalMapMatrix;

  if(!fInfo.exists())
    {
      QMessageBox::warning(0, tr("File does not exist"), "<qt>" + tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(path.name()) + "</qt>", QMessageBox::Ok, 0);
      return false;
    }

  if(!fInfo.size())
    {
      QMessageBox::warning(0, tr("File is empty"), "<qt>" + tr("The selected file<BR><B>%1</B><BR>is empty!").arg(path.name()) + "</qt>", QMessageBox::Ok, 0);
      return false;
    }

  if(!path.open(IO_ReadOnly))
    {
      QMessageBox::warning(0, tr("No permission"), "<qt>" + tr("You don't have permission to access file<BR><B>%1</B>").arg(path.name()) + "</qt>", QMessageBox::Ok, 0);
      return false;
    }

  QDomDocument doc;
  QPtrList<Waypoint> wpList;
  FlightTask *f, *firstTask = 0;

  doc.setContent(&path);

  if (doc.doctype().name() == "KFLogTask")
    {
      QDomNodeList nl = doc.elementsByTagName("Task");

      for (uint i = 0; i < nl.count(); i++)
        {
          QDomNodeList childNodes = nl.item(i).childNodes();
          QDomNamedNodeMap nmTask =  nl.item(i).attributes();

          wpList.clear();
          for (uint childIdx = 0; childIdx < childNodes.count(); childIdx++)
            {
              QDomNamedNodeMap nm =  childNodes.item(childIdx).attributes();

              Waypoint *w = new Waypoint;

              w->name = nm.namedItem("Name").toAttr().value().left(6).upper();
              w->description = nm.namedItem("Description").toAttr().value();
              w->icao = nm.namedItem("ICAO").toAttr().value().upper();
              w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
              w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
              w->projP = _globalMapMatrix.wgsToMap(w->origP);
              w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
              w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
              w->isLandable = nm.namedItem("Landable").toAttr().value().toInt();
              w->runway = nm.namedItem("Runway").toAttr().value().toInt();
              w->length = nm.namedItem("Length").toAttr().value().toInt();
              w->surface = nm.namedItem("Surface").toAttr().value().toInt();
              w->comment = nm.namedItem("Comment").toAttr().value();

              if (w->runway == 0 && w->length == 0)
                  w->runway = w->length = -1;

              wpList.append(w);
            }

          f = new FlightTask(wpList, false, genTaskName());
          f->setPlanningType(nmTask.namedItem("PlanningType").toAttr().value().toInt());
          f->setPlanningDirection(nmTask.namedItem("PlanningDirection").toAttr().value().toInt());
          // remember first task in file
          if (firstTask == 0)
              firstTask = f;
          flightList.append(f);
          emit newTaskAdded(f);
        }

      if (firstTask)
          slotSetFlight(firstTask);

      return true;
    }
  else
    {
      QMessageBox::warning(0, tr("Error occurred!"), tr("wrong doctype ") + doc.doctype().name(), QMessageBox::Ok, 0);
      return false;
    }
}

QString MapContents::genTaskName()
{
  static int tCount = 1;
  QString tmp;

  tmp.sprintf("TASK%03d", tCount++);
  return tmp;
}

QString MapContents::genTaskName(QString suggestion)
{
  BaseFlightElement* bfe;
  FlightTask* ft=0;
  for (bfe = flightList.first(); bfe; bfe = flightList.next()) {
    ft=dynamic_cast<FlightTask*>(bfe);
    if (ft) {
      if (ft->getName() == suggestion) {
        //name is allready in use
        return QString("%1 (%2)").arg(suggestion).arg(genTaskName());
      }
    }
  }
  return suggestion;
}


/***********************************************
 * isoListEntry
 ***********************************************/

isoListEntry::isoListEntry(QRegion* region, int height)
{
  this->region=region;
  this->height=height;
}

isoListEntry::~isoListEntry()
{
  if(region) delete region;
}

/** Re-projects any flights and tasks that may be loaded. */
void MapContents::reProject()
{
  QPtrListIterator<BaseFlightElement> it(flightList); // iterator for flightlist

  for ( ; it.current(); ++it )
    {
      BaseFlightElement *fe = it.current();
      fe->reProject();
    }
}

/*!
    \fn MapContents::getElevation(QPoint)
 */
int MapContents::getElevation(QPoint coord)
{
//   extern MapConfig _globalMapConfig;

  isoListEntry* entry;
  int height=-1; //default 'unknown' value

  for(unsigned int i = 0; i < regIsoLinesWorld.count(); i++) {
    entry = regIsoLinesWorld.at(i);
    if (entry->region->contains(coord))
      height=std::max(height,entry->height);
  }
  if (height == -1)
    return height;
  return topoLevels[height];
}
