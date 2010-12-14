/**********************************************************************
 **
 **   mapcontents.cpp
 **
 **   This file is part of KFLog4.
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

#include <QtGui>
#include <QtXml>

#include "airport.h"
#include "airspace.h"
#include "basemapelement.h"
#include "elevationfinder.h"
#include "flight.h"
#include "flightgroup.h"
#include "flightselectiondialog.h"
#include "glidersite.h"
#include "isohypse.h"
#include "lineelement.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "mapcalc.h"
#include "openairparser.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "welt2000.h"

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
      tA.setPoint(i, _globalMapMatrix->wgsToMap(lat_temp, lon_temp)); \
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

MapContents::MapContents( QObject* object ) :
  QObject(object),
  isFirstLoad(true)
{
  qDebug() << "MapContents()";

  currentFlight  = 0;
  downloadManger = 0;

  // Setup a hash used as reverse mapping from isoLine value to array index to
  // speed up loading of ground and terrain files.
  for ( int i = 0; i < ISO_LINE_NUM; i++ )
    {
      isoHash.insert( std::pair<int, int>(isoLines[i],i) );
    }

  sectionArray.resize(MAX_TILE_NUMBER);
  sectionArray.fill(false);

  for( int loop = 0; loop < ISO_LINE_NUM; loop++ )
    {
      isoList.append(new QList<Isohypse*>);
    }

  // Create all needed map directories.
  createMapDirectories();

  // The user is asked once after each startup, if he wants to download
  // missing map files.
  extern QSettings _settings;

 _settings.setValue("/Internet/AutomaticMapDownload", ADT_NotSet);
}

MapContents::~MapContents()
{
  qDebug() << "~MapContents()";

  while(!addSitesList.empty())
        delete addSitesList.takeFirst();
  while(!airportList.empty())
      delete airportList.takeFirst();
  while(!airspaceList.empty())
      delete airspaceList.takeFirst();
  while(!cityList.empty())
      delete cityList.takeFirst();
//  while(!downloadList.empty())
//      delete downloadList.takeFirst();
  while(!gliderfieldList.empty())
      delete gliderfieldList.takeFirst();
  while(!flightList.empty())
      delete flightList.takeFirst();
  while(!hydroList.empty())
      delete hydroList.takeFirst();
  while(!isoList.empty())
      delete isoList.takeFirst();
  while(!landmarkList.empty())
      delete landmarkList.takeFirst();
  while(!navList.empty())
      delete navList.takeFirst();
  while(!obstacleList.empty())
      delete obstacleList.takeFirst();
  while(!outLandingList.empty())
      delete outLandingList.takeFirst();
  while(!railList.empty())
      delete railList.takeFirst();
  while(!regIsoLines.empty())
      delete regIsoLines.takeFirst();
  while(!regIsoLinesWorld.empty())
      delete regIsoLinesWorld.takeFirst();
  while(!reportList.empty())
      delete reportList.takeFirst();
  while(!roadList.empty())
      delete roadList.takeFirst();
//  while(!stationList.empty())
//      delete stationList.takeFirst();
  while(!topoList.empty())
      delete topoList.takeFirst();
  while(!villageList.empty())
      delete villageList.takeFirst();
  while(!wpList.empty())
      delete wpList.takeFirst();
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

  if(number.indexIn(inDegree) != -1)
      return inDegree.toInt();
  else if(degree.indexIn(inDegree) != -1)
    {
      int deg = 0, min = 0, sec = 0, result = 0;

      QRegExp deg1("°");
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg3("\"");
      sec = inDegree.mid(0, deg3.indexIn(inDegree)).toInt();

      result = (int)((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));

      // We add 1 to avoid rounding-errors ...
      result += 1;

      QRegExp dir("[swSW]$");
      if(dir.indexIn(inDegree) >= 0) return -result;

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
  if(currentFlight != 0)
    {
      emit closingFlight(currentFlight);
      for (int i = 0; i < flightList.count(); i++)
        {
          FlightGroup *fg = (FlightGroup *) flightList.at(i);
          if(fg->getTypeID() == BaseMapElement::FlightGroup)
              fg->removeFlight(currentFlight);

        }

      int i = flightList.indexOf(currentFlight);
      if(i != -1)
        delete flightList.takeAt(i);

      currentFlight = flightList.last();
      currentFlightListIndex = flightList.indexOf(currentFlight);

      emit currentFlightChanged();
    }
}


/**
 * Try to download a missing ground/terrain/map file.
 *
 * @param file The name of the file without any path prefixes.
 * @param directory The destination directory.
 *
 */
