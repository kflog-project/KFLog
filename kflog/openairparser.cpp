/***********************************************************************
 **
 **   openairparser.cpp
 **
 **   This file is part of KFlog2.
 **
 ************************************************************************
 **
 **   Copyright (c):  2005 by André Somers, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

//standard libs includes
#include <math.h>
#include <cstdlib>
#include <unistd.h>

//Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

//project includes
#include "airspace.h"
#include "filetools.h"
#include "distance.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "mapcontents.h"
#include "openairparser.h"
#include "projectionbase.h"
#include "resource.h"

// All is prepared for additional calculation, storage and
// reconstruction of a bounding box. Be free to switch on/off it via
// conditional define.

#undef BOUNDING_BOX
// #define BOUNDING_BOX 1

// type definition for compiled airspace files
#define FILE_TYPE_AIRSPACE_C 0x61

// version used for files created from OpenAir data
#define FILE_VERSION_AIRSPACE_C 200

extern MapContents  _globalMapContents;
extern MapMatrix    _globalMapMatrix;

OpenAirParser::OpenAirParser()
{
  _boundingBox = (QRect *) 0;
  _bufData = (QCString *) 0;
  _outbuf = (QDataStream *) 0;

  initializeBaseMapping();
}


OpenAirParser::~OpenAirParser()
{
}

/**
 * Searchs on default places for openair files. That can be source
 * files or compiled versions of them.
 *
 * @returns number of successfully loaded files
 * @param list the list of Airspace objects the objects in this
 *   file should be added to.
 */

uint OpenAirParser::load( QPtrList<Airspace>& list )
{
  extern QSettings _settings;
  QTime t;
  t.start();
  uint loadCounter = 0; // number of successfully loaded files

  QString mapDir = _settings.readEntry("/KFLog/Path/DefaultMapDirectory", QDir::homeDirPath() + "/.kflog/mapdata/");
  QStringList preselect;

  MapContents::addDir(preselect, mapDir + "/airspaces", "*.txt");
  MapContents::addDir(preselect, mapDir + "/airspaces", "*.TXT");

  if(preselect.count() == 0)
    {
      qWarning( "OpenAirParser: No Open Air files could be found in the map directories" );
      return loadCounter;
    }

  while ( ! preselect.isEmpty() ) {
    QString txtName = preselect.first();

    if( parse( txtName, list ) )
      loadCounter++;

    preselect.remove( preselect.begin() );
  } // End of While

  qDebug("OpenAirParser: %d OpenAir file(s) loaded in %dms", loadCounter, t.elapsed());
  return loadCounter;
}


bool OpenAirParser::parse(const QString& path, QPtrList<Airspace>& list)
{
  QTime t;
  t.start();
  QFile source(path);

  if (!source.open(IO_ReadOnly)) {
    qWarning("OpenAirParser: Cannot open airspace file %s!", path.latin1());
    return false;
  }

  resetState();
  initializeStringMapping( path );

  QTextStream in(&source);
  //start parsing
  QString line=in.readLine();
  _lineNumber++;
  while (!line.isNull()) {
    //qDebug("reading line %d: '%s'", _lineNumber, line.latin1());
    line = line.simplifyWhiteSpace();
    if (line.startsWith("*") || line.startsWith("#")) {
      //comment, ignore
    } else if (line.isEmpty()) {
      //empty line, ignore
    } else {
      // delete comments at the end of the line before parsing it
      line = QStringList::split('*', line)[0];
      line = QStringList::split('#', line)[0];
      parseLine(line);
    }
    line=in.readLine();
    _lineNumber++;
  }

  if (_isCurrentAirspace)
    finishAirspace();

  for (size_t i  = 0; i < _airlist.count(); i++) {
    list.append(_airlist.at(i));
  }

  QFileInfo fi( path );
  qDebug( "OpenAirParser: %d airspace objects read from file %s in %dms",
          _objCounter, fi.fileName().latin1(), t.elapsed() );

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
  if (line.startsWith("AC ")) {
    //type of record. This also indicates we're starting a new object
    if (_isCurrentAirspace)
      finishAirspace();
    newAirspace();
    parseType(line);
    return;
  }

  //the rest of the records don't make sence if we're not parsing an object
  int lat, lon;
  double radius;
  bool ok;

  if (!_isCurrentAirspace)
    return;

  if (line.startsWith("AN ")) {
    //name
    asName = line.mid(3);
    return;
  }

  if (line.startsWith("AH ")) {
    //airspace ceiling
    QString alt = line.mid(3);
    parseAltitude(alt, asUpperType, asUpper);
    return;
  }

  if (line.startsWith("AL ")) {
    //airspace floor
    QString alt = line.mid(3);
    parseAltitude(alt, asLowerType, asLower);
    return;
  }

  if (line.startsWith("DP ")) {
    //polygon coordinate
    QString coord = line.mid(3);
    parseCoordinate(coord, lat, lon);
    asPA.resize(asPA.count()+1);
    asPA.setPoint(asPA.count()-1,lat,lon);
    // qDebug( "addDP: lat=%d, lon=%d", lat, lon );
    return;
  }

  if (line.startsWith("DC ")) {
    //circle
    radius=line.mid(3).toDouble(&ok);
    if (ok) {
      addCircle(radius);
    }
    return;
  }

  if (line.startsWith("DA ")) {

    makeAngleArc(line.mid(3));
    return;
  }

  if (line.startsWith("DB ")) {
    makeCoordinateArc(line.mid(3));
    return;
  }

  if (line.startsWith("DY ")) {
    //airway
    return;
  }

  if (line.startsWith("V ")) {
    parseVariable(line.mid(2));
    return;
  }

  //ignored record types
  if (line.startsWith("AT ")) {
    //label placement, ignore
    return;
  }

  if (line.startsWith("TO ")) {
    //terrain open polygon, ignore
    return;
  }

  if (line.startsWith("TC ")) {
    //terrain closed polygon, ignore
    return;
  }

  if (line.startsWith("SP ")) {
    //pen definition, ignore
    return;
  }

  if (line.startsWith("SB ")) {
    //brush definition, ignore
    return;
  }
  //unknown record type
}


