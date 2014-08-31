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
 **                   2010-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **
 ***********************************************************************/

#include <unistd.h>
#include <cmath>

#include <QtGui>
#include <QtXml>

#include "airfield.h"
#include "airspace.h"
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
#include "openairparser.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "welt2000.h"
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

MapContents::MapContents( QObject* object ) :
  QObject(object),
  currentFlight(0),
  askUser(true),
  loadWelt2000(true),
  loadAirspaces(true),
  m_downloadManger(0),
  m_downloadMangerW2000(0),
  m_downloadOpenAipAsManger(0),
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

  // The user is asked once after each startup, if he wants to download
  // missing map files.
  extern QSettings _settings;

 _settings.setValue("/Internet/AutomaticMapDownload", ADT_NotSet);
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
  extern QSettings _settings;

  if( _settings.value( "/Internet/AutomaticMapDownload", ADT_NotSet ).toInt() == Inhibited )
    {
      qDebug() << "Auto Download Inhibited";
      return false;
    }

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

  QString srvUrl = _settings.value( "/MapData/MapServer",
                                    "http://www.kflog.org/data/landscape/" ).toString();

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

  QString msg;
  msg = QString(tr("%1 download(s) with %2 error(s) done.")).arg(requests).arg(errors);

  QMessageBox::information( _mainWindow,
                            tr("Downloads finished"),
                            msg );
}

/** Called, if a network error occurred during the downloads. */
void MapContents::slotNetworkError()
{
  // A network error has occurred. We do stop all further downloads.
  m_downloadManger->deleteLater();
  m_downloadManger = static_cast<DownloadManager *> (0);

  QString msg;
  msg = QString(tr("Network error occurred.\nAll downloads are canceled!"));

  QMessageBox::information( _mainWindow,
                            tr("Network Error"),
                            msg );
}

/**
 * This slot is called to download the Welt2000 file from the Internet.
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

  if( m_downloadMangerW2000 == static_cast<DownloadManager *> (0) )
    {
      m_downloadMangerW2000 = new DownloadManager(this);

      connect( m_downloadMangerW2000, SIGNAL(finished( int, int )),
               this, SLOT(slotWelt2000DownloadFinished( int, int )) );

      connect( m_downloadMangerW2000, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );

      connect( m_downloadMangerW2000, SIGNAL(status(const QString&)),
               _mainWindow, SLOT(slotSetStatusMsg(const QString &)) );
    }

  QString welt2000FileName = _settings.value( "/Welt2000/FileName", "WELT2000.TXT").toString();
  QString welt2000Link     = _settings.value( "/Welt2000/Link", "http://www.segelflug.de/vereine/welt2000/download").toString();

  _settings.setValue( "/Welt2000/FileName", welt2000FileName );
  _settings.setValue( "/Welt2000/Link", welt2000Link );

  QString url  = welt2000Link + "/" + welt2000FileName;
  QString dest = getMapRootDirectory() + "/airfields/WELT2000.TXT.new";

  m_downloadMangerW2000->downloadRequest( url, dest );
}

/**
 * Called, if the Welt2000 file download is finished successfully.
 */
void MapContents::slotWelt2000DownloadFinished( int requests, int errors )
{
  qDebug() << "MapContents::slotWelt2000DownloadFinished():" << requests << errors;

  // All has finished, free not more needed resources
  m_downloadMangerW2000->deleteLater();
  m_downloadMangerW2000 = static_cast<DownloadManager *> (0);

  // Check, if the file content of the new Welt2000 file has been changed.
  QString curW2000 = getMapRootDirectory() + "/airfields/WELT2000.TXT";
  QString newW2000 = getMapRootDirectory() + "/airfields/WELT2000.TXT.new";

  QFileInfo curFi(curW2000);
  QFileInfo newFi(newW2000);

  if( newFi.exists() == false )
    {
      // No new file available, abort further processing.
      return;
    }

  if( curFi.exists() == false || curFi.size() != newFi.size() )
    {
      // Current file is not available or file sizes are different.
      // Rename new file and initiate a load of it.
      QFile::remove( curW2000 );
      QFile::rename( newW2000, curW2000 );
      slotReloadWelt2000Data();
      return;
    }

  // Compare both files, line by line
  QFile curFile(curW2000);
  QFile newFile(newW2000);

  bool differ = false;

  bool curOk = curFile.open( QIODevice::ReadOnly );

  if( curOk == false )
    {
      return;
    }

  bool newOk = newFile.open( QIODevice::ReadOnly );

  if( newOk == false )
    {
      curFile.close();
      return;
    }

  while( true )
    {
      QByteArray curBA = curFile.readLine();
      QByteArray newBA = newFile.readLine();

      if( curBA.size() == 0 || newBA.size() == 0 )
        {
          // EOF or error, do break only
          break;
        }

      if( curBA.contains(newBA) == false )
        {
          // both do differ, do break
          differ = true;
          break;
        }
    }

  curFile.close();
  newFile.close();

  QFile::remove( curW2000 );
  QFile::rename( newW2000, curW2000 );

  if( differ == true )
    {
      slotReloadWelt2000Data();
    }

  return;
}

