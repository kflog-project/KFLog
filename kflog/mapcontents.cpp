/**********************************************************************
 **
 **   mapcontents.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2010-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <cmath>

#include <QtWidgets>
#include <QtXml>

#include "airfield.h"
#include "airspace.h"
#include "AirspaceHelper.h"
#include "basemapelement.h"
#include "distance.h"
#include "elevationfinder.h"
#include "flight.h"
#include "flightgroup.h"
#include "flightselectiondialog.h"
#include "isohypse.h"
#include "kflogconfig.h"
#include "lineelement.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "mapcalc.h"
#include "OpenAipPoiLoader.h"
#include "openairparser.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "wgspoint.h"

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

#define READ_POINT_LIST \
    in >> locLength; \
    all.resize(locLength); \
    for(uint i = 0; i < locLength; i++) \
      { \
        in >> lat_temp; \
        in >> lon_temp; \
        all.setPoint( i, _globalMapMatrix->wgsToMap(lat_temp, lon_temp) ); \
      }

// List of elevation levels in meters (51 in total):
const short MapContents::isoLevels[] =
{
  -10, 0, 10, 25, 50, 75, 100, 150, 200, 250,
  300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750,
  2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000, 4250, 4500,
  4750, 5000, 5250, 5500, 5750, 6000, 6250, 6500, 6750, 7000, 7250,
  7500, 7750, 8000, 8250, 8500, 8750
};

extern MainWindow *_mainWindow;
extern QSettings _settings;

MapContents::MapContents( QObject* object ) :
  QObject(object),
  currentFlight(0),
  loadPoints(true),
  loadAirspaces(true),
  m_downloadManger(0),
  m_downloadOpenAipAsManger(0),
  m_downloadOpenAipPoiManger(0),
  m_currentFlightListIndex(-1)
{
  // Setup a hash used as reverse mapping from isoLine value to array index to
  // speed up loading of ground and terrain files.
  for( int i = 0; i < ISO_LINE_LEVELS; i++ )
    {
      isoHash.insert( isoLevels[i], i );
    }

  _nextIsoLevel = 10000;
  _lastIsoLevel = -1;
  _isoLevelReset = true;
  _lastIsoEntry = 0;

  // Create all needed map directories.
  createMapDirectories();
}

MapContents::~MapContents()
{
  qDeleteAll(flightList);
  qDeleteAll(wpList);
}

extern MapContents* _globalMapContents;

MapContents* MapContents::instance()
{
  return _globalMapContents;
}

void MapContents::slotCloseFlight()
{
  // qDebug() << "MapContents::slotCloseFlight()";

  /*
   * Closes the current flight.
   */
  if( currentFlight != 0 )
    {
      emit clearFlightCursor();

      for( int i = 0; i < flightList.count(); i++ )
        {
          // Remove current flight from all flight groups.
          FlightGroup *fg = dynamic_cast<FlightGroup *> (flightList.at( i ));

          if( fg )
            {
              fg->removeFlight( currentFlight );
            }
        }

      // Signals object tree, that a flight element has been modified.
      // Object tree has to update all flight groups too.
      emit closingFlight( currentFlight );

      int i = flightList.indexOf( currentFlight );

      if( i != -1 )
        {
          delete flightList.takeAt( i );
        }

      if( flightList.size() != 0 )
        {
          currentFlight = flightList.last();
          m_currentFlightListIndex = flightList.size() - 1;
        }
      else
        {
          currentFlight = 0;
          m_currentFlightListIndex = -1;
        }

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
  qDebug() << "MapContents::__downloadMissingMapFile()" << file;

  if( m_downloadManger == static_cast<DownloadManager *> (0) )
    {
      m_downloadManger = new DownloadManager(this);

      connect( m_downloadManger, SIGNAL(finished( int, int )),
               this, SLOT(slotDownloadsFinished( int, int )) );

      connect( m_downloadManger, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );

      connect( m_downloadManger, SIGNAL(status(const QString&)),
               _mainWindow, SLOT(slotSetStatusMsg(const QString &)) );
    }

  // That is the new map link at git hub.
  QString mapSrvLink = "https://raw.githubusercontent.com/kflog-project/MapConverting/master/Landscape/";
  QString mapSrvLinkOld = "http://www.kflog.org/data/landscape/";
  QString srvUrl = _settings.value( "/MapData/MapServer",
                                    "http://www.kflog.org/data/landscape/" ).toString();

  if( srvUrl == mapSrvLinkOld )
    {
      // Replace old link against the new one at git hub.
      srvUrl = mapSrvLink;
    }

  _settings.setValue( "/MapData/MapServer", srvUrl );

  QString url = srvUrl + file;
  QString dest = directory + "/" + file;

  m_downloadManger->downloadRequest( url, dest );
  return true;
}

/** Called, if all downloads are finished. */
void MapContents::slotDownloadsFinished( int requests, int errors )
{
  // All has finished, free not more needed resources
  m_downloadManger->deleteLater();
  m_downloadManger = static_cast<DownloadManager *> (0);

  // initiate a new map load
  emit contentsChanged();

  if( errors )
    {
      QString msg;
      msg = QString(tr("%1 download(s) with %2 error(s) done.")).arg(requests).arg(errors);

      QMessageBox::warning( _mainWindow,
                            tr("Downloads finished"),
                            msg );
    }
}

/** Called, if a network error occurred during the downloads. */
void MapContents::slotNetworkError()
{
  // A network error has occurred. We do stop all further downloads.
  m_downloadManger->deleteLater();
  m_downloadManger = static_cast<DownloadManager *> (0);

  QString msg;
  msg = QString(tr("Network error occurred.\nAll downloads are canceled!"));

  QMessageBox::warning( _mainWindow,
                        tr("Network Error"),
                        msg );
}

void MapContents::slotReloadPointData()
{
  airfieldList.clear();
  gliderfieldList.clear();
  outLandingList.clear();
  navaidList.clear();
  hotspotList.clear();
  reportList.clear();

  loadPoints = true;
  emit contentsChanged();
}

void MapContents::slotDownloadOpenAipAirspaceFiles()
{
  QString countries = _settings.value("/Airspace/Countries", "").toString();

  if( countries.isEmpty() )
    {
      qWarning() << "MapContents::slotDownloadOpenAipAirspaceFiles(): No countries defined!";
      return;
    }

  if( m_downloadOpenAipAsManger == static_cast<DownloadManager *> (0) )
    {
      m_downloadOpenAipAsManger = new DownloadManager(this);

      connect( m_downloadOpenAipAsManger, SIGNAL(finished( int, int )),
               this, SLOT(slotOpenAipAsDownloadsFinished( int, int )) );

      connect( m_downloadOpenAipAsManger, SIGNAL(networkError()),
               this, SLOT(slotOpenAipAsNetworkError()) );

      connect( m_downloadOpenAipAsManger, SIGNAL(status(const QString&)),
               _mainWindow, SLOT(slotSetStatusMsg(const QString &)) );
    }

  QStringList countryList = countries.split(QRegExp("[ ,;]"));

  const QString svrUrl = _settings.value("/OpenAip/Link", "").toByteArray();
  const QString destPrefix = getMapRootDirectory() + "/airspaces/";

  for( int i = 0; i < countryList.size(); i++ )
    {
      // File name format: <country-code>_asp.aip, example: de_asp.aip
      QString file = countryList.at(i).toLower() + "_asp.json";
      QString url  = QString( svrUrl ).arg(file);
      QString dest = destPrefix + file;

      m_downloadOpenAipAsManger->downloadRequest( url, dest );
    }
}

