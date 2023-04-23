/***********************************************************************
**
**   OpenAip.cpp
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

#include <QtCore>

#include "AirspaceHelper.h"
#include "distance.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "OpenAip.h"

extern MapMatrix* _globalMapMatrix;
extern QSettings  _settings;

OpenAip::OpenAip() :
  m_filterRadius(0.0),
  m_filterRunwayLength(0.0)
{
}

OpenAip::~OpenAip()
{
}

void OpenAip::loadUserFilterValues()
{
  m_countryFilterSet.clear();
  m_filterRadius = 0.0;

  m_homePosition = _globalMapMatrix->getHomeCoord();

  QString cFilter = _settings.value( "/Points/Countries", "" ).toString().toUpper();

  QStringList clist = cFilter.split( QRegExp("[, ]"), Qt::SkipEmptyParts );

  for( int i = 0; i < clist.size(); i++ )
    {
      m_countryFilterSet.insert( clist.at(i) );
    }

  // Get filter radius around the home position in kilometers.
  m_filterRadius = _settings.value( "/Points/HomeRadius", 0 ).toDouble();

  // Get runway length filter in meters.
  // m_filterRunwayLength = 0.0;
}

void OpenAip::fillRunwaySurfaceMapper()
{
  m_runwaySurfaceMapper.insert( 0, Runway::Asphalt );
  m_runwaySurfaceMapper.insert( 1, Runway::Concrete );
  m_runwaySurfaceMapper.insert( 2, Runway::Grass );
  m_runwaySurfaceMapper.insert( 3, Runway::Sand );
  m_runwaySurfaceMapper.insert( 4, Runway::Water );
  m_runwaySurfaceMapper.insert( 5, Runway::Asphalt );
  m_runwaySurfaceMapper.insert( 6, Runway::Stone );
  m_runwaySurfaceMapper.insert( 7, Runway::Asphalt );
  m_runwaySurfaceMapper.insert( 8, Runway::Stone );
  m_runwaySurfaceMapper.insert( 9, Runway::Stone );
  m_runwaySurfaceMapper.insert( 10, Runway::Clay );
  m_runwaySurfaceMapper.insert( 11, Runway::Earth );
  m_runwaySurfaceMapper.insert( 12, Runway::Gravel );
  m_runwaySurfaceMapper.insert( 13, Runway::Earth );
  m_runwaySurfaceMapper.insert( 14, Runway::Ice );
  m_runwaySurfaceMapper.insert( 15, Runway::Snow );
  m_runwaySurfaceMapper.insert( 16, Runway::Rubber );
  m_runwaySurfaceMapper.insert( 17, Runway::Metal );
  m_runwaySurfaceMapper.insert( 18, Runway::Metal );
  m_runwaySurfaceMapper.insert( 19, Runway::Metal );
  m_runwaySurfaceMapper.insert( 20, Runway::Wood );
  m_runwaySurfaceMapper.insert( 21, Runway::Earth );
  m_runwaySurfaceMapper.insert( 22, Runway::Unknown );
}

bool OpenAip::readSinglePoints( QString fileName,
                                int type,
                                QList<SinglePoint>& spList,
                                QString& errorInfo,
                                bool useFiltering )
{
  if( useFiltering )
    {
      // Load the user's defined filter data.
      loadUserFilterValues();
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readSinglePoints: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readSinglePoints: cannot open file:" << fileName;
      return false;
    }

  QString content = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8(), &error );

  qDebug() << fileName << "SinglePoints Json Parse result:" << error.errorString();

  if( doc.isNull() == true )
    {
      errorInfo = QObject::tr("Json parser error for file: ") + fileName;
      qWarning() << "OpenAip::readSinglePoints: file"
                 << "'" + fileName + "'"
                 << "Json parse error:"
                 << error.errorString();
      return false;
    }

  if( doc.isArray() == false )
    {
      errorInfo = QObject::tr("Json format error for file: ") + fileName;
      qWarning() << "OpenAip::readSinglePoints: Error, expecting a Json array as first element";
      return false;
   }

  QJsonArray array = doc.array();

  qDebug() << "SinglePoints Doc is a Json Array of size:" << array.size();

  // step over the json array to extract the single point objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();
      QString name;

      if( object.contains("name") && object["name"].isString() )
        {
          name = object["name"].toString();
        }
      else
        {
          // Ignore object without a name
          continue;
        }

      SinglePoint sp;
      sp.setTypeID( static_cast<enum BaseMapElement::objectType>(type) );

      // Set point name
      QString wpName = name;
      upperLowerName( wpName );

      // Long name
      sp.setName( wpName );

      // Short name only 8 characters long
      sp.setWPName( wpName.left(8) );

      // iterate over the single point object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          // qDebug() << "Key: " << it.key() << "Val: " << it.value();

          if( it.key() == "compulsory" )
            {
              sp.setCompulsory( it.value().toBool( false ) );
            }
          else if( it.key() == "country" )
            {
              sp.setCountry( it.value().toString().toUpper() );
            }
          else if( it.key() == "remarks" )
            {
              sp.setComment( it.value().toString() );
            }
          else if( it.key() == "elevation" )
            {
              QJsonObject object = it.value().toObject();
              sp.setElevation( getJElevation( object ) );
            }
          else if( it.key() == "geometry" )
            {
              QJsonObject object = it.value().toObject();

              if( setJGeoLocation( object, sp ) == false )
                {
                  // ignore data set
                  continue;
                }
            }
        } // end of for loop of object list

      if( useFiltering == true &&
          checkRadius( sp.getWGSPositionPtr() ) == false )
        {
          // The radius filter said no. To far away from home.
          continue;
        }

      spList.append( sp );
    }  // end of array for loop

  return true;
}

bool OpenAip::readNavAids( QString fileName,
                           QList<RadioPoint>& navAidList,
                           QString& errorInfo,
                           bool useFiltering )
{
  if( useFiltering )
    {
      // Load the user's defined filter data.
      loadUserFilterValues();
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readNavAids: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readNavAids: cannot open file:" << fileName;
      return false;
    }

  QString content = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8(), &error );

  qDebug() << fileName << "NavAids Json Parse result:" << error.errorString();

  if( doc.isNull() == true )
    {
      errorInfo = QObject::tr("Json parser error for file: ") + fileName;
      qWarning() << "OpenAip::readNavAids: file"
                 << "'" + fileName + "'"
                 << "Json parse error:"
                 << error.errorString();
      return false;
    }

  if( doc.isArray() == false )
    {
      errorInfo = QObject::tr("Json format error for file: ") + fileName;
      qWarning() << "OpenAip::readNavAids: Error, expecting a Json array as first element";
      return false;
   }

  QJsonArray array = doc.array();

  qDebug() << "NavAids Doc is a Json Array of size:" << array.size();

  // step over the json array to extract the thermal objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();
      QString name;

      if( object.contains("name") && object["name"].isString() )
        {
          name = object["name"].toString();
        }
      else
        {
          // Ignore object without a name
          continue;
        }

      RadioPoint rp;
      rp.setName( name );

      // iterate over the navaids object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          // qDebug() << "Key: " << it.key() << "Val: " << it.value();

          if( it.key() == "identifier" )
            {
              rp.setWPName( it.value().toString() );
              // rp.setICAO( it.value().toString() );
            }
          else if( it.key() == "type" )
            {
              int type = it.value().toInt();

              // OpenAip NavAidTypes: dme=0, tacan=1, ndb=2, vor=3, vor_dme=4, vortac=5, dvor=6, dvor_dme=7, dvortac=8
              // Cumulus: Vor = VOR, VorDme = VORDME, VorTac = VORTAC, Ndb = NDB, CompPoint = COMPPOINT

              if( type == 7 /* "DVOR" */ || type == 3 /* "VOR" */ )
                {
                  rp.setTypeID( BaseMapElement::Vor );
                }
              else if( type == 7 /* "DVOR-DME" */ || type == 4 /* "VOR-DME" */ ||
                       type == 0 /* "DME" */ || type == 6 /* "DME" */ )
                {
                  rp.setTypeID( BaseMapElement::VorDme );
                }
              else if( type == 1 ) // "TACAN"
                {
                  rp.setTypeID( BaseMapElement::Tacan );
                }
              else if( type == 8 /*"DVORTAC"*/ || type == 5 /* "VORTAC" */ )
                {
                  rp.setTypeID( BaseMapElement::VorTac );
                }
              else if( type == 2 ) // "NDB"
                {
                  rp.setTypeID( BaseMapElement::Ndb );
                }
              else
                {
                  rp.setTypeID( BaseMapElement::NotSelected );
                  qWarning() << "OpenAip::readNavAidRecord: unknown Navaid type"
                             << type << "for" << name;
                }
            }
          else if( it.key() == "remarks" )
            {
              rp.setComment( it.value().toString() );
            }
          else if( it.key() == "country" )
            {
              rp.setCountry( it.value().toString().toUpper() );
            }
          else if( it.key() == "channel" )
            {
              rp.setChannel( it.value().toString() );
            }
          else if( it.key() == "range" )
            {
              // The range of the navaid. Always 'NM'. We convert it to meters.
              QJsonObject object = it.value().toObject();
              rp.setRange( getJNavaidRange( object ) * Distance::mFromNMile );
           }
          else if( it.key() == "alignedTrueNorth" )
            {
              rp.setRange( it.value().toBool( false ) );
            }
          else if( it.key() == "magneticDeclination" )
            {
              rp.setDeclination( it.value().toDouble() );
            }
          else if( it.key() == "elevation" )
            {
              QJsonObject object = it.value().toObject();
              rp.setElevation( getJElevation( object ) );
            }
          else if( it.key() == "frequency" )
            {
              QJsonObject object = it.value().toObject();
              rp.addFrequency( getJNavaidFrequency( object ) );
            }
          else if( it.key() == "geometry" )
            {
              QJsonObject object = it.value().toObject();

              if( setJGeoLocation( object, rp ) == false )
                {
                  // ignore data set
                  continue;
                }
            }
        } // end of for loop of object list

      if( useFiltering == true &&
          checkRadius( rp.getWGSPositionPtr() ) == false )
        {
          // The radius filter said no. To far away from home.
          continue;
        }

      navAidList.append( rp );
    }  // end of array for loop

  return true;
}

