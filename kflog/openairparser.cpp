/***********************************************************************
 **
 **   openairparser.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2005      by André Somers
 **                   2009-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include <cstdlib>
#include <unistd.h>

#include <QtCore>

#include "airspace.h"
#include "openairparser.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapdefaults.h"
#include "mapmatrix.h"
#include "resource.h"

extern MapContents* _globalMapContents;
extern QSettings    _settings;

OpenAirParser::OpenAirParser()
{
  initializeBaseMapping();
}

OpenAirParser::~OpenAirParser()
{
}

/**
 * Searches on default places for openair files. That can be source
 * files or compiled versions of them.
 *
 * @returns number of successfully loaded files
 * @param list the list of Airspace objects the objects in this
 *   file should be added to.
 */
uint OpenAirParser::load( QList<Airspace>& list )
{
  QTime t;
  t.start();
  uint loadCounter = 0; // number of successfully loaded files
  QStringList mapDirs;
  mapDirs << _globalMapContents->getMapRootDirectory();

  QStringList preselect;

  for ( int i = 0; i < mapDirs.size(); i++ )
    {
      MapContents::addDir(preselect, mapDirs.at(i) + "/airspaces", "*.txt");
      MapContents::addDir(preselect, mapDirs.at(i) + "/airspaces", "*.TXT");
    }

  if (preselect.count() == 0)
    {
      qWarning( "OpenAirParser: No Open Air files could be found in the map directories" );
      return loadCounter;
    }

  preselect.sort();

  // Check, which files shall be loaded.
  QStringList files = _settings.value( "/Airspace/FileList", QStringList(QString("All"))).toStringList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OpenAirParser: No Open Air files defined for loading!";
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

  while( !preselect.isEmpty() )
    {
      QString txtName;

      txtName = preselect.first();

      if( parse( txtName, list ) )
        {
          loadCounter++;
        }

      preselect.removeAt( 0 );
    } // End of While

  qDebug("OpenAirParser: %d OpenAir file(s) loaded in %dms", loadCounter, t.elapsed());
  return loadCounter;
}

bool OpenAirParser::parse(const QString& path, QList<Airspace>& list)
{
  QTime t;
  t.start();
  QFile source(path);

  if (!source.open(QIODevice::ReadOnly))
    {
      qWarning() << "OpenAirParser: Cannot open Airspace file"
                 << path
                 << "!";
      return false;
    }

  resetState();
  initializeStringMapping( path );

  QTextStream in(&source);
  in.setCodec( "ISO 8859-15" );

  //start parsing
  QString line=in.readLine();
  _lineNumber++;

  while (!line.isNull())
    {
      // qDebug("reading line %d: '%s'", _lineNumber, line.toLatin1().data());
      line = line.simplified();

      if (line.startsWith("*") || line.startsWith("#"))
        {
          //comment, ignore
        }
      else if (line.isEmpty())
        {
          //empty line, ignore
        }
      else
        {
          // delete comments at the end of the line before parsing it
          line = line.split('*')[0];
          line = line.split('#')[0];
          parseLine(line);
        }

      line=in.readLine();
      _lineNumber++;
    }

  if (_isCurrentAirspace)
    {
      finishAirspace();
    }

  for (int i  = 0; i < _airlist.count(); i++)
    {
      list.append(_airlist.at(i));
    }

  QFileInfo fi( path );
  qDebug( "OpenAirParser: %d airspace objects read from file %s in %dms",
          _objCounter, fi.fileName().toLatin1().data(), t.elapsed() );

  source.close();
  return true;
}


void OpenAirParser::resetState()
{
  _airlist.clear();
  _direction = 1;
  _lineNumber = 0;
  _objCounter = 0;
  _isCurrentAirspace = false;
}


