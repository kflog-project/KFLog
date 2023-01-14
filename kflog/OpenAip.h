/***********************************************************************
**
**   OpenAip.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2013-2023 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class OpenAip
 *
 * \author Axel Pauli
 *
 * \brief A class for reading aeronautical data from openAIP files.
 *
 * A class for reading data from openAIP Json files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2013-2023
 *
 * \version 1.2
 */

#pragma once

#include <QList>
#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QJsonDocument>

#include "airfield.h"
#include "airspace.h"
#include "altitude.h"
#include "Frequency.h"
#include "radiopoint.h"
#include "ThermalPoint.h"

class OpenAip
{
 public:

  OpenAip();

  virtual ~OpenAip();

  Q_DISABLE_COPY( OpenAip )

  /**
   * Reads in a navigation aid file provided as openAIP json format.
   *
   * \param filename File containing navigation aid definitions
   *
   * \param navAidList List in which the read data are stored
   *
   * \param errorInfo Info about read errors
   *
   * \param useFiltering If enabled, different filter rules will apply
   *
   * \return true as success otherwise false
   */
  bool readNavAids( QString fileName,
                    QList<RadioPoint>& navAidList,
                    QString& errorInfo,
                    bool useFiltering=false );

  /**
   * Reads in a hotspot file provided as openAIP json format.
   *
   * \param filename File containing hotspot definitions
   *
   * \param hotspotList List in which the read data are stored
   *
   * \param errorInfo Info about read errors
   *
   * \param useFiltering If enabled, different filter rules will apply
   *
   * \return true as success otherwise false
   */
  bool readHotspots( QString fileName,
                     QList<ThermalPoint>& hotspotList,
                     QString& errorInfo,
                     bool useFiltering=false );

  /**
   * Reads in a single point file provided as openAIP json format.
   *
   * \param filename File containing json definitions
   *
   * \param type Type to be set for the single point
   *
   * \param spList List in which the read data are stored
   *
   * \param errorInfo Info about read errors
   *
   * \param useFiltering If enabled, different filter rules will apply
   *
   * \return true as success otherwise false
   */
  bool readSinglePoints( QString fileName,
                         int type,
                         QList<SinglePoint>& spList,
                         QString& errorInfo,
                         bool useFiltering=false );

  /**
   * Reads in an airfield file provided as openAIP json format.
   *
   * \param filename File containing airfield definitions
   *
   * \param airfieldList List in which the read data are stored
   *
   * \param errorInfo Info about read errors
   *
   * \param useFiltering If enabled, different filter rules will apply
   *
   * \return true as success otherwise false
   */
  bool readAirfields( QString fileName,
                      QList<Airfield>& airfieldList,
                      QString& errorInfo,
                      bool useFiltering=false );

  /**
   * Reads in an airspace file provided as openAIP json format.
   *
   * \param filename File containing airspace definitions
   *
   * \param airspaceList List in which the read airspaces are stored
   *
   * \param errorInfo Info about read errors
   *
   * \return true on success otherwise false
   */
  bool readAirspaces( QString fileName,
  										QList<Airspace>& airspaceList,
                      QString& errorInfo );

  /**
   * Upper and lower the words in the passed string.
   *
   * \param name The name to be processed.
   */
  void upperLowerName( QString& name );

  /**
   * Create an unique short name by removing undesired characters.
   *
   * \param name The name to be shorten.
   *
   * \return new short name 8 characters long
   */
  QString shortName( const QString& name );

  /**
   * \return A reference to the short name set.
   */
  QSet<QString>& getShortNameSet()
    {
      return m_shortNameSet;
    };

 private:

  /**
   * Read and set the frequency value from a navaid json object.
   *
   * @param object
   * @return float
   */
  Frequency getJNavaidFrequency( QJsonObject& object );

  /**
   * Read and set the range value from a navaid json object.
   *
   * @param object
   * @return float
   */
  int getJNavaidRange( QJsonObject& object );

  /**
   * Read and return the elevation value from a json object as meters.
   *
   * @param object
   * @return float
   */
  float getJElevation( QJsonObject& object );

  /**
   * Read and set the coordinates from a json object.
   */
  bool setJGeoLocation( QJsonObject& object, SinglePoint& sp );

  /**
   * Sets the airfield type.
   */
  bool setJAirfieldType( const int type, Airfield& af );

  /**
   * Read and set Json freqeuncy data of an airfield.
   */
  void setJAirfieldFrequencies( QJsonArray& array, Airfield& af );

  /**
   * Read and set Json runway data of an airfield.
   */
  void setJAirfieldRunways( QJsonArray& array, Airfield& af );

  /**
   * Read and set Json runway dimension data of an airfield.
   */
  void setJAirfieldRunwayDimensions( const QJsonObject& object, Runway& rw );

  /**
   * Read and set Json runway surface data of an airfield.
   */
  void setJAirfieldRunwaySurface( const QJsonObject& object, Runway& rw );

  /**
   * Reads the airspace limit data.
   */
 bool readJAirspaceLimit( const QString& asName,
                           const QJsonObject& object,
                           BaseMapElement::elevationType& reference,
                           Altitude& altitude );
 /**
  * Reads the airspace coordinates data.
  */
  bool readJAirspaceGeometrie( const QJsonObject& object, Airspace& as );

  /**
   * Loads the user's defined filter values from the configuration data.
   */
  void loadUserFilterValues();

  /**
   * Fills the mapping hash with the runway surface mapping data.
   */
  void fillRunwaySurfaceMapper();

  /**
   * Check if point is inside a certain radius.
   *
   * \return true, if point is inside other wise false
   */
  bool checkRadius( WGSPoint* point );

  /**
   * Country filter with countries as two letter code in upper case.
   */
  QSet<QString> m_countryFilterSet;

  /**
   * Home position, used as center point for radius filter.
   */
  QPoint m_homePosition;

  /**
   * Radius in Km to be used for filtering around the home position.
   *  If set to <= 0, the radius filter is ignored.
   */
  double m_filterRadius;

  /**
   * Runway length filter in meters. If set to <= 0, the filter is switched off.
   */
  float m_filterRunwayLength;

  /**
   * Contains an airspace type mapping between read item and related Cumulus
   * airspace item.
   */
  QMap<QString, BaseMapElement::objectType> m_airspaceTypeMapper;

  /**
   * Contains an airspace type mapping between read item and related Cumulus
   * airspace item.
   */
  QHash<int, Runway::SurfaceType> m_runwaySurfaceMapper;

  /** Contains all short names of parsed file. */
  QSet<QString> m_shortNameSet;
};
