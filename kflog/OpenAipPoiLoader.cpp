/***********************************************************************
**
**   OpenAipPoiLoader.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "mapcontents.h"
#include "OpenAip.h"
#include "OpenAipPoiLoader.h"

extern QSettings _settings;

// set static member variable
QMutex OpenAipPoiLoader::m_mutexAf;
QMutex OpenAipPoiLoader::m_mutexNa;
QMutex OpenAipPoiLoader::m_mutexHs;

OpenAipPoiLoader::OpenAipPoiLoader()
{
}

OpenAipPoiLoader::~OpenAipPoiLoader()
{
}

int OpenAipPoiLoader::load( QList<Airfield>& airfieldList )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutexAf );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QString mapDir = MapContents::instance()->getMapRootDirectory() + "/airfields";
  QStringList preselect;

  // Setup a filter for the desired file extensions.
  QString filter = "*_wpt.aip";

  MapContents::addDir( preselect, mapDir, filter );

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No airfield files found in the map directory!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList files = _settings.value( "/Points/FileList", QStringList(QString("All"))).toStringList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No airfield files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  while( ! preselect.isEmpty() )
    {
      QString srcName;
      OpenAip openAip;
      QString errorInfo;

      srcName = preselect.first();

      // Remove source file to be read from the list.
      preselect.removeAt(0);

      bool ok = openAip.readAirfields( srcName,
				       airfieldList,
				       errorInfo,
				       true );
      if( ok )
	{
	  loadCounter++;
	}
    }

  qDebug( "OAIP: %d airfield file(s) with %d items loaded in %dms",
          loadCounter, airfieldList.size(), t.elapsed() );

  return loadCounter;
}

int OpenAipPoiLoader::load( QList<RadioPoint>& navaidsList )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutexNa );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QString mapDir = MapContents::instance()->getMapRootDirectory() + "/airfields";
  QStringList preselect;

  // Setup a filter for the desired file extensions.
  QString filter = "*_nav.aip";

  MapContents::addDir( preselect, mapDir, filter );

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No navaid files found in the map directory!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList files = _settings.value( "/Points/FileList", QStringList(QString("All"))).toStringList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No navaid files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  while( ! preselect.isEmpty() )
    {
      QString srcName;
      OpenAip openAip;
      QString errorInfo;

      srcName = preselect.first();

      // Remove source file to be read from the list.
      preselect.removeAt(0);

      bool ok = openAip.readNavAids( srcName,
                                     navaidsList,
				     errorInfo,
				     true );

      if( ok )
	{
	  loadCounter++;
	}
    }

  qDebug( "OAIP: %d navaid file(s) with %d items loaded in %dms",
          loadCounter, navaidsList.size(), t.elapsed() );

  return loadCounter;
}

int OpenAipPoiLoader::load( QList<SinglePoint>& hotspotList )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutexHs );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QString mapDir = MapContents::instance()->getMapRootDirectory() + "/airfields";
  QStringList preselect;

  // Setup a filter for the desired file extensions.
  QString filter = "*_hot.aip";

  MapContents::addDir( preselect, mapDir, filter );

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No hotspot files found in the map directory!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList files = _settings.value( "/Points/FileList", QStringList(QString("All"))).toStringList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No hotspot files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  while( ! preselect.isEmpty() )
    {
      QString srcName;
      OpenAip openAip;
      QString errorInfo;

      srcName = preselect.first();

      // Remove source file to be read from the list.
      preselect.removeAt(0);

      bool ok = openAip.readHotspots( srcName,
                                      hotspotList,
				      errorInfo,
				      true );

      if( ok )
	{
	  loadCounter++;
	}
    }

  qDebug( "OAIP: %d hotspot file(s) with %d items loaded in %dms",
          loadCounter, hotspotList.size(), t.elapsed() );

  return loadCounter;
}
