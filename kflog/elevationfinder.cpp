/***********************************************************************
**
**   elevationfinder.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c): 2004 by André Somers <andre@kflog.org>
**                  2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "elevationfinder.h"
#include "mapcontents.h"
#include "mapmatrix.h"

extern MapContents *_globalMapContents;
extern MapMatrix *_globalMapMatrix;

ElevationFinder::ElevationFinder(QObject *parent) :
 QObject(parent),
 useOGIE(false),
 demFile(0),
 demStream(0)
{
  setObjectName("ElevationFinder");

  if (tryOpenGLIGCexplorer())
    {
      useOGIE = true;
    }

  timer = new QTimer(this);
  timer->setSingleShot( true );
  connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

ElevationFinder::~ElevationFinder()
{
  timeout(); //takes care of deleting demStream and demFile
}

ElevationFinder* ElevationFinder::instance()
{
  static ElevationFinder instance(0);

  return &instance;
}

int ElevationFinder::elevationWgs(const QPoint& coordinates)
{
  if( useOGIE ) //use openGLIGCexplorer's DEM file
    {
      return findDEMelevation( coordinates );
    }

  //use the 'old' method
  return _globalMapContents->getElevation( _globalMapMatrix->wgsToMap( coordinates ), 0 );
}

int ElevationFinder::elevationMap(const QPoint& coordinates)
{
  if( useOGIE ) //use openGLIGCexplorer's DEM file
    {
      return findDEMelevation( _globalMapMatrix->mapToWgs( coordinates ) );
    }

  //use the 'old' method
  return _globalMapContents->getElevation( coordinates, 0 );
}

int ElevationFinder::elevation(const QPoint& WgsCoordinates, const QPoint& MapCoordinates)
{
  if( useOGIE ) //use openGLIGCexplorer's DEM file
    {
      return findDEMelevation( WgsCoordinates );
    }

  //use the 'old' method
  return _globalMapContents->getElevation( MapCoordinates, 0 );
}

int ElevationFinder::findDEMelevation(const QPoint& coordinates)
{
  timer->stop();

  if( !demFile )
    {
      demFile = new QFile( demFileName );
      demFile->open( QIODevice::ReadOnly );
      demStream = new QDataStream( demFile );
    }

  QRect r(QPoint(demBR.x(), demTL.y()), QPoint(demTL.x(), demBR.y()));

  if( !r.contains( coordinates.x(), coordinates.y() ) )
    {
      //qDebug("Requested point outside DEM coverage.");
      return -1;
    }

  int row = (demTL.x() - coordinates.x()) / demGridLat;
  int col = -1 * (demTL.y() - coordinates.y()) / demGridLon;
  qint64 cell = 2 * (row * demCols + col);
  qint16 h;

  demFile->seek( cell );
  *demStream >> h;

  if( h == -9999 )
    {
      h = 0;
    }

  return h;
}

void ElevationFinder::timeout()
{
  if( demStream )
    {
      delete demStream;
    }

  if( demFile )
    {
      demFile->close();
      delete demFile;
    }

  demStream = 0;
  demFile = 0;
}

bool ElevationFinder::tryOpenGLIGCexplorer()
{
  QString line;
  int flags = 0;
  int dem_name = 1;
  int dem_top = 2;
  int dem_bottom = 4;
  int dem_left = 8;
  int dem_right = 16;
  int dem_rows = 32;
  int dem_cols = 64;
  int dem_grid_lat = 128;
  int dem_grid_lon = 256;


  // can we find openGLIGCexplorer's configuration file?
  QFile file( QDir::home().path() + "/.openGLIGCexplorerrc" );

  if( !file.exists() )
    {
      return false;
    }

  if( !file.open( QIODevice::ReadOnly ) )
    {
      qWarning() << "Cannot open file:" << file.fileName();
      return false;
    }

  QTextStream s(&file);

  //let's try to find the DEM file in use from the configuration file
  while( ! s.atEnd() )
    {
      line = s.readLine();

      if( line.isEmpty() )
        {
          continue;
        }
      //qDebug("line: %s",line.toLatin1().data());

      if( line.startsWith( "DEM_FILE" ) )
        {
          flags |= dem_name;
          demFileName = line.mid( 9 ).trimmed();
        }

      else if( line.startsWith( "DEM_ROWS" ) )
        {
          flags |= dem_rows;
          demRows = line.mid( 9 ).trimmed().toInt();
        }

      else if( line.startsWith( "DEM_COLUMNS" ) )
        {
          flags |= dem_cols;
          demCols = line.mid( 12 ).trimmed().toInt();
        }

      else if( line.startsWith( "DEM_LAT_MIN" ) )
        {
          flags |= dem_top;
          demBR.setX( (int) line.mid( 12 ).trimmed().toDouble()
              * 600000 );
        }

      else if( line.startsWith( "DEM_LAT_MAX" ) )
        {
          flags |= dem_bottom;
          demTL.setX( (int) line.mid( 12 ).trimmed().toDouble()
              * 600000 );
        }

      else if( line.startsWith( "DEM_LON_MIN" ) )
        {
          flags |= dem_left;
          demTL.setY( (int) line.mid( 12 ).trimmed().toDouble()
              * 600000 );
        }

      else if( line.startsWith( "DEM_LON_MAX" ) )
        {
          flags |= dem_right;
          demBR.setY( (int) line.mid( 12 ).trimmed().toDouble()
              * 600000 );
        }

      else if( line.startsWith( "DEM_GRID_LAT" ) )
        {
          flags |= dem_grid_lat;
          demGridLat = int( line.mid( 13 ).trimmed().toDouble()
              * 600000.0 );
          qDebug( "%s -> %d",
                  line.mid( 13 ).trimmed().toLatin1().data(), demGridLat );
        }

      else if( line.startsWith( "DEM_GRID_LON" ) )
        {
          flags |= dem_grid_lon;
          demGridLon = int( line.mid( 13 ).trimmed().toDouble()
              * 600000.0 );
          qDebug( "%s -> %d",
                  line.mid( 13 ).trimmed().toLatin1().data(), demGridLat );
        }

      if( flags == 511 ) //all needed fields are found
        {
          break;
        }
    }

  file.close();

  //qDebug("flags: %d", flags);

  if (flags != 511) //we did not find all the fields we need
    {
      return false;
    }

  // qDebug("demTL.x(): %d, demTL.y() %d", demTL.x(), demTL.y());
  // qDebug("demBR.x(): %d, demBR.y() %d", demBR.x(), demBR.y());

  qDebug("Good! Found DEM file using openGLIGCexplorer's configuration file!");

  return true;
}