void MapContents::slotOpenAipAsDownloadsFinished( int requests, int errors )
{
  // All has finished, free not more needed resources
  m_downloadOpenAipAsManger->deleteLater();
  m_downloadOpenAipAsManger = static_cast<DownloadManager *> (0);

  // initiate a reload of all airspace data
  slotReloadAirspaceData();

  if( errors )
    {
      QString msg;
      msg = QString(tr("%1 download(s) with %2 error(s) done.")).arg(requests).arg(errors);

      QMessageBox::warning( _mainWindow,
			    tr("openAIP Airspace Downloads finished"),
			    msg );
    }

  emit airspacesDownloaded();
}

/**
 * Called, if a network error occurred during the openAIP airspace file
 * downloads.
 */
void MapContents::slotOpenAipAsNetworkError()
{
  // A network error has occurred. We do stop all further downloads.
  m_downloadOpenAipAsManger->deleteLater();
  m_downloadOpenAipAsManger = static_cast<DownloadManager *> (0);

  QString msg;
  msg = QString(tr("Network error occurred.\nAll downloads are canceled!"));

  QMessageBox::warning( _mainWindow,
                        tr("Network Error"),
                        msg );
}

/**
 * Reload airspace data. Can be called after a configuration change.
 */
void MapContents::slotReloadAirspaceData()
{
  airspaceList.clear();
  airspaceRegionList.clear();

  loadAirspaces = true;
  emit contentsChanged();
}

void MapContents::slotDownloadOpenAipPointFiles()
{
  QString countries = _settings.value("/Points/Countries", "").toString();

  if( countries.isEmpty() )
    {
      qWarning() << "MapContents::slotDownloadOpenAipPointFiles(): No countries defined!";
      return;
    }

  if( m_downloadOpenAipPoiManger == static_cast<DownloadManager *> (0) )
    {
      m_downloadOpenAipPoiManger = new DownloadManager(this);

      connect( m_downloadOpenAipPoiManger, SIGNAL(finished( int, int )),
               this, SLOT(slotOpenAipPoiDownloadsFinished( int, int )) );

      connect( m_downloadOpenAipPoiManger, SIGNAL(networkError()),
               this, SLOT(slotOpenAipPoiNetworkError()) );

      connect( m_downloadOpenAipPoiManger, SIGNAL(status(const QString&)),
               _mainWindow, SLOT(slotSetStatusMsg(const QString &)) );
    }

  QStringList countryList = countries.split(QRegExp("[ ,;]"));

  const QString svrUrl = _settings.value("/OpenAip/Link", "").toByteArray();
  const QString destPrefix = getMapRootDirectory() + "/points/";

  for( int i = 0; i < countryList.size(); i++ )
    {
      // Airfield file name format: <country-code>_apt.json, example: de_apt.json
      QString file = countryList.at(i).toLower() + "_apt.json";
      QString url  = QString( svrUrl ).arg(file);
      QString dest = destPrefix + file;
      m_downloadOpenAipPoiManger->downloadRequest( url, dest );

      // Navaids file name format: <country-code>_nav.json, example: de_nav.json
      file = countryList.at(i).toLower() + "_nav.json";
      url  = QString( svrUrl ).arg(file);
      dest = destPrefix + file;
      m_downloadOpenAipPoiManger->downloadRequest( url, dest );

      // Hotspot file name format: <country-code>_hot.json, example: de_hot.json
      file = countryList.at(i).toLower() + "_hot.json";
      url  = QString( svrUrl ).arg(file);
      dest = destPrefix + file;
      m_downloadOpenAipPoiManger->downloadRequest( url, dest );

      // Radio point file name format: <country-code>_rpp.json, example: de_rpp.json
      file = countryList.at(i).toLower() + "_rpp.json";
      url  = QString( svrUrl ).arg(file);
      dest = destPrefix + file;
      m_downloadOpenAipPoiManger->downloadRequest( url, dest );
    }
}

void MapContents::slotOpenAipPoiDownloadsFinished( int requests, int errors )
{
  // All has finished, free not more needed resources
  m_downloadOpenAipPoiManger->deleteLater();
  m_downloadOpenAipPoiManger = static_cast<DownloadManager *> (0);

  // initiate a reload of all point data
  slotReloadPointData();

  if( errors )
    {
      QString msg;
      msg = QString(tr("%1 download(s) with %2 error(s) done.")).arg(requests).arg(errors);

      QMessageBox::warning( _mainWindow,
                            tr("openAIP Point Data Downloads finished"),
                            msg );
    }

  emit pointsDownloaded();
}

/**
 * Called, if a network error occurred during the openAIP airspace file
 * downloads.
 */
void MapContents::slotOpenAipPoiNetworkError()
{
  // A network error has occurred. We do stop all further downloads.
  m_downloadOpenAipPoiManger->deleteLater();
  m_downloadOpenAipPoiManger = static_cast<DownloadManager *> (0);

  QString msg;
  msg = QString(tr("Network error occurred.\nAll downloads are canceled!"));

  QMessageBox::warning( _mainWindow,
                        tr("Network Error"),
                        msg );
}

void MapContents::slotCheckOpenAipPointData4Update()
{
  if( _settings.value( "/Points/Source", KFLogConfig::OpenAIP ).toInt() != KFLogConfig::OpenAIP )
    {
      // Not selected as point data source.
      return;
    }

  if( _settings.value( "/Points/EnableUpdates", true ).toBool() == false )
    {
      // Updates disabled by the user.
      return;
    }

  QStringList countries = _settings.value( "/Points/Countries", "" ).toString().split(QRegExp("[ ,;]"));

  if( countries.isEmpty() )
    {
      qDebug() << "MapContents::slotCheckOpenAipPointData4Update(): no countries defined!";
      return;
    }

  // Get the update period in days
  int days = _settings.value( "/Points/UpdatePeriod", 30 ).toInt();

  QStringList files;

  for( int i = 0; i < countries.size(); i++ )
    {
      files << countries.at(i) + "_apt.json";
      files << countries.at(i) + "_nav.json";
      files << countries.at(i) + "_hot.json";
      files << countries.at(i) + "_rpp.json";
    }

  QString mapDir = getMapRootDirectory();

  for( int j = 0; j < files.size(); j++ )
    {
      // Search for the openAIP point source file.
      QString path2File = mapDir + QDir::separator() +
	                        "points" + QDir::separator() + files.at(j);

      QFileInfo test( path2File );

      if( ! test.exists() )
        {
          // No file exists. In this case we ignore that because not all three
	        // files are exist in every country.
          continue;
        }

      // Check if update period has expired. Default are 30 days.
      if( test.lastModified().secsTo(QDateTime::currentDateTime()) < 3600 * 24 * days )
        {
          continue;
        }

      // If one file is out of date we download all files.
      slotDownloadOpenAipPointFiles();
      break;
    }
}

