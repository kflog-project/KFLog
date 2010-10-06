/***********************************************************************
 **
 **   welt2000.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2006-2008 by Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id: welt2000.cpp 2697 2008-04-11 18:37:43Z axel $
 **
 ***********************************************************************/

#include <math.h>
#include <unistd.h>

#include <qbuffer.h>
#include <qdatetime.h>
#include <qdict.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpoint.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qtextstream.h>

#include "airport.h"
#include "basemapelement.h"
#include "filetools.h"
#include "glidersite.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "projectionbase.h"
#include "resource.h"
#include "runway.h"
#include "wgspoint.h"
#include "distance.h"

#include "welt2000.h"

// All is prepared for additional calculation, storage and
// reconstruction of a bounding box. Be free to switch on/off it via
// conditional define.

#undef BOUNDING_BOX
// #define BOUNDING_BOX 1


// type definition for compiled airfield files
#define FILE_TYPE_AIRFIELD_C 0x62

// version used for files created from welt2000 data
#define FILE_VERSION_AIRFIELD_C 201

extern MapContents  _globalMapContents;
extern MapMatrix    _globalMapMatrix;

Welt2000::Welt2000()
{
  h_projection = (ProjectionBase *) 0;

  // prepare base mappings of cumulus
  c_baseTypeMap.insert( "IntAirport", BaseMapElement::IntAirport );
  c_baseTypeMap.insert( "Airport", BaseMapElement::Airport );
  c_baseTypeMap.insert( "MilAirport", BaseMapElement::MilAirport );
  c_baseTypeMap.insert( "CivMilAirport", BaseMapElement::CivMilAirport );
  c_baseTypeMap.insert( "Airfield", BaseMapElement::Airfield );
  c_baseTypeMap.insert( "ClosedAirfield", BaseMapElement::ClosedAirfield );
  c_baseTypeMap.insert( "CivHeliport", BaseMapElement::CivHeliport );
  c_baseTypeMap.insert( "MilHeliport", BaseMapElement::MilHeliport );
  c_baseTypeMap.insert( "AmbHeliport", BaseMapElement::AmbHeliport );
  c_baseTypeMap.insert( "Glidersite", BaseMapElement::Glidersite );
  c_baseTypeMap.insert( "UltraLight", BaseMapElement::UltraLight );
  c_baseTypeMap.insert( "HangGlider", BaseMapElement::HangGlider );
}


Welt2000::~Welt2000()
{
  if( h_projection )
    {
      delete h_projection;
    }
}


/**
 * search on default places a welt2000 file and load it. A source can
 * be the original ascii file or a compiled version of it. The results
 * are put in the passed lists
 */
bool Welt2000::load( QPtrList<Airport>& airportList, QPtrList<GliderSite>& gliderSiteList )
{
  // Rename WELT2000.TXT -> welt2000.txt.
  QString wl = "welt2000.txt";
  QString wu = "WELT2000.TXT";
  QString sd = "/airfields/";

  extern QSettings _settings;
  QString mapDir = _settings.readEntry("/KFLog/Path/DefaultMapDirectory", QDir::homeDirPath() + "/.kflog/mapdata");

  QString pl = mapDir + sd + wl;
  QString pu = mapDir + sd + wu;
  rename( pu.latin1(), pl.latin1() );

  QString w2PathTxt;

  QFile test;
  test.setName(mapDir + "/airfields/welt2000.txt");
  if (test.exists()) {
    w2PathTxt=test.name();
  }
  else {
    qWarning( "W2000: No Welt2000 files could be found in the map directories" );
    return false;
  }

  // parse source file
  return parse( w2PathTxt, airportList, gliderSiteList, true );
}


/**
 * The passed file has to be a welt2000 file. All not relevant
 * entries, like turn points, will be filtered out. A new file is
 * written with the same name and an own header.
 */