bool MapContents::__downloadMapFile( QString &file, QString &directory )
{
  extern QSettings _settings;

  if( _settings.value( "/Internet/AutomaticMapDownload", ADT_NotSet ).toInt() == Inhibited )
    {
      qDebug() << "Auto Download Inhibited";
      return false;
    }

  if( downloadManger == static_cast<DownloadManager *> (0) )
    {
      downloadManger = new DownloadManager(this);

      connect( downloadManger, SIGNAL(finished( int, int )),
               this, SLOT(slotDownloadsFinished( int, int )) );

      connect( downloadManger, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );
    }

  QString srvUrl = _settings.value( "/MapData/MapServer",
                                    "http://www.kflog.org/data/landscape/" ).toString();

  _settings.setValue( "/MapData/MapServer", srvUrl );

  QString url = srvUrl + file;
  QString dest = directory + "/" + file;

  downloadManger->downloadRequest( url, dest );
  return true;
}

/** Called, if all downloads are finished. */
void MapContents::slotDownloadsFinished( int requests, int errors )
{
  extern MainWindow *_mainWindow;

  // All has finished, free not more needed resources
  downloadManger->deleteLater();
  downloadManger = static_cast<DownloadManager *> (0);

  // initiate a new load
  slotReloadMapData();

  QString msg;
  msg = QString(tr("%1 download(s) with %2 error(s) done.")).arg(requests).arg(errors);

  QMessageBox::information( _mainWindow,
                            tr("Downloads finished"),
                            msg );
}

/** Called, if a network error occurred during the downloads. */
void MapContents::slotNetworkError()
{
  extern MainWindow *_mainWindow;

  // A network error has occurred. We do stop all further downloads.
  downloadManger->deleteLater();
  downloadManger = static_cast<DownloadManager *> (0);

  QString msg;
  msg = QString(tr("Network error occurred.\nAll downloads are canceled!"));

  QMessageBox::information( _mainWindow,
                            tr("Network Error"),
                            msg );
}

/**
 * This slot is called to download the Welt2000 file from the internet.
 */
void MapContents::slotDownloadWelt2000()
{
  qDebug() << "MapContents::slotDownloadWelt2000()";

  extern QSettings _settings;

  if( __askUserForDownload() != Automatic )
    {
      qDebug() << "Welt2000: Auto Download Inhibited";
      return;
    }

  if( downloadManger == static_cast<DownloadManager *> (0) )
    {
      downloadManger = new DownloadManager(this);

      connect( downloadManger, SIGNAL(finished( int, int )),
               this, SLOT(slotDownloadsFinished( int, int )) );

      connect( downloadManger, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );
    }

  connect( downloadManger, SIGNAL(welt2000Downloaded()),
           this, SLOT(slotReloadWelt2000Data()) );

  QString welt2000FileName = _settings.value( "/MapData/Welt2000FileName", "WELT2000.TXT").toString();
  QString welt2000Link     = _settings.value( "/MapData/Welt2000Link", "http://www.segelflug.de/vereine/welt2000/download").toString();

  _settings.setValue( "/MapData/Welt2000FileName", welt2000FileName );
  _settings.setValue( "/MapData/Welt2000Link", welt2000Link );

  QString url  = welt2000Link + "/" + welt2000FileName;
  QString dest = getMapRootDirectory() + "/airfields/welt2000.txt";

  qDebug() << "URL=" << url;
  qDebug() << "Dest=" << dest;

  downloadManger->downloadRequest( url, dest );
}

/**
 * Reload Welt2000 data file. Can be called after a configuration change or
 * a dowonload.
 */
void MapContents::slotReloadWelt2000Data()
{
  //airportList.clear();
  //gliderfieldList.clear();
  qDeleteAll(airportList);
  qDeleteAll(gliderfieldList);

  qDebug() << "MapContents: Reloading Welt2000 started";

  Welt2000 welt2000;

  welt2000.load( airportList, gliderfieldList );

  qDebug() << "MapContents: Reloading Welt2000 finished";

  emit contentsChanged();
}