void OpenAirParser::newAirspace()
{
  asName = "(unnamed)";
  asType = BaseMapElement::NotSelected;
  //asTypeLetter = "";
  asPA.resize(0);
  asUpper = BaseMapElement::NotSet;
  asUpperType = BaseMapElement::NotSet;
  asLower = BaseMapElement::NotSet;
  asLowerType = BaseMapElement::NotSet;
  _isCurrentAirspace = true;
  _direction = 1; //must be reset according to specs
}


void OpenAirParser::newPA()
{
  asPA.resize(0);
}


void OpenAirParser::finishAirspace()
{
  uint cnt=asPA.count();
  QPointArray PA(cnt);
  for (uint i=0; i<cnt; i++) {
    PA.setPoint(i, _globalMapMatrix.wgsToMap(asPA.point(i)));
  }

  Airspace * a = new Airspace(asName,
                              asType,
                              PA,
                              asUpper, asUpperType,
                              asLower, asLowerType);
  _airlist.append(a);
  _objCounter++;
  _isCurrentAirspace = false;
  // qDebug("finalized airspace %s. %d points in airspace", asName.latin1(), cnt);

}


void OpenAirParser::initializeBaseMapping()
{
  // create a mapping from a string representation of the supported
  // aispace types in Cumulus to their integer codes
  m_baseTypeMap.clear();

  m_baseTypeMap.insert("AirA", BaseMapElement::AirA);
  m_baseTypeMap.insert("AirB", BaseMapElement::AirB);
  m_baseTypeMap.insert("AirC", BaseMapElement::AirC);
  m_baseTypeMap.insert("AirD", BaseMapElement::AirD);
  m_baseTypeMap.insert("AirElow", BaseMapElement::AirElow);
  m_baseTypeMap.insert("AirEhigh", BaseMapElement::AirEhigh);
  m_baseTypeMap.insert("AirF", BaseMapElement::AirF);
  m_baseTypeMap.insert("ControlC", BaseMapElement::ControlC);
  m_baseTypeMap.insert("ControlD", BaseMapElement::ControlD);
  m_baseTypeMap.insert("Danger", BaseMapElement::Danger);
  m_baseTypeMap.insert("Restricted", BaseMapElement::Restricted);
  m_baseTypeMap.insert("Prohibited", BaseMapElement::Prohibited);
  m_baseTypeMap.insert("LowFlight", BaseMapElement::LowFlight);
  m_baseTypeMap.insert("Tmz", BaseMapElement::Tmz);
  m_baseTypeMap.insert("SuSector", BaseMapElement::SuSector);
}