bool OpenAip::setJGeoLocation( QJsonObject& object, SinglePoint& sp )
{
  double lon = INT_MIN;
  double lat = INT_MIN;

  if( object.contains("type") && object["type"].isString() )
    {
      QString value = object["type"].toString();

      if( value != "Point")
        {
          qWarning() << "GeoLocation is not a Point. Read" << value;
          return false;
        }
    }

  if( object.contains("coordinates") && object["coordinates"].isArray() )
    {
      QJsonArray value = object["coordinates"].toArray();

      if( value.size() != 2 )
        {
          qWarning() << "GeoLocation array size != 2. Read" << value.size();
          return false;
        }

      lon = value[0].toDouble( INT_MIN );
      lat = value[1].toDouble( INT_MIN );

      if( lat == INT_MIN || lon == INT_MIN )
        {
          qWarning() << "GeoLocation coordinate fault";
          return false;
        }

      // Convert latitude and longitude into KFLog's internal
      // integer format.
      int ilat = static_cast<int> (rint( 600000.0 * lat ));
      int ilon = static_cast<int> (rint( 600000.0 * lon ));
      WGSPoint wgsPoint( ilat, ilon );
      sp.setWGSPosition( wgsPoint );

      // Map WGS point to map projection
      sp.setPosition( _globalMapMatrix->wgsToMap(wgsPoint) );
      return true;
    }

  return false;
}