bool MapContents::__readTerrainFile( const int fileSecID,
                                     const int fileTypeID)
{
  extern const MapMatrix *_globalMapMatrix;

  QString path, file;

  path = getMapRootDirectory() + "/landscape/";

  file.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);

  QString pathName = path + file;

  QFile mapfile(pathName);

  if(!mapfile.open(QIODevice::ReadOnly))
    {
      qWarning() << "KFLog: Can not open terrain file" << pathName;

      int answer = __askUserForDownload();

      if( answer == Automatic )
        {
          __downloadMapFile( file, path );
        }
      else
        {
          qDebug() << "No Auto Download";
        }

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
    qWarning("KFLog: Trying to open old or invalid map-file; aborting!");
    qWarning("%s", (const char*)pathName);
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
    qWarning("KFLog: File format too old! (version %d, expecting: %d)",
        formatID, expFormatID );
    return false;
  } else if (formatID > expFormatID) {
    qWarning("KFLog: File format too new! (version %d, expecting: %d)",
        formatID, expFormatID );
    return false;
  }

#warning "Remove fallback terrain solution from code"

  if (formatID == expOldFormatID)
    {   // this is for old terrain and ground files
      qWarning("KFLog: You are using old map files. Please consider re-installing\n"
              "       the terrain and ground files. Support for the old file format\n"
              "       will cease in the next major KFLog release.");
    }

  in >> loadSecID;
  if(loadSecID != fileSecID) {
    // wrong tile number.
    return false;
  }

  in >> createDateTime;

  qDebug("Reading File=%s, Magic=0x%x, TypeId=%c, formatId=%d, Date=%s",
         file.toLatin1().data(), magic, loadTypeID, formatID,
         createDateTime.toString(Qt::ISODate).toLatin1().data() );

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

      Q3PointArray tA(locLength);

      for(int i = 0; i < locLength; i++) {
        in >> latList_temp;
        in >> lonList_temp;

        tA.setPoint(i, _globalMapMatrix->wgsToMap(latList_temp, lonList_temp));
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
  extern const MapMatrix *_globalMapMatrix;

  QString path, file;

  path = getMapRootDirectory() + "/landscape/";

  file.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);

  QString pathName = path + file;

  QFile mapfile(pathName);

  if(!mapfile.open(QIODevice::ReadOnly))
    {
      qWarning() << "KFLog: Can not open map file" << pathName;

      int answer = __askUserForDownload();

      if( answer == Automatic )
        {
          __downloadMapFile( file, path );
        }

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
    qWarning("KFLog: File format too old! (version %d, expecting: %d)",
        formatID, FILE_VERSION_MAP );
    return false;
  } else if(formatID > FILE_VERSION_MAP) {
    qWarning("KFLog: File format too new! (version %d, expecting: %d)",
        formatID, FILE_VERSION_MAP );
    return false;
  }

  in >> loadSecID;
  if(loadSecID != fileSecID)  return false;

  in >> createDateTime;

  unsigned int gesamt_elemente = 0;

  qDebug("Reading File=%s, Magic=0x%x, TypeId=%c, formatId=%d, Date=%s",
         file.toLatin1().data(), magic, loadTypeID, formatID,
         createDateTime.toString(Qt::ISODate).toLatin1().data() );

  while(!in.eof()) {
    in >> typeIn;
    locLength = 0;
    name = "";

    Q3PointArray tA;

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
            _globalMapMatrix->wgsToMap(lat_temp, lon_temp)));
        break;
      case BaseMapElement::Spot:
        if(formatID >= FILE_VERSION_MAP) in >> elev;
        in >> lat_temp;
        in >> lon_temp;
        obstacleList.append(new SinglePoint("Spot", "", typeIn,
            WGSPoint(lat_temp, lon_temp),
            _globalMapMatrix->wgsToMap(lat_temp, lon_temp), 0, index));
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
          _globalMapMatrix->wgsToMap(lat_temp, lon_temp),0,lm_typ));
        break;
    }
  }
  return true;
}

BaseFlightElement* MapContents::getFlight()
{
  // if list is empty, NULL will be returned
  return currentFlight;
}

QList<BaseFlightElement*> *MapContents::getFlightList()
{
  return &flightList;
}


void MapContents::appendFlight(Flight* flight)
{
  flightList.append(flight);
  currentFlight = flight;
  currentFlightListIndex = flightList.indexOf(flightList.last());

  emit newFlightAdded((Flight*)flightList.last());

  emit currentFlightChanged();
}

