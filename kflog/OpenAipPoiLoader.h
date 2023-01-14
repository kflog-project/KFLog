/***********************************************************************
**
**   OpenAipPoiLoader.h
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

/**
 * \class OpenAipPoiLoader
 *
 * \author Axel Pauli
 *
 * \brief A class for reading point data from openAIP Json files.
 *
 * A class for reading point data from OpenAIP Json files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2014-2023
 *
 * \version 1.1
 */

#pragma once

#include <QList>
#include <QMutex>

#include "airfield.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "ThermalPoint.h"

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
   * Searches on default places openAIP hotspot files and load them. A source
   * can be an original Json file or a compiled version of it. The
   * results are appended to the passed list.
   *
   * \param hotspotList All read hotspots have to be appended to this list.
   *
   * \param readSource If true the source files have to be read instead of
   * compiled sources.
   *
   * \return number of loaded hotspot files
   */
  int load( QList<ThermalPoint>& hotspotList );

  /**
   * Searches on default places openAIP files according to the given filter
   * string and load them. A source can be an original Json file or a compiled
   * version of it. The results are appended to the passed list.
   *
   * \param filter Files to be loaded as wildcard definition.
   *
   * \param type Type of single point
   *
   * \param spList All read single points have to be appended to this list.
   *
   * \param readSource If true the source files have to be read instead of
   * compiled sources.
   *
   * \return number of loaded files
   */
  int load( QString filter,
            int type,
            QList<SinglePoint>& spList );

 private:

  /** Mutex to ensure thread safety. */
  static QMutex m_mutexAf;
  static QMutex m_mutexNa;
  static QMutex m_mutexHs;
  static QMutex m_mutexSp;
};
