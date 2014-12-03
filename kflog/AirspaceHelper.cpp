/***********************************************************************
**
**   AirspaceHelper.cpp
**
**   Created on: 03.02.2014
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

#include "AirspaceHelper.h"
#include "mapcontents.h"
#include "OpenAip.h"
#include "openairparser.h"
#include "resource.h"

extern QSettings _settings;

QMap<QString, BaseMapElement::objectType> AirspaceHelper::m_airspaceTypeMap;

QSet<int> AirspaceHelper::m_airspaceDictionary;

QMutex AirspaceHelper::m_mutex;

int AirspaceHelper::loadAirspaces( QList<Airspace>& list )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QTime t; t.start();
  uint loadCounter = 0; // number of successfully loaded files

  m_airspaceDictionary.clear();

  QString mapDir = MapContents::instance()->getMapRootDirectory() + "/airspaces";
  QStringList preselect;

  // Setup a filter for the desired file extensions.
  QString filter = "*.txt *.TXT *.aip";

  MapContents::addDir( preselect, mapDir, filter );

  if( preselect.count() == 0 )
    {
      qWarning( "ASH: No Airspace files found in the map directory!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList files = _settings.value( "/Airspace/FileList", QStringList(QString("All"))).toStringList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "ASH: No Airspace files defined for loading!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Check for desired files to be loaded. All other items are removed from
      // the files list.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  OpenAirParser oap;
  OpenAip oaip;
  QString errorInfo;

  while( ! preselect.isEmpty() )
    {
      QString srcName;

      if( preselect.first().endsWith(QString(".TXT")) ||
	  preselect.first().endsWith(QString(".txt")) )
        {
          srcName = preselect.first();

          if( oap.parse(srcName, list) )
            {
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }

      if( preselect.first().endsWith(QString(".aip")) )
        {
          // there can't be the same name aic after this aip
          // parse found aip file
          srcName = preselect.first();

          if( oaip.readAirspaces(srcName, list, errorInfo ) )
            {
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }

    } // End of While

  qDebug("ASH: %d Airspace file(s) loaded in %dms", loadCounter, t.elapsed());

//    for(int i=0; i < list.size(); i++ )
//      {
//        list.at(i)->debug();
//      }

  return loadCounter;
}

void AirspaceHelper::loadAirspaceTypeMapping()
{
  // Creates a mapping from a string representation of the supported
  // airspace types in KFLog to their integer codes
  m_airspaceTypeMap.insert("AirA", BaseMapElement::AirA);
  m_airspaceTypeMap.insert("AirB", BaseMapElement::AirB);
  m_airspaceTypeMap.insert("AirC", BaseMapElement::AirC);
  m_airspaceTypeMap.insert("AirD", BaseMapElement::AirD);
  m_airspaceTypeMap.insert("AirE", BaseMapElement::AirE);
  //m_airspaceTypeMap.insert("AirG", BaseMapElement::AirG);
  m_airspaceTypeMap.insert("WaveWindow", BaseMapElement::WaveWindow);
  m_airspaceTypeMap.insert("AirF", BaseMapElement::AirF);
  m_airspaceTypeMap.insert("AirFIR", BaseMapElement::AirFir);
  m_airspaceTypeMap.insert("ControlC", BaseMapElement::ControlC);
  m_airspaceTypeMap.insert("ControlD", BaseMapElement::ControlD);
  m_airspaceTypeMap.insert("Danger", BaseMapElement::Danger);
  m_airspaceTypeMap.insert("Restricted", BaseMapElement::Restricted);
  m_airspaceTypeMap.insert("Prohibited", BaseMapElement::Prohibited);
  m_airspaceTypeMap.insert("LowFlight", BaseMapElement::LowFlight);
  m_airspaceTypeMap.insert("Rmz", BaseMapElement::Rmz);
  m_airspaceTypeMap.insert("Tmz", BaseMapElement::Tmz);
  m_airspaceTypeMap.insert("GliderSector", BaseMapElement::GliderSector);
  m_airspaceTypeMap.insert("AirUkn", BaseMapElement::AirUkn);
}

QMap<QString, BaseMapElement::objectType>
AirspaceHelper::initializeAirspaceTypeMapping(const QString& mapFilePath)
{
  QMap<QString, BaseMapElement::objectType> typeMap;

  QFileInfo fi(mapFilePath);

  if( fi.suffix().toLower() == "txt" )
    {
      // OpenAir default airspace mapping
      typeMap.insert("A", BaseMapElement::AirA);
      typeMap.insert("B", BaseMapElement::AirB);
      typeMap.insert("C", BaseMapElement::AirC);
      typeMap.insert("D", BaseMapElement::AirD);
      typeMap.insert("E", BaseMapElement::AirE);
      typeMap.insert("F", BaseMapElement::AirF);
      typeMap.insert("UKN", BaseMapElement::AirUkn);
      typeMap.insert("GP", BaseMapElement::Restricted);
      typeMap.insert("R", BaseMapElement::Restricted);
      typeMap.insert("P", BaseMapElement::Prohibited);
      typeMap.insert("TRA", BaseMapElement::Restricted);
      typeMap.insert("Q", BaseMapElement::Danger);
      typeMap.insert("CTR", BaseMapElement::ControlD);
      typeMap.insert("RMZ", BaseMapElement::Rmz);
      typeMap.insert("TMZ", BaseMapElement::Tmz);
      typeMap.insert("W", BaseMapElement::WaveWindow);
      typeMap.insert("GSEC", BaseMapElement::GliderSector);
    }
  else if( fi.suffix().toLower() == "aip" )
    {
      // OpenAip default airspace mapping
      typeMap.insert("A", BaseMapElement::AirA);
      typeMap.insert("B", BaseMapElement::AirB);
      typeMap.insert("C", BaseMapElement::AirC);
      typeMap.insert("D", BaseMapElement::AirD);
      typeMap.insert("E", BaseMapElement::AirE);
      typeMap.insert("F", BaseMapElement::AirF);
      typeMap.insert("FIR", BaseMapElement::AirFir);
      typeMap.insert("CTR", BaseMapElement::ControlD);
      typeMap.insert("DANGER", BaseMapElement::Danger);
      typeMap.insert("RESTRICTED", BaseMapElement::Restricted);
      typeMap.insert("PROHIBITED", BaseMapElement::Prohibited);
      typeMap.insert("RMZ", BaseMapElement::Rmz);
      typeMap.insert("TMA", BaseMapElement::ControlD);
      typeMap.insert("TMZ", BaseMapElement::Tmz);
      typeMap.insert("GLIDING", BaseMapElement::GliderSector);
      typeMap.insert("WAVE", BaseMapElement::WaveWindow);
      typeMap.insert("OTH", BaseMapElement::AirUkn);
    }
  else
    {
      qWarning() << "ASH::initializeAirspaceTypeMapping failed, unknown file suffix"
                 << fi.suffix();
      return typeMap;
    }

  QStringList fList;

  // Default user airspace type mapping file
  fList << fi.path() + "/airspace_mappings.conf";

  // User airspace type mapping file only for the current airspace file
  fList << fi.path() + "/" + fi.baseName() + "_mappings.conf";

  for (int i = 0; i < fList.size(); i++)
    {
      fi.setFile(fList.at(i));

      if (fi.exists() && fi.isFile() && fi.isReadable())
        {
          QFile f(fList.at(i));

          if (!f.open(QIODevice::ReadOnly))
            {
              qWarning() << "ASH: Cannot open airspace mapping file" << fList.at(i)
                         << "!";
              return typeMap;
            }

          QTextStream in(&f);
          qDebug() << "Parsing mapping file" << fList.at(i);

          // start parsing
          while (!in.atEnd())
            {
              QString line = in.readLine().simplified();

              if (line.startsWith("*") || line.startsWith("#")
                  || line.isEmpty())
                {
                  continue;
                }

              int pos = line.indexOf("=");

              if (pos > 1 && pos < line.length() - 1)
                {
                  QString key = line.left(pos).simplified();
                  QString value = line.mid(pos + 1).simplified();

                  if (key.isEmpty() || value.isEmpty())
                    {
                      continue;
                    }

                  if (isAirspaceBaseTypeKnown(value) == false)
                    {
                      continue;
                    }

                  qDebug() << "ASH: Airspace type mapping changed by user:"
                           << key << "-->" << value;

                  typeMap.remove(key);
                  typeMap.insert(key, mapAirspaceBaseType(value));
                }
            }

          f.close();
        }
    }

  return typeMap;
}

/*---------------------- AirspaceHelperThread --------------------------------*/

#include <csignal>

AirspaceHelperThread::AirspaceHelperThread( QObject *parent ) :
  QThread( parent )
{
  setObjectName( "AirspaceHelperThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

AirspaceHelperThread::~AirspaceHelperThread()
{
}

void AirspaceHelperThread::run()
{
#ifndef WIN32
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );
#endif

  // Check is signal is connected to a slot.
  if( receivers( SIGNAL( loadedList( int, SortableAirspaceList* )) ) == 0 )
    {
      qWarning() << "AirspaceHelperThread: No Slot connection to Signal loadedList!";
      return;
    }

  SortableAirspaceList* airspaceList = new SortableAirspaceList;

  int ok = AirspaceHelper::loadAirspaces( *airspaceList );

  /* It is expected that a receiver slot is connected to this signal. The
   * receiver is responsible to delete the passed lists. Otherwise a big
   * memory leak will occur.
   */
  emit loadedList( ok, airspaceList );
}
