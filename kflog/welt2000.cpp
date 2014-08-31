/***********************************************************************
 **
 **   welt2000.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2006-2014 by Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id: welt2000.cpp 4274 2010-09-22 08:28:26Z axel $
 **
 ***********************************************************************/

#include <cmath>
#include <unistd.h>

#include <QtCore>

#include "airfield.h"
#include "basemapelement.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "resource.h"
#include "runway.h"
#include "wgspoint.h"
#include "distance.h"

#include "welt2000.h"

#ifdef _WIN32
#define basename(y) (y)
#endif

// KFLog's configuration settings
extern QSettings _settings;

// All is prepared for additional calculation, storage and
// reconstruction of a bounding box. Be free to switch on/off it via
// conditional define.

#undef BOUNDING_BOX
// #define BOUNDING_BOX 1

extern MapContents*  _globalMapContents;
extern MapMatrix*    _globalMapMatrix;

Welt2000::Welt2000() :
  c_homeRadius(0.0)
{
  // prepare base mappings of KFLog
  c_baseTypeMap.insert( "IntAirport", BaseMapElement::IntAirport );
  c_baseTypeMap.insert( "Airport", BaseMapElement::Airport );
  c_baseTypeMap.insert( "MilAirport", BaseMapElement::MilAirport );
  c_baseTypeMap.insert( "CivMilAirport", BaseMapElement::CivMilAirport );
  c_baseTypeMap.insert( "Airfield", BaseMapElement::Airfield );
  c_baseTypeMap.insert( "ClosedAirfield", BaseMapElement::ClosedAirfield );
  c_baseTypeMap.insert( "CivHeliport", BaseMapElement::CivHeliport );
  c_baseTypeMap.insert( "MilHeliport", BaseMapElement::MilHeliport );
  c_baseTypeMap.insert( "AmbHeliport", BaseMapElement::AmbHeliport );
  c_baseTypeMap.insert( "Gliderfield", BaseMapElement::Gliderfield );
  c_baseTypeMap.insert( "UltraLight", BaseMapElement::UltraLight );
  c_baseTypeMap.insert( "HangGlider", BaseMapElement::HangGlider );
}

Welt2000::~Welt2000()
{
}

bool Welt2000::check4update()
{
  static bool hasCalled = false;

  qDebug() << "Welt2000::check4update(): hasCalled=" << hasCalled;


  if( hasCalled == true )
    {
      // This method is callable only once to avoid a call dead lock.
      return false;
    }

  hasCalled = true;

  QString wu = "WELT2000.TXT";
  QString sd = "/airfields/";
  QString mapDir = _globalMapContents->getMapRootDirectory();

  // Search for the Welt2000 source file.
  QString path2File = mapDir + sd + wu;

  QFileInfo test( path2File );

  if( ! test.exists() )
    {
      // No welt2000 exists and we return false in this case because we cannot
      // check the update state.
      return false;
    }

  if( test.lastModified().date() == QDate::currentDate() )
    {
      qDebug() << "Welt2000::check4update(): Dates are equal, return false.";
      // The file was already updated today. Don't allow further updates.
      return false;
    }

  // Only one update try per day is allowed.
  return true;
}

/**
 * Search on default places a welt2000 file and load it. The results
 * are put in the passed lists
 */
bool Welt2000::load( QList<Airfield>& airfieldList,
                     QList<Airfield>& gliderfieldList,
                     QList<Airfield>& outlandingList )
{
  qDebug() << "Welt2000::load";

  QString wu = "WELT2000.TXT";
  QString sd = "/airfields/";
  QString mapDir = _globalMapContents->getMapRootDirectory();

  QString path2File = mapDir + sd + wu;

  QFileInfo test( path2File );

  if( ! test.exists() )
    {
      qWarning( "W2000: No Welt2000 file found in the map airfields directory" );
      return false;
    }

  // parse source file
  return parse( path2File, airfieldList, gliderfieldList, outlandingList );
}

/**
 * Parses the passed file in Welt2000 format and put the appropriate
 * entries in the related lists.
 *
 * arg1 path: Full name with path of welt2000 file
 * arg2 airfieldList: All airports have to be stored in this list
 * arg3 gliderfieldList: All gilder fields have to be stored in this list
 * arg4 outlandibgList: All outlanding fields have to be stored in this list, when
 *                      the outlanding option is set in the user configuration
 * returns true (success) or false (error occurred)
 */
