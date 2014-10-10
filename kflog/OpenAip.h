/***********************************************************************
**
**   OpenAip.h
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
***********************************************************************/

/**
 * \class OpenAip
 *
 * \author Axel Pauli
 *
 * \brief A class for reading data from openAIP XML files.
 *
 * A class for reading data from openAIP XML files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2013-2014
 *
 * \version $Id$
 */

#ifndef OpenAip_h
#define OpenAip_h

#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QXmlStreamReader>

#include "airfield.h"
#include "airspace.h"
#include "altitude.h"
#include "radiopoint.h"

class OpenAip
{
 public:

  OpenAip();

  virtual ~OpenAip();

  /**
   * Opens the passed file and looks, which kind of OpenAip data is provided.
   *
   * \param filename File containing OpenAip XML data definitions.
   *
   * \param dataFormat The OpenAip DATAFORMAT attribute
   *
   * \param dataItem The second root element of the file after the OPENAIP tag.
   *
   * \return true as success otherwise false
   */
  bool getRootElement( QString fileName, QString& dataFormat, QString& dataItem );

  /**
   * Reads in a navigation aid file provided as open aip xml format.
   *
   * \param filename File containing navigation aid definitions
   *
   * \param errorInfo Info about read errors
   *
   * \param useFiltering If enabled, filter rules will apply
  *
   * \return true as success otherwise false
   */
  bool readNavAids( QString fileName,
                    QList<RadioPoint>& navAidList,
                    QString& errorInfo,
                    bool useFiltering=false );

  /**
   * Reads in a hotspot file provided as open aip xml format.
   *
   * \param filename File containing hotspot definitions
   *
   * \return true as success otherwise false
   */
  bool readHotspots( QString fileName, QList<SinglePoint>& hotspotList, QString& errorInfo );

  /**
   * Reads in an airfield file provided as open aip xml format.
   *
   * \param filename File containing airfield definitions
   *
   * \param airfieldList List in which the read data are stored
   *
   * \param errorInfo Info about read errors
   *
   * \param useFiltering If enabled, filter rules will apply
   *
   * \return true as success otherwise false
   */
  bool readAirfields( QString fileName,
                      QList<Airfield>& airfieldList,
                      QString& errorInfo,
                      bool useFiltering=false );

  /**
   * Reads in an airspace file provided as open aip xml format.
   *
   * \param filename File containing airspace definitions
   *
   * \param airspaceList List in which the read airspaces are stored
   *
   * \param errorInfo Info about read errors
   *
   * \return true on success otherwise false
   */
  bool readAirspaces( QString fileName, QList<Airspace>& airspaceList,
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
   * Read version and format attribute from OPENAIP tag. Returns true in case
   * of success otherwise false.
   */
  bool readVersionAndFormat( QXmlStreamReader& xml,
                             QString& version,
                             QString& format );

  bool readNavAidRecord( QXmlStreamReader& xml, RadioPoint& rp );

  bool readGeoLocation( QXmlStreamReader& xml, SinglePoint& sp );

  bool readRadio( QXmlStreamReader& xml, RadioPoint& rp );

  bool readParams( QXmlStreamReader& xml, RadioPoint& rp );

  bool readHotspotRecord( QXmlStreamReader& xml, SinglePoint& sp );

  bool readAirfieldRecord( QXmlStreamReader& xml, Airfield& af );

  bool readAirfieldRadio( QXmlStreamReader& xml, Airfield& af );

  /**
   * Read runway data from data format 1.0.
   */
  bool readAirfieldRunway10( QXmlStreamReader& xml, Airfield& af );

  /**
   * Read runway data from data format 1.1.
   */
  bool readAirfieldRunway11( QXmlStreamReader& xml, Airfield& af );

  /**
   * Read a complete airspace record.
   */
  bool readAirspaceRecord( QXmlStreamReader& xml, Airspace& as );

  bool readAirspaceLimitReference( QXmlStreamReader& xml,
                                   BaseMapElement::elevationType& reference );

  bool readAirspaceAltitude( QXmlStreamReader& xml,
                             QString& unit,
                             Altitude& altitude );

  bool readAirspaceGeometrie( QXmlStreamReader& xml, Airspace& as );

  /**
   * Converts a string number with unit to an integer value.
   *
   * \param number The number to be converted
   *
   * \param unit The unit of the number, can be "M" or "FT"
   *
   * \param result The calculated integer value
   *
   * \return true in case of success otherwise false
   */
  bool getUnitValueAsInteger( const QString number, const QString unit, int& result );

  /**
   * Converts a string number with unit to an float value.
   *
   * \param number The number to be converted
   *
   * \param unit The unit of the number, can be "M" or "FT"
   *
   * \param result The calculated float value
   *
   * \return true in case of success otherwise false
   */
  bool getUnitValueAsFloat( const QString number, const QString unit, float& result );

  /**
   * Loads the user's defined filter values from the configuration data.
   */
  void loadUserFilterValues();

  /**
   * Containing all supported OpenAip data formats.
   */
  QSet<QString> m_supportedDataFormats;

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
   * Value of VERSION attribute from OPENAIP tag of the current read file.
   */
  QString m_oaipVersion;
  /**
   * Value of DATAFORMAT attribute from OPENAIP tag of the current read file.
   */
  QString m_oaipDataFormat;

  /**
   * Contains an airspace type mapping between read item and related Cumulus
   * airspace item.
   */
  QMap<QString, BaseMapElement::objectType> m_airspaceTypeMapper;

  /** Contains all short names of parsed file. */
  QSet<QString> m_shortNameSet;
};

#endif /* OpenAip_h */