bool Welt2000::filter( QString& path )
{
  QString header1 = "# Welt2000 file was filtered by cumulus at ";
  QString header2 = "# Please do not modify or remove this header";

  QFile in(path);
  QString fout = path + ".filtered";
  QFile out(fout);

  if( !in.open(IO_ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.latin1());
      return false;
    }

  QTextStream ins(&in);
  QTextStream outs;

  uint outLines = 0; // counter for written lines

  while( ! ins.atEnd() )
    {
      QString line = ins.readLine();

      if( outLines == 0 )
        {
          if( line.startsWith( header1 ) )
            {
              // File was already filtered, stop all doing
              in.close();
              return true;
            }

          // open output file for filtering
          if( !out.open(IO_WriteOnly) )
            {
              in.close();
              qWarning("W2000: Cannot open temporary file %s!", fout.latin1());
              return false;
            }

          outs.setDevice( &out );
          outs << header1.latin1() << QDateTime::currentDateTime().toString().latin1() << '\n'
               << header2.latin1() << '\n' << '\n';
          outLines += 2;
        }

      // remove white spaces and line end characters
      line = line.stripWhiteSpace();

      // remove temporary commented out entries
      if( line.startsWith("$-") || line.startsWith("$+") ||
          line.startsWith("$*") || line.startsWith("$$") )
        {
          continue;
        }

      // real comments are not filtered out
      if( line.startsWith("#") || line.startsWith("$") )
        {
          outs << line.latin1() << '\n';
          outLines++;
          continue;
        }

      if( line.length() < 62 )
        {
          // country sign not included, skip over it
          continue;
        }

      // look, what kind of line was read.
      // COL5 = 1 Airfield
      // COL5 = 2 Outlanding, can contain UL fields
      QString kind = line.mid( 5, 1 );

      if( kind != "1" && kind != "2" )
        {
          continue; // skip it, not of interest for us
        }

      // Check, if we have an outlanding point. Under this we have to
      // expect also ul fields. If no ul field is defined we will
      // ignore the line.
      if( kind == "2" && line.mid( 23, 4 ) != "*ULM" )
        {
          // ignore outlandings
          continue;
        }

      outs << line.latin1() << '\n';
      outLines++;
    }

  in.close();
  out.close();

  if( outLines > 2 )
    {
      // overwrite old file with new extracted file
      rename( fout.latin1(), path.latin1() );
    }
  else
    {
      // remove unneeded file, if nothing could be extracted
      unlink( fout.latin1() );
    }

  return true;
}


/**
 * The passed file can contain country information, to be used during
 * parsing of welt2000.txt file. The entries country and home radius
 * can be overwritten by user values defined in GeneralConfig class.
 *
 * File syntax: [#$] These 2 signs starts a comment line, it ends with the newline
 *              FILTER countries=<country_1>,<country_2>,...<country_n>'\nl'
 *              FILTER countries=....
 *              MAP_ICAO <name>=[IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|CivHeliport|MilHeliport|AmbHeliport|Glidersite|UltraLight|HangGlider]
 *              MAP_SHORT_NAME <name>=[IntAirport|Airport|MilAirport|CivMilAirport|Airfield|ClosedAirfield|CivHeliport|MilHeliport|AmbHeliport|Glidersite|UltraLight|HangGlider]
 *
 * You can define several filter lines, all will be processed.
 *
 */