void OpenAirParser::parseLine(QString& line)
{
  if (line.startsWith("AC "))
    {
      //type of record. This also indicates we're starting a new object
      if (_isCurrentAirspace)
        finishAirspace();
      newAirspace();
      parseType(line);
      return;
    }

  //the rest of the records don't make sense if we're not parsing an object
  int lat, lon;
  double radius;
  bool ok;

  if (!_isCurrentAirspace)
    return;

  if (line.startsWith("AN "))
    {
      //name
      asName = line.mid(3);
      return;
    }

  if (line.startsWith("AH "))
    {
      //airspace ceiling
      QString alt = line.mid(3);
      parseAltitude(alt, asUpperType, asUpper);
      return;
    }

  if (line.startsWith("AL "))
    {
      //airspace floor
      QString alt = line.mid(3);
      parseAltitude(alt, asLowerType, asLower);
      return;
    }

  if (line.startsWith("DP "))
    {
      //polygon coordinate
      QString coord = line.mid(3);

      if( parseCoordinate(coord, lat, lon) )
        {
          asPA.append(QPoint(lat, lon));
        }

      // qDebug( "addDP: lat=%d, lon=%d", lat, lon );
      return;
    }

  if (line.startsWith("DC "))
    {
      //circle
      radius=line.mid(3).toDouble(&ok);

      if (ok)
        {
          addCircle(radius);
        }

      return;
    }

  if (line.startsWith("DA "))
    {

      makeAngleArc(line.mid(3));
      return;
    }

  if (line.startsWith("DB "))
    {
      makeCoordinateArc(line.mid(3));
      return;
    }

  if (line.startsWith("DY "))
    {
      //airway
      return;
    }

  if (line.startsWith("V "))
    {
      parseVariable(line.mid(2));
      return;
    }

  //ignored record types
  if (line.startsWith("AT "))
    {
      //label placement, ignore
      return;
    }

  if (line.startsWith("TO "))
    {
      //terrain open polygon, ignore
      return;
    }

  if (line.startsWith("TC "))
    {
      //terrain closed polygon, ignore
      return;
    }

  if (line.startsWith("SP "))
    {
      //pen definition, ignore
      return;
    }

  if (line.startsWith("SB "))
    {
      //brush definition, ignore
      return;
    }

  //unknown record type
  qDebug( "OAP::parseLine: unknown type at line (%d): %s", _lineNumber,
          line.toAscii().data());
}


void OpenAirParser::newAirspace()
{
  asName = "(unnamed)";
  asType = BaseMapElement::NotSelected;
  //asTypeLetter = "";
  asPA.clear();
  asUpper = BaseMapElement::NotSet;
  asUpperType = BaseMapElement::NotSet;
  asLower = BaseMapElement::NotSet;
  asLowerType = BaseMapElement::NotSet;
  _isCurrentAirspace = true;
  _direction = 1; //must be reset according to specifications
}

void OpenAirParser::newPA()
{
  asPA.clear();
}

void OpenAirParser::finishAirspace()
{
  extern MapMatrix * _globalMapMatrix;

  // @AP: Airspaces are stored as polygons and should not contain the start point
  // twice as done in OpenAir description.
  if ( asPA.count() > 2 && asPA.first() == asPA.last() )
    {
      // remove the last point because it is identical to the first point
      asPA.remove(asPA.count()-1);
    }

  // Translate all WGS84 points to current map projection
  QPolygon astPA;

  for (int i = 0; i < asPA.count(); i++)
    {
      astPA.append( _globalMapMatrix->wgsToMap(asPA.at(i)) );
    }

  Airspace as( asName,
               asType,
               astPA,
               asUpper, asUpperType,
               asLower, asLowerType );

  _airlist.append(as);
  _objCounter++;
  _isCurrentAirspace = false;
  //qDebug("finalized airspace %s. %d points in airspace", asName.toLatin1().data(), asPA.count());
}