int MapContents::__askUserForDownload()
{
  qDebug() << "MapContents::__askUserForDownload()";

  extern MainWindow *_mainWindow;
  extern QSettings  _settings;

  int result = _settings.readNumEntry( "/Internet/AutomaticMapDownload", ADT_NotSet );

  if( isFirstLoad == true && result == ADT_NotSet )
    {
      _settings.setValue("/Internet/AutomaticMapDownload", Inhibited);

      int ret = QMessageBox::question(_mainWindow, tr("Automatic data download?"),
                tr("<html>There are data missing under the directory tree<br><b>%1."
                "</b><br> Do you want to download these data automatically?<br>"
                "(If you want to change the root directory, "
                "press <i>Cancel</i> and change it in the Settings menu.)</html>")
                 .arg( getMapRootDirectory() ),
                 QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );

      switch (ret)
        {
          case QMessageBox::Yes:

            _settings.setValue("/Internet/AutomaticMapDownload", Automatic);
            result = Automatic;
            isFirstLoad = false;

            break;

          case QMessageBox::No:

            _settings.setValue( "/Internet/AutomaticMapDownload", Inhibited );
            result = Inhibited;
            isFirstLoad = false;
            break;

          case QMessageBox::Cancel:

            _settings.setValue( "/Internet/AutomaticMapDownload", ADT_NotSet );
            result = ADT_NotSet;
            isFirstLoad = true;
            break;
        }
    }

  return result;
}


/**
 * Checks the existence of the three required map directories.
 *
 * \return True if all is okay otherwise false.
 */
bool MapContents::checkMapDirectories()
{
  QString mapRootDir = getMapRootDirectory();

  QStringList list;

  list << "airspaces" << "airfields" << "landscape";

  for( int i = 0; i < list.size(); i++ )
    {
      QDir dir( mapRootDir + "/" + list.at(i) );

      if( ! dir.exists() || ! dir.isReadable() )
        {
          qWarning() << "Map directory:" << dir.absolutePath() << "does not exist!";
          return false;
        }

      if( ! dir.isReadable() )
        {
          qWarning() << "Map directory:" << dir.absolutePath() << "not readable!";
          return false;
        }
    }

  return true;
}

/**
 * Creates all required map directories.
 *
 * \return True if all is okay otherwise false.
 */
bool MapContents::createMapDirectories()
{
  qDebug() << "MapContents::createMapDirectories()";

  QString mapRootDir = getMapRootDirectory();

  QStringList list;

  list << "airspaces" << "airfields" << "landscape";

  for( int i = 0; i < list.size(); i++ )
    {
      QString path = mapRootDir + "/" + list.at(i);

      QDir dir( path );

      if( ! dir.exists() )
        {
          dir.mkpath( path );

          if( ! dir.exists() )
            {
              qWarning() << "MapContents: Cannot create Map directory:" << path;
              return false;
            }
        }

      if( ! dir.isReadable() )
        {
          qWarning() << "MapContents: Map directory:" << path << "not readable!";
          return false;
        }
    }

  return true;
}

/**
 * Returns the map root directory.
 *
 * \return The map root directory.
 */
QString MapContents::getMapRootDirectory()
{
  extern QSettings _settings;

  QString mapDefRootDir = QDir::homePath() + "/.kflog/mapdata";

  // qDebug() << "MapContents: mapDefRootDir:" << mapDefRootDir;

  QString mapRootDir = _settings.value( "/Path/DefaultMapDirectory", "" ).toString();

  if( mapRootDir.isEmpty() )
    {
      qWarning() << "Settings item /Path/DefaultMapDirectory is empty!";
      mapRootDir = mapDefRootDir;

      _settings.setValue( "/Path/DefaultMapDirectory", mapRootDir );
    }

  return mapRootDir;
}