bool Welt2000::readConfigEntries( QString &path )
{
  c_countryList.clear();
  c_homeRadius = 0.0;
  c_icaoMap.clear();
  c_shortMap.clear();

  QFile in(path);

  if( !in.open(IO_ReadOnly) )
    {
      qWarning("W2000: User has not provided a configuration file %s!", path.latin1());
      return false;
    }

  QTextStream ins(&in);

  while( ! ins.atEnd() )
    {
      QString line = ins.readLine();

      if( line.isEmpty() )
        {
          continue; // skip empty lines
        }

      // remove white spaces and line end characters
      line = line.stripWhiteSpace();

      // step over comment lines
      if( line.startsWith("#") || line.startsWith("$") )
        {
          continue;
        }

      if( line.startsWith("FILTER") || line.startsWith("filter") )
        {
          QStringList list = QStringList::split(QRegExp("[=,]"), line);

          if( list.count() < 2 || list[0].contains("countries", false) == 0 )
            {
              // No country elements to find in list
              continue;
            }

          // remove first entry, it is the filter-country key
          list.remove( list.begin() );

          for( uint i = 0; i < list.count(); i++ )
            {
              QString e = list[i].stripWhiteSpace().upper();

              if( c_countryList.contains(e) )
                continue;

              c_countryList += e;
            }

          c_countryList.sort();
          continue;
        }

      if( line.startsWith("MAP_") || line.startsWith("map_") )
        {
          QStringList list = QStringList::split(QRegExp("[=]"), line);

          if( list.count() < 2 )
            {
              // No map elements to find in list
              continue;
            }

          if( list[0].contains("MAP_ICAO", false) )
            {
              list[0].remove( 0, 8 );
              list[0] = list[0].stripWhiteSpace().upper(); // icao name of airfield
              list[1] = list[1].stripWhiteSpace(); // new map type for airfield
              c_icaoMap.insert( list[0], list[1] );
              // qDebug("W2000: c_icaoMap.insert(%s, %s)", list[0].latin1(), list[1].latin1());
            }
          else if( list[0].contains("MAP_SHORT_NAME", false) )
            {
              list[0].remove( 0, 14 );
              list[0] = list[0].stripWhiteSpace(); // short name of airfield
              list[1] = list[1].stripWhiteSpace(); // new map type for airfield
              c_shortMap.insert( list[0], list[1] );
              // qDebug("W2000: c_shortMap.insert(%s, %s)", list[0].latin1(), list[1].latin1());
            }
        }

    } // End of while( ! in.atEnd() )

  in.close();

  return true;
}


/**
 * Parses the passed file in welt 2000 format and put the approriate
 * entries in the related lists.
 * 
 * arg1 path: Full name with path of welt2000 file
 * arg2 airportList: All airports have to be stored in this list
 * arg3 glidertList: All gilder fields have to be stored in this list
 * arg4 doCompile: create a binary file of the parser results,
 *                 if flag is set to true. Default is false.
 * returns true (success) or false (error occured)
 */