bool OpenAip::readHotspots( QString fileName,
                            QList<ThermalPoint>& hotspotList,
                            QString& errorInfo,
                            bool useFiltering )
{
  if( useFiltering )
    {
      // Load the user's defined filter data.
      loadUserFilterValues();
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readHotspots: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readHotspots: cannot open file:" << fileName;
      return false;
    }

  QString content = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8(), &error );

  qDebug() << fileName << "Hotspots Json Parse result:" << error.errorString();

  if( doc.isNull() == true )
    {
      errorInfo = QObject::tr("Json parser error for file: ") + fileName;
      qWarning() << "OpenAip::readHotspots: file"
                 << "'" + fileName + "'"
                 << "Json parse error:"
                 << error.errorString();
      return false;
    }

  if( doc.isArray() == false )
    {
      errorInfo = QObject::tr("Json format error for file: ") + fileName;
      qWarning() << "OpenAip::readHotspots: Error, expecting a Json array as first element";
      return false;
   }

  // Start index in hotspotList
  int startIdx = hotspotList.size();

  // Number counter for short name building.
  uint hsno = 0;

  QJsonArray array = doc.array();

  qDebug() << "Hotspot Doc is a Json Array of size:" << array.size();

  // step over the json array to extract the thermal objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();
      QString name;

      if( object.contains("name") && object["name"].isString() )
        {
          name = object["name"].toString();
        }
      else
        {
          // Ignore object without a name
          continue;
        }

      ThermalPoint tp;

      // Set thermal's name
      QString wpName = name;
      upperLowerName( wpName );

      // Long name
      tp.setName( wpName );

      // Short name only 8 characters long
      tp.setWPName( wpName.left(8) );

      // iterate over the thermal object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          // qDebug() << "Key: " << it.key() << "Val: " << it.value();

          if( it.key() == "type" )
            {
              tp.setType( it.value().toInt() );
            }
          else if( it.key() == "reliability" )
            {
              tp.setReliability( it.value().toInt() );
            }
          else if( it.key() == "occurrence" )
            {
              tp.setOccurrence( it.value().toInt() );
            }
          else if( it.key() == "category" )
            {
              tp.setCategory( it.value().toInt() );
            }
          else if( it.key() == "remarks" )
            {
              tp.setComment( it.value().toString() );
            }
          else if( it.key() == "country" )
            {
              tp.setCountry( it.value().toString().toUpper() );
            }
          else if( it.key() == "elevation" )
            {
              QJsonObject object = it.value().toObject();
              tp.setElevation( getJElevation( object ) );
            }
          else if( it.key() == "geometry" )
            {
              QJsonObject object = it.value().toObject();

              if( setJGeoLocation( object, tp ) == false )
                {
                  // ignore data set
                  continue;
                }
            }
        } // end of for loop of object list

      if( useFiltering == true &&
          checkRadius( tp.getWGSPositionPtr() ) == false )
        {
          // The radius filter said no. To far away from home.
          continue;
        }

      // Increment name counter for the hotspot.
      hsno++;

      // Set record number as WP name
      tp.setWPName( QString("%1").arg(hsno) );
      hotspotList.append( tp );
    } // end of array for loop

  if( startIdx < hotspotList.size() )
    {
      // Create a short name for the hotspot. The name is build from:
      // H = Hotspot
      // Two letter country code
      // consecutive number
      int fill = QString("%1").arg(hotspotList.size() - startIdx ).size();

      for( int i = startIdx; i < hotspotList.size(); i++ )
        {
          ThermalPoint& tp = hotspotList[i];
          int idx = tp.getWPName().toInt();
          tp.setWPName( "H" + tp.getCountry() + QString("%1").arg(idx, fill, 10, QChar('0')));
        }
    }

  return true;
}