void OpenAirParser::initializeStringMapping(const QString& mapFilePath)
{
  //fist, initialize the mapping QMap with the defaults
  m_stringTypeMap.clear();

  m_stringTypeMap.insert("A", "AirA");
  m_stringTypeMap.insert("B", "AirB");
  m_stringTypeMap.insert("C", "AirC");
  m_stringTypeMap.insert("D", "AirD");
  m_stringTypeMap.insert("E", "AirElow");
  m_stringTypeMap.insert("F", "AirF");
  m_stringTypeMap.insert("GP", "Restricted");
  m_stringTypeMap.insert("R", "Restricted");
  m_stringTypeMap.insert("P", "Prohibited");
  m_stringTypeMap.insert("TRA", "Restricted");
  m_stringTypeMap.insert("Q", "Danger");
  m_stringTypeMap.insert("CTR", "ControlC");
  m_stringTypeMap.insert("TMZ", "Tmz");
  m_stringTypeMap.insert("W", "AirEhigh");
  m_stringTypeMap.insert("GSEC", "SuSector");

  //then, check to see if we need to update this mapping
  //construct file name for mapping file
  QFileInfo fi(mapFilePath);

  QString path = fi.dirPath() + "/" + fi.baseName() + "_mappings.conf";
  fi.setFile(path);
  if (fi.exists() && fi.isFile() && fi.isReadable()) {
    QFile f(path);
    if (!f.open(IO_ReadOnly)) {
      qWarning("OpenAirParser: Cannot open airspace mapping file %s!", path.latin1());
      return;
    }

    QTextStream in(&f);
    qDebug("Parsing mapping file '%s'.", path.latin1());

    //start parsing
    QString line = in.readLine();
    while (!line.isNull()) {
      line = line.simplifyWhiteSpace();
      if (line.startsWith("*") || line.startsWith("#")) {
        //comment, ignore
      } else if (line.isEmpty()) {
        //empty line, ignore
      } else {
        int pos = line.find("=");
        if (pos>0 && pos < int(line.length())) {
          QString key = line.left(pos).simplifyWhiteSpace();
          QString value = line.mid(pos+1).simplifyWhiteSpace();
          qDebug("  added '%s' => '%s' to mappings", key.latin1(), value.latin1());
          m_stringTypeMap.replace(key, value);
        }
      }
      line=in.readLine();
    }
  }
}


void OpenAirParser::parseType(QString& line)
{
  line=line.mid(3);

  if (!m_stringTypeMap.contains(line)) {
    //no mapping from the found type to a Cumulus basetype was found
    qWarning("OpenAirParser: Line=%d Type, '%s' not mapped to basetype. Object not interpretted.", _lineNumber, line.latin1());
    _isCurrentAirspace = false; //stop accepting other lines in this object
    return;
  } else {
    QString stringType = m_stringTypeMap[line];
    if (!m_baseTypeMap.contains(stringType)) {
      //the indicated basetype is not a valid Cumulus basetype.
      qWarning("OpenAirParser:=Line %d, Type '%s' is not a valid basetype. Object not interpretted.", _lineNumber, stringType.latin1());
      _isCurrentAirspace = false; //stop accepting other lines in this object
      return;
    } else {
      //all seems to be right with the world!
      asType = m_baseTypeMap[stringType];
    }
  }
}