void MapContents::proofeSection(bool isPrint)
{
  qDebug() << "MapContents::proofeSection() isPrint=" << isPrint;

  extern MainWindow *_mainWindow;
  extern MapMatrix  *_globalMapMatrix;
  extern QSettings  _settings;
  QRect mapBorder;

  if(isPrint)
      mapBorder = _globalMapMatrix->getPrintBorder();
  else
      mapBorder = _globalMapMatrix->getViewBorder();

  int westCorner = ( ( mapBorder.left() / 600000 / 2 ) * 2 + 180 ) / 2;
  int eastCorner = ( ( mapBorder.right() / 600000 / 2 ) * 2 + 180 ) / 2;
  int northCorner = ( ( mapBorder.top() / 600000 / 2 ) * 2 - 88 ) / -2;
  int southCorner = ( ( mapBorder.bottom() / 600000 / 2 ) * 2 - 88 ) / -2;

  if(mapBorder.left() < 0)  westCorner -= 1;
  if(mapBorder.right() < 0)  eastCorner -= 1;
  if(mapBorder.top() < 0) northCorner += 1;
  if(mapBorder.bottom() < 0) southCorner += 1;

  if( ! checkMapDirectories() )
    {
      /* The map directory does not exist. Ask the user */
      QMessageBox::warning(_mainWindow, tr("Map directories not found"),
        "<html>" +
        tr("The directories for the map files do not exist.") + "<br>" +
        tr("Please select the map root directory.") +
        "</html>", QMessageBox::Ok );

      mapDir = QFileDialog::getExistingDirectory( _mainWindow,
                                                  tr("Select a map root directory!"),
                                                  QDir::homePath() );

      if(  mapDir.isEmpty() )
        {
          qWarning() << "MapContents::proofeSection(): File Dialog returned NIL!";
          return;
        }

      _settings.setValue( "/Path/DefaultMapDirectory", mapDir );

    }

  emit loadingMessage(tr("Loading map data ..."));

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

  // Checking for Airspaces
  if( airspaceList.isEmpty() )
    {
      OpenAirParser oap;
      oap.load( airspaceList );
    }

  // Checking for Airfield, Gliderfield and Outlanding data
  if( airportList.isEmpty() && gliderfieldList.isEmpty() )
    {
      Welt2000 welt2000;

      if( !welt2000.load( airportList, gliderfieldList ) )
        {
          // Welt2000 load failed, try to download a new Welt2000 File
          // from the Internet web page.
          slotDownloadWelt2000();
        }
    }
}

int MapContents::getListLength(int listIndex) const
{
  switch(listIndex) {
  case AirportList:
    return airportList.count();
  case GliderfieldList:
    return gliderfieldList.count();
  case OutLandingList:
    return outLandingList.count();
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
  case HydroList:
    return hydroList.count();
  case TopoList:
    return topoList.count();
  default:
    return 0;
  }
}


Airspace* MapContents::getAirspace(unsigned int index)
{
  return airspaceList.value(index);
}


Airport* MapContents::getAirport(unsigned int index)
{
  return airportList.value(index);
}


GliderSite* MapContents::getGlidersite(unsigned int index)
{
  return gliderfieldList.value(index);
}


BaseMapElement* MapContents::getElement(int listIndex, unsigned int index)
{
  switch(listIndex) {
  case AirportList:
    return airportList.value(index);
  case GliderfieldList:
    return gliderfieldList.value(index);
  case OutLandingList:
    return outLandingList.value(index);
  case NavList:
    return navList.value(index);
  case AirspaceList:
    return airspaceList.value(index);
  case ObstacleList:
    return obstacleList.value(index);
  case ReportList:
    return reportList.value(index);
  case CityList:
    return cityList.value(index);
  case VillageList:
    return villageList.value(index);
  case LandmarkList:
    return landmarkList.value(index);
  case RoadList:
    return roadList.value(index);
  case RailList:
    return railList.value(index);
  case HydroList:
    return hydroList.value(index);
  case TopoList:
    return topoList.value(index);
  default:
    // Should never happen!
    qWarning("KFLog: trying to access unknown map element list");
    return 0;
  }
}


SinglePoint* MapContents::getSinglePoint(int listIndex, unsigned int index)
{
  switch(listIndex) {
  case AirportList:
    return airportList.value(index);
  case GliderfieldList:
    return gliderfieldList.value(index);
  case OutLandingList:
    return outLandingList.value(index);
  case NavList:
    return navList.value(index);
  case ObstacleList:
    return obstacleList.value(index);
  case ReportList:
    return reportList.value(index);
  case VillageList:
    return villageList.value(index);
  case LandmarkList:
    return landmarkList.value(index);
  default:
    return 0;
  }
}