void MapContents::slotCheckOpenAipAsData4Update()
{
  if( _settings.value( "/Airspace/EnableUpdates", true ).toBool() == false )
    {
      // Updates disabled by the user.
      return;
    }

  QStringList countries = _settings.value( "/Airspace/Countries", "" ).toString().split(QRegExp("[ ,;]"));

  if( countries.isEmpty() )
    {
      qDebug() << "MapContents::slotCheckOpenAipAsData4Update(): no countries defined!";
      return;
    }

  // Get the update period in days
  int days = _settings.value( "/Airspace/UpdatePeriod", 30 ).toInt();

  QString mapDir = getMapRootDirectory();

  for( int i = 0; i < countries.size (); i++ )
    {
      // Example airspace file name for CZ: cz_asp.json
      // Search for the openAIP airspace source file.
      QString path2File = mapDir + QDir::separator() +
	                        "airspaces" + QDir::separator() +
			                    countries.at(i) + "_asp.json";

      qDebug() << "Check AS File" << path2File;

      QFileInfo test( path2File );

      if( ! test.exists() )
        {
          // No file exists we ignore that.
          continue;
        }

      // Check if update period has expired. Default are 30 days.
      if( test.lastModified().secsTo(QDateTime::currentDateTime()) < 3600 * 24 * days )
        {
          continue;
        }

      // If one file is out of date we download all files.
      slotDownloadOpenAipAirspaceFiles();
      break;
    }
}

void MapContents::slotGetOpenAipPoints()
{
  static bool firstCall = true;

  if( firstCall == false )
    {
      // To avoid endless loops, allow only one call of this method.
      return;
    }

  firstCall = false;

  // This method checks, if countries are set by the user and tries to download
  // openAIP point data for the configured countries.
  //
  // As first the GUI language is determined.
  QString guiLang = __getGuiLanguage();

  // Look, if countries for openAIP points are defined.
  QString pc = _settings.value("/Points/Countries", "").toString();

  if( pc.isEmpty() && guiLang != "??" )
    {
      // There are no countries defined for openAIP points. We assume that
      // is a first call after the installation and try to download the country,
      // defined by the GUI language.
      _settings.setValue("/Points/Countries", guiLang );
      pc = guiLang;
    }

  if( pc.isEmpty() == false )
    {
      slotDownloadOpenAipPointFiles();
    }
}

void MapContents::slotGetOpenAipAirspaces()
{
  static bool firstCall = true;

  if( firstCall == false )
    {
      // To avoid endless loops, allow only one call of this method.
      return;
    }

  firstCall = false;

  // This method checks, if countries are set by the user and tries to download
  // openAIP airspace data for the configured countries.
  //
  // As first the GUI language is determined.
  QString guiLang = __getGuiLanguage();

  // Look, if countries for openAIP airspaces are defined.
  QString ac = _settings.value("/Airspace/Countries", "").toString();

  if( ac.isEmpty() && guiLang != "??" )
    {
      // There are no countries defined for openAIP airspaces. We assume that
      // is a first call after the installation and try to download the country,
      // defined by the GUI language.
      _settings.setValue("/Airspace/Countries", guiLang );
      ac = guiLang;
    }

  if( ac.isEmpty() == false )
    {
      slotDownloadOpenAipAirspaceFiles();
    }
}

QString MapContents::__getGuiLanguage()
{
  // As first the GUI language is determined.
  QString guiLang = _settings.value( "/PersonalData/Language", "??" ).toString();

  if( guiLang == "??" )
    {
      // The GUI language is not configured. Try to get it from the OS
      QString guiLang = QLocale::system().name();

      if( guiLang.isEmpty() == false && guiLang.size() >= 2 )
        {
	  guiLang = guiLang.left(2).toLower();
        }
    }

  return guiLang;
}

bool MapContents::__readTerrainFile( const int fileSecID,
                                     const int fileTypeID )
{
  extern MapMatrix *_globalMapMatrix;

  if ( fileTypeID != FILE_TYPE_TERRAIN && fileTypeID != FILE_TYPE_GROUND )
    {
      qWarning( "Requested terrain file type 0x%X is unsupported!", fileTypeID );
      return false;
    }

  QString path = getMapRootDirectory() + "/landscape/";
  QString file = QString( "%1_%2.kfl" ).arg( QChar(fileTypeID) )
                                       .arg( fileSecID, 5, 10, QChar('0') );

  QString pathName = path + file;

  QFile mapfile(pathName);

  if( ! mapfile.open(QIODevice::ReadOnly) )
    {
      qWarning() << "KFLog: Can not open Terrain file" << pathName;

      __downloadMapFile( file, path );
      return false;
    }

  QDataStream in( &mapfile );
  in.setVersion( QDataStream::Qt_3_3 );

  // qDebug("reading file %s", pathName.toLatin1().data());

  qint8 loadTypeID;
  quint16 loadSecID, formatID;
  quint32 magic;
  QDateTime createDateTime;

  in >> magic;
  in >> loadTypeID;
  in >> formatID;
  in >> loadSecID;
  in >> createDateTime;

  if( magic != KFLOG_FILE_MAGIC )
    {
      mapfile.close();

      // We remove the wrong file to over come this dead lock by a new download
      // of this file from the KFLog map room..
      unlink( pathName.toLatin1().data() );

      qWarning( "KFLog: %s Wrong magic key %x read! Abort loading...",
                pathName.toLatin1().data(), magic );

      return false;
    }

  if( loadTypeID != fileTypeID ) // wrong type
    {
      mapfile.close();

      qWarning("KFLog: %s Wrong load type identifier %x read! Abort loading...",
                pathName.toLatin1().data(), loadTypeID );

      return false;
    }

  // Determine, which file format id is expected
  int expFormatID;

  if( fileTypeID == FILE_TYPE_TERRAIN )
    {
      expFormatID = FILE_VERSION_TERRAIN;
    }
  else
    {
      expFormatID = FILE_VERSION_GROUND;
    }

  qDebug( "Reading File=%s, Magic=0x%x, TypeId=%c, formatId=%d, Date=%s",
          pathName.toLatin1().data(), magic, loadTypeID, formatID,
          createDateTime.toString(Qt::ISODate).toLatin1().data() );

  // Check map file
  if ( formatID < expFormatID )
    {
      mapfile.close();

      // too old ...
      qWarning("KFLog: File format too old! (version %d, expecting: %d) "
               "Aborting ...", formatID, expFormatID );
      return false;
    }
  else if (formatID > expFormatID )
    {
      // too new ...
      mapfile.close();

      qWarning("KFLog: File format too new! (version %d, expecting: %d) "
               "Aborting ...", formatID, expFormatID );
      return false;
    }

  if ( loadSecID != fileSecID )
    {
      mapfile.close();

      qWarning( "KFLog: %s: Wrong section, bogus file name! Arborting ...",
                pathName.toLatin1().data() );

      return false;
    }

  while ( ! in.atEnd() )
    {
      qint16 elevation;
      qint32 pointNumber, lat, lon;
      QPolygon isoline;

      in >> elevation;
      in >> pointNumber;
      isoline.resize( pointNumber );

      for (int i = 0; i < pointNumber; i++)
        {
          in >> lat;
          in >> lon;

          // This is what causes the long delays, lots of floating point calculations
          isoline.setPoint( i, _globalMapMatrix->wgsToMap(lat, lon));
        }

      // Check, if first point and last point of the isoline identical. In this
      // case we can remove the last point and repeat the check.
      for( int i = isoline.size() - 1; i >= 0; i-- )
        {
          if( isoline.point(0) == isoline.point(i) )
             {
               //qWarning( "Isoline Tile=%d has same start and end point. Remove end point.",
               //           loadSecID );

               // remove last point and check again
               isoline.remove(i);
               continue;
             }

          break;
        }

      if( isoline.size() < 3)
        {
          // ignore to small isolines
          qWarning( "Isoline Tile=%d, elevation=%dm has too less points!",
                     loadSecID, elevation );
          continue;
        }

      // determine elevation index, 0 is returned as default for not existing values
      uchar elevationIdx = isoHash.value( elevation, 0 );

      Isohypse newItem( isoline, elevation, elevationIdx, fileSecID, fileTypeID );

      // Check in which map the isohypse has to be stored. We do use two
      // different maps, one for Ground and another for Terrain. The default
      // is set to terrain because there are a lot more.
      QMap<int, QList<Isohypse> > *usedMap = &terrainMap;

      if( fileTypeID == FILE_TYPE_GROUND )
        {
          usedMap = &groundMap;
        }

      // Store new isohypse in the isomap. The tile section identifier is the key.
      if( usedMap->contains( fileSecID ))
        {
          // append isohypse to existing vector
          QList<Isohypse>& isoList = (*usedMap)[fileSecID];
          isoList.append( newItem );
        }
      else
        {
          // create a new entry in the isomap
          QList<Isohypse> isoList;
          isoList.append( newItem );
          usedMap->insert( fileSecID, isoList );
        }
    }

  mapfile.close();
  return true;
}