bool Welt2000::parse( QString& path,
                      QPtrList<Airport>& airportList,
                      QPtrList<GliderSite>& gliderSiteList,
                      bool /*doCompile*/ )
{
  QTime t;
  t.start();

  // Filter out the needed extract for us from the welt 2000
  // file. That will reduce the file size over the half.
  if( filter( path ) == false )
    {
      // It seems, that no welt 2000 file has been passed
      return false;
    }

  QFile in(path);

  if( !in.open(IO_ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.latin1());
      return false;
    }

  QTextStream ins(&in);

  // look, if a config file is accessable. If yes read out its data.
  QFileInfo fi( path );
  QString confFile = fi.dirPath(TRUE) + "/welt2000.conf";

  // It is expected that the filter file is located in the same
  // directory as the welt2000.txt file and carries the name
  // welt2000.conf
  readConfigEntries( confFile );

  // Check, if in GeneralConfig other definitions exist. These will
  // overwrite the definitions in the config file.

  extern QSettings _settings;
  QString cFilter = _settings.readEntry("/KFLog/MapData/Welt2000CountryFilter", "");

  if( cFilter.length() > 0 )
    {
      // load new country filter definitions
      c_countryList.clear();

      QStringList clist = QStringList::split( QRegExp("[, ]"), cFilter );

      for( uint i = 0; i < clist.count(); i++ )
        {
          QString e = clist[i].stripWhiteSpace().upper();
      
          if( c_countryList.contains(e) )
            continue;

          c_countryList += e;
        }

      c_countryList.sort();
    }

  // get home radius from config data
  int radius = _settings.readNumEntry("/KFLog/MapData/Welt2000HomeRadius", 0);

  if( radius == 0 )
    {
      // default is 1000 kilometers
      c_homeRadius = 1000.0;
    }
  else
    {
      // we must look, what unit the user has choosen. This unit must
      // be considered during load of airfield data.
      c_homeRadius = getDistanceInKm( radius );
    }

  qDebug( "W2000: File welt2000.conf contains %d country entries", c_countryList.count() );
  qDebug( "W2000: File welt2000 defines the home radius to %.1f Km", c_homeRadius );


  // put all entries of contry list into a dictionary for faster
  // access
  QDict<char> countryDict( 101, FALSE );

  for( uint i = 0; i < c_countryList.count(); i++ )
    {
      // populate country dictionary
      countryDict.insert( c_countryList[i], c_countryList[i] );
    }

  // Prepare all for a binary storage of the parser results.
  QString compileFile;
  QFile   compFile;
  QDataStream out;
  QCString bufdata;
  QBuffer buffer(bufdata);
  QDataStream outbuf;

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  uint lineNo = 0;
  QString lastName = "";
//  uint counter = 0;
  uint lastCounter = 0;

  // statistics counter
  uint ul, gl, af;
  ul = gl = af = 0;

  // Input file was taken from Michael Meiers welt 2000 data dase
  //
  // 0         1         2         3         4         5         6
  // 0123456789012345678901234567890123456789012345678901234567890123
  // 1234567890123456789012345678901234567890123456789012345678901230
  // AACHE1 AACHEN  MERZBRUC#EDKAA 53082612287 189N504923E0061111DEO5
  // AICHA1 AICHACH         # S !G 43022012230 440N482824E0110807DEX
  // ARGEN2 ARGENBUEHL EISE?*ULM G 40082612342 686N474128E0095744DEN
  // BASAL2 BAD SALZUNGEN UL*ULM G 65092712342 233N504900E0101305DEN
  // FUERS1 FUERSTENWALDE   #EDALG 80112912650  55N522323E0140539DEO3
  // BERLT1 BERLIN  TEGEL   #EDDTA303082611870  37N523335E0131716DEO
  // BERSC1 BERLIN SCHOENFEL#EDDBC300072512002  49N522243E0133114DEO
  // BERTE1 BERLIN TEMPELHOF#EDDIC208092711810  52N522825E0132406DEO

  while( ! in.atEnd() )
    {
      bool ok;
      QString line, buf;
      int result = in.readLine(line, 128);
      lineNo++;

      if( result <= 0 )
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
      line = line.stripWhiteSpace();

      // replace markers against space
      line.replace( QRegExp("[!?]+"), " " );

      if( line.length() < 62 )
        {
          // country sign not included
          continue;
        }

      // get short name for user mapping before changing line
      QString shortName = line.mid( 0, 6 );

      // convert all to upper case
      line = line.upper();

      // Extract country sign. It is coded according to ISO 3166.
      QString country = line.mid( 60, 2 );

      if( ! countryDict.isEmpty() )
        {
          if( countryDict[country] == 0  )
            continue;
        }

      // look, what kind of line was read.
      // COL5 = 1 Airfield
      // COL5 = 2 Outlanding, contains also UL places

      QString kind = line.mid( 5, 1 );

      if( kind != "1" && kind != "2" )
        {
          continue; // not of interest for us
        }

      bool ulField = false;
      bool glField = false;
      bool afField = false;
      QString icao;

      if( kind == "2" ) // can be an UL field
        {
          if( line.mid( 23, 4 ) == "*ULM" )
            {
              ulField = true;
            }
          else
            {
              // step over other outlandings
              continue;
            }
        }
      else if( line.mid( 23, 3 ) == "# S" )
        {
          // Glider field
          glField = true;
        }
      else
        {
          afField = true;
          icao = line.mid( 24, 4 ).stripWhiteSpace().upper();

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
      afName = afName.simplifyWhiteSpace();

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
          afType = BaseMapElement::Glidersite;
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

      // make the user's wanted mapping for short name
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
      afName = afName.lower();

      QChar lastChar(' ');

      // convert airfield names to upper-lower
      for( uint i=0; i < afName.length(); i++ )
        {
          if( lastChar == ' ' )
            {
              afName.replace( i, 1, afName.mid(i,1).upper() );
            }

          lastChar = afName[i];
        }

      // gps name, we use 8 characters without spaces
      QString gpsName = afName.left(8);

      if( lastName == gpsName )
        {
          gpsName.replace( gpsName.length()-1, 1, QString::number(++lastCounter) );
        }
      else
        {
          lastName = gpsName;
          lastCounter = 0;
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

      Q_INT32 lat, lon;
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
                    lineNo, afName.latin1(), country.latin1() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude minute value, ignoring entry!",
                    lineNo, afName.latin1(), country.latin1() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude second value, ignoring entry!",
                    lineNo, afName.latin1(), country.latin1() );
          continue;
        }

      double latTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lat = (Q_INT32) rint(latTmp);

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
                    lineNo, afName.latin1(), country.latin1() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude minute value, ignoring entry!",
                    lineNo, afName.latin1(), country.latin1() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude second value, ignoring entry!",
                    lineNo, afName.latin1(), country.latin1() );
          continue;
        }

      double lonTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lon = (Q_INT32) rint(lonTmp);

      if( line[52] == 'W' )
        {
          lon = -lon;
        }

      if( countryDict.isEmpty() )
        {
          // No countries are defined to be filtered out, we will
          // compute the distance between the home position and the
          // read point. Is the distance is over the user defined
          // value away we will ignore this point.

          QPoint home( _settings.readNumEntry("/KFLog/MapData/HomesiteLatitude"),
                       _settings.readNumEntry("/KFLog/MapData/HomesiteLongitude") );
          QPoint af( lat, lon );

          double d = dist( &home, &af );

          if( d > c_homeRadius )
            {
              // Distance is greater than defined radius in generalconfig
              // qDebug("Ignoring Dist=%f, AF=%s", d, afName.latin1());
              continue;
            }
        }