float OpenAip::getJElevation( QJsonObject& object )
{
  double value = 0.0;

  if( object.contains("value") && object["value"].isDouble() )
    {
      value = object["value"].toDouble();
    }

  return static_cast<float>(value);
}

Frequency OpenAip::getJNavaidFrequency( QJsonObject& object )
{
  float value = 0.0;
  quint8 unit = 0;

  if( object.contains("value") && object["value"].isString() )
    {
      value = object["value"].toString().toFloat();
    }

  if( object.contains("unit") && object["unit"].isDouble() )
    {
      unit = object["unit"].toInt();
    }

  return Frequency( value, unit, Frequency::Other, "", true, true );
}

/**
 * Read and set the range value from a navaid json object.
 *
 * @param object
 * @return float
 */
int OpenAip::getJNavaidRange( QJsonObject& object )
{
  int value = 0;

  if( object.contains("value") && object["value"].isDouble() )
    {
      value = object["value"].toInt();
    }

  return value;
}

bool OpenAip::readAirfields( QString fileName,
                             QList<Airfield>& airfieldList,
                             QString& errorInfo,
                             bool useFiltering )
{
  m_shortNameSet.clear();
  fillRunwaySurfaceMapper();

  if( useFiltering )
    {
      // Load the user's defined filter data.
      loadUserFilterValues();
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readAirfields: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readAirfields: cannot open file:" << fileName;
      return false;
    }

  QString content = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8(), &error );

  qDebug() << fileName << "Airfield Json Parse result:" << error.errorString();

  if( doc.isNull() == true )
    {
      errorInfo = QObject::tr("Json parser error for file: ") + fileName;
      qWarning() << "OpenAip::readAirfields: file"
                 << "'" + fileName + "'"
                 << "Json parse error:"
                 << error.errorString();
      return false;
    }

  if( doc.isArray() == false )
    {
      errorInfo = QObject::tr("Json format error for file: ") + fileName;
      qWarning() << "OpenAip::readAirfields: Error, expecting a Json array as first element";
      return false;
   }

  QJsonArray array = doc.array();

  qDebug() << "Airfield Doc is a Json Array of size:" << array.size();

  // step over the json array to extract the airfield objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();
      QString name;

      if( object.contains("name") && object["name"].isString() )
        {
          name = object["name"].toString();
        }
      else
        {
          // Ignore object without a name
          continue;
        }

      Airfield af;

      // Set airfield's name
      QString wpName = name;
      upperLowerName( wpName );

      // Long name
      af.setName( wpName );

      // Set a unique short name
      af.setWPName( shortName(name) );

      // iterate over the airfield object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          // qDebug() << "Key: " << it.key() << "Val: " << it.value();

          if( it.key() == "type" )
            {
              setJAirfieldType( it.value().toInt(0), af );
            }
          else if( it.key() == "icaoCode" )
            {
              af.setICAO( it.value().toString() );
            }
          else if( it.key() == "country" )
            {
              af.setCountry( it.value().toString().toUpper() );
            }
          else if( it.key() == "elevation" )
            {
              QJsonObject object = it.value().toObject();
              af.setElevation( getJElevation( object ) );
            }
          else if( it.key() == "geometry" )
            {
              QJsonObject object = it.value().toObject();

              if( setJGeoLocation( object, af ) == false )
                {
                  // ignore data set
                  continue;
                }
            }
          else if( it.key() == "remarks" )
            {
              af.setComment( it.value().toString() );
            }
          else if( it.key() == "ppr" )
            {
              af.setPPR( it.value().toBool(false) );
            }
          else if( it.key() == "private" )
            {
              af.setPrivate( it.value().toBool(false) );
            }
          else if( it.key() == "skydiveActivity" )
            {
              af.setSkyDiving( it.value().toBool(false) );
            }
          else if( it.key() == "winchOnly" )
            {
              af.setWinch( it.value().toBool(false) );
            }
          else if( it.key() == "frequencies" )
            {
              QJsonArray array = it.value().toArray();
              setJAirfieldFrequencies( array, af );
            }
          else if( it.key() == "runways" )
            {
              QJsonArray array = it.value().toArray();
              setJAirfieldRunways( array, af );
            }
        } // end of for loop

      if( useFiltering == true &&
          checkRadius( af.getWGSPositionPtr() ) == false )
        {
          // The radius filter said no. To far away from home.
          continue;
        }

      airfieldList.append( af );
    }

  return true;
}