bool Welt2000::parse( QString& path,
                      QList<Airfield>& airfieldList,
                      QList<Airfield>& gliderfieldList,
                      QList<Airfield>& outlandingList )
{
  QTime t;
  t.start();

  QFile in(path);

  if( !in.open(QIODevice::ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QTextStream ins(&in);
  ins.setCodec( "ISO 8859-15" );

  // Check, if in KFLOg settings other definitions exist. These will
  // overwrite the definitions in the configuration file.
  QString cFilter = _settings.value( "/Welt2000/CountryFilter", "" ).toString();

  if( cFilter.length() > 0 )
    {
      // load new country filter definitions
      c_countryList.clear();

      QStringList clist = cFilter.split( QRegExp("[,; ]"), QString::SkipEmptyParts );

      for( int i = 0; i < clist.count(); i++ )
        {
          QString e = clist[i].trimmed().toUpper();

          if( c_countryList.contains(e) )
            {
              continue;
            }

          c_countryList += e;
        }

      c_countryList.sort();
    }

  // get outlanding load flag from configuration data
  bool outlandings = _settings.value( "/Welt2000/LoadOutlandings", true ).toBool();

  // get home radius from configuration data
  int radius = _settings.value( "/Welt2000/HomeRadius", 0 ).toInt();

  if( c_countryList.size() == 0 && radius == 0 )
    {
      // Define a default radius of 500Km, if no country filter is defined.
      radius = 500;
    }

  if( radius > 0 )
    {
      // we must look, what unit the user has chosen. This unit must
      // be considered during load of airfield data.
      c_homeRadius = getDistanceInKm( radius );
    }

  qDebug() << "W2000: Country Filter contains"
           << c_countryList.count()
           << "entries." << c_countryList;
  qDebug() << "W2000: Read Outlandings=" << outlandings;
  qDebug( "W2000: Home radius is set to %.1f Km", c_homeRadius );

  // put all entries of country list into a dictionary for faster
  // access
  QHash<QString, QString> countryDict;

  for( int i = 0; i < c_countryList.count(); i++ )
    {
      // populate country dictionary
      countryDict.insert( c_countryList[i], c_countryList[i] );
    }

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  uint lineNo = 0;
  QSet<QString> shortNameSet; // contains all short names already in use

  // Contains the coordinates of the objects put in the lists. Used as filter
  // to avoid multiple entries at the same point.
  QSet<QString> pointFilter;

  // statistics counter
  uint ul, gl, af, ol;
  ul = gl = af = ol = 0;

  // Input file was taken from Michael Meiers Welt2000 data dase
  //
  // 0         1         2         3         4         5         6
  // 0123456789012345678901234567890123456789012345678901234567890123
  // 1234567890123456789012345678901234567890123456789012345678901230
  // AACHE1 AACHEN  MERZBRUC#EDKAA 53082612287 189N504923E0061111DEO5
  // AICHA1 AICHACH         # S !G 43022012230 440N482824E0110807DEX
  // ARGEN2 ARGENBUEHL EISE?*ULM G 40082612342 686N474128E0095744DEN
  // BASAL2 BAD SALZUNGEN UL*ULM G 65092712342 233N504900E0101305DEN
  // FUERS1 FUERSTENWALDE   #EDALG 80112912650  55N522323E0140539DEO3
  // German international airport, ICAO starts with EDD
  // BERLT1 BERLIN  TEGEL   #EDDTA303082611870  37N523335E0131716DEO
  // BERSC1 BERLIN SCHOENFEL#EDDBC300072512002  49N522243E0133114DEO
  // German military airport, ICAO starts with ET
  // HOLZD1 HOLZDORF MIL    #ETSHA242092712210  82N514605E0131003DEQ0
  // UL Fields new coding variant
  // SIEWI1 SIEWISCH UL    !# ULMG 51082612342  89N514115E0141231DEO0
  // OUTLANDING EXAMPLES
  // ESPIN2 ESPINASSES     !*FL10S 3509271     648N442738E0061305FRQ0
  // BAERE2 BAERENTAL       *FELDS 2505231     906N475242E0080643DEO3
  // BAIBR2 BAIERSBRON CLS  *WIESG 2317351     507N483217E0082354DEM5
  // BAIYY2 BAIERSBRONN     *FL03S 2205231     574N483056E0082224DEO0
  // DAMGA2 DAMGARTEN CLS   *   !C200072512150   5N541551E0122640DEE0
  // PIEVE2 PIEVERSTORF 25M *AGR!A 3208261      27N534906E0110841DEX0

  while( ! in.atEnd() )
    {
      bool ok, ok1;
      QString line, buf;
      line = in.readLine(128);
      lineNo++;

      if( line.isEmpty() )
        {
          continue;
        }

      // step over comment or invalid lines
      if( line.startsWith("#") || line.startsWith("$") ||
          line.startsWith("\t") || line.startsWith(" ") )
        {
          continue;
        }

      // remove white spaces and line end characters
      line = line.trimmed();

      // replace markers against space
      line.replace( QRegExp("[!?]+"), " " );

      if( line.length() < 62 )
        {
          // country sign not included
          continue;
        }

      // get short name for user mapping before changing line
      QString shortName = line.mid( 0, 6 );

      // convert all to toUpper case
      line = line.toUpper();

      // Extract country sign. It is coded according to ISO 3166.
      QString country = line.mid( 60, 2 ).toUpper();

      if( ! countryDict.isEmpty() )
        {
          if( ! countryDict.contains(country) )
            {
              continue;
            }
        }

      // look, what kind of line was read.
      // COL5 = 1 Airfield or also UL site
      // COL5 = 2 Outlanding, contains also UL places
      QString kind = line.mid( 5, 1 );

      if( kind != "1" && kind != "2" )
        {
          continue; // not of interest for us
        }

      bool ulField = false;
      bool glField = false;
      bool afField = false;
      bool olField = false;

      QString commentShort;
      QString commentLong;
      QString icao;

      if( kind == "2" ) // can be an UL field
        {
          if( line.mid( 23, 4 ) == "*ULM" )
            {
              ulField = true;
            }
          else
            {
              // outlanding found
              if( outlandings == false )
                {
                  // ignore outlandings
                  continue;
                }

              olField = true;
              commentShort = line.mid( 24, 4 );
              commentShort.replace(QRegExp("[!?]+"), " " );
              commentShort = commentShort.toUpper().trimmed();

              if( commentShort.startsWith( "FL" ) )
                {
                  commentLong = QString( QObject::tr("Emergency Field No: ")) +
                                commentShort.mid( 2, 2 );
                }
            }
        }
      else if( line.mid( 23, 4 ) == "#GLD" )
        {
          // Glider field
          glField = true;
        }
      else if( line.mid( 23, 5 ) == "# ULM" )
        {
          // newer coding for UL field
          ulField = true;
        }
      else
        {
          afField = true;
          icao = line.mid( 24, 4 ).trimmed().toUpper();

          if( line.mid( 20, 4 ) == "GLD#" )
            {
              // other possibility for a glider field with ICAO code
              glField = true;
            }
        }

      // Airfield name
      QString afName = line.mid( 7, 16 );

      // remove special mark signs
      afName.replace( QRegExp("[!?]+"), "" );

      // remove resp. replace white spaces against one space
      afName = afName.simplified();

      if( afName.length() == 0 )
        {
          qWarning( "W2000, Line %d: Airfield name is undefined, ignoring entry!",
                    lineNo );
          continue;
        }

      // airfield type
      BaseMapElement::objectType afType = BaseMapElement::NotSelected;

      // determine airfield type so good as possible
      if( ulField == true )
        {
          afType = BaseMapElement::UltraLight;
        }
      else if( glField == true )
        {
          afType = BaseMapElement::Gliderfield;
        }
      else if( olField == true )
        {
          afType = BaseMapElement::Outlanding;
        }
      else if( afField == true )
        {
          if( icao.startsWith("ET") )
            {
              // German military airport
              afType = BaseMapElement::MilAirport;
            }
          else if( afName.contains(QRegExp(" MIL$")) )
            {
              // should be an military airport but not 100% sure
              afType = BaseMapElement::MilAirport;
            }
          else if( icao.startsWith("EDD") )
            {
              // German international airport
              afType = BaseMapElement::IntAirport;
            }
          else
            {
              afType = BaseMapElement::Airfield;
            }
        }

      // make the user's desired mapping for short name
      if( c_shortMap.contains(shortName) )
        {
          QString val = c_shortMap[shortName];

          if( c_baseTypeMap.contains(val) )
            {
              afType = c_baseTypeMap[val];
            }
        }

      // make the user's wanted mapping for icao
      if( ! icao.isEmpty() && c_icaoMap.contains(icao) )
        {
          QString val = c_icaoMap[icao];

          if( c_baseTypeMap.contains(val) )
            {
              afType = c_baseTypeMap[val];
            }
        }

      // airfield name
      afName = afName.toLower();

      QChar lastChar(' ');

      // convert airfield names to upper-lower
      for( int i=0; i < afName.length(); i++ )
        {
          if( lastChar == ' ' )
            {
              afName.replace( i, 1, afName.mid(i,1).toUpper() );
            }

          lastChar = afName[i];
        }

      // gps name, we use 8 characters without spaces
      QString gpsName = afName;
      gpsName.remove(QChar(' '));
      gpsName = gpsName.left(8);

      if( ! shortNameSet.contains( gpsName) )
        {
          shortNameSet.insert( gpsName );
        }
      else
        {
          // Try to generate an unique short name. The assumption is that we never have
          // more than 10 equal names.
          for( int i=0; i <= 9; i++ )
            {
              gpsName.replace( gpsName.length()-1, 1, QString::number(i) );

              if( ! shortNameSet.contains( gpsName) )
                {
                  shortNameSet.insert( gpsName );
                  break;
                }
            }
        }

      if( ulField  )
        {
          if( afName.right(3) == " Ul" )
            {
              // Convert lower l of Ul to upper case
              afName.replace( afName.length()-1, 1, "L" );
            }
          else
            {
              // append UL substring
              // afName += " UL";
            }
        }

      qint32 lat, lon;
      QString degree, min, sec;
      double d, m, s;

      // convert latitude
      degree = line.mid(46,2);
      min    = line.mid(48,2);
      sec    = line.mid(50,2);

      d = degree.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude degree value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude minute value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude second value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      double latTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lat = (qint32) rint(latTmp);

      if( line[45] == 'S' )
        {
          lat = -lat;
        }

      // convert longitude
      degree = line.mid(53,3);
      min    = line.mid(56,2);
      sec    = line.mid(58,2);

      d = degree.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude degree value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude minute value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude second value, ignoring entry!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      double lonTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lon = (qint32) rint(lonTmp);

      if( line[52] == 'W' )
        {
          lon = -lon;
        }

      if( c_homeRadius > 0 )
        {
          // Compute the distance between the home position and the
          // read point, if a radius is defined. Is the distance is greater
          // than as the user defined value we skip this point.

          QPoint home = _globalMapMatrix->getHomeCoord();
          QPoint af( lat, lon );

          double d = dist( &home, &af );

          if( d > c_homeRadius )
            {
              // Distance is greater than defined radius in GeneralConfig
              // qDebug("Ignoring Dist=%f, AF=%s", d, afName.toLatin1().data());
              continue;
            }
        }

#ifdef BOUNDING_BOX
      // update the bounding box
      _globalMapContents->AddPointToRect( boundingBox, QPoint(lat, lon) );
#endif

      WGSPoint wgsPos(lat, lon);

      // We do check here, if the coordinates of the object are already known to
      // filter out multiple entries. Only the first entry do pass the filter.
      QString corrString = WGSPoint::coordinateString( wgsPos );

      if( pointFilter.contains( corrString ) )
        {
          // An object with the same coordinates do already exist.
          // We do ignore this one.
          qWarning( "W2000, Line %d: %s (%s) skipping entry, coordinates already in use!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      // store coordinates in filter
      pointFilter.insert( corrString );

      QPoint position = _globalMapMatrix->wgsToMap(wgsPos);

      // elevation
      buf = line.mid(41,4 ).trimmed();

      ok = false;
      qint16 elevation = 0;

      if( ! buf.isEmpty() )
        {
          elevation = buf.toInt(&ok);
        }

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) missing or wrong elevation, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          elevation = 0;
        }

      // frequency
      QString frequency = line.mid(36,3) + "." + line.mid(39,2).trimmed();

      float fFrequency = frequency.toFloat(&ok);

      if( ( !ok || fFrequency < 108 || fFrequency > 137.0 ) )
        {
          if( olField == false )
            {
              // Don't display warnings for outlandings
              qWarning( "W2000, Line %d: %s (%s) missing or wrong frequency, set value to 0!",
                        lineNo, afName.toLatin1().data(), country.toLatin1().data() );
            }

          fFrequency = 0.0; // reset frequency to unknown
        }
      else
        {
          // check, what has to be appended as last digit
          // check, what has to be appended as last digit
          if( line[40] == '2' || line[40] == '7' )
            {
              fFrequency += 0.005;
            }
        }

      /* Runway description from Welt2000.txt file
       *
       * A: 08/26 MEANS THAT THERE IS ONLY ONE RUNWAYS 08 AND (26=08 + 18)
       * B: 17/07 MEANS THAT THERE ARE TWO RUNWAYS,
       *          BUT 17 IS THE MAIN RWY SURFACE LENGTH
       * C: IF BOTH DIRECTIONS ARE IDENTICAL (04/04),
       *    THIS DIRECTION IS STRONGLY RECOMMENDED
       */

      // runway direction have two digits, we consider both directions
      buf = line.mid(32,2).trimmed();

      ok = false;
      ok1 = false;

      ushort rwDir1 = 0;
      ushort rwDir2 = 0;
      ushort rwCount = 0;

      QPair<ushort, ushort> rwDirections[2];
      rwDirections[0] = QPair<ushort, ushort>( 0, 0);
      rwDirections[1] = QPair<ushort, ushort>( 0, 0);

      if( ! buf.isEmpty() )
        {
          rwDir1 = buf.toUShort(&ok);
        }

      // extract second direction
      buf = line.mid(34,2).trimmed();

      if( ! buf.isEmpty() )
        {
          rwDir2 = buf.toUShort(&ok1);
        }

      if( ! ok || ! ok1 || rwDir1 < 1 || rwDir1 > 36 || rwDir2 < 1 || rwDir2 > 36 )
        {
          qWarning( "W2000, Line %d: %s (%s) missing or wrong runway direction, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
        }
      else
        {
          if( rwDir1 == rwDir2 || abs( rwDir1 - rwDir2 ) == 18 )
            {
              // We have only one runway
              rwDirections[0].first  = rwDir1;
              rwDirections[0].second = rwDir2;
              rwCount = 1;
            }
          else
            {
              // WE have two runways
              rwDirections[0].first  = rwDir1;
              rwDirections[0].second = ((rwDir1 > 18) ? rwDir1 - 18 : rwDir1 + 18 );
              rwDirections[1].first  = rwDir2;
              rwDirections[1].second = ((rwDir2 > 18) ? rwDir2 - 18 : rwDir2 + 18 );
              rwCount = 2;
            }
        }

      // runway length in meters, must be multiplied by 10
      buf = line.mid(29,3).trimmed();

      ok = false;
      ushort rwLen = 0;

      if( ! buf.isEmpty() )
        {
          rwLen = buf.toUInt(&ok);
        }

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) missing or wrong runway length, set value to 0!",
                    lineNo, afName.toLatin1().data(), country.toLatin1().data() );
          rwLen = 0;
        }
      else
        {
          rwLen *= 10;
        }

      // runway surface
      enum Runway::SurfaceType rwSurface;
      QChar rwType = line[28];

      if( rwType == 'A' )
        {
          rwSurface = Runway::Asphalt;
        }
      else if( rwType == 'C' )
        {
          rwSurface = Runway::Concrete;
        }
      else if( rwType == 'G' )
        {
          rwSurface = Runway::Grass;
        }
      else if( rwType == 'S' )
        {
          rwSurface = Runway::Sand;
        }
      else
        {
          rwSurface = Runway::Unknown;
        }

      //---------------------------------------------------------------
      // append a new record to the related list
      //---------------------------------------------------------------

      // count output records separated by kind
      if( ulField )
        {
          ul++;
        }
      else if( glField )
        {
          gl++;
        }
      else if( olField )
        {
          ol++;
        }
      else
        {
          af++;
        }

      Airfield af( afName, icao.trimmed(), gpsName, afType,
                   wgsPos, position, elevation, fFrequency, country, commentLong );

      for( int i = 0; i < rwCount; i++ )
        {
          // create an runway object
          Runway rw( rwLen, rwDirections[i], rwSurface, true );
          af.addRunway( rw );
        }

      if( afType == BaseMapElement::Outlanding )
        {
          // Add an outlanding site to the list.
          outlandingList.append( af );
        }
      else if( afType == BaseMapElement::Gliderfield )
        {
          // Add a glider site to the related list.
          gliderfieldList.append( af );
        }
      else
        {
          // Add an airfield or an ultralight field to the list
          airfieldList.append( af );
        }
    } // End of while( ! in.atEnd() )

  in.close();

  qDebug( "W2000, Statistics from file %s: Parsing Time=%dms, Sum=%d, Airfields=%d, GL=%d, UL=%d, OL=%d",
          basename(path.toLatin1().data()), t.elapsed(), af+gl+ul+ol, af, gl, ul, ol );

  return true;
}

/**
 * Get the distance back in kilometers according to the set unit by
 * the user.
 *
 * @param distance as number
 * @returns distance as double in kilometers
 */
double Welt2000::getDistanceInKm( const int distance )
{
  // we must look, what unit the user has chosen.
  Distance::distanceUnit distUnit = Distance::getUnit();
  Distance dist;
  double unit = 0.0;

  if( distUnit == Distance::kilometers )
    {
      dist.setKilometers( distance );
      unit = dist.getKilometers();
    }
  else if( distUnit == Distance::miles )
    {
      dist.setMiles( distance );
      unit = dist.getKilometers();
    }
  else // if( distUnit == Distance::nautmiles )
    {
      dist.setNautMiles( distance );
      unit = dist.getKilometers();
    }

  return unit;
}
