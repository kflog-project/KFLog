/***********************************************************************
**
**   openaipairfieldloader.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2013-2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "mapcontents.h"
#include "OpenAip.h"
#include "openaipairfieldloader.h"

extern QSettings _settings;

// set static member variable
QMutex OpenAipAirfieldLoader::m_mutex;

OpenAipAirfieldLoader::OpenAipAirfieldLoader()
{
}

OpenAipAirfieldLoader::~OpenAipAirfieldLoader()
{
}

int OpenAipAirfieldLoader::load( QList<Airfield>& airfieldList )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QString mapDir = MapContents::instance()->getMapRootDirectory() + "/airfields";
  QStringList preselect;

  // Setup a filter for the desired file extensions.
  QString filter = "*.aip";

  MapContents::addDir( preselect, mapDir, filter );

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No airfield files found in the map directory!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList files = _settings.value( "/Airfield/FileList", QStringList(QString("All"))).toStringList();

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

/*------------------------- OpenAipThread ------------------------------------*/

#include <csignal>

OpenAipThread::OpenAipThread( QObject *parent ) :
  QThread( parent )
{
  setObjectName( "OpenAipThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

OpenAipThread::~OpenAipThread()
{
}

void OpenAipThread::run()
{
#ifndef WIN32
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );
#endif

  // Check is signal is connected to a slot.
  if( receivers( SIGNAL( loadedList( int, QList<Airfield>* )) ) == 0 )
    {
      qWarning() << "OpenAipThread: No Slot connection to Signal loadedList!";
      return;
    }

  QList<Airfield>* airfieldList = new QList<Airfield>;

  OpenAipAirfieldLoader oaipl;

  int ok = oaipl.load( *airfieldList );

  /* It is expected that a receiver slot is connected to this signal. The
   * receiver is responsible to delete the passed lists. Otherwise a big
   * memory leak will occur.
   */
  emit loadedList( ok, airfieldList );
}