bool OpenAip::setJAirfieldType( const int type, Airfield& af )
{
      /* OpenAip types:

      The type of the airport. Possible values:

      0: Airport (civil/military)
      1: Glider Site
      2: Airfield Civil
      3: International Airport
      4: Heliport Military
      5: Military Aerodrome
      6: Ultra Light Flying Site
      7: Heliport Civil
      8: Aerodrome Closed
      9: Airport resp. Airfield IFR
      10: Airfield Water
      11: Landing Strip
      12: Agricultural Landing Strip
      13: Altiport

      Cumulus types:
      IntAirport, Airport, MilAirport, CivMilAirport,
      Airfield, ClosedAirfield, CivHeliport,
      MilHeliport, AmbHeliport, Gliderfield, UltraLight, Altiport
      */

      if( type == 8 )
        {
          af.setTypeID( BaseMapElement::ClosedAirfield );
        }
      else if( type == 5 )
        {
          af.setTypeID( BaseMapElement::MilAirport );
        }
      else if( type == 2 || type == 9 || type == 10 )
        {
          af.setTypeID( BaseMapElement::Airfield );
        }
      else if( type == 0 )
        {
          af.setTypeID( BaseMapElement::CivMilAirport );
        }
      else if( type == 3 )
        {
          af.setTypeID( BaseMapElement::IntAirport );
        }
      else if( type == 1 )
        {
          af.setTypeID( BaseMapElement::Gliderfield );
        }
      else if( type == 7 )
        {
          af.setTypeID( BaseMapElement::CivHeliport );
        }
      else if( type == 4 )
        {
          af.setTypeID( BaseMapElement::MilHeliport );
        }
      else if( type == 6 )
        {
          af.setTypeID( BaseMapElement::UltraLight );
        }
      else if( type == 11 || type == 12 )
        {
          af.setTypeID( BaseMapElement::Outlanding );
        }
      else if( type == 13 )
        {
          // Altiport
          af.setTypeID( BaseMapElement::Airfield );
        }
      else
        {
          af.setTypeID( BaseMapElement::NotSelected );
          qWarning() << "OpenAip::setJAirfieldType: unknown airfield type"
                     << type
                     << "for"
                     << af.getName();
        }

  return true;
}

void OpenAip::setJAirfieldFrequencies( QJsonArray& array, Airfield& af )
{
  QList<Frequency>& fl = af.getFrequencyList();

  // step over the json array to extract the frequency objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();

      Frequency fq;

      // iterate over the frequency object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          // qDebug() << "Frequency Key: " << it.key() << "Val: " << it.value();
          if( it.key() == "name" )
            {
              fq.setName( it.value().toString() );
            }
          else if( it.key() == "value" )
            {
              fq.setValue( it.value().toString().toFloat() );
            }
          else if( it.key() == "unit" )
            {
              fq.setUnit( it.value().toInt() );
            }
          else if( it.key() == "type" )
            {
              fq.setType( it.value().toInt() );
            }
          else if( it.key() == "primary" )
            {
              fq.setPrimary( it.value().toBool(false) );
            }
          else if( it.key() == "publicUse" )
            {
              fq.setPublicUse( it.value().toBool(false) );
            }
        } // End of for loop

      fl.append( fq );
    }
}