bool MapContents::__readBinaryFile( const int  fileSecID,
                                    const char fileTypeID )
{
  extern MapMatrix *_globalMapMatrix;

  QString path = getMapRootDirectory() + "/landscape/";
  QString file = QString( "%1_%2.kfl" ).arg( QChar(fileTypeID) )
                                       .arg( fileSecID, 5, 10, QChar('0') );

  QString pathName = path + file;

  QFile mapfile(pathName);

  if( !mapfile.open( QIODevice::ReadOnly ) )
    {
      qWarning() << "KFLog: Can not open map file" << pathName;

      __downloadMapFile( file, path );
      return false;
    }

  QDataStream in( &mapfile );
  in.setVersion( QDataStream::Qt_2_0 );

  qint8 loadTypeID;
  quint16 loadSecID, formatID;
  quint32 magic;
  QDateTime createDateTime;

  in >> magic;

  if( magic != KFLOG_FILE_MAGIC )
    {
      mapfile.close();

      // We remove the wrong file to over come this dead lock by a new download
      // of this file from the KFLog map room..
      unlink( pathName.toLatin1().data() );

      qWarning( "KFLog: %s Wrong magic key %x read! Abort loading...",
                pathName.toLatin1().data(), magic );

      return false;
    }

  in >> loadTypeID;

  if( loadTypeID != fileTypeID ) // wrong type
    {
      mapfile.close();
      qWarning("KFLog: %s Wrong load type identifier %x read! Abort loading...",
                pathName.toLatin1().data(), loadTypeID );

      return false;
    }

  in >> formatID;

  if( formatID < FILE_VERSION_MAP )
    {
      qWarning( "KFLog: File format too old! (version %d, expecting: %d)",
                formatID, FILE_VERSION_MAP );

      return false;
    }
  else if( formatID > FILE_VERSION_MAP )
    {
      qWarning( "KFLog: File format too new! (version %d, expecting: %d)",
                 formatID, FILE_VERSION_MAP );

      return false;
    }

  in >> loadSecID;

  if( loadSecID != fileSecID )
    {
      mapfile.close();

      qWarning( "KFLog: %s: Wrong section, bogus file name! Arborting ...",
                pathName.toLatin1().data() );

      return false;
    }

  in >> createDateTime;

  qDebug( "Reading File=%s, Magic=0x%x, TypeId=%c, formatId=%d, Date=%s",
           pathName.toLatin1().data(), magic, loadTypeID, formatID,
           createDateTime.toString(Qt::ISODate).toLatin1().data() );

  quint8 lm_typ;
  qint8 sort, elev;
  qint32 lat_temp, lon_temp;
  quint32 locLength = 0;
  QString name = "";

  uint allElements = 0;

  while( ! in.atEnd() )
    {
      BaseMapElement::objectType typeIn = BaseMapElement::NotSelected;

      in >> (quint8 &)typeIn;

      locLength = 0;
      name = "";

      QPolygon all;
      QPoint single;

      allElements++;

      switch (typeIn)
        {
        case BaseMapElement::Motorway:
          READ_POINT_LIST

          highwayList.append( LineElement("", typeIn, all, false, fileSecID) );
          break;

        case BaseMapElement::Road:
        case BaseMapElement::Trail:
          READ_POINT_LIST

          roadList.append( LineElement("", typeIn, all, false, fileSecID) );
          break;

        case BaseMapElement::Aerial_Cable:
        case BaseMapElement::Railway:
        case BaseMapElement::Railway_D:
          READ_POINT_LIST

          railList.append( LineElement("", typeIn, all, false, fileSecID) );
          break;

        case BaseMapElement::Canal:
        case BaseMapElement::River:
        case BaseMapElement::River_T:

          typeIn = BaseMapElement::River; //don't use different river types internally
          in >> name;
          READ_POINT_LIST

          hydroList.append( LineElement(name, typeIn, all, false, fileSecID) );
          break;

        case BaseMapElement::City:
          in >> sort;
          in >> name;

          READ_POINT_LIST

          cityList.append( LineElement(name, typeIn, all, sort, fileSecID) );
          // qDebug("added city '%s'", name.toLatin1().data());
          break;

        case BaseMapElement::Lake:
        case BaseMapElement::Lake_T:

          typeIn=BaseMapElement::Lake; // don't use different lake type internally
          in >> sort;
          in >> name;

          READ_POINT_LIST

          lakeList.append( LineElement(name, typeIn, all, sort, fileSecID) );
          break;

        case BaseMapElement::Forest:
        case BaseMapElement::Glacier:
        case BaseMapElement::PackIce:
          in >> sort;
          in >> name;

          READ_POINT_LIST

          topoList.append( LineElement(name, typeIn, all, sort, fileSecID) );
          break;

        case BaseMapElement::Village:

          in >> name;
          in >> lat_temp;
          in >> lon_temp;

          single = _globalMapMatrix->wgsToMap(lat_temp, lon_temp);

          villageList.append( SinglePoint(name, "",
                                          typeIn,
                                          WGSPoint(lat_temp, lon_temp),
                                          single,
                                          0.0,
                                          "",
                                          "",
                                          fileSecID ));
          // qDebug("added village '%s'", name.toLatin1().data());
          break;

        case BaseMapElement::Spot:

          in >> elev;
          in >> lat_temp;
          in >> lon_temp;

          single = _globalMapMatrix->wgsToMap(lat_temp, lon_temp);

          obstacleList.append( SinglePoint( "Spot",
                                            "",
                                            typeIn,
                                            WGSPoint(lat_temp, lon_temp),
                                           single,
                                           0.0,
                                           "",
                                           "",
                                           fileSecID));
          break;

        case BaseMapElement::Landmark:

          in >> lm_typ;
          in >> name;
          in >> lat_temp;
          in >> lon_temp;

          single = _globalMapMatrix->wgsToMap(lat_temp, lon_temp);

          landmarkList.append( SinglePoint( name,
                                            "",
                                            typeIn,
                                            WGSPoint(lat_temp, lon_temp),
                                            single,
                                            0.0,
                                            "",
                                            "",
                                            fileSecID ));
          // qDebug("added landmark '%s'", name.toLatin1().data());
          break;

        default:

          qWarning ("MapContents::__readBinaryFile; Type not handled in switch: %d", typeIn);
          break;
        }
    }

  mapfile.close();

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
  m_currentFlightListIndex = flightList.size() - 1;

  // Signal to object tree about new flight to slotNewFlightAdded
  emit newFlightAdded( flight );
  emit currentFlightChanged();
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

  list << "airspaces" << "points" << "landscape";

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

  list << "airspaces" << "points" << "landscape";

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
  QString mapDefRootDir = QDir::homePath() + "/KFLog/mapdata";

  QString mapRootDir = _settings.value( "/Path/DefaultMapDirectory", "" ).toString();

  if( mapRootDir.isEmpty() )
    {
      qWarning() << "Settings item /Path/DefaultMapDirectory is empty!";
      mapRootDir = mapDefRootDir;

      _settings.setValue( "/Path/DefaultMapDirectory", mapRootDir );
    }

  return mapRootDir;
}