void MapContents::slotReloadMapData()
{
  qDebug() << "MapContents::slotReloadMapData()";

  airportList.clear();
  gliderfieldList.clear();
  addSitesList.clear();
  outLandingList.clear();
  navList.clear();
  airspaceList.clear();
  obstacleList.clear();
  reportList.clear();
  cityList.clear();
  villageList.clear();
  landmarkList.clear();
  roadList.clear();
  railList.clear();
  hydroList.clear();
  topoList.clear();
  isoList.clear();

  for(int loop = 0; loop < ISO_LINE_NUM; loop++)
    {
      isoList.append(new QList<Isohypse*>);
    }

  sectionArray.fill(false);
  emit contentsChanged();
}


void MapContents::printContents(QPainter* targetPainter, bool isText)
{
  proofeSection(true);

  BaseMapElement *element;
  foreach(element, topoList)
    element->printMapElement(targetPainter, isText);

  foreach(element, hydroList)
    element->printMapElement(targetPainter, isText);

  foreach(element, railList)
    element->printMapElement(targetPainter, isText);

  foreach(element, cityList)
    element->printMapElement(targetPainter, isText);

  foreach(element, villageList)
    element->printMapElement(targetPainter, isText);

  foreach(element, navList)
    element->printMapElement(targetPainter, isText);

  foreach(element, airspaceList)
    element->printMapElement(targetPainter, isText);

  foreach(element, obstacleList)
    element->printMapElement(targetPainter, isText);

  foreach(element, reportList)
    element->printMapElement(targetPainter, isText);

  foreach(element, landmarkList)
    element->printMapElement(targetPainter, isText);

  foreach(element, airportList)
    element->printMapElement(targetPainter, isText);

  foreach(element, gliderfieldList)
    element->printMapElement(targetPainter, isText);

  foreach(element, outLandingList)
    element->printMapElement(targetPainter, isText);

  foreach(element, flightList)
    element->printMapElement(targetPainter, isText);
}