#ifdef BOUNDING_BOX
      // update the bounding box
      _globalMapContents.AddPointToRect( boundingBox, QPoint(lat, lon) );
#endif

      WGSPoint wgsPos(lat, lon);
      QPoint position = _globalMapMatrix.wgsToMap(wgsPos);

      // elevation
      buf = line.mid(41,4 ).stripWhiteSpace();

      ok = false;
      Q_INT16 elevation = 0;

      if( ! buf.isEmpty() )
        {
          elevation = buf.toInt(&ok);
        }

      if( ! ok )
        {
          //qWarning( "W2000, Line %d: %s (%s) missing or wrong elevation, set value to 0!",
          //          lineNo, afName.latin1(), country.latin1() );
          elevation = 0;
        }

      // frequency
      QString frequency = line.mid(36,3) + "." +
        line.mid(39,2).stripWhiteSpace();

      double f = frequency.toDouble(&ok);

      if( !ok || f < 117.97 || f > 137.0 )
        {
          //qWarning( "W2000, Line %d: %s (%s) missing or wrong frequency, set value to 0!",
          //          lineNo, afName.latin1(), country.latin1() );
          frequency = "000.000";
        }

      // check, what has to be appended as last digit
      if( line[40] == '2' || line[40] == '7' )
        {
          frequency += "5";
        }
      else
        {
          frequency += "0";
        }

      // runway direction as two digits, we consider only the first entry
      buf = line.mid(32,2).stripWhiteSpace();

      ok = false;
      uint rwDir = 0;

      if( ! buf.isEmpty() )
        {
          rwDir = buf.toUInt(&ok);
        }

      if( ! ok )
        {
          //qWarning( "W2000, Line %d: %s (%s) missing or wrong runway direction, set value to 0!",
          //          lineNo, afName.latin1(), country.latin1() );
          rwDir = 0;
        }

      // runway length in meters, must be multiplied by 10
      buf = line.mid(29,3).stripWhiteSpace();

      ok = false;
      uint rwLen = 0;

      if( ! buf.isEmpty() )
        {
          rwLen = buf.toUInt(&ok);
        }

      if( ! ok )
        {
          //qWarning( "W2000, Line %d: %s (%s) missing or wrong runway length, set value to 0!",
          //          lineNo, afName.latin1(), country.latin1() );
          rwLen = 0;
        }
      else
        {
          rwLen *= 10;
        }

      // runway surface
      uint rwSurface;
      QChar rwType = line[28];

      if( rwType == 'A' )
        {
          rwSurface = Airport::Asphalt;
        }
      else if( rwType == 'C' )
        {
          rwSurface = Airport::Concrete;
        }
      else if( rwType == 'G' )
        {
          rwSurface = Airport::Grass;
        }
      else
        {
          rwSurface = Airport::Unknown;
        }

      //---------------------------------------------------------------
      // append a new record to the related list
      //---------------------------------------------------------------

      // count output records separated by kind
      if( ulField )
        ul++;
      else if( glField )
        gl++;
      else
        af++;

      // create an runway object
      new runway( rwLen ,rwDir*10, rwSurface, 1 );

      if( afType != BaseMapElement::Glidersite )
        {
          // Add a non glider site to the list. That can be an
          // airfield or an ultralight field
          // FIXME: To be conform with cumulus we should add the runway information here!
          Airport *ap = new Airport( afName, icao.stripWhiteSpace(), gpsName.upper(), afType,
                                     wgsPos, position, elevation, frequency, false );

          airportList.append( ap );
        }
      else
        {
          // Add a glider site to the list.
          // FIXME: To be conform with cumulus we should add the runway information here!
          GliderSite *gl = new GliderSite( afName, icao.stripWhiteSpace(), gpsName.upper(),
                                           wgsPos, position, elevation, frequency, false );

          gliderSiteList.append( gl );
        }

    } // End of while( ! in.atEnd() )

  in.close();

  //qDebug( "W2000, Statistics from file %s: Parsing Time=%dms, Sum=%d, Airfields=%d, GL=%d, UL=%d",
  //        basename(path.latin1()), t.elapsed(), af+gl+ul, af, gl, ul );

  return true;
}