void OpenAip::setJAirfieldRunways( QJsonArray& array, Airfield& af )
{
  QList<Runway>& rwl = af.getRunwayList();

  // step over the json array to extract the runway objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();

      Runway rwy;

      // iterate over the runway object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          // qDebug() << "Runway Key: " << it.key() << "Val: " << it.value();
          if( it.key() == "designator" )
            {
              rwy.setName( it.value().toString() );
            }
          else if( it.key() == "trueHeading" )
            {
              rwy.setHeading( it.value().toInt() );
            }
          else if( it.key() == "alignedTrueNorth" )
            {
              rwy.setAlignedTrueNorth( it.value().toBool( false ) );
            }
          else if( it.key() == "operations" )
            {
              rwy.setOperations( it.value().toInt( 2 ) );
            }
          else if( it.key() == "mainRunway" )
            {
              rwy.setMainRunway( it.value().toBool( false ) );
            }
          else if( it.key() == "turnDirection" )
            {
              rwy.setTurnDirection( it.value().toInt( 2 ) );
            }
          else if( it.key() == "takeOffOnly" )
            {
              rwy.setTakeOffOnly( it.value().toBool( false ) );
            }
          else if( it.key() == "landingOnly" )
            {
              rwy.setLandingOnly( it.value().toBool( false ) );
            }
          else if( it.key() == "dimension" )
            {
              setJAirfieldRunwayDimensions( it.value().toObject(), rwy );
            }
          else if( it.key() == "surface" )
            {
              setJAirfieldRunwaySurface( it.value().toObject(), rwy );
            }
        } // End of for loop

      // Check runway designator
      if( rwy.getName().isEmpty() )
        {
          QString name;

          int rd = ( rwy.getHeading() + 5 ) / 10;

          if( rd == 0 ) {
              rd = 36;
          }

          if( rd < 10 ) {
              name += '0';
          }

          rwy.setName( name + QString::number( rd ) );

        }

      rwl.append( rwy );
    }
}

/**
 * Read Json runway dimension data of an airfield.
 */
void OpenAip::setJAirfieldRunwayDimensions( const QJsonObject& object, Runway& rw )
{
  // iterate over the runway dimension object list
  for( auto it = object.begin(), end=object.end(); it != end; ++it )
    {
      // qDebug() << "Runway dimension Key: " << it.key() << "Val: " << it.value();

      if( it.key() == "length" )
        {
          QJsonObject obj = it.value().toObject();

          if( obj.contains("value") == true )
            {
              rw.setLength( obj.value("value").toInt() ); // meters expected
            }
        }
      else if( it.key() == "width" )
        {
          QJsonObject obj = it.value().toObject();

          if( obj.contains("value") == true )
            {
              rw.setWidth( obj.value("value").toInt() ); // meters expected
            }
        }
    }
}

/**
 * Read and set Json runway surface data of an airfield.
 */
void OpenAip::setJAirfieldRunwaySurface( const QJsonObject& object, Runway& rw )
{
  // iterate over the runway surface object list
  for( auto it = object.begin(), end=object.end(); it != end; ++it )
    {
      // qDebug() << "Runway surface Key: " << it.key() << "Val: " << it.value();

      if( it.key() == "mainComposite" )
        {
          quint8 type = m_runwaySurfaceMapper.value( it.value().toInt(),
                                                     Runway::Unknown );
          rw.setSurface( type );

          if( type == Runway::Unknown )
            {
              qWarning() << "OpenAip::setJAirfieldRunwaySurface: unknown runway surface type"
                         << type;
            }
        }
    }
}

void OpenAip::upperLowerName( QString& name )
{
  name = name.toLower();

  QSet<QChar> set;
  set.insert( ' ' );
  set.insert( '/' );
  set.insert( '-' );
  set.insert( '(' );

  QChar lastChar(' ');

  // Convert name to upper-lower cases
  for( int i=0; i < name.size(); i++ )
    {
      if( set.contains(lastChar) )
        {
          name.replace( i, 1, name.mid(i,1).toUpper() );
        }

      lastChar = name[i];
    }
}

/**
 * Create an unique short name by removing undesired characters.
 *
 * \param name The name to be shorten.
 *
 * \return new short name 8 characters long
 */
QString OpenAip::shortName( const QString& name )
{
  QString shortName;

  for( int i = 0; i < name.size(); i++ )
    {
      if( name.at(i).isLetterOrNumber() == false )
        {
          continue;
        }

      shortName.append( name[i] );

      if( shortName.size() == 8 )
        {
          // Limit short name to 8 characters.
          break;
        }
    }

  // Check, if short name is already in use. In this case create another one.
  if( ! m_shortNameSet.contains( shortName) )
    {
      m_shortNameSet.insert( shortName );
    }
  else
    {
      // Try to generate an unique short name. The assumption is that we never have
      // more than 9 equal names.
      for( int i=1; i < 10; i++ )
        {
          shortName.replace( shortName.length()-1, 1, QString::number(i) );

          if( ! m_shortNameSet.contains( shortName) )
            {
              m_shortNameSet.insert( shortName );
              break;
            }
        }
    }

  return shortName;
}