void OpenAirParser::initializeBaseMapping()
{
  // create a mapping from a string representation of the supported
  // airspace types in Cumulus to their integer codes
  m_baseTypeMap.clear();

  m_baseTypeMap.insert("AirA", BaseMapElement::AirA);
  m_baseTypeMap.insert("AirB", BaseMapElement::AirB);
  m_baseTypeMap.insert("AirC", BaseMapElement::AirC);
  m_baseTypeMap.insert("AirD", BaseMapElement::AirD);
  m_baseTypeMap.insert("AirE", BaseMapElement::AirE);
  m_baseTypeMap.insert("WaveWindow", BaseMapElement::WaveWindow);
  m_baseTypeMap.insert("AirF", BaseMapElement::AirF);
  m_baseTypeMap.insert("ControlC", BaseMapElement::ControlC);
  m_baseTypeMap.insert("ControlD", BaseMapElement::ControlD);
  m_baseTypeMap.insert("Danger", BaseMapElement::Danger);
  m_baseTypeMap.insert("Restricted", BaseMapElement::Restricted);
  m_baseTypeMap.insert("Prohibited", BaseMapElement::Prohibited);
  m_baseTypeMap.insert("LowFlight", BaseMapElement::LowFlight);
  m_baseTypeMap.insert("Tmz", BaseMapElement::Tmz);
  m_baseTypeMap.insert("GliderSector", BaseMapElement::GliderSector);
}

void OpenAirParser::initializeStringMapping(const QString& mapFilePath)
{
  //fist, initialize the mapping QMap with the defaults
  m_stringTypeMap.clear();

  m_stringTypeMap.insert("A", "AirA");
  m_stringTypeMap.insert("B", "AirB");
  m_stringTypeMap.insert("C", "AirC");
  m_stringTypeMap.insert("D", "AirD");
  m_stringTypeMap.insert("E", "AirE");
  m_stringTypeMap.insert("F", "AirF");
  m_stringTypeMap.insert("GP", "Restricted");
  m_stringTypeMap.insert("R", "Restricted");
  m_stringTypeMap.insert("P", "Prohibited");
  m_stringTypeMap.insert("TRA", "Restricted");
  m_stringTypeMap.insert("Q", "Danger");
  m_stringTypeMap.insert("CTR", "ControlD");
  m_stringTypeMap.insert("TMZ", "Tmz");
  m_stringTypeMap.insert("W", "WaveWindow");
  m_stringTypeMap.insert("GSEC", "GliderSector");

  //then, check to see if we need to update this mapping
  //construct file name for mapping file
  QFileInfo fi(mapFilePath);

  QString path = fi.path() + "/" + fi.baseName() + "_mappings.conf";
  fi.setFile(path);

  if (fi.exists() && fi.isFile() && fi.isReadable())
    {
      QFile f(path);

      if (!f.open(QIODevice::ReadOnly))
        {
          qWarning("OpenAirParser: Cannot open airspace mapping file %s!", path.toLatin1().data());
          return;
        }

      QTextStream in(&f);
      qDebug("Parsing mapping file '%s'.", path.toLatin1().data());

      //start parsing
      QString line = in.readLine();

      while (!line.isNull())
        {
          line = line.simplified();
          if (line.startsWith("*") || line.startsWith("#"))
            {
              //comment, ignore
            }
          else if (line.isEmpty())
            {
              //empty line, ignore
            }
          else
            {
              int pos = line.indexOf("=");
              if (pos>0 && pos < int(line.length()))
                {
                  QString key = line.left(pos).simplified();
                  QString value = line.mid(pos+1).simplified();
                  qDebug("  added '%s' => '%s' to mappings", key.toLatin1().data(), value.toLatin1().data());
                  m_stringTypeMap.remove(key);
                  m_stringTypeMap.insert(key, value);
                }
            }

          line=in.readLine();
        }
    }
}


void OpenAirParser::parseType(QString& line)
{
  line=line.mid(3);

  if (!m_stringTypeMap.contains(line))
    {
      //no mapping from the found type to a Cumulus base type was found
      qWarning("OpenAirParser: Line=%d Type, '%s' not mapped to base type. Object not interpretted.", _lineNumber, line.toLatin1().data());
      _isCurrentAirspace = false; //stop accepting other lines in this object
      return;
    }
  else
    {
      QString stringType = m_stringTypeMap[line];
      if (!m_baseTypeMap.contains(stringType))
        {
          //the indicated base type is not a valid Cumulus base type.
          qWarning("OpenAirParser:=Line %d, Type '%s' is not a valid base type. Object not interpretted.", _lineNumber, stringType.toLatin1().data());
          _isCurrentAirspace = false; //stop accepting other lines in this object
          return;
        }
      else
        {
          //all seems to be right with the world!
          asType = m_baseTypeMap[stringType];
        }
    }
}


