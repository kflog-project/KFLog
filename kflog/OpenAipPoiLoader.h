/***********************************************************************
**
**   OpenAipPoiLoader.h
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

/**
 * \class OpenAipPoiLoader
 *
 * \author Axel Pauli
 *
 * \brief A class for reading point data from openAIP XML files.
 *
 * A class for reading point data from OpenAIP XML files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2014
 *
 * \version 1.0
 */

#ifndef OpenAip_Poi_Loader_h_
#define OpenAip_Poi_Loader_h_

#include <QList>
#include <QMutex>

#include "airfield.h"
#include "radiopoint.h"
#include "singlepoint.h"

class OpenAipPoiLoader
{
 public:

  OpenAipPoiLoader();

  virtual ~OpenAipPoiLoader();

  /**
   * Searches on default places openAIP airfield files and load them. The
   * results are appended to the passed list.
   *
   * \param airfieldList All read airfields have to be appended to this list.
   *
   * \return number of loaded airfield files
   */
  int load( QList<Airfield>& airfieldList );

  /**
   * Searches on default places openAIP navAids files and load them. The
   * results are appended to the passed list.
   *
   * \param navaidList All read navaids have to be appended to this list.
   *
   * \return number of loaded navaid files
   */
  int load( QList<RadioPoint>& navaidList );

  /**
   * Searches on default places openAIP navAids files and load them. The
   * results are appended to the passed list.
   *
   * \param hotspotList All read hotspots have to be appended to this list.
   *
   * \return number of loaded hotspot files
   */
  int load( QList<SinglePoint>& hotspotList );

 private:

  /** Mutex to ensure thread safety. */
  static QMutex m_mutexAf;
  static QMutex m_mutexNa;
  static QMutex m_mutexHs;
};

#endif /* OpenAip_Poi_Loader_h_ */