void MapContents::proofeSection( bool isPrint )
{
  extern MainWindow *_mainWindow;
  extern MapMatrix  *_globalMapMatrix;
  QRect mapBorder;

  // Store the last used map center to decide a reload of all map data, if we
  // moved to far away.
  static QPoint lastMapCenter;
  static bool first = true;

  if( first == true )
    {
      // Load last map center at startup.
      first = false;
      lastMapCenter = _globalMapMatrix->getMapCenter();
    }

  // Current used map center
  QPoint mapCenter = _globalMapMatrix->getMapCenter();

  // Current used projection type
  int projectionType = _settings.value( "/MapData/ProjectionType" ).toInt();

  // latitude delta in degrees
  double delta = fabs( (lastMapCenter.x() - mapCenter.x()) / 600000.0 );

  if( projectionType == ProjectionBase::Cylindric && delta > 1.0 )
    {
      // Set a new center parallel for the cylinder map projection, if
      // the latitude delta is bigger a one degree to ensure a low-distortion
      // map display.
      int parallel = static_cast<int>(round( mapCenter.x() / 600000.0 ) );
      _settings.setValue( "/CylindricalProjection/Parallel", parallel * 600000 );

      // save last map center
      lastMapCenter = mapCenter;

      // clear all maps, they will be reloaded by this method later on.
      clearMaps();
      loadAirspaces = true;
      loadPoints = true;

      qDebug() << "Setting new cylinder parallel: Delta-Lat" << delta << "parallel" << parallel;
    }

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

  TilePartMap::Iterator it;

  for(int row = northCorner; row <= southCorner; row++)
    {
      for(int col = westCorner; col <= eastCorner; col++)
        {
          int secID = row + (col + (row * 179));

          if( secID >= 0 && secID <= MAX_TILE_NUMBER )
            {
              // a valid tile (2x2 degree area) must be in the range 0 ... 16200
              if (! tileSectionSet.contains(secID))
                {
                  // qDebug(" Tile %d is missing", secID );
                  char step = 0;
                  char hasstep = 0;
                  // check to see if parts of this tile has already been loaded before
                  it = tilePartMap.find(secID);

                  if (it == tilePartMap.end())
                    {
                      //not found
                      hasstep = 0;
                    }
                  else
                    {
                      hasstep = it.value();
                    }

                  //try loading the currently unloaded files
                  if (!(hasstep & 1))
                    {
                      if (__readTerrainFile(secID, FILE_TYPE_GROUND))
                        {
                          step |= 1;
                        }
                    }

                  if (!(hasstep & 2))
                    {
                      if (__readTerrainFile(secID, FILE_TYPE_TERRAIN))
                        {
                          step |= 2;
                        }
                    }

                  if (!(hasstep & 4))
                    {
                      if (__readBinaryFile(secID, FILE_TYPE_MAP))
                        {
                          step |= 4;
                        }
                    }

                  if (step == 7) //set the correct flags for this map tile
                    {
                      tileSectionSet.insert(secID);  // add section id to set
                      tilePartMap.remove(secID); // make sure we don't leave it as partly loaded
                    }
                  else
                    {
                      if (step > 0)
                        {
                          tilePartMap.insert(secID, step);
                        }
                    }
                }
            }
        }
    }

  // Checking for Airspaces
  if( loadAirspaces == true || airspaceList.isEmpty() )
    {
      loadAirspaces = false;

      int res = AirspaceHelper::loadAirspaces( airspaceList );

      if( res == 0 )
        {
          // No airspace files found, try to download any.
          QTimer::singleShot(500, this, SLOT(slotGetOpenAipAirspaces()));
        }
      else
        {
          // finally, sort the airspaces
          airspaceList.sort();

          // Say the world that airspaces have been changed.
          updateFlightAirspaceIntersections();
          emit airspacesLoaded();
        }
    }

  // Checking for point data
  if( loadPoints == true )
    {
      loadPoints = false;

      int pointSource = _settings.value( "/Points/Source", KFLogConfig::OpenAIP ).toInt();

      // At the moment only openAIP is supported for download.
      if( pointSource == KFLogConfig::OpenAIP )
        {
          OpenAipPoiLoader poiLoader;
          int res = poiLoader.load( airfieldList );
          res += poiLoader.load( navaidList );
          res += poiLoader.load( hotspotList );
          res += poiLoader.load( "*_rpp.json",
                                 BaseMapElement::CompPoint,
                                 reportList );

          if( res == 0 )
            {
              // No openAIP point data loaded, try to download any.
              QTimer::singleShot(500, this, SLOT(slotGetOpenAipPoints()));
            }
        }
    }
}

int MapContents::getListLength(int listIndex) const
{
  switch(listIndex)
    {
      case AirfieldList:
        return airfieldList.count();
      case GliderfieldList:
        return gliderfieldList.count();
      case OutLandingList:
        return outLandingList.count();
      case NavaidList:
        return navaidList.count();
      case HotspotList:
        return hotspotList.count();
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
      case HighwayList:
        return highwayList.count();
      case RoadList:
        return roadList.count();
      case RailList:
        return railList.count();
      case HydroList:
        return hydroList.count();
      case LakeList:
        return lakeList.count();
      case TopoList:
        return topoList.count();
      default:
        return 0;
    }
}


Airspace* MapContents::getAirspace(uint index)
{
  return &airspaceList[index];
}


Airfield* MapContents::getAirfield(uint index)
{
  return &airfieldList[index];
}


Airfield* MapContents::getGliderfield(uint index)
{
  return &gliderfieldList[index];
}

Airfield* MapContents::getOutlanding(uint index)
{
  return &outLandingList[index];
}