void OpenAirParser::parseAltitude(QString& line, BaseMapElement::elevationType& type, int& alt)
{
  bool convertFromMeters = false;
  bool altitudeIsFeet = false;
  QString input = line;
  QStringList elements;
  int len = 0, pos = 0;
  QString part;
  bool ok;
  int num = 0;

  type = BaseMapElement::NotSet;
  alt = 0;
  // qDebug("line %d: parsing altitude '%s'", _lineNumber, line.toLatin1().data());
  //fist, split the string in parsable elements
  //we start with the text parts
  QRegExp reg("[A-Za-z]+");

  while (line.length() > 0)
    {
      pos = reg.indexIn(line, pos + len);
      len = reg.matchedLength();
      if (pos < 0)
        {
          break;
        }
      elements.append(line.mid(pos, len));
    }

  //now, get our number parts

  reg.setPattern("[0-9]+");
  pos=0;
  len=0;

  while (line.length()>0)
    {
      pos = reg.indexIn(line, pos+len);
      len = reg.matchedLength();

      if (pos<0)
        {
          break;
        }
      elements.append(line.mid(pos, len));
      line=line.mid(len);
    }

  // now, try parsing piece by piece
  for ( QStringList::Iterator it = elements.begin(); it != elements.end(); ++it )
    {
      part = (*it).toUpper();
      BaseMapElement::elevationType newType = BaseMapElement::NotSet;

      // first, try to interpret as elevation type
      if ( part == "AMSL" || part == "MSL" || part == "ALT" )
        {
          newType = BaseMapElement::MSL;
        }
      else if ( part == "GND" || part == "SFC" || part == "ASFC" ||
                part == "AGL" || part == "GROUND" )
        {
          newType = BaseMapElement::GND;
        }
      else if (part.startsWith("UNL"))
        {
          newType = BaseMapElement::UNLTD;
        }
      else if (part == "FL")
        {
          newType = BaseMapElement::FL;
        }
      else if (part == "STD")
        {
          newType = BaseMapElement::STD;
        }

      if (type == BaseMapElement::NotSet && newType != BaseMapElement::NotSet)
        {
          type = newType;
          continue;
        }

      if (type != BaseMapElement::NotSet && newType != BaseMapElement::NotSet)
        {
          // @AP: Here we stepped into a problem. We found a second
          // elevation type. That can be only a mistake in the data
          // and will be ignored.
          qWarning( "OpenAirParser: Line=%d, '%s' contains more than one elevation type. Only first one is taken",
                    _lineNumber, input.toLatin1().data());
          continue;
        }

      //see if it is a way of setting units to feet
      if (part == "FT")
        {
          altitudeIsFeet = true;
          continue;
        }

      //see if it is a way of setting units to meters
      if (part == "M")
        {
          convertFromMeters = true;
          continue;
        }

      //try to interpret as a number
      num = part.toInt(&ok);
      if (ok)
        {
          alt = num;
        }

      //ignore other parts
    }
  if ( altitudeIsFeet && type == BaseMapElement::NotSet )
    {
      type = BaseMapElement::MSL;
    }

  if (convertFromMeters)
    {
      alt = (int) rint( alt/Distance::mFromFeet);
    }

  if( alt == 0 && type == BaseMapElement::NotSet )
    {
      // @AP: Altitude is zero but no type is assigned. In this case GND
      // is assumed. Found that in a polish airspace file.
      type=BaseMapElement::GND;
    }

  // qDebug("Line %d: Returned altitude %d, type %d", _lineNumber, alt, int(type));
}


bool OpenAirParser::parseCoordinate(QString& line, int& lat, int& lon)
{
  bool result=true;
  line=line.toUpper();

  int pos=0;
  lat=0;
  lon=0;

  QRegExp reg("[NSEW]");
  pos = reg.indexIn(line, 0);

  if( pos == -1 )
    {
      qWarning() << "OAP::parseCoordinate: line"
                 << _lineNumber
                 << "missing sky directions!";

      return false;
    }

  QString part1 = line.left(pos+1);
  QString part2 = line.mid(pos+1);

  result &= parseCoordinatePart(part1, lat, lon);
  result &= parseCoordinatePart(part2, lat, lon);

  return result;
}

