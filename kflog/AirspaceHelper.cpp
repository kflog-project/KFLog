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
**   Copyright (c):  2014-2023 by Axel Pauli <kflog.cumulus@gmail.com>
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

QSet<QString> AirspaceHelper::m_airspaceDictionary;

QMap<quint16, BaseMapElement::objectType> AirspaceHelper::m_icaoClassMap;

QMutex AirspaceHelper::m_mutex;

int AirspaceHelper::loadAirspaces( QList<Airspace>& list )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t;
  t.start();
  uint loadCounter = 0; // number of successfully loaded files

  m_airspaceDictionary.clear();

  QString mapDir = MapContents::instance()->getMapRootDirectory() + "/airspaces";
  QStringList preselect;

  // Setup a filter for the desired file extensions.
  QString filter = "*.txt *.TXT *.json *.JSON";

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

      if( preselect.first().toUpper().endsWith( QString(".TXT") ) )
        {
          srcName = preselect.first();

          if( oap.parse( srcName, list ) )
            {
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }

      if( preselect.first().toLower().endsWith(QString(".json")) )
        {
          srcName = preselect.first();

          if( oaip.readAirspaces( srcName, list, errorInfo ) )
            {
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }
    } // End of While

  qDebug("ASH: %d Airspace file(s) loaded in %lldms", loadCounter, t.elapsed());

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
  m_airspaceTypeMap.insert("AirG", BaseMapElement::AirG);
  m_airspaceTypeMap.insert("WaveWindow", BaseMapElement::WaveWindow);
  m_airspaceTypeMap.insert("AirF", BaseMapElement::AirF);
  m_airspaceTypeMap.insert("AirFIR", BaseMapElement::AirFir);
  m_airspaceTypeMap.insert("Ctr", BaseMapElement::Ctr);
  m_airspaceTypeMap.insert("Danger", BaseMapElement::Danger);
  m_airspaceTypeMap.insert("Restricted", BaseMapElement::Restricted);
  m_airspaceTypeMap.insert("Prohibited", BaseMapElement::Prohibited);
  m_airspaceTypeMap.insert("SUA", BaseMapElement::Sua);
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
      typeMap.insert("G", BaseMapElement::AirG);
      typeMap.insert("UKN", BaseMapElement::AirUkn);
      typeMap.insert("GP", BaseMapElement::Restricted);
      typeMap.insert("R", BaseMapElement::Restricted);
      typeMap.insert("P", BaseMapElement::Prohibited);
      typeMap.insert("TRA", BaseMapElement::Restricted);
      typeMap.insert("Q", BaseMapElement::Danger);
      typeMap.insert("CTR", BaseMapElement::Ctr);
      typeMap.insert("RMZ", BaseMapElement::Rmz);
      typeMap.insert("TMZ", BaseMapElement::Tmz);
      typeMap.insert("W", BaseMapElement::WaveWindow);
      typeMap.insert("GSEC", BaseMapElement::GliderSector);
    }
  else if( fi.suffix().toLower() == "json" )
    {
      // OpenAip default airspace mapping
      typeMap.insert("0", BaseMapElement::AirUkn);
      typeMap.insert("1", BaseMapElement::Restricted);
      typeMap.insert("2", BaseMapElement::Danger);
      typeMap.insert("3", BaseMapElement::Prohibited);
      typeMap.insert("4", BaseMapElement::Ctr);
      typeMap.insert("5", BaseMapElement::Tmz);
      typeMap.insert("6", BaseMapElement::Rmz);
      typeMap.insert("7", BaseMapElement::Sua); // TMA
      typeMap.insert("8", BaseMapElement::Sua); // TRA
      typeMap.insert("9", BaseMapElement::Sua); // TSA
      typeMap.insert("10", BaseMapElement::AirFir);
      typeMap.insert("11", BaseMapElement::Sua); // UIR
      typeMap.insert("12", BaseMapElement::Sua); // ADIZ
      typeMap.insert("13", BaseMapElement::Sua); // ATZ
      typeMap.insert("14", BaseMapElement::Sua); // Alert Area
      typeMap.insert("15", BaseMapElement::Sua); // Airway
      typeMap.insert("16", BaseMapElement::Sua); // Military Training Route (MTR)
      typeMap.insert("17", BaseMapElement::Sua); // Alert Area
      typeMap.insert("18", BaseMapElement::Sua); // Warning Area
      typeMap.insert("19", BaseMapElement::Restricted); // Protected Area
      typeMap.insert("20", BaseMapElement::Sua); // Helicopter Traffic Zone (HTZ)
      typeMap.insert("21", BaseMapElement::GliderSector);
      typeMap.insert("22", BaseMapElement::Tmz); // Transponder Setting (TRP)
      typeMap.insert("23", BaseMapElement::AirFir); // Traffic Information Zone (TIZ)
      typeMap.insert("24", BaseMapElement::AirFir); // Traffic Information Area (TIA)
      typeMap.insert("25", BaseMapElement::Sua); // Military Training Area (MTA)
      typeMap.insert("26", BaseMapElement::Ctr); // Controlled Area (CTA)
      typeMap.insert("27", BaseMapElement::Sua); // ACC Sector (ACC)
      typeMap.insert("28", BaseMapElement::Sua); // Aerial Sporting Or Recreational Activity
      typeMap.insert("29", BaseMapElement::Sua); // Low Altitude Overflight Restriction
      typeMap.insert("30", BaseMapElement::Sua); // MRT Military Route (used in Poland)
      typeMap.insert("31", BaseMapElement::Sua); // TSA/TRA Feeding Route (TFR) (used in Poland)
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
          qDebug() << "ASH: Parsing mapping file" << fList.at(i);

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

/**
 * Initialize a mapping from an openAIP ICAO class integer to the related
 * Cumulus integer type.
 */
void AirspaceHelper::loadIcaoClassMapping()
{
  m_icaoClassMap.clear();
  m_icaoClassMap.insert( 0, BaseMapElement::AirA );
  m_icaoClassMap.insert( 1, BaseMapElement::AirB );
  m_icaoClassMap.insert( 2, BaseMapElement::AirC );
  m_icaoClassMap.insert( 3, BaseMapElement::AirD );
  m_icaoClassMap.insert( 4, BaseMapElement::AirE );
  m_icaoClassMap.insert( 5, BaseMapElement::AirF );
  m_icaoClassMap.insert( 6, BaseMapElement::AirG );
  m_icaoClassMap.insert( 8, BaseMapElement::Sua );
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
