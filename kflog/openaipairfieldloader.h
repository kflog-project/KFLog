/***********************************************************************
**
**   openaipairfieldloader.h
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

/**
 * \class OpenAipAirfieldLoader
 *
 * \author Axel Pauli
 *
 * \brief A class for reading airfield data from OpenAIP XML files.
 *
 * A class for reading airfield data from OpenAIP XML files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2013-2014
 *
 * \version $Id$
 */

#ifndef OpenAip_Airfield_Loader_h_
#define OpenAip_Airfield_Loader_h_

#include <QList>
#include <QMutex>

#include "airfield.h"
#include "projectionbase.h"

class OpenAipAirfieldLoader
{
 public:

  OpenAipAirfieldLoader();

  virtual ~OpenAipAirfieldLoader();

  /**
   * Searches on default places openAIP airfield files and load them. A source
   * can be an original XML file or a compiled version of it. The
   * results are appended to the passed list.
   *
   * \param airfieldList All read airfields have to be appended to this list.
   *
   * \return number of loaded airfield files
   */
  int load( QList<Airfield>& airfieldList );

 private:

  /** Mutex to ensure thread safety. */
  static QMutex m_mutex;
};

/******************************************************************************/

#include <QThread>

/**
* \class OpenAipThread
*
* \author Axel Pauli
*
* \brief Class to read OpenAIP data files in an extra thread.
*
* This class can read, parse and filter OpenAIP airfield data files and store
* the content in a binary format. All work is done in an extra thread.
* The results are returned via the signal \ref loadedList.
*
* \date 2013-2014
*
* \version $Id$
*/

class OpenAipThread : public QThread
{
  Q_OBJECT

 public:

  OpenAipThread( QObject *parent=0 );

  virtual ~OpenAipThread();

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
  * \param airfieldList    The list with the airfield data
  *
  */
  void loadedList( int loadedLists, QList<Airfield>* airfieldList );

};

#endif /* OpenAip_Airfield_Loader_h_ */