bool OpenAirParser::parseCoordinatePart(QString& line, int& lat, int& lon)
{
  bool ok, ok1, ok2 = false;
  int value = 0;

  if( line.isEmpty() )
    {
      qWarning("OAP: Tried to parse empty coordinate part! Line %d", _lineNumber);
      return false;
    }

  // A input line can contain elements like:
  // P1= "50:11:31.1504N" P2= " 17:42:38.5171E"

  QStringList sl = line.split(QChar(':'));
  QString skyDirection;

  if( sl.size() == 1 )
    {
      // One element is contained, that means decimal degrees
      QString deg = sl.at(0).trimmed();
      skyDirection = deg.right(1);

      if( skyDirection != "N" && skyDirection != "S" && skyDirection != "W" && skyDirection != "E" )
        {
          qWarning() << "OAP::parseCoordinatePart: wrong sky direction at line" << _lineNumber;
          return false;
        }

      deg = deg.left( deg.size() - 1 );

      double ddeg = deg.toDouble(&ok);

      if( ! ok )
        {
          qWarning() << "OAP::parseCoordinatePart: wrong coordinate value"
                     << line << "at line" << _lineNumber;
          return false;
        }

      value = static_cast<int> (rint(ddeg * 600000.0));
    }
  else if( sl.size() == 2 )
    {
      // Two elements are contained
      QString deg = sl.at(0).trimmed();
      QString min = sl.at(1).trimmed();

      skyDirection = min.right(1);

      if( skyDirection != "N" && skyDirection != "S" && skyDirection != "W" && skyDirection != "E" )
        {
          qWarning() << "OAP::parseCoordinatePart: wrong sky direction at line" << _lineNumber;
          return false;
        }

      min = min.left( min.size() - 1 );

      double ddeg = deg.toDouble(&ok);
      double dmin = min.toDouble(&ok1);

      if( ! ok || ! ok1 )
        {
          qWarning() << "OAP::parseCoordinatePart: wrong coordinate value"
                      << line << "at line" << _lineNumber;
          return false;
        }

      value = static_cast<int> (rint((ddeg * 600000.0) + (dmin * 10000.0)));
    }
  else if( sl.size() == 3 )
    {
      // Three elements are contained
      QString deg = sl.at(0).trimmed();
      QString min = sl.at(1).trimmed();
      QString sec = sl.at(2).trimmed();

      skyDirection = sec.right(1);

      if( skyDirection != "N" && skyDirection != "S" && skyDirection != "W" && skyDirection != "E" )
        {
          qWarning() << "OAP::parseCoordinatePart: wrong sky direction" << skyDirection << "at line" << _lineNumber;
          return false;
        }

      sec = sec.left( sec.size() - 1 );

      double ddeg = deg.toDouble(&ok);
      double dmin = min.toDouble(&ok1);
      double dsec = sec.toDouble(&ok2);

      if( ! ok || ! ok1 || ! ok2 )
        {
          qWarning() << "OAP::parseCoordinatePart: wrong coordinate value"
                      << line << "at line" << _lineNumber;
          return false;
        }

      value = static_cast<int> (rint((600000.0 * ddeg) + (10000.0 * (dmin + (dsec / 60.0)))));
    }
  else
    {
      qWarning("OAP::parseCoordinatePart: unknown format! Line %d", _lineNumber);
      return false;
    }

  if( skyDirection == "N" )
    {
      lat = value;
      return true;
    }

  if( skyDirection == "S" )
    {
      lat = -value;
      return true;
    }

  if( skyDirection == "E" )
    {
      lon = value;
      return true;
    }

  if( skyDirection == "W" )
    {
      lon = -value;
      return true;
    }

  return false;
}

bool OpenAirParser::parseCoordinate(QString& line, QPoint& coord)
{
  int lat=0, lon=0;
  bool result = parseCoordinate(line, lat, lon);
  coord.setX(lat);
  coord.setY(lon);
  return result;
}