BaseMapElement* MapContents::getElement(int listIndex, uint index)
{
  switch(listIndex)
  {
    case AirfieldList:
      return &airfieldList[index];
    case GliderfieldList:
      return &gliderfieldList[index];
    case OutLandingList:
      return &outLandingList[index];
    case NavaidList:
      return &navaidList[index];
    case HotspotList:
      return &hotspotList[index];
    case AirspaceList:
      return &airspaceList[index];
    case ObstacleList:
      return &obstacleList[index];
    case ReportList:
      return &reportList[index];
    case CityList:
      return &cityList[index];
    case VillageList:
      return &villageList[index];
    case LandmarkList:
      return &landmarkList[index];
    case HighwayList:
      return &highwayList[index];
    case RoadList:
      return &roadList[index];
    case RailList:
      return &railList[index];
    case HydroList:
      return &hydroList[index];
    case LakeList:
      return &lakeList[index];
    case TopoList:
      return &topoList[index];

    default:
      qWarning("KFLog: Trying to access unknown map element list");
      return static_cast<BaseMapElement *> (0);
    }
}


SinglePoint* MapContents::getSinglePoint(int listIndex, uint index)
{
  switch(listIndex)
  {
  case AirfieldList:
    return &airfieldList[index];
  case GliderfieldList:
    return &gliderfieldList[index];
  case OutLandingList:
    return &outLandingList[index];
  case NavaidList:
    return &navaidList[index];
  case HotspotList:
    return &hotspotList[index];
  case ObstacleList:
    return &obstacleList[index];
  case ReportList:
    return &reportList[index];
  case VillageList:
    return &villageList[index];
  case LandmarkList:
    return &landmarkList[index];
  default:
    qWarning("KFLog: Trying to access unknown single point element list");
    return static_cast<SinglePoint *> (0);
  }
}


void MapContents::slotReloadMapData()
{
  // qDebug() << "MapContents::slotReloadMapData(): Clears all Maps!";
  clearMaps();
  emit contentsChanged();
}

void MapContents::clearMaps()
{
  airspaceList.clear();
  airspaceRegionList.clear();
  airfieldList = QList<Airfield>();
  gliderfieldList = QList<Airfield>();
  outLandingList = QList<Airfield>();
  navaidList = QList<RadioPoint>();
  hotspotList = QList<ThermalPoint>();
  obstacleList = QList<SinglePoint>();
  reportList = QList<SinglePoint>();
  cityList = QList<LineElement>();
  villageList = QList<SinglePoint>();
  landmarkList = QList<SinglePoint>();
  highwayList = QList<LineElement>();
  roadList = QList<LineElement>();
  railList = QList<LineElement>();
  hydroList = QList<LineElement>();
  lakeList = QList<LineElement>();
  topoList = QList<LineElement>();

  // all isolines are cleared
  groundMap.clear();
  terrainMap.clear();

  // map tiles are cleared
  tileSectionSet.clear();
  tilePartMap.clear();
}


void MapContents::printContents(QPainter* targetPainter, bool isText)
{
  proofeSection(true);

  BaseMapElement *elementPtr;

  for (int i = 0; i < obstacleList.size(); i++)
    obstacleList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < reportList.size(); i++)
    reportList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < cityList.size(); i++)
     cityList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < villageList.size(); i++)
    villageList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < landmarkList.size(); i++)
    landmarkList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < highwayList.size(); i++)
    highwayList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < roadList.size(); i++)
    roadList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < railList.size(); i++)
    railList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < hydroList.size(); i++)
    hydroList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < lakeList.size(); i++)
    lakeList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < topoList.size(); i++)
    topoList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < airspaceList.size(); i++)
    airspaceList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < navaidList.size(); i++)
    navaidList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < hotspotList.size(); i++)
    hotspotList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < airfieldList.size(); i++)
    airfieldList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < gliderfieldList.size(); i++)
    gliderfieldList[i].printMapElement(targetPainter, isText);

  for (int i = 0; i < outLandingList.size(); i++)
    outLandingList[i].printMapElement(targetPainter, isText);

  foreach(elementPtr, flightList)
    elementPtr->printMapElement(targetPainter, isText);
}


void MapContents::drawList( QPainter* targetPainter,
                            unsigned int listID,
                            QList<BaseMapElement *>& drawnElements )
{
  switch(listID)
    {
      case AirfieldList:
        for (int i = 0; i < airfieldList.size(); i++)
          airfieldList[i].drawMapElement(targetPainter);
        break;

      case GliderfieldList:
        for (int i = 0; i < gliderfieldList.size(); i++)
          gliderfieldList[i].drawMapElement(targetPainter);
        break;

      case OutLandingList:
        for (int i = 0; i < outLandingList.size(); i++)
          outLandingList[i].drawMapElement(targetPainter);
        break;

      case NavaidList:
        for (int i = 0; i < navaidList.size(); i++)
          navaidList[i].drawMapElement(targetPainter);
        break;

      case HotspotList:
        for (int i = 0; i < hotspotList.size(); i++)
          hotspotList[i].drawMapElement(targetPainter);
        break;

      case AirspaceList:
        for (int i = 0; i < airspaceList.size(); i++)
          airspaceList[i].drawMapElement(targetPainter);
        break;

      case ObstacleList:
        for (int i = 0; i < obstacleList.size(); i++)
          obstacleList[i].drawMapElement(targetPainter);
        break;

      case ReportList:
        for (int i = 0; i < reportList.size(); i++)
          reportList[i].drawMapElement(targetPainter);
        break;

      case CityList:
        for (int i = 0; i < cityList.size(); i++)
          {
           if( cityList[i].drawMapElement(targetPainter) )
             {
               drawnElements.append( &cityList[i] );
             }
          }
        break;

      case VillageList:
        for (int i = 0; i < villageList.size(); i++)
          villageList[i].drawMapElement(targetPainter);
        break;

      case LandmarkList:
        for (int i = 0; i < landmarkList.size(); i++)
          landmarkList[i].drawMapElement(targetPainter);
        break;

      case HighwayList:
        for (int i = 0; i < highwayList.size(); i++)
          highwayList[i].drawMapElement(targetPainter);
        break;

      case RoadList:
        for (int i = 0; i < roadList.size(); i++)
          roadList[i].drawMapElement(targetPainter);
        break;

      case RailList:
        for (int i = 0; i < railList.size(); i++)
          railList[i].drawMapElement(targetPainter);
        break;

      case HydroList:
        for (int i = 0; i < hydroList.size(); i++)
          hydroList[i].drawMapElement(targetPainter);
        break;

      case LakeList:
        for (int i = 0; i < lakeList.size(); i++)
          lakeList[i].drawMapElement(targetPainter);
        break;

      case TopoList:
        for (int i = 0; i < topoList.size(); i++)
          topoList[i].drawMapElement(targetPainter);
        break;

      case FlightList:
        // In some cases, getFlightIndex returns a non-valid index :-(
        if (flightList.size() > 0 && getFlightIndex() >= 0 &&
              getFlightIndex() < flightList.size() )
            flightList.at(getFlightIndex())->drawMapElement(targetPainter);
        break;

      default:
        qWarning("MapContents::drawList(): unknown listID %d", listID);
        break;
    }
}