void OpenAirParser::parseAltitude(QString& line, BaseMapElement::elevationType& type, int& alt)
{
  bool convertFromMeters=false;
  bool altitudeIsFeet=false;
  QString input = line;
  QStringList elements;
  int len=0, pos=0;
  QString part;
  bool ok;
  int num=0;

  type = BaseMapElement::NotSet;
  alt=0;
  // qDebug("line %d: parsing altitude '%s'", _lineNumber, line.latin1());
  //fist, split the string in parsable elements
  //we start with the text parts
  QRegExp reg("[A-Za-z]+");
  while (line.length()>0) {
    pos = reg.match(line, pos+len, &len);
    if (pos<0) {
      break;
    }
    elements.append(line.mid(pos, len));
    //qDebug("element: '%s'", line.mid(pos, len).latin1());
    //line=line.mid(len);
  }

  //now, get our number parts

  reg.setPattern("[0-9]+");
  pos=0;
  len=0;
  while (line.length()>0) {
    pos = reg.match(line, pos+len, &len);
    if (pos<0) {
      break;
    }
    elements.append(line.mid(pos, len));
    //qDebug("element: '%s'", line.mid(pos, len).latin1());
    line=line.mid(len);
  }

  // now, try parsing piece by piece
  // print it out

  for ( QStringList::Iterator it = elements.begin(); it != elements.end(); ++it ) {
    part = (*it).upper();
    BaseMapElement::elevationType newType = BaseMapElement::NotSet;

    // first, try to interpret as elevation type
    if (part=="AMSL" || part=="MSL") {
      newType=BaseMapElement::MSL;
    }
    else if (part=="GND" || part=="SFC" || part=="ASFC" || part=="AGL") {
      newType=BaseMapElement::GND;
    }
    else if (part.startsWith("UNL")) {
      newType=BaseMapElement::UNLTD;
    }
    else if (part=="FL") {
      newType=BaseMapElement::FL;
    }
    else if (part=="STD") {
      newType=BaseMapElement::STD;
    }
        
    if( type == BaseMapElement::NotSet && newType != BaseMapElement::NotSet ) {
      type = newType;
      continue;
    }

    if( type != BaseMapElement::NotSet && newType != BaseMapElement::NotSet ) {
      // @AP: Here we stept into a problem. We found a second
      // elevation type. That can be only a mistake in the data
      // and will be ignored.
      qWarning( "OpenAirParser: Line=%d, '%s' contains more than one elevation type. Only first one is taken",
                _lineNumber, input.latin1());
      continue;
    }

    //see if it is a way of setting units to feet
    if (part=="FT") {
      altitudeIsFeet=true;
      continue;
    }

    //see if it is a way of setting units to meters
    if (part=="M") {
      convertFromMeters=true;
      continue;
    }

    //try to interpret as a number
    num = part.toInt(&ok);
    if (ok) {
      alt = num;
    }

    //ignore other parts
  }
  if( altitudeIsFeet && type == BaseMapElement::NotSet )
      type = BaseMapElement::MSL;

  if (convertFromMeters)
    alt = (int) rint( alt/Distance::mFromFeet);

  // qDebug("Line %d: Returned altitude %d, type %d", _lineNumber, alt, int(type));
}


bool OpenAirParser::parseCoordinate(QString& line, int& lat, int& lon)
{
  bool result=true;
  line=line.upper();

  int len=0, pos=0;
  lat=0;
  lon=0;

  QRegExp reg("[NSEW]");
  pos=reg.match(line, 0, &len);
  if (pos==-1)
    return false;

  QString part1=line.left(pos+1);
  QString part2=line.mid(pos+1);
  result &= parseCoordinatePart(part1, lat, lon);
  result &= parseCoordinatePart(part2, lat, lon);

  return result;
}