bool OpenAirParser::parseVariable(QString line)
{
  QStringList arguments=line.split('=');
  if (arguments.count()<2)
    return false;

  QString variable=arguments[0].simplified().toUpper();
  QString value=arguments[1].simplified();
  // qDebug("line %d: variable = '%s', value='%s'", _lineNumber, variable.toLatin1().data(), value.toLatin1().data());
  if (variable=="X")
    {
      //coordinate
      return parseCoordinate(value, _center);
    }

  if (variable=="D")
    {
      //direction
      if (value=="+")
        {
          _direction=+1;
        }
      else if (value=="-")
        {
          _direction=-1;
        }
      else
        {
          return false;
        }
      return true;
    }

  if (variable=="W")
    {
      //airway width
      bool ok;
      double result=value.toDouble(&ok);
      if (ok)
        {
          _awy_width = result;
          return true;
        }
      return false;
    }

  if (variable=="Z")
    {
      //zoom visiblity at zoom level; ignore
      return true;
    }

  return false;
}


// DA radius, angleStart, angleEnd
// radius in nm, center defined by using V X=...
bool OpenAirParser::makeAngleArc(QString line)
{
  //qDebug("OpenAirParser::makeAngleArc");
  bool ok;
  double radius, angle1, angle2;

  QStringList arguments = line.split(',');
  if (arguments.count()<3)
    return false;

  radius=arguments[0].trimmed().toDouble(&ok);

  if ( !ok )
    {
      return false;
    }

  angle1=arguments[1].trimmed().toDouble(&ok);

  if (!ok)
    {
      return false;
    }

  angle2=arguments[2].trimmed().toDouble(&ok);

  if (!ok)
    {
      return false;
    }

  int lat = _center.x();
  int lon = _center.y();

  // grenzen 180 oder 90 beachten!
  double distLat = dist( lat, lon, lat + 10000, lon );
  double distLon = dist( lat, lon, lat, lon + 10000 );

  double kmr = radius * MILE_kfl / 1000.;
  //qDebug( "distLat=%f, distLon=%f, radius=%fkm", distLat, distLon, kmr );

  addArc( kmr/(distLat/10000.), kmr/(distLon/10000.), angle1/180*M_PI, angle2/180*M_PI );
  return true;
}


/**
   Calculate the bearing from point p1 to point p2 from WGS84
   coordinates to avoid distortions caused by projection to the map.
*/
double OpenAirParser::bearing( QPoint& p1, QPoint& p2 )
{
  // Arcus computing constant for kflog corordinates. PI is devided by
  // 180 degrees multiplied with 600.000 because one degree in kflog
  // is multiplied with this resolution factor.
  const double pi_180 = M_PI / 108000000.0;

  // qDebug( "x1=%d y1=%d, x2=%d y2=%d",  p1.x(), p1.y(), p2.x(), p2.y() );

  int dx = p2.x() - p1.x(); // latitude
  int dy = p2.y() - p1.y(); // longitude

  // compute latitude distance in meters
  double latDist = dx * MILE_kfl / 10000.; // b

  // compute latitude average
  double latAv = ( ( p2.x() + p1.x() ) / 2.0);

  // compute longitude distance in meters
  double lonDist = dy * cos( pi_180 * latAv ) * MILE_kfl / 10000.; // a

  // compute angle
  double angle = asin( fabs(lonDist) / hypot( latDist, lonDist ) );

  // double angleOri = angle;

  // assign computed angle to the right quadrant
  if ( dx >= 0 && dy < 0 )
    {
      angle = (2 * M_PI) - angle;
    }
  else if ( dx <=0 && dy <= 0 )
    {
      angle =  M_PI + angle;
    }
  else if ( dx < 0 && dy >= 0 )
    {
      angle = M_PI - angle;
    }

  //qDebug( "dx=%d, dy=%d - AngleRadOri=%f, AngleGradOri=%f - AngleRadCorr=%f, AngleGradCorr=%f",
  //  dx, dy, angleOri, angleOri * 180/M_PI, angle, angle * 180/M_PI);

  return angle;
}