bool OpenAip::readAirspaces( QString fileName,
                             QList<Airspace>& airspaceList,
                             QString& errorInfo )
{
  const char* method = "OpenAip::readAirspaces:";

  QFileInfo fi(fileName);
  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readAirspaces: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readAirspaces: cannot open file:" << fileName;
      return false;
    }

  QString content = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson( content.toUtf8(), &error );

  qDebug() << fileName << "Airspaces Json Parse result:" << error.errorString();

  if( doc.isNull() == true )
    {
      errorInfo = QObject::tr("Json parser error for file: ") + fileName;
      qWarning() << "OpenAip::readAirspaces: file"
                 << "'" + fileName + "'"
                 << "Json parse error:"
                 << error.errorString();
      return false;
    }

  if( doc.isArray() == false )
    {
      errorInfo = QObject::tr("Json format error for file: ") + fileName;
      qWarning() << "OpenAip::readAirspaces: Error, expecting a Json array as first element";
      return false;
   }

  // Initialize airspace type mapper
  m_airspaceTypeMapper = AirspaceHelper::initializeAirspaceTypeMapping( fileName );

  if( m_airspaceTypeMapper.isEmpty() )
    {
      errorInfo = QObject::tr("Cannot load airspace type mapper for") + " " + fileName;
      qWarning() << method << "Cannot load airspace type mapper for" << fileName;
      return false;
    }

  QJsonArray array = doc.array();

  qDebug() << "Airspace Doc is a Json Array of size:" << array.size();

  // step over the json array to extract the airfield objects
  for( int i=0; i < array.size(); i++ )
    {
      QJsonObject object = array[i].toObject();
      QString name;

      if( object.contains("name") && object["name"].isString() )
        {
          name = object["name"].toString();
        }
      else
        {
          // Ignore object without a name
          continue;
        }

      Airspace as;
      as.setName( name );
      bool ok = true;

      // iterate over the airfield object list
      for( auto it = object.begin(), end=object.end(); it != end; ++it )
        {
          if( it.key() == "type" )
            {
              // qDebug() << "AS-Name=" << name << "AS-Type=" << it.value();

              QString type = QString::number( it.value().toInt() );

              // Check not senseful because airspace names are not unique
              if( m_airspaceTypeMapper.contains( type ) == false )
                {
                  qWarning() << method
                             << "AS-Name:" << name + ","
                             << "Ignoring Airspace, unknown type:" << type;
                  ok = false;
                  break;
                }

              as.setTypeID( m_airspaceTypeMapper[ type ] );
            }
          else if( it.key() == "icaoClass" )
            {
             as.setIcaoClass( it.value().toInt() );
            }
          else if( it.key() == "country" )
            {
              as.setCountry( it.value().toString() );
            }
          else if( it.key() == "activity" )
            {
              as.setActivity( it.value().toInt() );
            }
          else if( it.key() == "byNotam" )
            {
              as.setByNotam( it.value().toBool( false ) );

              if( as.isByNotam() )
                {
                  qDebug() << "AS" << name << "activated by NOTAM";
                }
            }
          else if( it.key() == "upperLimit" )
            {
              Altitude alt;
              BaseMapElement::elevationType ref;
              QJsonObject obj = it.value().toObject();

              ok = readJAirspaceLimit( name, obj, ref, alt );

              if( ok == false )
                {
                  // ignore data
                  break;
                }

              as.setUpperL( alt );
              as.setUpperT( ref );
            }
          else if( it.key() == "lowerLimit" )
            {
              Altitude alt;
              BaseMapElement::elevationType ref;
              QJsonObject obj = it.value().toObject();

              ok = readJAirspaceLimit( name, obj, ref, alt );

              if( ok == false )
                {
                  // ignore data
                  break;
                }

              as.setLowerL( alt );
              as.setLowerT( ref );
            }
          else if( it.key() == "geometry" )
            {
              QJsonObject obj = it.value().toObject();

              ok = readJAirspaceGeometrie( obj, as );

              if( ok == false )
                {
                  // ignore data
                  break;
                }
            }
        } // End of object for loop

      if( ok == true )
        {
          // Check, if set type is not unknown. In this case the ICAO class
          // has to be mapped as new type.
          if( as.getTypeID() == BaseMapElement::AirUkn )
            {
              BaseMapElement::objectType newType =
                  AirspaceHelper::mapIcaoClassId( as.getIcaoClass() );

              as.setTypeID( newType );
            }

          airspaceList.append( as.createAirspaceObject() );
        }

    } // End of i for loop

  return true;
}

