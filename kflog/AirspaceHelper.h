/***********************************************************************
**
**   AirspaceHelper.h
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
**   $Id$
**
***********************************************************************/

/**
 * \class AirspaceHelper
 *
 * \author Axel Pauli
 *
 * \brief Helper class for airspaces
 *
 * This class contains methods for airspace parsing and loading.
 * Two kinds of airspace source files are supported.
 *
 * <ul>
 * <li>OpenAir format, an ASCII text description of the airspaces</li>
 * <li>OpenAIP format, a XML description of the airspaces</li>
 * </ul>
 *
 * \date 2014
 *
 * \version $Id$
 */

#ifndef AIRSPACE_HELPER_H
#define AIRSPACE_HELPER_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QSet>
#include <QString>

#include "airspace.h"
#include "basemapelement.h"

class AirspaceHelper
{
  public:

  /**
   * Constructor
   */
  AirspaceHelper()
  {
  };

  /**
   * Destructor
   */
  virtual ~AirspaceHelper()
  {
  };

  /**
   * Searches on a default place for OpenAir and OpenAip airspace files.
   *
   * @returns The number of successfully loaded files
   *
   * @param list The list where the Airspace objects should be added from the
   *        read files.
   */
  static int loadAirspaces( QList<Airspace>& list );

  /**
   * Initialize a mapping from an airspace type string to the KFLog integer type.
   */
  static QMap<QString, BaseMapElement::objectType>
         initializeAirspaceTypeMapping(const QString& path);

  /**
   * Checks, if a KFLog airspace type name is contained in the airspace
   * type map.
   *
   * \return True in case of success otherwise false
   */
  static bool isAirspaceBaseTypeKnown( const QString& type )
  {
    if( m_airspaceTypeMap.isEmpty() )
      {
        loadAirspaceTypeMapping();
      }

    return m_airspaceTypeMap.contains( type );
  }

  /**
   * Maps a KFLog airspace type name to its integer code.
   *
   * \param type airspace base type name
   *
   * \return short airspace type name or empty string, if mapping fails
   */
  static BaseMapElement::objectType mapAirspaceBaseType( const QString& type )
  {
    if( m_airspaceTypeMap.isEmpty() )
      {
        loadAirspaceTypeMapping();
      }

    return m_airspaceTypeMap.value( type, BaseMapElement::AirUkn );
  }

  /**
   * Reports, if a airspace object is already loaded or not.
   *
   * \param id airspace identifier
   *
   * \return true in case of contained otherwise false
   */
  static bool isAirspaceKnown( const int id )
  {
    return m_airspaceDictionary.contains( id );
  };

  /**
   * Adds an airspace identifier to the airspace dictionary.
   *
   * \param id airspace identifier
   *
   * \return true in case of added otherwise false
   */
  static bool addAirspaceIdentifier( const int id )
  {
    if( m_airspaceDictionary.contains( id ) )
      {
        return false;
      }

    m_airspaceDictionary.insert(id);
    return true;
  }

 private:

  /**
   * Creates a mapping from a string representation of the supported
   * airspace types in KFLog to their integer codes.
   *
   * \return Map of airspace items as key value pair
   */
  static void loadAirspaceTypeMapping();

  /**
   */
  static QMap<QString, BaseMapElement::objectType> m_airspaceTypeMap;

  /**
   * Contains all read airspaces to avoid duplicates.
   */
  static QSet<int> m_airspaceDictionary;

  /** Mutex to ensure thread safety. */
  static QMutex m_mutex;
};

/******************************************************************************/

#include <QThread>

/**
* \class AirspaceHelperThread
*
* \author Axel Pauli
*
* \brief Class to read OpenAIP airspace data files in an extra thread.
*
* This class can read and parse openAIP airspace data files and store
* their content in a binary format. All work is done in an extra thread.
* The results are returned via the signal \ref loadedList.
*
* \date 2014
*
* \version $Id$
*/

class AirspaceHelperThread : public QThread
{
  Q_OBJECT

 public:

  AirspaceHelperThread( QObject *parent=0 );

  virtual ~AirspaceHelperThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 signals:

  /**
  * This signal emits the results of the OpenAIP load. The receiver slot is
  * responsible to delete the dynamic allocated list in every case.
  *
  * \param loadedLists     The number of loaded lists
  * \param airspaceList    The list with the airspace data
  *
  */
  void loadedList( int loadedLists, SortableAirspaceList* airspaceList );
};

#endif /* AIRSPACE_HELPER_H */