bool OpenAirParser::parseCoordinatePart(QString& line, int& lat, int& lon)
{
  const double factor[4]= {
    600000, 10000, 166.666666667, 0
  };
  const double factor100[4]= {
    600000, 600000, 10000, 0
  };
  bool decimal=false;
  int cur_factor=0;
  double part=0;
  bool ok=false, found=false;
  int value=0;
  int len=0, pos=0;
  // qDebug("line=%s", line.latin1() );

  QRegExp reg("[0-9]+");

  if (line.isEmpty()) {
    qWarning("Tried to parse empty coordinate part! Line %d", _lineNumber);
    return false;
  }

  while (cur_factor<3 && line.length()) {
    pos=reg.match(line,pos+len,&len);
    if (pos==-1) {
      break;
    } else {
      part=line.mid(pos, len).toInt(&ok);
      if (ok) {
        if( decimal )   {
          value+=(int) rint(part / pow(10,len) * factor100[cur_factor]);
          // qDebug("part=%f add=%d v=%d  cf=%d %d", part, int(part / pow(10,len) * factor100[cur_factor]), value, cur_factor, decimal );
        } else {
          value+=(int) rint(part * factor[cur_factor]);
          // qDebug("part=%f add=%d v=%d  cf=%d %d", part, int(part * factor[cur_factor]), value, cur_factor, decimal );
        }
        found=true;
      } else {
        break;
      }
    }
    if (line.mid(pos+len,1)==":") {
      len++;
      decimal = false;
      cur_factor++;
      continue;
    } else if (line.mid(pos+len,1)==".") {
      len++;
      cur_factor++;
      decimal = true;
      continue;
    }
  }

  if (!found)
    return false;

  // qDebug("%s value=%d", line.ascii(), value);

  if (line.find('N')>0) {
    lat=value;
    return true;
  }
  if (line.find('S')>0) {
    lat=-value;
    return true;
  }
  if (line.find('E')>0) {
    lon=value;
    return true;
  }
  if (line.find('W')>0) {
    lon=-value;
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
  QStringList arguments=QStringList::split('=', line);
  if (arguments.count()<2)
    return false;

  QString variable=arguments[0].simplifyWhiteSpace().upper();
  QString value=arguments[1].simplifyWhiteSpace();
  // qDebug("line %d: variable = '%s', value='%s'", _lineNumber, variable.latin1(), value.latin1());
  if (variable=="X") {
    //coordinate
    return parseCoordinate(value, _center);
  }

  if (variable=="D") {
    //direction
    if (value=="+") {
      _direction=+1;
    } else if (value=="-") {
      _direction=-1;
    } else {
      return false;
    }
    return true;
  }

  if (variable=="W") {
    //airway width
    bool ok;
    double result=value.toDouble(&ok);
    if (ok) {
      _awy_width = result;
      return true;
    }
    return false;
  }

  if (variable=="Z") {
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

  QStringList arguments = QStringList::split(',', line);
  if (arguments.count()<3)
    return false;

  radius=arguments[0].stripWhiteSpace().toDouble(&ok);

  if ( !ok ) {
    return false;
  }

  angle1=arguments[1].stripWhiteSpace().toDouble(&ok);

  if (!ok) {
    return false;
  }

  angle2=arguments[2].stripWhiteSpace().toDouble(&ok);

  if (!ok) {
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
  if( dx >= 0 && dy < 0 ) {
    angle = (2 * M_PI) - angle;
  } else if( dx <=0 && dy <= 0 ) {
    angle =  M_PI + angle;
  } else if( dx < 0 && dy >= 0 ) {
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
  QStringList arguments = QStringList::split(',', line);
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
  int pai=asPA.count();
  asPA.resize(pai+360);
  // qDebug("rLat: %d, rLon:%d", rLat, rLon);

  for (int i=0; i<360; i+=1) {
    phi=double(i)/180.0*M_PI;
    x = cos(phi)*rLat;
    y = sin(phi)*rLon;
    x +=_center.x();
    y +=_center.y();
    asPA.setPoint(pai, QPoint(int(rint(x)), int(rint(y))));
    pai++;
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
  int pai=asPA.count();

  if (_direction>0) {
    if (angle1>=angle2)
      angle2+=2.0*M_PI;
  } else {
    if (angle2>=angle1)
      angle1+=2.0*M_PI;
  }

  int nsteps=abs(int(((angle2-angle1)*180)/(STEP_WIDTH*M_PI)))+2;
  asPA.resize(pai+nsteps);

  //qDebug("delta=%d pai=%d",int(((angle2-angle1)*180)/(STEP_WIDTH*M_PI)), pai );

  const double step=(STEP_WIDTH*M_PI)/180.0;

  double phi=angle1;
  for (int i=0; i<nsteps-1; i++) {
    x = ( cos(phi)*rX ) + _center.x();
    y = ( sin(phi)*rY ) + _center.y();
    asPA.setPoint(pai, (int) rint(x), (int) rint(y));
    pai++;
    if (_direction>0) { //clockwise
      phi+=step;
    } else {
      phi-=step;
    }
  }
  x = ( cos(angle2)*rX ) + _center.x();
  y = ( sin(angle2)*rY ) + _center.y();
  asPA.setPoint(pai, (int) rint(x), (int) rint(y));
}


/**
 * Get the header data of a compiled file and put it in the class
 * variables.
 *
 * @param path Full name with path of OpenAir binary file
 * @returns true (success) or false (error occured)
 */
bool OpenAirParser::setHeaderData( QString &path )
{
  h_headerIsValid = false; // save read result here too

  h_magic = 0;
  h_fileType = 0;
  h_fileVersion = 0;

  QFile inFile(path);
  if( !inFile.open(IO_ReadOnly) ) {
    qWarning("OpenAirParser: Cannot open airspace file %s!", path.latin1());
    return false;
  }

  QDataStream in(&inFile);

  in >> h_magic;

  if( h_magic != KFLOG_FILE_MAGIC ) {
    qWarning( "OpenAirParser: wrong magic key %x read! Aborting ...", h_magic );
    inFile.close();
    return false;
  }

  in >> h_fileType;

  if( h_fileType != FILE_TYPE_AIRSPACE_C ) {
    qWarning( "OpenAirParser: wrong file type %x read! Aborting ...", h_fileType );
    inFile.close();
    return false;
  }

  in >> h_fileVersion;

  if( h_fileVersion != FILE_VERSION_AIRSPACE_C ) {
    qWarning( "OpenAirParser: wrong file version %x read! Aborting ...", h_fileVersion );
    inFile.close();
    return false;
  }

  in >> h_creationDateTime;

#ifdef BOUNDING_BOX
  in >> h_boundingBox;
#endif

  inFile.close();
  h_headerIsValid = true; // save read result here too
  return true;
}