void MapContents::drawIsoList( QPainter* targetP, QRect windowRect )
{
  // qDebug() << "MapContents::drawIsoList():";

  QElapsedTimer t;
  t.start();

  extern MapConfig* _globalMapConfig;

  _lastIsoEntry = 0;
  _isoLevelReset = true;
  pathIsoLines.clear();

  int count = 2; // draw only the ground

  bool drawTerrain = false; // Could be switched off

  // shall we draw the terrain too?
  if( drawTerrain )
    {
      count++; // yes
    }

  QMap< int, QList<Isohypse> >* isoMaps[2] = { &groundMap, &terrainMap };

  for( int i = 0; i < count; i++ )
    {
      // assign the map to be drawn to the iterator
      QMapIterator<int, QList<Isohypse> > it(*isoMaps[i]);

      while (it.hasNext())
        {
          // Iterate over all tiles in the isomap and fetch the isoline lists.
          // The isoline list contains all isolines of a tile in ascending order.
          it.next();

          const QList<Isohypse> &isoList = it.value();

          for (int i = 0; i < isoList.size(); i++)
            {
              Isohypse isoLine = isoList.at(i);

              // Choose contour color.
              // The index of the isoList has a fixed relation to the isocolor list
              // normally with an offset of one.
              int colorIdx = isoLine.getElevationIndex();

              targetP->setPen(QPen(_globalMapConfig->getIsoColor(colorIdx), 1, Qt::SolidLine));
              targetP->setBrush(QBrush(_globalMapConfig->getIsoColor(colorIdx), Qt::SolidPattern));

              // draw the single isoline
              QPainterPath* Path = isoLine.drawRegion( targetP,
                                                       windowRect,
                                                       true,
                                                       false );
              if( Path )
                {
                  // store drawn path in extra list for elevation finding
                  IsoListEntry entry( Path, isoLine.getElevation() );
                  pathIsoLines.append( entry );
                  //qDebug("  added Iso: %04x, %d", (int)reg, iso2.getElevation() );
                }
            }
        }
    }

  pathIsoLines.sort();
  _isoLevelReset = false;

  qDebug( "IsoList, drawTime=%lldms", t.elapsed() );

#if 0
  QString isos;

  for ( int l = 0; l < pathIsoLines.count(); l++ )
    {
      isos += QString("%1, ").arg(pathIsoLines.at(l).height);
    }

  qDebug( isos.toLatin1().data() );
#endif
}

void MapContents::addDir (QStringList& list, const QString& _path, const QString& filter)
{
  QDir path (_path, filter);

  if ( ! path.exists() )
    {
      return;
    }

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
      list += path.absoluteFilePath (*it);
  }
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
  // qDebug() << "MapContents::slotNewTask()";

  FlightTask *ft = new FlightTask(genTaskName());
  flightList.append(ft);
  m_currentFlightListIndex = flightList.size() - 1;
  currentFlight = ft;

  // Calls ObjectTree::slotNewTaskAdded()
  emit newTaskAdded(ft);

  QString
  helpText = tr(  "<html>"
                  "<b>Graphical Task Planning</b><br><br>"
                  "If a new task is created the graphical task planning is activated automatically. "
                  "Press the left mouse button on a point at the map and you will get displayed a popup menu "
                  "with the possible actions. Position the mouse pointer at an existing waypoint "
                  "to add or delete it from the task. Position the mouse pointer at a free point "
                  "at the map to create a new waypoint. The new created waypoint will be added to the task. "
                  "Waypoints are always appended at the end of the task. If you need "
                  "more flexibility, you should open the task in the task editor. With the editor you "
                  "have more possibilities to modify the task."
                  "<br><br>"
                  "To finish the graphical task planning use the menu point <b><i>End task planning</i></b>. "
                  "It is important to do that otherwise the task is not closed."
                  "</html>"
                );

  // opens help window.
  emit taskHelp(helpText);
  emit currentFlightChanged();
}

void MapContents::slotAppendTask(FlightTask *ft)
{
  // qDebug() << "MapContents::slotAppendTask FT=" << ft;

  flightList.append(ft);
  emit newTaskAdded(ft);
}

/** create a new, empty flight group */
void MapContents::slotNewFlightGroup()
{
  static int gCount = 1;
  QList <Flight*> fl;
  BaseFlightElement *f;
  QString tmp;

  FlightSelectionDialog *fsd = new FlightSelectionDialog( _mainWindow );

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
      for( int i = 0; i < fsd->selectedFlights.count(); i++ )
        {
          fl.append( dynamic_cast<Flight *>(fsd->selectedFlights.at(i)) );
        }

      tmp = QString(tr("Group-%1")).arg( gCount++, 3, 10, QChar('0') );

      FlightGroup* flightGroup = new FlightGroup(fl, tmp);

      flightList.append(flightGroup);
      m_currentFlightListIndex = flightList.size() - 1;
      currentFlight = flightGroup;

      emit newFlightGroupAdded(flightGroup);
      emit currentFlightChanged();
    }

  delete fsd;
}

void MapContents::slotSetFlight( QAction *action )
{
  int id = action->data().toInt();

  if (id >= 0 && id < flightList.count())
    {
      currentFlight = flightList.at(id);
      m_currentFlightListIndex = id;
      emit currentFlightChanged();
    }
}

void MapContents::slotSetFlight(BaseFlightElement *bfe)
{
  if( flightList.contains( bfe ) )
    {
      m_currentFlightListIndex = flightList.indexOf( bfe );
      currentFlight = flightList.at(m_currentFlightListIndex);
      emit currentFlightChanged();
    }
}