void MapContents::drawList( QPainter* targetPainter,
                            QPainter* maskPainter,
                            unsigned int listID )
{
  BaseMapElement *element;

  switch(listID)
    {
      case AirportList:
        foreach(element, airportList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case GliderfieldList:
        foreach(element, gliderfieldList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case OutLandingList:
        foreach(element, outLandingList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case NavList:
        foreach(element, navList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case AirspaceList:
        foreach(element, airspaceList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case ObstacleList:
        foreach(element, obstacleList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case ReportList:
        foreach(element, reportList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case CityList:
        foreach(element, cityList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case VillageList:
        foreach(element, villageList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case LandmarkList:
        foreach(element, landmarkList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case RoadList:
        foreach(element, roadList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case RailList:
        foreach(element, railList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case HydroList:
        foreach(element, hydroList)
            element->drawMapElement(targetPainter, maskPainter);
        break;
      case TopoList:
        foreach(element, topoList)
            element->drawMapElement(targetPainter, maskPainter);
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

  extern MapConfig *_globalMapConfig;

  regIsoLines.clear();

  QList<Isohypse*> *iso;
  foreach(iso, isoList)
    {
      if(iso->count() == 0) continue;

      for(unsigned int pos = 0; pos < ISO_LINE_NUM; pos++)
        {
          if(isoLines[pos] == iso->first()->getElevation())
            {
              if(iso->first()->isValley())
                  height = pos + 1;
              else
                  height = pos + 2;

              break;
            }
        }

      targetP->setPen(QPen(_globalMapConfig->getIsoColor(height), 1, Qt::SolidLine));
      targetP->setBrush(QBrush(_globalMapConfig->getIsoColor(height),
          Qt::SolidPattern));

      Isohypse *iso2;
      foreach(iso2, *iso)
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
 *
 * @returns true if equal; otherwise false
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
  currentFlightListIndex = flightList.indexOf(flightList.last());
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
  QList <Flight*> fl;
  BaseFlightElement *f;
  QString tmp;

  FlightSelectionDialog *fsd = new FlightSelectionDialog(0, "flight selection dialog");

  for(int i = 0; i < flightList.count(); i++)
    {
      f = flightList.value(i);
      if (f->getTypeID() == BaseMapElement::Flight)
        {
          fsd->availableFlights.append(f);
        }
    }

  if (fsd->exec() == QDialog::Accepted)
    {
      for( uint i = 0; i < fsd->selectedFlights.count(); i++ )
        {
          fl.append((Flight *)fsd->selectedFlights.at(i));
        }

      tmp.sprintf("GROUP%03d", gCount++);

      FlightGroup * flightGroup = new FlightGroup(fl, tmp);
      flightList.append(flightGroup);
      currentFlightListIndex = flightList.indexOf(flightList.last());
      emit newFlightGroupAdded(flightGroup);
      emit currentFlightChanged();
    }
  delete fsd;
}
/** No descriptions */
void MapContents::slotSetFlight(int id)
{
  if (id >= 0 && id < flightList.count())
    {
      flightList.at(id);
      currentFlightListIndex = id;
      emit currentFlightChanged();
    }
}

void MapContents::slotSetFlight(BaseFlightElement *f)
{
  if (flightList.count(f)>0)
    {
//      flightList.findRef(f);
      currentFlightListIndex = flightList.indexOf(f);
      emit currentFlightChanged();
    }
}

/** No descriptions */
void MapContents::slotEditFlightGroup()
{
  QList<Flight*> fl;
  BaseFlightElement *f;
  BaseFlightElement *fg;

  FlightSelectionDialog *fsd = new FlightSelectionDialog(0, "flight selection dialog");
  fg = getFlight();

  if (fg->getTypeID() == BaseMapElement::FlightGroup)
    {
      fl = ((FlightGroup *)fg)->getFlightList();
      for (int i = 0; i < flightList.count(); i++)
        {
          f = flightList.at(i);
          if (f->getTypeID() == BaseMapElement::Flight)
            {
              if (fl.count((Flight *)f)>0)
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
          for ( uint i = 0; i < fsd->selectedFlights.count(); i++)
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
  extern MainWindow *_mainWindow;

  QFileInfo fInfo(path);

  extern const MapMatrix *_globalMapMatrix;

  if(!fInfo.exists())
    {
      QMessageBox::warning( _mainWindow, tr("File does not exist"), "<html>" + tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(path.name()) + "</html>", QMessageBox::Ok );
      return false;
    }

  if(!fInfo.size())
    {
      QMessageBox::warning( _mainWindow, tr("File is empty"), "<html>" + tr("The selected file<BR><B>%1</B><BR>is empty!").arg(path.name()) + "</html>", QMessageBox::Ok );
      return false;
    }

  if(!path.open(QIODevice::ReadOnly))
    {
      QMessageBox::warning( _mainWindow, tr("No permission"), "<html>" + tr("You don't have permission to access file<BR><B>%1</B>").arg(path.name()) + "</html>", QMessageBox::Ok );
      return false;
    }

  QDomDocument doc;
  QList<Waypoint*> wpList;
  FlightTask *f, *firstTask = 0;

  doc.setContent(&path);

  if (doc.doctype().name() == "KFLogTask")
    {
      QDomNodeList nl = doc.elementsByTagName("Task");

      for(int i = 0; i < nl.count(); i++)
        {
          QDomNodeList childNodes = nl.item(i).childNodes();
          QDomNamedNodeMap nmTask =  nl.item(i).attributes();

          wpList.clear();
          for(int childIdx = 0; childIdx < childNodes.count(); childIdx++)
            {
              QDomNamedNodeMap nm =  childNodes.item(childIdx).attributes();

              Waypoint *w = new Waypoint;

              w->name = nm.namedItem("Name").toAttr().value().left(6).upper();
              w->description = nm.namedItem("Description").toAttr().value();
              w->icao = nm.namedItem("ICAO").toAttr().value().upper();
              w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
              w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
              w->projP = _globalMapMatrix->wgsToMap(w->origP);
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
      QMessageBox::warning(_mainWindow,
                           tr("Error occurred!"),
                           tr("wrong doctype ") + doc.doctype().name(), QMessageBox::Ok);
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
  foreach(bfe, flightList) {
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
  BaseFlightElement *flight;
  foreach(flight, flightList)
      flight->reProject();
}

/*!
    \fn MapContents::getElevation(QPoint)
 */
int MapContents::getElevation(QPoint coord)
{
  isoListEntry* entry;
  int height=-1; //default 'unknown' value

  for(int i = 0; i < regIsoLinesWorld.count(); i++) {
    entry = regIsoLinesWorld.at(i);
    if (entry->region->contains(coord))
      height = qMax(height,entry->height);
  }
  if (height == -1)
    return height;
  return topoLevels[height];
}