bool OpenAip::readJAirspaceLimit( const QString& asName,
                                  const QJsonObject& object,
                                  BaseMapElement::elevationType& reference,
                                  Altitude& altitude )
{
  // QJsonObject({"referenceDatum":1,"unit":1,"value":1000})

  reference = BaseMapElement::NotSet;
  int value = -1;
  int unit = -1;
  int referenceDatum = -1;

  // iterate over the limit object list
  for( auto it = object.begin(), end=object.end(); it != end; ++it )
    {
      // qDebug() << "Runway dimension Key: " << it.key() << "Val: " << it.value();

      if( it.key() == "value" )
        {
          value = it.value().toInt( -1 );
        }
      else if( it.key() == "unit" )
        {
          unit = it.value().toInt( -1 );
        }
      else if( it.key() == "referenceDatum" )
        {
          referenceDatum = it.value().toInt( -1 );
        }
    }

  if( value == -1 || unit == -1 )
    {
      qWarning() << "Airspace limit of"
                 << asName << "has no value or unit assigned, ignoring it!";
      return false;
    }

  switch( unit )
    {
    case 0: // m
      altitude.setMeters( value );
      break;
    case 1: // ft
      altitude.setFeet( value );
      break;
    case 6: // FL
      altitude.setFeet( value );
      break;
    default:
      qWarning() << "Airspace limit of"
                 << asName << "has an unknown unit"
                 << unit << "assigned, ignoring it!";
      return false;
    }

  if( referenceDatum == -1 )
    {
      qWarning() << "Airspace limit of"
                 << asName << "has no reference datum assigned, ignoring it!";
      return false;
    }

  switch( referenceDatum )
    {
    case 0: // GND
      reference = BaseMapElement::GND;
      break;
    case 1: // MSL
      reference = BaseMapElement::MSL;
      break;
    case 2: // STD
      reference = BaseMapElement::FL;
      break;
    default:
      qWarning() << "Airspace limit of"
                 << asName << "has an unknown reference datum"
                 << referenceDatum << "assigned, ignoring it!";
      return false;
    }

  return true;
}

bool OpenAip::readJAirspaceGeometrie( const QJsonObject& object, Airspace& as )
{
  // iterate over the limit object list
  for( auto it = object.begin(), end=object.end(); it != end; ++it )
    {
      // qDebug() << "AS Geo Key: " << it.key() << "Val: " << it.value();

      if( it.key() == "type" && it.value().toString() != "Polygon" )
        {
          qWarning() << "Airspace geometry of"
                     << as.getName() << "has an unknown type"
                     << it.value().toString() << "assigned."
                     << "Can only process Polygon, ignoring airspace!";
          return false;
        }
      else if( it.key() == "coordinates" )
        {
          QJsonArray array0 = it.value().toArray();
          QJsonArray array1 = array0[0].toArray();

          QPolygon asPolygon( array1.size() );
          extern MapMatrix* _globalMapMatrix;

          // step over the json array to extract the coordinate objects
          for( int i=0; i < array1.size(); i++ )
            {
              QJsonArray lonlat = array1[i].toArray();

              if( lonlat.size() != 2 )
                {
                  qWarning() << "Airspace geometry of"
                             << as.getName() << "has an inconsistent lon/lat pair,"
                             << "ignoring airspace!";

                  return false;
                }

              double lon = lonlat[0].toDouble();
              double lat = lonlat[1].toDouble();

              if( lon < -180.0 || lon > 180.0 )
                {
                  qWarning() << "Airspace geometry: wrong longitude value"
                             << lon
                             << "read for airspace" << as.getName()
                             << "ignoring airspace!";
                  return false;
                }

              if( lat < -90.0 || lat > 90.0 )
                {
                  qWarning() << "Airspace geometry: wrong latitude value"
                             << lat
                             << "read for airspace" << as.getName()
                             << "ignoring airspace!";
                  return false;
                }

              // Convert coordinates into KFLog format
              int latInt = static_cast<int> (rint(600000.0 * lat));
              int lonInt = static_cast<int> (rint(600000.0 * lon));

              // Project coordinates to map datum and store them in a polygon
              asPolygon.setPoint( i, _globalMapMatrix->wgsToMap( latInt, lonInt ) );
            }

          // Airspaces are stored as polygons and should not contain the start point
          // twice as done in OpenAip description.
          if( asPolygon.count() > 2 && asPolygon.first() == asPolygon.last() )
            {
              // remove the last point because it is identical to the first point
              asPolygon.remove( asPolygon.count() - 1 );
            }

          if( asPolygon.count() < 3 )
            {
              // A polygon should contain at least 3 coordinates.
              qWarning() << "Airspace geometry: " << as.getName()
                         << "contains to less coordinates! Ignoring airspace!";
              return false;
            }

          as.setProjectedPolygon( asPolygon );
        }
    }

  return true;
}

/**
 * Check if point is inside a certain radius.
 *
 * \return true, if point is inside other wise false
 */
bool OpenAip::checkRadius( WGSPoint* point )
{
  if( m_filterRadius > 0.0 )
    {
      double d = dist( &m_homePosition, point );

      if( d > m_filterRadius )
        {
          // The radius filter said no. To far away from home.
          return false;
        }
    }

  return true;
}