/** No descriptions */
void MapContents::slotEditFlightGroup()
{
  FlightGroup *fg = dynamic_cast<FlightGroup *>(getFlight());

  if( fg == static_cast<FlightGroup *>(0) )
    {
      // Current flight is not a flight group.
      return;
    }

  FlightSelectionDialog *fsd = new FlightSelectionDialog( _mainWindow );

  QList<Flight*> fl = fg->getFlightList();

  for (int i = 0; i < flightList.size(); i++)
    {
      BaseFlightElement *f = flightList.at(i);

      if (f->getTypeID() == BaseMapElement::Flight)
        {
          if (fl.contains(dynamic_cast<Flight *>(f)) )
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

      for ( int i = 0; i < fsd->selectedFlights.size(); i++)
        {
          fl.append( dynamic_cast<Flight *>(fsd->selectedFlights.at(i)) );
        }

      fg->setFlightList(fl);

      emit currentFlightChanged();
    }

  delete fsd;
}

/** read a task file and append all tasks to flight list
switch to first task in file */
bool MapContents::loadTask(QFile& path)
{
  extern MainWindow *_mainWindow;

  QFileInfo fInfo(path);

  extern MapMatrix *_globalMapMatrix;

  if(!fInfo.exists())
    {
      QMessageBox::warning( _mainWindow, tr("File does not exist"), "<html>" +
                            tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(path.fileName()) + "</html>",
                            QMessageBox::Ok );
      return false;
    }

  if(!fInfo.size())
    {
      QMessageBox::warning( _mainWindow, tr("File is empty"), "<html>" +
                            tr("The selected file<BR><B>%1</B><BR>is empty!").arg(path.fileName()) + "</html>",
                            QMessageBox::Ok );
      return false;
    }

  if(!path.open(QIODevice::ReadOnly))
    {
      QMessageBox::warning( _mainWindow, tr("No permission"), "<html>" +
                            tr("You don't have permission to access file<BR><B>%1</B>").arg(path.fileName()) + "</html>",
                            QMessageBox::Ok );
      return false;
    }

  QDomDocument doc;
  QList<Waypoint*> wpList;
  FlightTask *firstTask = nullptr;

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

              w->name = nm.namedItem("Name").toAttr().value().left(6).toUpper();
              w->description = nm.namedItem("Description").toAttr().value();
              w->icao = nm.namedItem("ICAO").toAttr().value().toUpper();
              w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
              w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
              w->projP = _globalMapMatrix->wgsToMap(w->origP);
              w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();

              float fqval = nm.namedItem("Frequency").toAttr().value().toDouble();
              Frequency fq;
              fq.setUnit( Frequency::MHz );
              fq.setValue( fqval );
              w->addFrequency( fq );
              w->comment = nm.namedItem("Comment").toAttr().value();

              ushort rwyHeading = nm.namedItem("Runway").toAttr().value().toUShort();
              ushort rwyH1 = rwyHeading >> 8;
              ushort rwyH2 = rwyHeading & 0xff;
              bool isLandable = nm.namedItem("Landable").toAttr().value().toInt();
              int rwyLength = nm.namedItem("Length").toAttr().value().toInt();
              enum Runway::SurfaceType rwySfc = (enum Runway::SurfaceType) nm.namedItem("Surface").toAttr().value().toInt();

              Runway rwy;
              rwy.setLength( rwyLength );
              rwy.setSurface( rwySfc );

              if( isLandable == true )
                {
                  rwy.setOperations( Runway::Active );
                }
              else
                {
                  rwy.setOperations( Runway::Closed );
                }

              // Add runway only, if it has a heading > 0.
              if( rwyH1 > 0 )
                {
                  rwy.setName( QString( "%1").arg( rwyH1, 2, 10, QChar('0') ) );
                  rwy.setHeading( rwyH1 * 10 );
                  w->addRunway( rwy );
                }

              if( rwyH2 > 0 )
                {
                  rwy.setName( QString( "%1").arg( rwyH2, 2, 10, QChar('0') ) );
                  rwy.setHeading( rwyH2 * 10 );
                  w->addRunway( rwy );
                }

              wpList.append(w);
            }

          QString taskName = nmTask.namedItem("Name").toAttr().value();

          if( taskName.isEmpty() || taskNameInUse(taskName) )
            {
              taskName = genTaskName();
            }

          FlightTask* f = new FlightTask(wpList, false, taskName);
          f->setPlanningType(nmTask.namedItem("PlanningType").toAttr().value().toInt());
          f->setPlanningDirection(nmTask.namedItem("PlanningDirection").toAttr().value().toInt());

          // remember first task in file
          if( firstTask == nullptr )
            {
              firstTask = f;
            }

          flightList.append(f);
          emit newTaskAdded(f);
        }

      if( firstTask )
        {
          slotSetFlight( firstTask );
        }

      return true;
    }
  else
    {
      QMessageBox::warning( _mainWindow,
                            tr("Load task failed!"),
                            tr("Wrong XML task document type ") + doc.doctype().name(),
                            QMessageBox::Ok );
      return false;
    }
}

QString MapContents::genTaskName()
{
  static quint16 tCount = 1;

  return QString( "TASK%1" ).arg( tCount++, 3, 10, QChar('0') );
}

QString MapContents::genTaskName(QString suggestion)
{
  BaseFlightElement* bfe;

  foreach(bfe, flightList)
    {
      FlightTask* ft = dynamic_cast<FlightTask*> ( bfe );

      if( ft )
        {
          if( ft->getFileName() == suggestion )
            {
              // Name is already in use. Generate an unique one.
              return genTaskName( genTaskName() );
            }
        }
    }

  return suggestion;
}

bool MapContents::taskNameInUse( QString name )
{
  BaseFlightElement* bfe;

  foreach(bfe, flightList)
    {
      FlightTask* ft = dynamic_cast<FlightTask*> ( bfe );

      if( ft )
        {
          if( ft->getFileName() == name )
            {
              // Name is already in use. Generate an unique one.
              return true;
            }
        }
    }

  return false;
}

/** Re-projects any flights and tasks that may be loaded. */
void MapContents::reProject()
{
  BaseFlightElement *flight;

  foreach(flight, flightList)
    {
      flight->reProject();
    }
}

/** Update airspace intersections in all flight. */
void MapContents::updateFlightAirspaceIntersections()
{
  // If airspaces are updated we must update too the airspace intersections,
  // because pointers to airspaces have become invalid.
  BaseFlightElement *bfe;

  foreach(bfe, flightList)
    {
      Flight *flight = dynamic_cast<class Flight *> (bfe);

      if( ! flight )
        {
          continue;
        }

      flight->calAirSpaceIntersections();
    }
}

/** coorMap coordinates are expected as map based!. */
int MapContents::getElevation( const QPoint& coordMap, Distance* errorDist )
{
  const IsoListEntry* entry = 0;
  int height = -1;
  double error = 0.0;

  // Use this only if input coordinate are WGS84 based
  //QPoint coordP1 = _globalMapMatrix->wgsToMap(coordP.x(), coordP.y());
  //QPoint coord   = _globalMapMatrix->map(coordP1);

  QPoint coord = coordMap;

  IsoList* list = getIsohypseRegions();

  if (_isoLevelReset)
    {
      qDebug("findElevation: Busy rebuilding the isomap. Returning last known result...");
      return _lastIsoLevel;
    }

  int cnt = list->count();

  for ( int i=0; i<cnt; i++ )
    {
      entry = &(list->at(i));
      // qDebug("i: %d entry->height %d contains %d",i,entry->height, entry->path->contains(coord) );
      // qDebug("Point x:%d y:%d", coord.x(), coord.y() );
      // qDebug("boundingRect l:%d r:%d t:%d b:%d", entry->path->boundingRect().left(),
      //                                 entry->path->boundingRect().right(),
      //                                 entry->path->boundingRect().top(),
      //                                 entry->path->boundingRect().bottom() );

      if (entry->height > height && /*there is no reason to search a lower level if we already have a hit on a higher one*/
          entry->height <= _nextIsoLevel) /* since the odds of skipping a level between two fixes are not too high, we can ignore higher levels, making searching more efficient.*/
        {
          if (entry->height == _lastIsoLevel && _lastIsoEntry)
            {
              //qDebug("Trying previous entry...");
              if (_lastIsoEntry->path->contains(coord))
                {
                  height = qMax(height, entry->height);
                  //qDebug("Found on height %d",entry->height);
                  break;
                }
            }

          if (entry == _lastIsoEntry)
            {
              continue; //we already tried this one, and it wasn't it.
            }

          //qDebug("Probing on height %d...", entry->height);

          if (entry->path->contains(coord))
            {
              height = qMax(height,entry->height);
              //qDebug("Found on height %d",entry->height);
              _lastIsoEntry = entry;
              break;
            }
        }
    }

  _lastIsoLevel = height;

  // if errorDist is set, set the correct error margin and correct height.
  if(errorDist)
    {
      // The real altitude is between the current and the next
      // isolevel, therefore reduce error by taking the middle
      if ( height <100 )
        {
          _nextIsoLevel = height+25;
          height += 12;
          error=12.5;
        }
      else if ( (height >=100) && (height < 500) )
        {
          _nextIsoLevel = height+50;
          height += 25;
          error=25.0;
        }
      else if ( (height >=500) && (height < 1000) )
        {
          _nextIsoLevel = height+100;
          height += 50;
          error=50.0;
        }
      else
        {
          _nextIsoLevel = height+250;
          height += 125;
          error = 125.0;
        }

      errorDist->setMeters(error);
    }

  return height;
}