/**
 * Get the header data of a compiled file and put it in our class
 * variables.
 */
bool Welt2000::setHeaderData( QString &path )
{
  h_headerIsValid = false; // save read result here too

  h_magic = 0;
  h_fileType = 0;
  h_fileVersion = 0;
  h_countryList.clear();
  h_homeRadius = 0.0;
  h_homeCoord.setX(0);
  h_homeCoord.setY(0);

  if( h_projection )
    {
      // delete an older projection object
      delete  h_projection;
      h_projection = 0;
    }

  QFile inFile(path);
  if( !inFile.open(IO_ReadOnly) )
    {
      qWarning("W2000: Cannot open airfield file %s!", path.latin1());
      return false;
    }

  QDataStream in(&inFile);

  in >> h_magic;

  if( h_magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "W2000: wrong magic key %x read! Aborting ...", h_magic );
      inFile.close();
      return false;
    }

  in >> h_fileType;

  if( h_fileType != FILE_TYPE_AIRFIELD_C )
    {
      qWarning( "W2000: wrong file type %x read! Aborting ...", h_fileType );
      inFile.close();
      return false;
    }

  in >> h_fileVersion;

  if( h_fileVersion != FILE_VERSION_AIRFIELD_C )
    {
      qWarning( "W2000: wrong file version %x read! Aborting ...", h_fileVersion );
      inFile.close();
      return false;
    }

  in >> h_creationDateTime;
  in >> h_countryList;
  in >> h_homeRadius;
  in >> h_homeCoord;

#ifdef BOUNDING_BOX

  in >> h_boundingBox;
#endif

  h_projection = LoadProjection(in);

  inFile.close();
  h_headerIsValid = true; // save read result here too
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
  // we must look, what unit the user has choosen.
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