/**
 * DB coordinate1, coordinate2
 * center defined by using V X=...
 */
bool OpenAirParser::makeCoordinateArc(QString line)
{
  // qDebug("OpenAirParser::makeCoordinateArc");
  double radius, angle1, angle2;

  //split of the coordinates, and check the number of arguments
  QStringList arguments = line.split(',');
  if (arguments.count()<2)
    return false;

  QPoint coord1, coord2;

  //try to parse the coordinates
  if (!(parseCoordinate(arguments[0], coord1) && parseCoordinate(arguments[1], coord2)))
    return false;

  //calculate the radius by taking the average of the two distances (in km)
  radius = (dist(&_center, &coord1) + dist(&_center, &coord2)) / 2.0;

  //qDebug( "Radius=%fKm, Dist1=%f, Dist2=%f",
  //radius, dist(&_center, &coord1), dist(&_center, &coord2) );

  int lat = _center.x();
  int lon = _center.y();

  // grenzen 180 oder 90 beachten!
  double distLat = dist( lat, lon, lat + 10000, lon );
  double distLon = dist( lat, lon, lat, lon + 10000 );

  //qDebug( "distLat=%f, distLon=%f, radius=%fkm", distLat, distLon, radius );

  // get the angles by calculating the bearing from the centerpoint to the WGS84 coordinates
  angle1 = bearing(_center, coord1);
  angle2 = bearing(_center, coord2);

  // add the arc to the point array
  addArc( radius/(distLat/10000.), radius/(distLon/10000.), angle1, angle2 );
  return true;
}


void OpenAirParser::addCircle(const double& rLat, const double& rLon)
{
  double x, y, phi;

  // qDebug("rLat: %d, rLon:%d", rLat, rLon);
  for (int i=0; i<360; i+=1)
    {
      phi=double(i)/180.0*M_PI;
      x = cos(phi)*rLat;
      y = sin(phi)*rLon;
      x +=_center.x();
      y +=_center.y();

      asPA.append( QPoint(int(rint(x)), int(rint(y))) );
    }
}


void OpenAirParser::addCircle(const double& radius)
{
  int lat = _center.x();
  int lon = _center.y();

  // Check limits 180 or 90 degrees?
  double distLat = dist( lat, lon, lat + 10000, lon );
  double distLon = dist( lat, lon, lat, lon + 10000 );

  double kmr = radius * MILE_kfl / 1000.;

  //qDebug( "distLat=%f, distLon=%f, radius=%fkm", distLat, distLon, kmr );

  addCircle( kmr/(distLat/10000.), kmr/(distLon/10000.) );  // kilometer/minute
}


#define STEP_WIDTH 1

void OpenAirParser::addArc(const double& rX, const double& rY,
                           double angle1, double angle2)
{
  //qDebug("addArc() dir=%d, a1=%f a2=%f",_direction, angle1*180/M_PI , angle2*180/M_PI );

  double x, y;

  if (_direction > 0)
    {
      if (angle1 >= angle2)
        angle2 += 2.0 * M_PI;
    }
  else
    {
      if (angle2 >= angle1)
        angle1 += 2.0 * M_PI;
    }

  int nsteps = abs(int(((angle2 - angle1) * 180) / (STEP_WIDTH * M_PI))) + 2;

  //qDebug("delta=%d pai=%d",int(((angle2-angle1)*180)/(STEP_WIDTH*M_PI)), pai );

  const double step = (STEP_WIDTH * M_PI) / 180.0;

  double phi = angle1;

  for (int i = 0; i < nsteps - 1; i++)
    {
      x = (cos(phi) * rX) + _center.x();
      y = (sin(phi) * rY) + _center.y();

      asPA.append( QPoint((int) rint(x), (int) rint(y)) );

      if (_direction > 0) //clockwise
        {
          phi += step;
        }
      else
        {
          phi -= step;
        }
    }

  x = (cos(angle2) * rX) + _center.x();
  y = (sin(angle2) * rY) + _center.y();

  asPA.append( QPoint( (int) rint(x), (int) rint(y)) );
}