/**
 * Reload Welt2000 data file. Can be called after a configuration change or
 * a download update.
 */
void MapContents::slotReloadWelt2000Data()
{
  airfieldList.clear();
  gliderfieldList.clear();
  outLandingList.clear();

  loadWelt2000 = true;
  emit contentsChanged();
}

void MapContents::slotDownloadOpenAipAirspaceFiles()
{
  qDebug() << "MapContents::slotDownloadOpenAipAirspaceFiles()";

  extern QSettings _settings;

  if( __askUserForDownload() != Automatic )
    {
      qDebug() << "openAipAirspaces: Auto Download Inhibited";
      return;
    }

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

  const QString urlPrefix = KFLogConfig::rot47(_settings.value("/Airspace/OpenAipLink", "").toByteArray()) + "/";
  const QString destPrefix = getMapRootDirectory() + "/airspaces/";

  for( int i = 0; i < countryList.size(); i++ )
    {
      // File name format: <country-code>_asp.aip, example: de_asp.aip
      QString file = countryList.at(i).toLower() + "_asp.aip";
      QString url  = urlPrefix + file;
      QString dest = destPrefix + file;
      m_downloadOpenAipAsManger->downloadRequest( url, dest );

      qDebug() << "Download:" << url << dest;
    }
}

void MapContents::slotOpenAipAsDownloadsFinished( int requests, int errors )
{
  // All has finished, free not more needed resources
  m_downloadOpenAipAsManger->deleteLater();
  m_downloadOpenAipAsManger = static_cast<DownloadManager *> (0);

  // initiate a new map load
  // TODO check next line
  emit contentsChanged();

  QString msg;
  msg = QString(tr("%1 download(s) with %2 error(s) done.")).arg(requests).arg(errors);

  QMessageBox::information( _mainWindow,
                            tr("openAIP Airspace Downloads finished"),
                            msg );
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

  QMessageBox::information( _mainWindow,
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

bool MapContents::__readTerrainFile( const int fileSecID,
                                     const int fileTypeID )
{
  extern const MapMatrix *_globalMapMatrix;

  if ( fileTypeID != FILE_TYPE_TERRAIN && fileTypeID != FILE_TYPE_GROUND )
    {
      qWarning( "Requested terrain file type 0x%X is unsupported!", fileTypeID );
      return false;
    }

  QString path, file;

  path = getMapRootDirectory() + "/landscape/";

  file.sprintf( "%c_%.5d.kfl", fileTypeID, fileSecID );

  QString pathName = path + file;

  QFile mapfile(pathName);

  if( ! mapfile.open(QIODevice::ReadOnly) )
    {
      qWarning() << "KFLog: Can not open Terrain file" << pathName;

      int answer = __askUserForDownload();

      if( answer == Automatic )
        {
          __downloadMapFile( file, path );
        }
      else
        {
          qDebug() << "Auto download is disabled! Bye";
        }

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
  extern const MapMatrix *_globalMapMatrix;

  QString path, file;

  path = getMapRootDirectory() + "/landscape/";

  file.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);

  QString pathName = path + file;

  QFile mapfile(pathName);

  if( !mapfile.open( QIODevice::ReadOnly ) )
    {
      qWarning() << "KFLog: Can not open map file" << pathName;

      int answer = __askUserForDownload();

      if( answer == Automatic )
        {
          __downloadMapFile( file, path );
        }

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

int MapContents::__askUserForDownload()
{
  extern MainWindow *_mainWindow;
  extern QSettings  _settings;

  int result = _settings.value( "/Internet/AutomaticMapDownload", ADT_NotSet ).toInt();;

  if( askUser == true && result == ADT_NotSet )
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
            askUser = false;

            break;

          case QMessageBox::No:

            _settings.setValue( "/Internet/AutomaticMapDownload", Inhibited );
            result = Inhibited;
            askUser = false;
            break;

          case QMessageBox::Cancel:

            _settings.setValue( "/Internet/AutomaticMapDownload", ADT_NotSet );
            result = ADT_NotSet;
            askUser = false;
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

void MapContents::proofeSection(bool isPrint)
{
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

  char step, hasstep; // used as small integers
  TilePartMap::Iterator it;

  for(int row = northCorner; row <= southCorner; row++)
    {
      for(int col = westCorner; col <= eastCorner; col++)
        {
          int secID = row + (col + (row * 179));
          // qDebug( "Needed BoxSecID=%d", secID );

          if( secID >= 0 && secID <= MAX_TILE_NUMBER )
            {
              // a valid tile (2x2 degree area) must be in the range 0 ... 16200
              if (! tileSectionSet.contains(secID))
                {
                  // qDebug(" Tile %d is missing", secID );
                  step = 0;
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

      OpenAirParser oap;
      oap.load( airspaceList );

      // finally, sort the airspaces
      airspaceList.sort();

      // Say the world that airspaces have been changed.
      updateFlightAirspaceIntersections();
      emit airspacesLoaded();
    }

  // Checking for Airfield, Gliderfield and Outlanding data
  if( loadWelt2000 == true )
    {
      loadWelt2000 = false;

      Welt2000 welt2000;

      if( welt2000.check4update() == true ||
          welt2000.load( airfieldList, gliderfieldList, outLandingList ) == false )
        {
          // Welt2000 update available or load failed, try to download a new
          // Welt2000 File from the Internet web page.
          slotDownloadWelt2000();
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
    case NavList:
      return &navList[index];
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
  case NavList:
    return &navList[index];
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

  airspaceList.clear();
  airspaceRegionList.clear();

  airfieldList.clear();
  gliderfieldList.clear();
  addSitesList.clear();
  outLandingList.clear();
  navList.clear();
  obstacleList.clear();
  reportList.clear();
  cityList.clear();
  villageList.clear();
  landmarkList.clear();
  highwayList.clear();
  roadList.clear();
  railList.clear();
  hydroList.clear();
  lakeList.clear();
  topoList.clear();

  // all isolines are cleared
  groundMap.clear();
  terrainMap.clear();

  // map tiles are cleared
  tileSectionSet.clear();
  tilePartMap.clear();

  emit contentsChanged();
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

  for (int i = 0; i < navList.size(); i++)
    navList[i].printMapElement(targetPainter, isText);

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

      case AddSitesList:
        for (int i = 0; i < addSitesList.size(); i++)
          addSitesList[i].drawMapElement(targetPainter);
        break;

      case OutLandingList:
        for (int i = 0; i < outLandingList.size(); i++)
          outLandingList[i].drawMapElement(targetPainter);
        break;

      case NavList:
        for (int i = 0; i < navList.size(); i++)
          navList[i].drawMapElement(targetPainter);
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

  QTime t;
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

  // qDebug( "IsoList, drawTime=%dms", t.elapsed() );

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

  extern const MapMatrix *_globalMapMatrix;

  if(!fInfo.exists())
    {
      QMessageBox::warning( _mainWindow, tr("File does not exist"), "<html>" + tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(path.fileName()) + "</html>", QMessageBox::Ok );
      return false;
    }

  if(!fInfo.size())
    {
      QMessageBox::warning( _mainWindow, tr("File is empty"), "<html>" + tr("The selected file<BR><B>%1</B><BR>is empty!").arg(path.fileName()) + "</html>", QMessageBox::Ok );
      return false;
    }

  if(!path.open(QIODevice::ReadOnly))
    {
      QMessageBox::warning( _mainWindow, tr("No permission"), "<html>" + tr("You don't have permission to access file<BR><B>%1</B>").arg(path.fileName()) + "</html>", QMessageBox::Ok );
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

              w->name = nm.namedItem("Name").toAttr().value().left(6).toUpper();
              w->description = nm.namedItem("Description").toAttr().value();
              w->icao = nm.namedItem("ICAO").toAttr().value().toUpper();
              w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
              w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
              w->projP = _globalMapMatrix->wgsToMap(w->origP);
              w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
              w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
              w->comment = nm.namedItem("Comment").toAttr().value();

              QPair<ushort, ushort> rwyHeadings = QPair<ushort, ushort>(0, 0);

              ushort rwyHeading = nm.namedItem("Runway").toAttr().value().toUShort();
              rwyHeadings.first = rwyHeading >> 8;
              rwyHeadings.second = rwyHeading & 0xff;

              bool isLandable = nm.namedItem("Landable").toAttr().value().toInt();
              int rwyLength = nm.namedItem("Length").toAttr().value().toInt();
              enum Runway::SurfaceType rwySfc = (enum Runway::SurfaceType) nm.namedItem("Surface").toAttr().value().toInt();

              Runway rwy( rwyLength, rwyHeadings, rwySfc, isLandable );
              w->rwyList.append( rwy );

              wpList.append(w);
            }

          QString taskName = nmTask.namedItem("Name").toAttr().value();

          if( taskName.isEmpty() || taskNameInUse(taskName) )
            {
              taskName = genTaskName();
            }

          f = new FlightTask(wpList, false, taskName);
          f->setPlanningType(nmTask.namedItem("PlanningType").toAttr().value().toInt());
          f->setPlanningDirection(nmTask.namedItem("PlanningDirection").toAttr().value().toInt());

          // remember first task in file
          if( firstTask == 0 )
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
  static int tCount = 1;
  QString tmp;

  tmp.sprintf("TASK%03d", tCount++);
  return tmp;
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
