/***********************************************************************
**
**   mapcontents.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>
#include <stdlib.h>
#include <iostream>

#include "mapcontents.h"
#include <mapcalc.h>
#include "flightselectiondialog.h"
#include "kflog.h"

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kdirselectdialog.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qdom.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <kconfig.h>

#include <mapmatrix.h>

#include <airport.h>
#include <airspace.h>
#include <basemapelement.h>
#include <flight.h>
#include <flightgroup.h>
#include <glidersite.h>
#include <isohypse.h>
#include <lineelement.h>
#include <radiopoint.h>
#include <singlepoint.h>
#include <downloadlist.h>

#define MAX_FILE_COUNT 16200
#define ISO_LINE_NUM 50

//#define KFLOG_FILE_MAGIC   0x404b464c
//
//#define TYPE_GROUND     0x47
//#define TYPE_TERRAIN    0x54
//#define TYPE_MAP        0x4d
//#define TYPE_AERO       0x41
//#define FORMAT_VERSION  100

#define KFLOG_FILE_MAGIC  0x404b464c
#define FILE_TYPE_GROUND  0x47
#define FILE_TYPE_TERRAIN 0x54
#define FILE_TYPE_MAP     0x4d
#define FILE_TYPE_AERO    0x41
#define FILE_FORMAT_ID    101

#define CHECK_BORDER if(i == 0) { \
    border.north = lat_temp;   border.south = lat_temp; \
    border.east = lon_temp;    border.west = lon_temp; \
  } else { \
    border.north = MAX(border.north, lat_temp); \
    border.south = MIN(border.south, lat_temp); \
    border.east = MAX(border.east, lon_temp); \
    border.west = MIN(border.west, lon_temp); \
  }

#define READ_POINT_LIST  in >> locLength; \
  tA.resize(locLength); \
  for(unsigned int i = 0; i < locLength; i++) { \
    in >> lat_temp;          in >> lon_temp; \
    tA.setPoint(i, _globalMapMatrix.wgsToMap(lat_temp, lon_temp)); \
  }

#define READ_CONTACT_DATA in >> contactCount; \
  for(unsigned int loop = 0; loop < contactCount; loop++) \
    { \
      in >> frequency; \
      in >> contactType; \
      in >> callSign; \
    }

#define READ_RUNWAY_DATA in >> rwCount; \
  for(unsigned int loop = 0; loop < rwCount; loop++) \
    { \
      in >> rwDirection; \
      in >> rwLength; \
      in >> rwMaterial; \
      in >> rwOpen; \
    }

// Liste der Höhenstufen (insg. 50 Stufen):
const int MapContents::isoLines[] = { 0, 10, 25, 50, 75, 100, 150, 200, 250,
          300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750,
          2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000, 4250, 4500,
          4750, 5000, 5250, 5500, 5750, 6000, 6250, 6500, 6750, 7000, 7250,
          7500, 7750, 8000, 8250, 8500, 8750};

MapContents::MapContents()
  : isFirst(true)
{
  sectionArray.resize(MAX_FILE_COUNT);
  sectionArray.fill(false);
//  for(unsigned int loop = 0; loop < MAX_FILE_COUNT; loop++)
//      sectionArray.clearBit(loop);

  // Wir nehmen zunächst 4 Schachtelungstiefen an ...
  for(unsigned int loop = 0; loop < ( ISO_LINE_NUM * 4 ); loop++)
      isoList.append(new QList<Isohypse>);

  airportList.setAutoDelete(true);
  airspaceList.setAutoDelete(true);
  gliderList.setAutoDelete(true);
  flightList.setAutoDelete(true);
  hydroList.setAutoDelete(true);
  landmarkList.setAutoDelete(true);
  navList.setAutoDelete(true);
  obstacleList.setAutoDelete(true);
  outList.setAutoDelete(true);
  railList.setAutoDelete(true);
  reportList.setAutoDelete(true);
  roadList.setAutoDelete(true);
//  stationList.setAutoDelete(true);
  topoList.setAutoDelete(true);
  wpList.setAutoDelete(false);
  regIsoLines.setAutoDelete(true);
  downloadList = new DownloadList();
}

MapContents::~MapContents()
{
  // Hier müssen ALLE Listen gelöscht werden!!!
/*  airportList.~QList();
  airspaceList.~QList();
  cityList.~QList();
  gliderList.~QList();
  flightList.~QList();
  hydroList.~QList();
  landmarkList.~QList();
  navList.~QList();
  obstacleList.~QList();
  outList.~QList();
  railList.~QList();
  reportList.~QList();
  roadList.~QList();
  stationList.~QList();
  topoList.~QList();
*/
}

int MapContents::degreeToNum(QString inDegree)
{
  /*
   * needed formats:
   *
   *  [g]gg° mm' ss"
   *  dddddddddd
   */
  QRegExp degree("^[0-9]?[0-9][0-9]°[ ]*[0-9][0-9]'[ ]*[0-9][0-9]\"");
  QRegExp number("^-?[0-9]+$");

  if(number.match(inDegree) != -1)
      return inDegree.toInt();
  else if(degree.match(inDegree) != -1)
    {
      int deg = 0, min = 0, sec = 0, result = 0;

      QRegExp deg1("°");
      deg = inDegree.mid(0, deg1.match(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.match(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.match(inDegree)).toInt();
      inDegree = inDegree.mid(deg2.match(inDegree) + 1, inDegree.length());

      QRegExp deg3("\"");
      sec = inDegree.mid(0, deg3.match(inDegree)).toInt();

      result = (int)((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));

      // We add 1 to avoid rounding-errors ...
      result += 1;

      QRegExp dir("[swSW]$");
      if(dir.match(inDegree) >= 0) return -result;

      return result;
    }

  // Wrong format!!!
  return 0;
}

void MapContents::closeFlight()
{
  /*
   * close current flight
   */
  BaseFlightElement *f = flightList.current();
  if(f != 0)
    {
      for (unsigned int i = 0; i < flightList.count(); i++)
        {
          FlightGroup *fg = (FlightGroup *) flightList.at(i);
          if(fg->getTypeID() == BaseMapElement::FlightGroup)
              fg->removeFlight(f);

        }

      flightList.remove(f);
      if (flightList.current() == 0) flightList.last();

      emit currentFlightChanged();
    }
}

bool MapContents::__readAsciiFile(const char* fileName)
{
  extern const MapMatrix _globalMapMatrix;

  QFile file(fileName);
  QTextStream stream(&file);
  QString line;

  unsigned int uLimit = 0, lLimit = 0, uLimitType = 0, lLimitType = 0;

  if(!file.open(IO_ReadOnly))
    {
      // Fehler-Handling bislang nicht ausreichend ...
      warning("KFLog: No Mapfile found: %s", fileName);
      return false;
    }

  warning("KFLog: parsing mapfile %s", fileName);

  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Import mapfile ..."));
  importProgress.setLabelText(
      i18n("Please wait while loading file<BR><B>%1</B>").arg(fileName));
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.show();
  importProgress.setMinimumDuration(0);

  importProgress.setProgress(0);

  QFileInfo fileInfo(fileName);
  unsigned int fileLength = fileInfo.size();
  unsigned int filePos = 0;

  bool isObject = false;
  bool isValley = false;
  int objectCount = 0;
  int l = 0;
  int* runwayDir;
  int* runwayLength;
  int* runwayType;
  int lat_temp = 0, lon_temp = 0;
  int* latitude;
  int* longitude;
  unsigned int posLength = 0;
  unsigned int runLength = 0;

  QString alias = 0;
  QString abbr = 0;
  unsigned int elev = 0;
  QString frequency = 0;
  QString name;
  unsigned int type = 0;
  bool vdf = false, winch = false, isWayP = false;
  int sortID = 0;

  QPointArray tA;
  QPoint position;
  WGSPoint wgsPos;

  while (!stream.eof())
    {
      if(importProgress.wasCancelled()) break;

      line = stream.readLine();
      filePos += line.length();
      importProgress.setProgress(( filePos * 100 ) / fileLength);
      line = line.simplifyWhiteSpace();
      // if it is a comment, ignore it!
      if(line.mid(0,1) == "#") continue;
      if(line.mid(0,5) == "[NEW]")
        {
          // a new object starts here
          isObject = true;
          objectCount++;

          l = 0;
          name = "";
          type = 0;
          uLimit = 0;
          vdf = false;
          isValley = false;
          frequency = "";
          alias = "";
          elev = 0;
          winch = false;
          lat_temp = 0;
          lon_temp = 0;
          posLength = 0;
          runLength = 0;
          sortID = 0;
          latitude = new int[1];
          longitude = new int[1];
          runwayDir = new int[1];
          runwayLength = new int[1];
          runwayType = new int[1];
          isWayP = false;

          tA.resize(0);
        }
      else if(isObject)
        {
          if(line.mid(0,4) == "TYPE")
              type = line.mid(5,2).toUInt();
          else if(line.mid(0,5) == "ALIAS")
              alias = line.mid(6,100);
          else if(line.mid(0,6) == "ABBREV")
              abbr = line.mid(7,6);
          else if(line.mid(0,2) == "AT")
            {
              unsigned int loop;
              for(loop = 3; loop < strlen(line); loop++)
                  if(line.mid(loop, 1) == " ") break;

              wgsPos.setPos(degreeToNum( line.mid( 3, ( loop - 3 ) ) ),
                  degreeToNum( line.mid( ( loop + 1 ), 100 ) ) );

              position = _globalMapMatrix.wgsToMap(wgsPos);
            }
          else if(line.mid(0,5) == "LTYPE")
              lLimitType = line.mid(6,1).toUInt();
          else if(line.mid(0,5) == "UTYPE")
              uLimitType = line.mid(6,1).toUInt();
          else if(line.mid(0,5) == "LOWER")
              lLimit = line.mid(6,100).toUInt();
          else if(line.mid(0,5) == "UPPER")
              uLimit = line.mid(6,100).toUInt();
          else if(line.mid(0,9) == "ELEVATION")
              elev = line.mid(10,100).toUInt();
          else if(line.mid(0,4) == "ELEV")
              elev = line.mid(5,100).toUInt();
          else if(line.mid(0,9) == "FREQUENCY")
              frequency = line.mid(10,100);
          else if(line.mid(0,8) == "MOUNTAIN")
            {
              if(line.mid(9,10).toInt() == 1)
                  isValley = true;
            }
          else if(line.mid(0,4) == "NAME")
              name = line.mid(5,1000);
          else if(line.mid(0,4) == "SORT")
              sortID = line.mid(5,10).toInt();
          else if(line.mid(0,6) == "RUNWAY")
            {
              runLength++;
              runwayDir = (int*)realloc(runwayDir,
                  (runLength * sizeof(int)));
              runwayLength = (int*)realloc(runwayLength,
                  (runLength * sizeof(int)));
              runwayType = (int*)realloc(runwayType,
                  (runLength * sizeof(int)));

              unsigned int loop;
              unsigned int loop2;
              for(loop = 0; loop < strlen(line); loop++)
                  if(line.mid(loop, 1) == " ") break;

              runwayDir[runLength - 1] = line.mid(7,(loop - 7)).toInt();
              loop2 = ++loop;
              for(loop = loop; loop < strlen(line); loop++)
                  if(line.mid(loop, 1) == " ") break;

              runwayLength[runLength - 1] = line.mid(loop2,
                  (loop - loop2)).toInt();
              runwayType[runLength - 1] = line.mid(++loop,1).toInt();
            }
          else if((line.mid(0,1) >= "0") && (line.mid(0,1) <= "9"))
            {
              posLength++;
              tA.resize(posLength);

              unsigned int loop;
              for(loop = 0; loop < strlen(line); loop++)
                  if(line.mid(loop, 1) == " ") break;

              lat_temp = degreeToNum(line.left(loop));
              lon_temp = degreeToNum(line.mid((loop + 1), line.length() - loop));
              tA.setPoint(posLength - 1,
                  _globalMapMatrix.wgsToMap(lat_temp, lon_temp));
            }
          else if(line.mid(0,3) == "VDF")
              vdf = line.mid(4,1).toUInt();
          else if(line.mid(0,5) == "WINCH")
              winch = line.mid(6,1).toUInt();
          else if(line.mid(0,8) == "WAYPOINT")
              isWayP = line.mid(9,1).toUInt();
          else if(line.mid(0,5) == "[END]")
            {
              switch (type)
                {
                  case BaseMapElement::IntAirport:
                  case BaseMapElement::Airport:
                  case BaseMapElement::MilAirport:
                  case BaseMapElement::CivMilAirport:
                  case BaseMapElement::Airfield:
                    airportList.append(new Airport(name, alias, abbr, type,
                        wgsPos, position, elev, frequency, vdf));
                    break;
                  case BaseMapElement::ClosedAirfield:
                    airportList.append(new Airport(name, 0, abbr, type,
                        wgsPos, position, 0, 0, 0));
                    break;
                  case BaseMapElement::CivHeliport:
                  case BaseMapElement::MilHeliport:
                  case BaseMapElement::AmbHeliport:
                    airportList.append(new Airport(name, alias, abbr, type,
                        wgsPos, position, elev, frequency, 0));
                    break;
                  case BaseMapElement::Glidersite:
                    // Wieso können hier keine Startbahn-Daten angegeben werden?
                    gliderList.append(new GliderSite(name, alias, abbr,
                        wgsPos, position, elev, frequency, winch));
                    break;
                  case BaseMapElement::UltraLight:
                  case BaseMapElement::HangGlider:
                  case BaseMapElement::Parachute:
                  case BaseMapElement::Ballon:
                    break;
                  case BaseMapElement::Outlanding:
                    break;
                  case BaseMapElement::Vor:
                  case BaseMapElement::VorDme:
                  case BaseMapElement::VorTac:
                  case BaseMapElement::Ndb:
                    navList.append(new RadioPoint(name, alias, abbr, type,
                        wgsPos, position, frequency));
                    break;
                  case BaseMapElement::AirA:
                  case BaseMapElement::AirB:
                  case BaseMapElement::AirC:
                  case BaseMapElement::AirD:
                  case BaseMapElement::ControlD:
                  case BaseMapElement::AirElow:
                  case BaseMapElement::AirEhigh:
                  case BaseMapElement::AirF:
                  case BaseMapElement::Restricted:
                  case BaseMapElement::Danger:
                  case BaseMapElement::LowFlight:
                  case BaseMapElement::Tmz:
                    airspaceList.append(new Airspace(name, type, tA,
                        uLimit, uLimitType, lLimit, lLimitType));
                    break;
                  case BaseMapElement::Obstacle:
                  case BaseMapElement::LightObstacle:
                  case BaseMapElement::ObstacleGroup:
                  case BaseMapElement::LightObstacleGroup:
                    obstacleList.append(new SinglePoint(0, 0, type, wgsPos,
                        position, elev));
                    break;
                  case BaseMapElement::CompPoint:
//                    reportList.append(
//                        new SinglePoint(name, abbr, type, position, isWayP));
                    break;
                  case BaseMapElement::City:
                    cityList.append(new LineElement(name, type, tA, sortID));
                    break;
                  case BaseMapElement::Landmark:
                    break;
                  case BaseMapElement::Highway:
                  case BaseMapElement::Road:
                  case BaseMapElement::Trail:
                    roadList.append(new LineElement(name, type, tA));
                    break;
                  case BaseMapElement::Railway:
                  case BaseMapElement::Railway_D:
                    railList.append(new LineElement(name, type, tA));
                    break;
                  case BaseMapElement::AerialRailway:
                    railList.append(new LineElement(name, type, tA));
                    break;
                  case BaseMapElement::Lake:
                  case BaseMapElement::Lake_T:
                  case BaseMapElement::River:
                  case BaseMapElement::River_T:
                    hydroList.append(new LineElement(name, type, tA, sortID));
                    break;
                  case BaseMapElement::Spot:
                    obstacleList.append(new SinglePoint(0, 0, type, wgsPos,
                        position, elev));
                    break;
                  case BaseMapElement::Glacier:
                  case BaseMapElement::PackIce:
                    topoList.append(new LineElement(name, type, tA));
                    break;
                  case BaseMapElement::Isohypse:
                    break;
                  default:
                    break;
                }
              isObject = false;
            }
          else
            {
              warning("KFLog: Unknown field detected: \"%s\"",
                (const char*)line);
            }
        }
    }

  file.close();

  debug("KFLog: %d mapobjects found", objectCount);

  return true;
}

void MapContents::__downloadFile(QString fileName, QString destString, bool wait){
  KConfig* config = KGlobal::config();
  config->setGroup("General Options");
  if (config->readBoolEntry("No Automatic Map Download",false))
    return;

  config->setGroup("Path");
  KURL src = KURL("http://maproom.kflog.org/data/");
  KURL dest = KURL(destString);
  src.addPath(fileName);
  dest.addPath(fileName);

  if (wait){
    KIO::NetAccess::copy(src, dest); // waits until file is transmitted
    QString errorString = KIO::NetAccess::lastErrorString();
    if (errorString!="")
      KMessageBox::error(0,errorString);
    slotReloadMapData();
  }
  else{
    downloadList->copyKURL(&src,&dest);
    connect(downloadList,SIGNAL(downloadFinished()),this,SLOT(slotReloadMapData()));
  }
}

bool MapContents::__readTerrainFile(const int fileSecID,
    const int fileTypeID)
{
  extern const MapMatrix _globalMapMatrix;

  QString pathName;
  pathName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
  pathName = mapDir + "/" + pathName;

  if(pathName == 0)
      // Data does not exist ...
      return false;

  QFile eingabe(pathName);
  if(!eingabe.open(IO_ReadOnly))
    {
      // Data exists, but can't be read:
      // We need a messagebox
      warning("KFLog: Can not open mapfile %s", (const char*)pathName);

      QString fileName;
      fileName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
      KConfig* config = KGlobal::config();
      QString dest = config->readPathEntry("DefaultMapDirectory");
      __downloadFile(fileName,dest);

      return false;
    }

  QDataStream in(&eingabe);

  in.setVersion(2);

  Q_INT8 loadTypeID;
  Q_UINT16 loadSecID, formatID;
  Q_UINT32 magic;
  QDateTime createDateTime;

  in >> magic;
  if(magic != KFLOG_FILE_MAGIC)
    {
      // wrong dataformat !!!
      warning("KFLog: Trying to open old or invalid map-file; aborting!");
      warning(pathName);
      return false;
    }

  in >> loadTypeID;
  if(loadTypeID != fileTypeID)
    {
      // loaded wrong datatype ...
//      warning("<------------------ Falsche Typ-ID");
      return false;
    }

  in >> formatID;
  if(formatID < FILE_FORMAT_ID)
    {
      // to old ...
    }
  else if(formatID > FILE_FORMAT_ID)
    {
      // to new ...
      warning("KFLog: Fileformat too new. Aborting ...");
      return false;
    }
  in >> loadSecID;
  if(loadSecID != fileSecID)
    {
      // Problem!!!
//      warning("<------------------- Falsche Kachel-ID");
      return false;
    }
  in >> createDateTime;
  
  while(!in.eof())
    {
      int sort_temp;

      Q_UINT8 type;
      Q_INT16 elevation;
      Q_INT8 valley, sort;
      Q_INT32 locLength, latList_temp, lonList_temp;

      in >> type;
      in >> elevation;
      in >> valley;
      in >> sort;
      in >> locLength;

      QPointArray tA(locLength);

      for(int i = 0; i < locLength; i++)
        {
          in >> latList_temp;
          in >> lonList_temp;

          tA.setPoint(i, _globalMapMatrix.wgsToMap(latList_temp,
              lonList_temp));
        }
      sort_temp = -1;

//      valley -= 1;
//      valley *= -1;
      // We must ignore it, when sort is more than 3 or less than 0!

//      valley = 0;
      
      if(sort >= 0 && sort <= 3)
        {
          for(unsigned int pos = 0; pos < ISO_LINE_NUM; pos++)
              if(isoLines[pos] == elevation)
                  sort_temp = ISO_LINE_NUM * (int)sort + pos + 0;

          // If sort_temp is -1 here, we have an unused elevation and
          // must ignore it!
          if(sort_temp != -1)
            {
              Isohypse* newItem = new Isohypse(tA, elevation, valley);
              isoList.at(sort_temp)->append(newItem);
            }
        }
    }

  return true;
}

bool MapContents::__readAirfieldFile(const char* pathName)
{
  extern const MapMatrix _globalMapMatrix;

  if(pathName == 0)
      // Data does not exist
      return false;


  QFile eingabe(pathName);
  if(!eingabe.open(IO_ReadOnly))
    {
      // Data exists, but can't be read:
      // We need a messagebox
     warning("KFLog: Can not open airfields file %s", (const char*)pathName);

//      QString fileName;
//      fileName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
//      dest = config->readPathEntry("DefaultMapDirectory");
//      __downloadFile(fileName,dest);

      return false;
    }

  QDataStream in(&eingabe);
  in.setVersion(2);

  Q_UINT8 typeIn;
  Q_INT8 loadTypeID;
  Q_UINT16 formatID;
  Q_INT32 lat_temp, lon_temp;
  Q_UINT32 magic;
  QDateTime createDateTime;

  QString name;
  QString idString, icaoName, gpsName;
  Q_INT16 elevation;
  Q_INT8 isWinch, vdf;

  QString frequency;
  Q_INT8 contactType;
  QString callSign;
  Q_UINT8 contactCount;

  Q_UINT8 rwCount;
  // 0 -> 36
  Q_UINT8 rwDirection;
  Q_UINT16 rwLength;
  Q_UINT8 rwMaterial;
  Q_INT8 rwOpen;
  QPoint position;
  WGSPoint wgsPos;

  in >> magic;
  if(magic != KFLOG_FILE_MAGIC)  return false;

  in >> loadTypeID;
  if(loadTypeID != FILE_TYPE_AERO)  return false;

  in >> formatID;
  if(formatID < FILE_FORMAT_ID)
    {
      // zu alt ...
    }
  else if(formatID > FILE_FORMAT_ID)
    {
      // zu neu ...
      warning("KFLog: Fileformat too new. Aborting ...");
      return false;
    }

  in >> createDateTime;

  int count = 0;

  while(!in.eof())
    {
      in >> typeIn;

      count++;

      //
      //  Die Werte müssen wieder zurückgesetzt werden!
      //

      //
      // Bislang wird immer nur die letzte eingelesene Frequenz an das
      // Element übergeben, da die Elemente noch nicht mit mehreren
      // Frequenzen umgehen können.
      //
      // Die Landebahndaten werden zwar eingelesen, aber nicht verarbeitet.
      //
      switch (typeIn)
        {
          case BaseMapElement::IntAirport:
          case BaseMapElement::Airport:
          case BaseMapElement::MilAirport:
          case BaseMapElement::CivMilAirport:
          case BaseMapElement::Airfield:
            in >> name;
            in >> idString;
            in >> icaoName;
            in >> gpsName;
            in >> lat_temp;
            in >> lon_temp;
            in >> elevation;

            READ_CONTACT_DATA

            READ_RUNWAY_DATA

            wgsPos.setPos(lat_temp, lon_temp);
            position = _globalMapMatrix.wgsToMap(wgsPos);

            airportList.append(new Airport(name, icaoName, gpsName, typeIn,
                wgsPos, position, elevation, frequency, (bool)vdf));
            break;
          case BaseMapElement::ClosedAirfield:
            in >> name;
            in >> idString;
            in >> icaoName;
            in >> gpsName;
            in >> lat_temp;
            in >> lon_temp;
            in >> elevation;

            wgsPos.setPos(lat_temp, lon_temp);
            position = _globalMapMatrix.wgsToMap(wgsPos);

            airportList.append(new Airport(name, icaoName, gpsName, typeIn,
                wgsPos, position, 0, 0, 0));

            break;
          case BaseMapElement::CivHeliport:
          case BaseMapElement::MilHeliport:
          case BaseMapElement::AmbHeliport:
            in >> name;
            in >> idString;
            in >> icaoName;
            in >> gpsName;
            in >> lat_temp;
            in >> lon_temp;
            in >> elevation;

            READ_CONTACT_DATA

            wgsPos.setPos(lat_temp, lon_temp);
            position = _globalMapMatrix.wgsToMap(wgsPos);

            airportList.append(new Airport(name, icaoName, gpsName, typeIn,
                wgsPos, position, elevation, frequency, 0));

            break;
          case BaseMapElement::Glidersite:
            in >> name;
            in >> idString;
            in >> icaoName;
            in >> gpsName;
            in >> lat_temp;
            in >> lon_temp;
            in >> elevation;
            in >> isWinch;

            READ_CONTACT_DATA

            READ_RUNWAY_DATA

            wgsPos.setPos(lat_temp, lon_temp);
            position = _globalMapMatrix.wgsToMap(wgsPos);

            gliderList.append(new GliderSite(name, icaoName, gpsName,
                wgsPos, position, elevation, frequency, isWinch));

            break;
        }
    }

  return true;
}

bool MapContents::__readAirspaceFile(const char* pathName)
{
  extern const MapMatrix _globalMapMatrix;

  if(pathName == 0)
      // Data does not exist...
      return false;

  Q_UINT8 typeIn;
  Q_INT8 loadTypeID;
  Q_UINT16 formatID;
  Q_UINT32 magic;
  QDateTime createDateTime;

  Q_INT32 lat_temp, lon_temp;

  QString name,idString, icaoName;
  Q_INT16 uLimit, lLimit;
  Q_INT8 uLimitType, lLimitType;

  QString contactFrequency;
  Q_INT8 contactType;
  QString contactCallSign;
  Q_UINT8 contactCount;

  Q_INT8 exception = 0;
  QString exceptionText;

//  QList<radioContact> contactList;

  Q_UINT32 locLength;

  QFile eingabe(pathName);
  if(!eingabe.open(IO_ReadOnly))
    {
      // Data exists, but can't be read:
      // We need a messagebox
      warning("KFLog: Can not open airspace file %s", (const char*)pathName);

//      QString fileName;
//      fileName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
//      dest = config->readPathEntry("DefaultMapDirectory");
//      __downloadFile(fileName,dest);

      return false;
    }

  QDataStream in(&eingabe);
  in.setVersion(2);

  in >> magic;
  if(magic != KFLOG_FILE_MAGIC)  return false;

  in >> loadTypeID;
  if(loadTypeID != FILE_TYPE_AERO)  return false;

  in >> formatID;
  if(formatID < FILE_FORMAT_ID)
    {
      // zu alt ...
    }
  else if(formatID > FILE_FORMAT_ID)
    {
      // zu neu ...
      warning("KFLog: Fileformat too new. Aborting ...");
      return false;
    }

  in >> createDateTime;

  int count = 0;

  unsigned int type;

  while(!in.eof())
    {
      in >> typeIn;

      locLength = 0;

      name = "";
      idString = "";
      icaoName = "";
      lLimitType = 0;
      lLimit = 0;
      uLimitType = 0;
      uLimit = 0;
      exception = 0;
      exceptionText = "";
      contactFrequency = "";
      contactType = 0;
      contactCallSign = "";

      QPointArray tA;

      count++;

      switch (typeIn)
        {
          case BaseMapElement::AirA:
          case BaseMapElement::AirB:
          case BaseMapElement::AirC:
          case BaseMapElement::AirD:
          case BaseMapElement::AirElow:
          case BaseMapElement::AirEhigh:
          case BaseMapElement::AirF:
          case BaseMapElement::ControlC:
          case BaseMapElement::ControlD:
          case BaseMapElement::Danger:
          case BaseMapElement::LowFlight:
          case BaseMapElement::Restricted:
          case BaseMapElement::Tmz:
            in >> name;
            in >> idString;
            in >> icaoName;
            in >> lLimitType;
            in >> lLimit;
            in >> uLimitType;
            in >> uLimit;
            in >> exception;

            if(exception != 0)  in >> exceptionText;

            in >> contactCount;
            for(unsigned int loop = 0; loop < contactCount; loop++)
              {
                in >> contactFrequency;
                in >> contactType;
                in >> contactCallSign;
              }

            READ_POINT_LIST

            type = (unsigned int)typeIn;
            airspaceList.append(new Airspace(name, type, tA,
                uLimit, uLimitType, lLimit, lLimitType));

            break;
        }
    }

  return true;
}

bool MapContents::__readBinaryFile(const int fileSecID,
    const char fileTypeID)
{
  extern const MapMatrix _globalMapMatrix;

  QString pathName;
  pathName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
  pathName = mapDir + "/" + pathName;

  if(pathName == 0)
      // File does not exist ...
      return false;

  QFile eingabe(pathName);
  if(!eingabe.open(IO_ReadOnly))
    {
      // Data exists, but can't be read:
      // We need a messagebox
      warning("KFLog: Can not open mapfile %s", (const char*)pathName);

      QString fileName;
      fileName.sprintf("%c_%.5d.kfl", fileTypeID, fileSecID);
      KConfig* config = KGlobal::config();
      QString dest = config->readPathEntry("DefaultMapDirectory");
      __downloadFile(fileName,dest);

      return false;
    }


  QDataStream in(&eingabe);
  in.setVersion(2);

  Q_UINT8 typeIn, lm_typ;
  Q_INT8 loadTypeID, sort, elev;
  Q_UINT16 loadSecID, formatID;
  Q_INT32 lat_temp, lon_temp;
  Q_UINT32 magic, locLength = 0;
  QDateTime createDateTime;
  QString name = "";

  in >> magic;
  if(magic != KFLOG_FILE_MAGIC)  return false;

  in >> loadTypeID;
  if(loadTypeID != fileTypeID)  return false;

  in >> formatID;
  if(formatID < FILE_FORMAT_ID)
    {
      // to old ...
    }
//  else if(formatID == "2")
//    {
      // Current Fileformat

//    }
  else if(formatID > FILE_FORMAT_ID)
    {
      // to young ...
      warning("KFLog: Fileformat too new. Aborting ...");
      return false;
    }

  in >> loadSecID;
  if(loadSecID != fileSecID)  return false;

  in >> createDateTime;

  unsigned int gesamt_elemente = 0;
  unsigned int river = 0;
  unsigned int rivert = 0;
  while(!in.eof())
    {
      in >> typeIn;
      locLength = 0;
      name = "";

      QPointArray tA;

      gesamt_elemente++;


      switch (typeIn)
        {
          case BaseMapElement::Highway:
          case BaseMapElement::Road:
          case BaseMapElement::Trail:
          case BaseMapElement::Railway:
          case BaseMapElement::Railway_D:
          case BaseMapElement::Aerial_Cable:
            READ_POINT_LIST
            roadList.append(new LineElement("", typeIn, tA));
            break;
          case BaseMapElement::Canal:
          case BaseMapElement::River:
          case BaseMapElement::River_T:
            if(formatID >= FILE_FORMAT_ID) in >> name;
            READ_POINT_LIST
            hydroList.append(new LineElement(name, typeIn, tA));
            break;
          case BaseMapElement::City:
            in >> sort;
            if(formatID >= FILE_FORMAT_ID) in >> name;
            READ_POINT_LIST
            cityList.append(new LineElement(name, typeIn, tA, sort));
            break;
          case BaseMapElement::Lake:
          case BaseMapElement::Lake_T:
            in >> sort;
            if(formatID >= FILE_FORMAT_ID) in >> name;
            READ_POINT_LIST
            hydroList.append(new LineElement(name, typeIn, tA, sort));
            break;
          case BaseMapElement::PackIce:
            // is currently not being used
            // stays anyway because of errors in the MapBin in the Data
            READ_POINT_LIST
            break;
          case BaseMapElement::Forest:
          case BaseMapElement::Glacier:
            in >> sort;
            if(formatID >= FILE_FORMAT_ID) in >> name;
            READ_POINT_LIST
            topoList.append(new LineElement(name, typeIn, tA, sort));
            break;
          case BaseMapElement::Village:
            if(formatID >= FILE_FORMAT_ID) in >> name;
            in >> lat_temp;
            in >> lon_temp;
            villageList.append(new SinglePoint(name, "", typeIn,
              WGSPoint(lat_temp, lon_temp),
              _globalMapMatrix.wgsToMap(lat_temp, lon_temp)));
            break;
          case BaseMapElement::Spot:
            if(formatID >= FILE_FORMAT_ID) in >> elev;
            in >> lat_temp;
            in >> lon_temp;
            obstacleList.append(new SinglePoint("Spot", "", typeIn,
              WGSPoint(lat_temp, lon_temp),
              _globalMapMatrix.wgsToMap(lat_temp, lon_temp)));
            break;
          case BaseMapElement::Landmark:
            if(formatID >= FILE_FORMAT_ID)
              {
                in >> lm_typ;
                in >> name;
              }
            in >> lat_temp;
            in >> lon_temp;
            landmarkList.append(new SinglePoint(name, "", typeIn,
              WGSPoint(lat_temp, lon_temp),
              _globalMapMatrix.wgsToMap(lat_temp, lon_temp)));
            break;
        }
    }
  return true;
}


/*
int MapContents::searchFlightPoint(QPoint cPos, flightPoint& fP)
{
  Flight *f = (Flight *)getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight)
      return (f->searchPoint(cPos, fP));

  return -1;
}
*/
//void MapContents::optimzeFlight()
//{
//  if(flightList()->count() &&
//        flightList->current()->optimizeTask())
//    {
//      __redrawMap();
//      showFlightData(_globalMapContents.getFlight());
//    }
//}

BaseFlightElement* MapContents::getFlight()
{
  if(flightList.count())  return flightList.current();

  return 0;
}

QList<BaseFlightElement>* MapContents::getFlightList()  {  return &flightList;  }

QList<Waypoint>* MapContents::getWaypointList()  {  return &wpList;  }

bool MapContents::loadFlight(QFile& igcFile)
{
  float temp_bearing = 0.0;

  QFileInfo fInfo(igcFile);
  if(!fInfo.exists())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>does not exist!").arg(igcFile.name()) + "</qt>");
      return false;
    }
  if(!fInfo.size())
    {
      KMessageBox::sorry(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is empty!").arg(igcFile.name()) + "</qt>");
      return false;
    }
  //
  // We need a better format-identification then only the extension ...
  //
  if(((QString)fInfo.extension()).lower() != "igc")
  {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is not an igc-file!").arg(igcFile.name()) + "</qt>");
      return false;
  }

  if(!igcFile.open(IO_ReadOnly))
    {
      KMessageBox::error(0,
          "<qt>" + i18n("You don't have permission to access file<BR><B>%1</B>").arg(igcFile.name() + "</qt>"),
          i18n("No permission"));
      return false;
    }

  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Import mapfile ..."));
  importProgress.setLabelText(
      "<qt>" + i18n("Please wait while loading file<BR><B>%1</B>").arg(igcFile.name()) + "</qt>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setTotalSteps(200);
  importProgress.show();
  importProgress.setMinimumDuration(0);

  importProgress.setProgress(0);

  unsigned int fileLength = fInfo.size();
  unsigned int filePos = 0;
  QString s;
  QTextStream stream(&igcFile);

  QString pilotName, gliderType, gliderID, recorderID;
  QDate date;
  char latChar, lonChar;
  bool launched = false, append = true, isFirst = true, isFirstWP = true;
  int dT, lat, latmin, latTemp, lon, lonmin, lonTemp, baroAltTemp, gpsAltTemp;
  int hh = 0, mm = 0, ss = 0, curTime = 0, preTime = 0;
  int cClass = Flight::NotSet;

  float v, speed;

  flightPoint newPoint;
  flightPoint prePoint;
  QList<flightPoint> flightRoute;
  QList<Waypoint> wpList;
  Waypoint* newWP;
  Waypoint* preWP;
  bool isValid = true;

  newPoint.dS = 0;
  newPoint.dH = 0;
  prePoint.dS = 0;
  prePoint.dH = 0;

  //
  // This regexp is used to check the syntax of the position-lines in
  // the igc-file.
  //
  // BHHMMSSDDMMMMMNDDDMMMMMEVPPPPPGGGGGAAASSNNN
  //
  // HHMMSS       : Time of Fix, given in UTC                 6 byte
  //
  // DDMMmmmN/S   : Latitude (degree, minutes,                8 byte
  //                decimal of minutes)
  //
  //
  //                       quality
  //    time   lat      lon   |   h   GPS   ???
  //   |----||------||-------|||---||----||----?
  // ^B0944584832663N00856771EA0037700400100004
  //
  QRegExp bRecord("^B[0-2][0-9][0-6][0-9][0-6][0-9][0-9][0-9][0-6][0-9][0-9][0-9][0-9][NS][0-1][0-9][0-9][0-6][0-9][0-9][0-9][0-9][EW][AV][0-9,-][0-9][0-9][0-9][0-9][0-9,-][0-9][0-9][0-9][0-9]");

  extern const MapMatrix _globalMapMatrix;

  int lineCount = 0;
  unsigned int wp_count = 0;
  int last0 = -1;
  bool isHeader = true;

  bool isAus = false;

  //
  // Sequence of records in the igc-file:
  //
  // A : FR manufactorer (single line, required)
  //
  // H : Header (multiple lines, required)
  //
  // I : Fix extension (single line, optional)
  //
  // J : Extension of K-record (single line, optional but required,
  //     if the K-record is used)
  //
  // C : Task-Definition (multiple lines, optional)
  //     the first C-record contains the UTC-date and UTC-time of the
  //     declaration, the local time of the intended das of the flight,
  //     the task ID, the number of points and a textstring which can
  //     be used to describe the task
  //
  // L : Logbook entry (multiple lines, optional, multiple occurrencies,
  //     may only appear after the H, I and J records, but before the G-record)
  //
  // D : Differential GPS (single line, optional, placed after all H, I, J
  //     records but before the first B-record)
  //
  // F : Initial Satellite Constallation (single line, optional)
  //
  // <---------------------------- End of Header ---------------------------->
  //
  // B : Logged Fix (multiple lines)
  //
  // K : Extension data as defined in the J Record (single line,
  //     multiple ocurrencies)
  //
  // F : Constallation change (single line, multiple ocurrencies)
  //
  // E : Pilot-Event [PEV] (single lines, multiple ocurrencies)
  //
  // <----------------------------- End of Body ----------------------------->
  //
  // G : Digital signature of the file
  //
  while (!stream.eof())
    {
      if(importProgress.wasCancelled()) return false;

      lineCount++;

      s = stream.readLine();
      filePos += s.length();
      importProgress.setProgress(( filePos * 200 ) / fileLength);
      if(s.mid(0,1) == "A" && isHeader)
        {
          // We have an manufactorer-id
          KGlobal::config()->setGroup("Manufactorer ID");
          recorderID = KGlobal::config()->readEntry(s.mid(1,3).upper(),
            i18n("unknown manufactorer"));
          recorderID = recorderID + " (" + s.mid(4,3) + ")";
          KGlobal::config()->setGroup(0);
        }
      else if(s.mid(0,1) == "H" && isHeader)
        {
          // We have a headerline
          if(s.mid(1, 4).upper() == "FPLT")
              pilotName = s.mid(s.find(':')+1,100);
          else if(s.mid(1, 4).upper() == "FGTY")
              gliderType = s.mid(s.find(':')+1,100);
          else if(s.mid(1, 4).upper() == "FGID")
              gliderID = s.mid(s.find(':')+1,100);
          else if(s.mid(1, 4).upper() == "FDTE")
            {
              if(s.mid(9, 2).toInt() < 50)
                  date.setYMD(2000 + s.mid(9, 2).toInt(),
                      s.mid(7, 2).toInt(), s.mid(5, 2).toInt());
              else
                  date.setYMD(s.mid(9, 2).toInt(),
                      s.mid(7, 2).toInt(), s.mid(5, 2).toInt());
            }
          else if(s.mid(1, 4).upper() == "FCCL")
            {
              // Searching the config-file for the Competition-Class
              KGlobal::config()->setGroup("CompetitionClasses");
              cClass = KGlobal::config()->readNumEntry(
                  s.mid(s.find(':')+1,100).upper(), Flight::Unknown);
              KGlobal::config()->setGroup(0);
            }
        }
      else if(s.mid(0,1) == "B")
        {
          isHeader = false;

          //
          // We have a point.
          // But we must proofe the linesyntax first.
          //
          if(bRecord.match(s) == -1)
            {
              // IO-Error !!!
              QString lineNr;
              lineNr.sprintf("%d", lineCount);
              KMessageBox::error(0,
                  "<qt>" + i18n("Syntax-error while loading igc-file"
                      "<BR><B>%1</B><BR>Aborting!").arg(igcFile.name()) + "</qt>",
                  i18n("Error in IGC-file"));
              warning("KFLog: Error in reading line %d in igc-file %s",
                  lineCount, (const char*)igcFile.name());
              return false;
            }
          sscanf(s.mid(1,23), "%2d%2d%2d%2d%5d%1c%3d%5d%1c",
              &hh, &mm, &ss, &lat, &latmin, &latChar, &lon, &lonmin, &lonChar);
          latTemp = lat * 600000 + latmin * 10;
          lonTemp = lon * 600000 + lonmin * 10;

          if(latChar == 'S') latTemp = -latTemp;
          if(lonChar == 'W') lonTemp = -lonTemp;

          sscanf(s.mid(25,10),"%5d%5d", &baroAltTemp, &gpsAltTemp);

          if( latTemp == 0 && lonTemp == 0 )
            {
              // Ignoring a wrong point ...
              continue;
            }

          curTime = 3600 * hh + 60 * mm + ss;

          newPoint.time = curTime;
          newPoint.origP = WGSPoint(latTemp, lonTemp);
          newPoint.projP = _globalMapMatrix.wgsToMap(newPoint.origP);
          newPoint.f_state = Flight::Straight;
          newPoint.height = baroAltTemp;
          newPoint.gpsHeight = gpsAltTemp;

          if(s.mid(24,1) == "A")
              isValid = true;
          else if(s.mid(24,1) == "V")
              isValid = false;
          else
              fatal("KFLog: Wrong value found in igc-line!");

          if(isFirst)
            {
              prePoint = newPoint;
              preTime = curTime;
              isFirst = false;
              newPoint.dS = 0;
              newPoint.dH = 0;
              newPoint.dT = 0;
              newPoint.bearing = 0;
              speed = 0;
              v = 0;

              continue;
            }

          //
          // dtime may change, even if the intervall, in wich the
          // logger gets the position, is allways the same. If the
          // intervall is f.e. 10 sec, dtime may change to 11 or 9 sec.
          //
          // In some files curTime and preTime are the same. In this case
          // we set dT = 1 to avoid a floating-point-exeption ...
          //
          dT = MAX( (curTime - preTime), 1);
          newPoint.dT = dT;
          newPoint.dH = newPoint.height - prePoint.height;
          newPoint.dS = (int)(dist(latTemp, lonTemp,
              prePoint.origP.lat(), prePoint.origP.lon()) * 1000.0);

          prePoint.bearing = getBearing(prePoint,newPoint) - temp_bearing;

          if(prePoint.bearing > PI)
            {
              prePoint.bearing = prePoint.bearing - 2.0 * PI;
            }
          else if(prePoint.bearing < -PI)
            {
              prePoint.bearing =  prePoint.bearing + 2.0 * PI;
            }

          if(prePoint.bearing > PI || prePoint.bearing < -PI)
            {
              warning("Wir haben ein Problem --- Bearing > 180");
            }

          temp_bearing = getBearing(prePoint,newPoint);

          // Versuch der Ausklink-Erkennung ...
          if(launched && ((isAus != true) &&
              (abs(prePoint.bearing) > (prePoint.dT * PI / 30.0))))
            {
              warning("Ausklinken erkannt nach %s",
                (const char*)printTime(preTime));
              warning("%.5f", prePoint.bearing);
              isAus = true;
            }

          speed = 3600 * newPoint.dS / dT;  // [km/h]
          v = newPoint.dH / dT * 1.0;       // [m/s]

          //
          // landing-detection only for valid points
          //
          if(launched)
            {
              flightRoute.last()->bearing = prePoint.bearing;
              flightRoute.append(new flightPoint);
              *(flightRoute.last()) = newPoint;

              if(!isValid)  continue;

              ////////////////////////////////////////////////////////////
              //
              // We disable landing-detection, because it makes trouble
              // if there is no altitude in the file ...
              //
              ////////////////////////////////////////////////////////////

              /*
              if(!append)
                {
                  //
                  // if ( speed > 10 AND |vario| < 0.5 )
                  //
                  if( ( speed > 10 ) && ( ( v > 0.5 ) || ( v < -0.5 ) ) )
                      append = true;
                  else
                      // We are realy back on the ground, again.
                      // Now we can stop reading the file!
                      break;
                }
              //
              // Die Landebedingungen sind, besonders bei einem großen
              // Zeitabstand der Messungen noch nicht korrekt!
              //
              // Bedingung sollte über mehrere Punkte gehen. Ausserdem
              // eventuell anhand dS und dH erfolgen.
              //
              //
              if( ( speed < 10 ) && ( ( v < 0.5 ) && ( v > -0.5 ) ) )
                  // We might be back on the ground, again. But
                  // we wait for the next point.
                  append = false;
              */
            }
          else
            {
              ////////////////////////////////////////////////////////////
              //
              // We disable take-off-detection, because it makes trouble
              // if there is no altitude in the file ...
              //
              ////////////////////////////////////////////////////////////

              //if( ( speed > 20 ) && ( v > 1.5 ) )
                {
                  launched = true;
                  flightRoute.append(new flightPoint);
                  *(flightRoute.last()) = prePoint;

                  flightRoute.append(new flightPoint);
                  *(flightRoute.last()) = newPoint;
                }
            }
          //
          // We only want to compare with valid points ...
          //
          if(isValid)
            {
              prePoint = newPoint;
              preTime = curTime;
            }
        }
      else if(s.mid(0,1) == "C" && isHeader)
        {
          if( ( ( ( s.mid( 8,1) == "N" ) || ( s.mid( 8,1) == "S" ) ) ||
                ( ( s.mid(17,1) == "W" ) || ( s.mid(17,1) == "E" ) ) ))
            {
              // We have a waypoint
              sscanf(s.mid(1,17), "%2d%5d%1c%3d%5d%1c",
                  &lat, &latmin, &latChar, &lon, &lonmin, &lonChar);

              latTemp = lat * 600000 + latmin * 10;
              lonTemp = lon * 600000 + lonmin * 10;

              if(latTemp != 0 && lonTemp != 0)
                {
                  if(latChar == 'S') latTemp = -latTemp;
                  if(lonChar == 'W') lonTemp = -lonTemp;

                  newWP = new Waypoint;
                  newWP->name = s.mid(18,20);
                  newWP->origP = WGSPoint(latTemp, lonTemp);
                  newWP->projP = _globalMapMatrix.wgsToMap(newWP->origP);
                  newWP->type = Flight::NotSet;
                  if(isFirstWP)
                      newWP->distance = 0;
                  else
                      newWP->distance = dist(newWP, preWP);

                  wpList.append(newWP);
                  isFirstWP = false;
                  preWP = newWP;
                }
              else
                {
                  // Sinnvoller wäre es aus der IGC Datei auszulesen wieviele
                  // WendePunkte es gibt. <- Ist IGC Datei immer korrekt??
                  if(wp_count != 0 && last0 != (int)(wp_count - 1))
                    {
                      newWP = new Waypoint;
                      newWP->name =  preWP->name;
                      newWP->origP = preWP->origP;
                      newWP->projP = preWP->projP;

                      wpList.append(newWP);
                    }
                  last0 = wp_count;
                }
              wp_count++;
            }
        }
      else if(s.mid(0,1) == "L")
        {
          if(s.mid(13,16) == "TAKEOFF DETECTED")
            {
              // Der Logger hat den Start erkannt !
              launched = true;

              flightRoute.append(new flightPoint);
              *(flightRoute.last()) = prePoint;
            }
        }
    }

  importProgress.close();

  if(!launched || !flightRoute.count())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>contains no flight!").arg(igcFile.name()) + "</qt>");
      return false;
    }

  flightList.append(new Flight(igcFile.name(), recorderID,
      flightRoute, pilotName, gliderType, gliderID, cClass, wpList, date));

  emit currentFlightChanged();
  return true;
}

void MapContents::proofeSection(bool isPrint)
{
  extern MapMatrix _globalMapMatrix;
  QRect mapBorder;

  KStandardDirs* globalDirs = KGlobal::dirs();
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  mapDir = config->readEntry("DefaultMapDirectory",
      globalDirs->findResource("appdata", "mapdata"));

  if(mapDir.isEmpty()) {
    /* We don't have a mapdir configured. This is a problem, as the user will be
       flooded with errordialogs and no maps will be loaded. Therefore, it is a
       better strategy to just ask the user. */
    warning("Mapdir not set.");
    //extern KFLogApp * kflog;
    //if (kflog->startLogo!=0) kflog->startLogo->hide(); //hide splashscreen.
    
    KMessageBox::sorry(0, i18n("<qt>The directory for maps has not been set.<br>Please select the directory where you have installed your maps.</qt>"),
                          i18n("Mapdirectory not set."));
    mapDir=KDirSelectDialog::selectDirectory(QString::null,true,0,i18n("Select map directory...")).path();
    config->writeEntry("DefaultMapDirectory", mapDir);
  }
      
  if(isPrint)
      mapBorder = _globalMapMatrix.getPrintBorder();
  else
      mapBorder = _globalMapMatrix.getViewBorder();

  int westCorner = ( ( mapBorder.left() / 600000 / 2 ) * 2 + 180 ) / 2;
  int eastCorner = ( ( mapBorder.right() / 600000 / 2 ) * 2 + 180 ) / 2;
  int northCorner = ( ( mapBorder.top() / 600000 / 2 ) * 2 - 88 ) / -2;
  int southCorner = ( ( mapBorder.bottom() / 600000 / 2 ) * 2 - 88 ) / -2;

  if(mapBorder.left() < 0)  westCorner -= 1;
  if(mapBorder.right() < 0)  eastCorner -= 1;
  if(mapBorder.top() < 0) northCorner += 1;
  if(mapBorder.bottom() < 0) southCorner += 1;

  if(isFirst)
    {
      QDir airspaceDir(mapDir + "/airspace/");
      QDir airfieldDir(mapDir + "/airfields/");

      if(!airspaceDir.exists())
        {
          emit errorOnMapLoading();
          KMessageBox::error(0,
            "<qt>" + i18n("The directory for the airspace-files does not exist:<br><b>%1</b></qt>").arg(airspaceDir.path() + "</qt>"),
            i18n("Directory not found"));
        }
      else
        {
          emit loadingMessage(i18n("Loading airspacedata ..."));

          QStringList airspace;
          airspace = airspaceDir.entryList("*.kfl");
          if(airspace.count() == 0)
            {
              // No mapfiles found
              emit errorOnMapLoading();
              KMessageBox::information(0,
                i18n("The directory for the airspace-files is empty.\n"
                     "To download the files, please visit our homepage:\n") +
                     "http://maproom.kflog.org/", i18n("directory empty"), "NoAirspaceFiles");
            }
          else
            {
              for(QStringList::Iterator it = airspace.begin(); it != airspace.end(); it++)
                  __readAirspaceFile(airspaceDir.path() + "/" + (*it).latin1());
            }
        }

      if(!airfieldDir.exists())
        {
          emit errorOnMapLoading();
          KMessageBox::error(0,
            i18n("<qt>The directory for the airfield-files does not exist:<br><b>%1</b></qt>").arg(airspaceDir.path()),
            i18n("Directory not found"));
        }
      else
        {
          emit loadingMessage(i18n("Loading airfielddata ..."));

          QStringList airfields;
          airfields = airfieldDir.entryList("*.kfl");
          if(airfields.count() == 0)
            {
              // No mapfiles found
              emit errorOnMapLoading();
              KMessageBox::information(0,
                i18n("The directory for the airfield-files is empty.\n"
                     "To download the files, please visit our homepage:\n") +
                     "http://maproom.kflog.org/", i18n("directory empty"), "NoAirfieldFiles");
            }
          else
            {
              for(QStringList::Iterator it = airfields.begin(); it != airfields.end(); it++)
                  __readAirfieldFile(airfieldDir.path() + "/" + (*it).latin1());
            }
        }
//      airspace = globalDirs->findAllResources("appdata", "mapdata/airspace/*.out");
//      for(QStringList::Iterator it = airspace.begin(); it != airspace.end(); it++)
//        {
//          __readAsciiFile((*it).latin1());
//          warning( "%s", (*it).latin1() );
//        }

      isFirst = false;
    }

  if(!QDir(mapDir).exists())
    {
      // Directory does not exist!
      emit errorOnMapLoading();
      KMessageBox::error(0,
        i18n("<qt>The directory for the map-files does not exist:<br><b>%1</b></qt>").arg(QDir(mapDir).path()),
        i18n("Directory not found"));
    }
  else if(QDir(mapDir).entryList("*.kfl").isEmpty())
    {
      // No mapfiles installed!
      emit errorOnMapLoading();
      int ret = KMessageBox::questionYesNoCancel(0,i18n("There are no map-files in the directory\n")
          + mapDir + i18n("\nyet. Do you want to download the data automatically?\n"
          "(You need to have write permissions. If you want to change the directory,"
          "press \"Cancel\" and change it in the Settings menu"));
      KConfig* config = KGlobal::config();
      config->setGroup("General Options");
      switch (ret){
        case KMessageBox::Yes:
          __downloadFile("G_03699.kfl",mapDir,true);
          if(QFile(mapDir+"/G_03699.kfl").exists()){
            config->writeEntry("No Automatic Map Download",false);
            KMessageBox::information(0,
              i18n("The automatic data download feature is available when "
              "a flight is loaded. Please open a flight."));
            break;
          }
//          else{
//            KMessageBox::information(0,
//              i18n("The directory ") + mapDir
//              +i18n(" is not writeable! Please specify correct path in the Settings dialog!"));
//          }
        case KMessageBox::No:
          config->writeEntry("No Automatic Map Download",false);
          break;
      }
//      KMessageBox::information(0,
//        i18n("The directory for the map-files is empty.\n"
//             "To download the files, please visit our homepage:\n") +
//             "http://maproom.kflog.org/", i18n("directory empty"), "NoMapFiles");
    }
  else
    {
      emit loadingMessage(i18n("Loading mapdata ..."));

      for(int row = northCorner; row <= southCorner; row++)
        {
          for(int col = westCorner; col <= eastCorner; col++)
            {
              if( !sectionArray.testBit( row + ( col + ( row * 179 ) ) ) )
                {
                  // Kachel fehlt!
                  int secID = row + ( col + ( row * 179 ) );

                  // Nun müssen die korrekten Dateien geladen werden ...
                  __readTerrainFile(secID, FILE_TYPE_GROUND);
                  __readTerrainFile(secID, FILE_TYPE_TERRAIN);

//cout << row << " / " << col << endl;
                  __readBinaryFile(secID, FILE_TYPE_MAP);


// cout << "secID " << secID << endl;
                  sectionArray.setBit( secID, true );
                }
            }
        }
    }

//  emit loadingMessage("");

}

unsigned int MapContents::getListLength(int listIndex) const
{
  switch(listIndex) {
    case AirportList:
      return airportList.count();
    case GliderList:
      return gliderList.count();
    case OutList:
      return outList.count();
    case NavList:
      return navList.count();
    case AirspaceList:
      return airspaceList.count();
    case ObstacleList:
      return obstacleList.count();
    case ReportList:
      return reportList.count();
    case CityList:
      return cityList.count();
    case VillageList:
      return villageList.count();
    case LandmarkList:
      return landmarkList.count();
    case RoadList:
      return roadList.count();
    case RailList:
      return railList.count();
//    case StationList:
//      return stationList.count();
    case HydroList:
      return hydroList.count();
    case TopoList:
      return topoList.count();
//    case IsohypseList:
//      //warning("Anzahl der Höhenlinien: %d", isohypseList.count());
//      return isohypseList.count();
    default:
      return 0;
  }
}

Airspace* MapContents::getAirspace(unsigned int index)
{
  return airspaceList.at(index);
}

Airport* MapContents::getAirport(unsigned int index)
{
  return airportList.at(index);
}

GliderSite* MapContents::getGlidersite(unsigned int index)
{
  return gliderList.at(index);
}

BaseMapElement* MapContents::getElement(int listIndex, unsigned int index)
{
  switch(listIndex) {
    case AirportList:
      return airportList.at(index);
    case GliderList:
      return gliderList.at(index);
    case OutList:
      return outList.at(index);
    case NavList:
      return navList.at(index);
    case AirspaceList:
      return airspaceList.at(index);
    case ObstacleList:
      return obstacleList.at(index);
    case ReportList:
      return reportList.at(index);
    case CityList:
      return cityList.at(index);
    case VillageList:
      return villageList.at(index);
    case LandmarkList:
      return landmarkList.at(index);
    case RoadList:
      return roadList.at(index);
    case RailList:
      return railList.at(index);
//    case StationList:
//      return stationList.at(index);
    case HydroList:
      return hydroList.at(index);
    case TopoList:
      return topoList.at(index);
    default:
      // Should never happen!
      fatal("KFLog: trying to acces unknown mapelementlist");
      return 0;
  }
}

SinglePoint* MapContents::getSinglePoint(int listIndex, unsigned int index)
{
  switch(listIndex)
    {
      case AirportList:
        return airportList.at(index);
      case GliderList:
        return gliderList.at(index);
      case OutList:
        return outList.at(index);
      case NavList:
        return navList.at(index);
      case ObstacleList:
        return obstacleList.at(index);
      case ReportList:
        return reportList.at(index);
      case VillageList:
        return villageList.at(index);
      case LandmarkList:
        return landmarkList.at(index);
//      case StationList:
//        return stationList.at(index);
      default:
        return 0;
    }
}

void MapContents::slotReloadMapData()
{
  airportList.clear();
  gliderList.clear();
  addSitesList.clear();
  outList.clear();
  navList.clear();
  airspaceList.clear();
  obstacleList.clear();
  reportList.clear();
  cityList.clear();
  villageList.clear();
  landmarkList.clear();
  roadList.clear();
  railList.clear();
//  stationList.clear();
  hydroList.clear();
  topoList.clear();
  isoList.clear();

  // Wir nehmen zunächst 4 Schachtelungstiefen an ...
  for(unsigned int loop = 0; loop < ( ISO_LINE_NUM * 4 ); loop++)
      isoList.append(new QList<Isohypse>);

  sectionArray.fill(false);

  isFirst = true;
}

void MapContents::printContents(QPainter* targetPainter, bool isText)
{
  proofeSection(true);

  for(unsigned int loop = 0; loop < topoList.count(); loop++)
      topoList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < hydroList.count(); loop++)
      hydroList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < railList.count(); loop++)
      railList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < roadList.count(); loop++)
      roadList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < cityList.count(); loop++)
      cityList.at(loop)->printMapElement(targetPainter, isText);

//  for(unsigned int loop = 0; loop < villageList.count(); loop++)
//      villageList.at(loop)->drawMapElement(targetPainter, isText);


  for(unsigned int loop = 0; loop < navList.count(); loop++)
      navList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < airspaceList.count(); loop++)
      airspaceList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < obstacleList.count(); loop++)
      obstacleList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < reportList.count(); loop++)
      reportList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < landmarkList.count(); loop++)
      landmarkList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < airportList.count(); loop++)
      airportList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < gliderList.count(); loop++)
      gliderList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < outList.count(); loop++)
      outList.at(loop)->printMapElement(targetPainter, isText);

  for(unsigned int loop = 0; loop < flightList.count(); loop++)
      flightList.at(loop)->printMapElement(targetPainter, isText);
}

void MapContents::drawList(QPainter* targetPainter, QPainter* maskPainter,
    unsigned int listID)
{
  switch(listID)
    {
      case AirportList:
        for(unsigned int loop = 0; loop < airportList.count(); loop++)
            airportList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case GliderList:
        for(unsigned int loop = 0; loop < gliderList.count(); loop++)
            gliderList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case OutList:
        for(unsigned int loop = 0; loop < outList.count(); loop++)
            outList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case NavList:
        for(unsigned int loop = 0; loop < navList.count(); loop++)
            navList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case AirspaceList:
        for(unsigned int loop = 0; loop < airspaceList.count(); loop++)
            airspaceList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case ObstacleList:
        for(unsigned int loop = 0; loop < obstacleList.count(); loop++)
            obstacleList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case ReportList:
        for(unsigned int loop = 0; loop < reportList.count(); loop++)
            reportList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case CityList:
        for(unsigned int loop = 0; loop < cityList.count(); loop++)
            cityList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
//      case VillageList:
//        for(unsigned int loop = 0; loop < villageList.count(); loop++)
//            villageList.at(loop)->drawMapElement(targetPainter, maskPainter);
//        break;
      case LandmarkList:
        for(unsigned int loop = 0; loop < landmarkList.count(); loop++)
            landmarkList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case RoadList:
        for(unsigned int loop = 0; loop < roadList.count(); loop++)
            roadList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case RailList:
        for(unsigned int loop = 0; loop < railList.count(); loop++)
            railList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case HydroList:
        for(unsigned int loop = 0; loop < hydroList.count(); loop++)
            hydroList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case TopoList:
        for(unsigned int loop = 0; loop < topoList.count(); loop++)
            topoList.at(loop)->drawMapElement(targetPainter, maskPainter);
        break;
      case FlightList:
        // In some cases, getFlightIndex returns a non-valid index :-(
        if (flightList.count() > 0 && getFlightIndex() >= 0 &&
              getFlightIndex() < (int)flightList.count())
            flightList.at(getFlightIndex())->drawMapElement(targetPainter, maskPainter);
        break;
      default:
        return;
    }
}

void MapContents::drawIsoList(QPainter* targetP, QPainter* maskP)
{
  int height = 0;
  QRegion * reg;

  extern MapConfig _globalMapConfig;

  regIsoLines.clear();

  for(unsigned int loop = 0; loop < isoList.count(); loop++)
    {
      if(isoList.at(loop)->count() == 0) continue;

      for(unsigned int pos = 0; pos < ISO_LINE_NUM; pos++)
        {
          if(isoLines[pos] == isoList.at(loop)->getFirst()->getElevation())
            {
              if(isoList.at(loop)->getFirst()->isValley())
                  height = pos + 1;
              else
                  height = pos + 2;

              break;
            }
        }

      
      targetP->setPen(QPen(_globalMapConfig.getIsoColor(height), 1, Qt::NoPen));
      targetP->setBrush(QBrush(_globalMapConfig.getIsoColor(height),
          QBrush::SolidPattern));
      for(unsigned int loop2 = 0; loop2 < isoList.at(loop)->count(); loop2++) {
        reg=isoList.at(loop)->at(loop2)->drawRegion(targetP, maskP);
        if (reg) {
          isoListEntry* entry=new isoListEntry(reg, height);
          regIsoLines.append(entry);
        }
      }
    }
}

/** Get the contents of the previous FlightPoint before number 'index' */
/*
int MapContents::searchGetPrevFlightPoint(int index, flightPoint & fP)
{
  Flight *f = (Flight *)getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight) {
    return f->searchGetPrevPoint(index, fP);
	}
	return -1;
}
*/
/** Get the contents of the next FlightPoint after number 'index' */
/*
int MapContents::searchGetNextFlightPoint(int index, flightPoint & fP)
{
  Flight *f = (Flight *)getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight) {
    return f->searchGetNextPoint(index, fP);
  }
	return -1;
}
*/
/** Get the contents of the next FlightPoint 'step' indexes after number 'index' */
/*
int MapContents::searchStepNextFlightPoint(int index, flightPoint & fP, int step)
{
  Flight *f = (Flight *)getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight && (step > 0)) {
    if (index+step < (int)f->getRouteLength()-1)
      index += step;
    else
			index = f->getRouteLength()-1;
    return f->searchGetNextPoint(index, fP);
	}
	return -1;
}
*/
/** Get the contents of the previous FlightPoint 'step' indexes before number 'index' */
/*
int MapContents::searchStepPrevFlightPoint(int index, flightPoint & fP, int step)
{
  Flight *f = (Flight *)getFlight();
  if(f && f->getTypeID() == BaseMapElement::Flight && (step > 0)) {
    if (index-step > 0)
      index -= step;
    else
			index = 1;
    return f->searchGetPrevPoint(index, fP);
	}
	return -1;
}
*/
/** create a new, empty task */
void MapContents::slotNewTask()
{
  FlightTask *f = new FlightTask(genTaskName());
  flightList.append(f);
  emit newTaskAdded(f);

  emit currentFlightChanged();
//  emit activatePlanning();
}

void MapContents::slotAppendTask(FlightTask *f)
{
  flightList.append(f);
  emit newTaskAdded(f);
}

/** create a new, empty flight group */
void MapContents::slotNewFlightGroup()
{
  static int gCount = 1;
  QList <Flight> fl;
  BaseFlightElement *f;
  unsigned int i;
  QString tmp;

  FlightSelectionDialog *fsd = new FlightSelectionDialog(0, "flight selection dialog");

  for (i = 0; i < flightList.count(); i++) {
    f = flightList.at(i);
    if (f->getTypeID() == BaseMapElement::Flight) {
      fsd->availableFlights.append(f);
    }
  }

  if (fsd->exec() == QDialog::Accepted) {
    for (i = 0; i < fsd->selectedFlights.count(); i++) {
      fl.append((Flight *)fsd->selectedFlights.at(i));
    }

    tmp.sprintf("GROUP%03d", gCount++);

    flightList.append(new FlightGroup(fl, tmp));
    emit currentFlightChanged();
  }
  delete fsd;
}
/** No descriptions */
void MapContents::slotSetFlight(int id)
{
  if (id >= 0 && id < (int)flightList.count()) {
    flightList.at(id);
    emit currentFlightChanged();
  }
}

void MapContents::slotSetFlight(BaseFlightElement *f)
{
  if (flightList.containsRef(f)) {
    flightList.findRef(f);
    emit currentFlightChanged();
  }
}

/** No descriptions */
void MapContents::slotEditFlightGroup()
{
  QList <Flight> fl;
  BaseFlightElement *f;
  BaseFlightElement *fg;
  unsigned int i;
  QString tmp;

  FlightSelectionDialog *fsd = new FlightSelectionDialog(0, "flight selection dialog");
  fg = getFlight();

  if (fg->getTypeID() == BaseMapElement::FlightGroup) {
    fl = ((FlightGroup *)fg)->getFlightList();
    for (i = 0; i < flightList.count(); i++) {
      f = flightList.at(i);
      if (f->getTypeID() == BaseMapElement::Flight) {
        if (fl.containsRef((Flight *)f)) {
          fsd->selectedFlights.append(f);
        }
        else {
          fsd->availableFlights.append(f);
        }
      }
    }

    if (fsd->exec() == QDialog::Accepted) {
      fl.clear();
      for (i = 0; i < fsd->selectedFlights.count(); i++) {
        fl.append((Flight *)fsd->selectedFlights.at(i));
      }
      ((FlightGroup *)fg)->setFlightList(fl);
    }
  }
  else {
    // oooops
  }

  emit currentFlightChanged();
  delete fsd;
}

/** No descriptions */
bool MapContents::importFlightGearFile(QFile& flightgearFile){
  float temp_bearing = 0.0;

  QFileInfo fInfo(flightgearFile);
  if(!fInfo.exists())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>does not exist!").arg(flightgearFile.name()) + "</qt>");
      return false;
    }
  if(!fInfo.size())
    {
      KMessageBox::sorry(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is empty!").arg(flightgearFile.name()) + "</qt>");
      return false;
    }
  //
  // We need a better format-identification then only the extension ...
  //
  if(((QString)fInfo.extension()).lower() != "flightgear")
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is not an flightgear-file!").arg(flightgearFile.name()) + "</qt>");
      return false;
    }

  if(!flightgearFile.open(IO_ReadOnly))
    {
      KMessageBox::error(0,
          "<qt>" + i18n("You don't have permission to access file<BR><B>%1</B>").arg(flightgearFile.name()) + "</qt>",
          i18n("No permission"));
      return false;
    }

  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Import mapfile ..."));
  importProgress.setLabelText(
      "<qt>" + i18n("Please wait while loading file<BR><B>%1</B>").arg(flightgearFile.name()) + "</qt>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setTotalSteps(200);
  importProgress.show();
  importProgress.setMinimumDuration(0);

  importProgress.setProgress(0);

  unsigned int fileLength = fInfo.size();
  unsigned int filePos = 0;
  QString s;
  QTextStream stream(&flightgearFile);


  QString pilotName, gliderType, gliderID, recorderID;
  QDate date;
  char latChar, lonChar, validChar;
  bool launched = false, append = true, isFirst = true, isFirstWP = true;
  int dT, lat, latmin, latTemp, lon, lonmin, lonTemp;
  int hh = 0, mm = 0, ss = 0, curTime = 0, preTime = 0;
  int cClass = Flight::NotSet;

  float v, speed;

  flightPoint newPoint;
  flightPoint prePoint;
  QList<flightPoint> flightRoute;
  QList<Waypoint> wpList;
  Waypoint* newWP;
  Waypoint* preWP;
  bool isValid = true;

  //
  // This regexp is used to check the syntax of the position-lines in
  // the flightear-file.
  //
  //  $GPRMC,200009,A,3736.811,N,12221.432,W,000.0,297.9,1903102,000.0,E*5C
  //  $PGRMZ,04,f,3*2F
  //
  // Format spec:
  //
  // all fields comma separated
  // $GPRMC 	header
  // 200009		UTC position (hhmmss)
  // A					status (A= data valid, V data not valid)
  // 3736.811	Lat (ddmm.mmm)
  // N					N/S indicator
  // 12221.432 	Long (ddmm.mmm)
  // W				E/W indicator
  // 000.0			Speed over ground
  // 297.9			Course over ground
  // 1903102		date (ddmmyy) ..    2002 = 102
  // 000.0			magnetic deviation
  //					E/W indicator
  // *5C			    checksum (*nn)
  //
  // $PGRMZ		Private message
  // 04				- contents can be ignored
  // f					- contents can be ignored
  // 3					- contents can be ignored
  // *2F				checksum (*nn)
  //
  QRegExp bRecord("^[$]GPRMC,[0-9][0-9][0-9][0-9][0-9][0-9],[AV],[0-9][0-9][0-9][0-9]\\.[0-9][0-9][0-9],[NS],[0-9][0-9][0-9][0-9][0-9]\\.[0-9][0-9][0-9],[EW],[0-9][0-9][0-9]\\.[0-9],[0-9][0-9][0-9]\\.[0-9],[0-9][0-9][0-9][0-9][0-9][0-9][0-9],[0-9][0-9][0-9]\\.[0-9],[EW],[*][0-9][0-9]$");

  extern const MapMatrix _globalMapMatrix;

  int lineCount = 0;
  unsigned int wp_count = 0;
  int last0 = -1;
  bool isHeader = true;
  bool isAus = false;

  float spd, brng, magdev,fLat, fLon;
  int day, month, year;	
  char magdevChar, checksum[2];


  while (!stream.eof())
    {
      if(importProgress.wasCancelled()) return false;

      lineCount++;

      s = stream.readLine();
      filePos += s.length();
      importProgress.setProgress(( filePos * 200 ) / fileLength);

		if(s.mid(0,6) == "$GPRMC")
        {
          //
          // We have a point.
          // But we must proofe the linesyntax first.
          //
/*          if(bRecord.match(s) == -1)
            {
              // IO-Error !!!
              QString lineNr;
              lineNr.sprintf("%d", lineCount);
              KMessageBox::error(0,
//                  i18n("Syntax-error while loading FlightGear-file"
//                      "<BR><B>%1</B><BR>Aborting!").arg(flightgearFile.name()),
//                  i18n("Error in FlightGear-file"));
//              warning("KFLog: Error in reading line %d in FlightGear-file %s",
                i18n("Sorry, but this function is not yet available"
                      "<BR><B>%1</B><BR>Aborting!").arg(flightgearFile.name()),
                i18n("Sorry..."));
                warning("KFLog: reading line %d in unspported FlightGear-file %s",
                lineCount, (const char*)flightgearFile.name());

              return false;
            }
*/
		spd = 0;
   		brng = 0;
   		magdev = 0;
   		fLat = 0;
   		fLon = 0;

		  // file is ok, now read data from line
          sscanf(s.mid(7,8), "%2d%2d%2d,%1c", &hh, &mm, &ss, &validChar);
          sscanf(s.mid(16,10), "%2d%f,1%c", &lat, &fLat, &latChar);
          sscanf(s.mid(27,11), "%3d%f,%1c", &lon, &fLon, &lonChar);
          sscanf(s.mid(39,12), "%f,%f", &spd, &brng);
          sscanf(s.mid(51,7), "%2d%2d%3d", &day, &month, &year);
          sscanf(s.mid(59,10), "%f,%1c*%2c", &magdev,&magdevChar,&checksum);

		  // NMEA year is number of years after 1900
          year +=1900;

		  // skip if lat & lon = 000.0N
		  if ((lat == 0.0) && (lon == 0.0))
			continue;

		  latmin = fLat * 1000;
          lonmin = fLon * 1000;
  		
		  // convert to internal KFLog format
	      latTemp = lat * 600000 + latmin * 10;
          lonTemp = lon * 600000 + lonmin * 10;

          if(latChar == 'S') latTemp = -latTemp;
          if(lonChar == 'W') lonTemp = -lonTemp;
		 	
          curTime = 3600 * hh + 60 * mm + ss;

          newPoint.time = curTime;
          newPoint.origP = WGSPoint(latTemp, lonTemp);
          newPoint.projP = _globalMapMatrix.wgsToMap(newPoint.origP);
          newPoint.f_state = Flight::Straight;

          if(s.mid(14,1) == "A")
              isValid = true;
          else if(s.mid(14,1) == "V")
              isValid = false;
          else
              fatal("ERROR!");

          if(isFirst)
            {
              prePoint = newPoint;
              preTime = curTime;
              isFirst = false;
              newPoint.dS = 0;
              newPoint.dH = 0;
              newPoint.dT = 0;
              newPoint.bearing = 0;
              speed = 0;
              v = 0;
              continue;
            }
          //
          // dtime may change, even if the intervall, in wich the
          // logger gets the position, is allways the same. If the
          // intervall is f.e. 10 sec, dtime may change to 11 or 9 sec.
          //
          // In some files curTime and preTime are the same. In this case
          // we set dT = 1 to avoid a floating-point-exeption ...
          //
          dT = MAX( (curTime - preTime), 1);
 		  newPoint.height = 0;
 		  newPoint.gpsHeight = 0;
          newPoint.dT = dT;
          newPoint.dH = newPoint.height - prePoint.height;
          newPoint.dS = (int)(dist(latTemp, lonTemp,
              prePoint.origP.lat(), prePoint.origP.lon()) * 1000.0);

          prePoint.bearing = getBearing(prePoint,newPoint) - temp_bearing;

          if(prePoint.bearing > PI)
            {
              prePoint.bearing = prePoint.bearing - 2.0 * PI;
            }
          else if(prePoint.bearing < -PI)
            {
              prePoint.bearing =  prePoint.bearing + 2.0 * PI;
            }

          if(prePoint.bearing > PI || prePoint.bearing < -PI)
            {
              warning("We have a problem --- Bearing > 180");
            }

          temp_bearing = getBearing(prePoint,newPoint);

          speed = 3600 * spd ;  // [km/h]
          v = newPoint.dH / dT * 1.0;       // [m/s]

          //
		  // Landing detection is not valid for FlightGear files, hence only accept launced at first fix
          //
          if(launched)
            {
              flightRoute.last()->bearing = prePoint.bearing;
              flightRoute.append(new flightPoint);
              *(flightRoute.last()) = newPoint;

              if(!isValid)  continue;

              if(!append)
                {
                  if( ( speed > 10 ) && ( ( v > 0.5 ) || ( v < -0.5 ) ) )
                      append = true;
                  else
                      // We are realy back on the ground, again.
                      // Now we can stop reading the file!
                      break;
                }
              //
              // Die Landebedingungen sind, besonders bei einem großen
              // Zeitabstand der Messungen noch nicht korrekt!
              //
              // Bedingung sollte über mehrere Punkte gehen. Ausserdem
              // eventuell anhand dS und dH erfolgen.
              //
              //
              if( ( speed < 10 ) && ( ( v < 0.5 ) && ( v > -0.5 ) ) )
                  // We might be back on the ground, again. But
                  // we wait for the next point.
                  append = false;
            }
          else
            {
                  launched = true;
                  flightRoute.append(new flightPoint);
                  *(flightRoute.last()) = prePoint;
            }
          //
          // We only want to compare with valid points ...
          //
          if(isValid)
            {
              prePoint = newPoint;
              preTime = curTime;
            }
        }
      else if(s.mid(0,6) == "$PGRMZ")
      {
       	// ignore private data for now...
 		continue;
      }
    }

  // close the import dialog, clean up and add the FlightRoute we just created
  importProgress.close();

  if(!launched || !flightRoute.count())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>contains no flight!").arg(flightgearFile.name()) + "</qt>");
      return false;
    }

  recorderID = "flightgear";
  pilotName = "flightgear";
  gliderType = "flightgear";
  gliderID = "flightgear";
  cClass = 0;

  flightList.append(new Flight(flightgearFile.name(), recorderID,
      flightRoute, pilotName, gliderType, gliderID, cClass, wpList, date));

  emit currentFlightChanged();
  return true;
}
/** Imports a file downloaded with Gardown in DOS  */
bool MapContents::importGardownFile(QFile& gardownFile){
  float temp_bearing = 0.0;

  QFileInfo fInfo(gardownFile);
  if(!fInfo.exists())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>does not exist!").arg(gardownFile.name()) + "</qt>");
      return false;
    }
  if(!fInfo.size())
    {
      KMessageBox::sorry(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is empty!").arg(gardownFile.name()) + "</qt>");
      return false;
    }
  //
  // We need a better format-identification then only the extension ...
  //
  if((((QString)fInfo.extension()).lower() != "gdn") && (((QString)fInfo.extension()).lower() != "trk"))
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is not an gardown-file!").arg(gardownFile.name()) + "</qt>");
      return false;
    }

  if(!gardownFile.open(IO_ReadOnly))
    {
      KMessageBox::error(0,
          "<qt>" + i18n("You don't have permission to access file<BR><B>%1</B>").arg(gardownFile.name()) + "</qt>",
          i18n("No permission"));
      return false;
    }

  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Import file ..."));
  importProgress.setLabelText(
      "<qt>" + i18n("Please wait while loading file<BR><B>%1</B>").arg(gardownFile.name()) + "</qt>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setTotalSteps(200);
  importProgress.show();
  importProgress.setMinimumDuration(0);

  importProgress.setProgress(0);

  unsigned int fileLength = fInfo.size();
  unsigned int filePos = 0;
  QString s;
  QTextStream stream(&gardownFile);


  QString pilotName, gliderType, gliderID, recorderID;
  QDate date;
  char latChar, lonChar, validChar;
  bool launched = false, append = true, isFirst = true, isFirstWP = true;
  int dT, lat, latmin, latTemp, lon, lonmin, lonTemp;
  int hh = 0, mm = 0, ss = 0, curTime = 0, preTime = 0;
  int cClass = Flight::NotSet;

  float v, speed;

  flightPoint newPoint;
  flightPoint prePoint;
  QList<flightPoint> flightRoute;
  QList<Waypoint> wpList;
  Waypoint* newWP;
  Waypoint* preWP;
  bool isValid = true;

  //
  // This regexp is used to check the syntax of the position-lines in
  // the file.
  //
  //  Format spec:
  //
  // TODO
  //
  QRegExp bRecord("^[$]GPRMC,[0-9][0-9][0-9][0-9][0-9][0-9],[AV],[0-9][0-9][0-9][0-9]\\.[0-9][0-9][0-9],[NS],[0-9][0-9][0-9][0-9][0-9]\\.[0-9][0-9][0-9],[EW],[0-9][0-9][0-9]\\.[0-9],[0-9][0-9][0-9]\\.[0-9],[0-9][0-9][0-9][0-9][0-9][0-9][0-9],[0-9][0-9][0-9]\\.[0-9],[EW],[*][0-9][0-9]$");

  extern const MapMatrix _globalMapMatrix;

  int lineCount = 0;
  unsigned int wp_count = 0;
  int last0 = -1;
  bool isHeader = true;
  bool isAus = false;

  float spd, brng, magdev,fLat, fLon;
  int day, month, year;	
  char magdevChar, checksum[2];


  while (!stream.eof())
    {
      if(importProgress.wasCancelled()) return false;

      lineCount++;

      s = stream.readLine();
      filePos += s.length();
      importProgress.setProgress(( filePos * 200 ) / fileLength);

		if(s.mid(0,2) == "T ")
        {
          //
          // We have a point.
          // But we must proofe the linesyntax first.
          //
/*          if(bRecord.match(s) == -1)
            {
              // IO-Error !!!
              QString lineNr;
              lineNr.sprintf("%d", lineCount);
              KMessageBox::error(0,
//                  i18n("Syntax-error while loading FlightGear-file"
//                      "<BR><B>%1</B><BR>Aborting!").arg(flightgearFile.name()),
//                  i18n("Error in FlightGear-file"));
//              warning("KFLog: Error in reading line %d in FlightGear-file %s",
                i18n("Sorry, but this function is not yet available"
                      "<BR><B>%1</B><BR>Aborting!").arg(flightgearFile.name()),
                i18n("Sorry..."));
                warning("KFLog: reading line %d in unspported FlightGear-file %s",
                lineCount, (const char*)flightgearFile.name());

              return false;
            }
*/
		spd = 0;
   		brng = 0;
   		magdev = 0;
   		fLat = 0;
   		fLon = 0;

		  // file is ok, now read data from line
  QString t = s.mid(3,11);
          sscanf(t,  "%1c%2d %f", &latChar, &lat, &fLat);
		t = s.mid(15,12);
          sscanf(t,  "%1c%3d %f", &lonChar, &lon, &fLon);

		  // skip if lat & lon = 000.0N
		  if ((lat == 0.0) && (lon == 0.0))
			continue;

		  latmin = fLat * 1000;
          lonmin = fLon * 1000;
  		
		  // convert to internal KFLog format
	      latTemp = lat * 600000 + latmin * 10;
          lonTemp = lon * 600000 + lonmin * 10;

          if(latChar == 'S') latTemp = -latTemp;
          if(lonChar == 'W') lonTemp = -lonTemp;
		 	
          curTime = 3600 * hh + 60 * mm + ss;

          newPoint.time = curTime;
          newPoint.origP = WGSPoint(latTemp, lonTemp);
          newPoint.projP = _globalMapMatrix.wgsToMap(newPoint.origP);
          newPoint.f_state = Flight::Straight;

//          if(s.mid(14,1) == "A")
              isValid = true;
//          else if(s.mid(14,1) == "V")
//              isValid = false;
//          else
//              fatal("FEHLER!");

          if(isFirst)
            {
              prePoint = newPoint;
              preTime = curTime;
              isFirst = false;
              newPoint.dS = 0;
              newPoint.dH = 0;
              newPoint.dT = 0;
              newPoint.bearing = 0;
              speed = 0;
              v = 0;
              continue;
            }
          //
          // dtime may change, even if the intervall, in wich the
          // logger gets the position, is allways the same. If the
          // intervall is f.e. 10 sec, dtime may change to 11 or 9 sec.
          //
          // In some files curTime and preTime are the same. In this case
          // we set dT = 1 to avoid a floating-point-exeption ...
          //
          dT = MAX( (curTime - preTime), 1);
 		  newPoint.height = 0;
 		  newPoint.gpsHeight = 0;
          newPoint.dT = dT;
          newPoint.dH = newPoint.height - prePoint.height;
          newPoint.dS = (int)(dist(latTemp, lonTemp,
              prePoint.origP.lat(), prePoint.origP.lon()) * 1000.0);

          prePoint.bearing = getBearing(prePoint,newPoint) - temp_bearing;

          if(prePoint.bearing > PI)
            {
              prePoint.bearing = prePoint.bearing - 2.0 * PI;
            }
          else if(prePoint.bearing < -PI)
            {
              prePoint.bearing =  prePoint.bearing + 2.0 * PI;
            }

          if(prePoint.bearing > PI || prePoint.bearing < -PI)
            {
              warning("We have a problem --- Bearing > 180");
            }

          temp_bearing = getBearing(prePoint,newPoint);

//          speed = 3600 * spd ;  // [km/h]
		  speed = 3600 * spd ;  // [km/h]
          v = newPoint.dH / dT * 1.0;       // [m/s]

          //
		  // Landing detection is not valid for FlightGear files, hence only accept launced at first fix
          //
          if(launched)
            {
              flightRoute.last()->bearing = prePoint.bearing;
              flightRoute.append(new flightPoint);
              *(flightRoute.last()) = newPoint;

              if(!isValid)  continue;
            }
          else
            {
                  launched = true;
                  flightRoute.append(new flightPoint);
                  *(flightRoute.last()) = prePoint;
            }
          //
          // We only want to compare with valid points ...
          //
          if(isValid)
            {
              prePoint = newPoint;
              preTime = curTime;
            }
        }
      else {
       	// ignore other lines in file for now...
 		continue;
      }
    }

  // close the import dialog, clean up and add the FlightRoute we just created
  importProgress.close();

  if(!launched || !flightRoute.count())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>contains no flight!").arg(gardownFile.name()) + "</qt>");
      return false;
    }

  recorderID = "gardown";
  pilotName = "gardown";
  gliderType = "gardown";
  gliderID = "gardown";
  cClass = 0;

  flightList.append(new Flight(gardownFile.name(), recorderID,
      flightRoute, pilotName, gliderType, gliderID, cClass, wpList, date));

  emit currentFlightChanged();
  return true;
}

/** read a task file and append all tasks to flight list
switch to first task in file */
bool MapContents::loadTask(QFile& path)
{
  QFileInfo fInfo(path);

  extern const MapMatrix _globalMapMatrix;

  if(!fInfo.exists()) {
    KMessageBox::error(0,  "<qt>" + i18n("The selected file<BR><B>%1</B><BR>does not exist!").arg(path.name()) + "</qt>");
    return false;
  }
  
  if(!fInfo.size()) {
    KMessageBox::sorry(0, "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is empty!").arg(path.name()) + "</qt>");
    return false;
  }

  if(!path.open(IO_ReadOnly)) {
    KMessageBox::error(0, "<qt>" + i18n("You don't have permission to access file<BR><B>%1</B>").arg(path.name()) + "</qt>");
    return false;
  }

  QDomDocument doc;
  QList<Waypoint> wpList;
  FlightTask *f, *firstTask = 0;

  doc.setContent(&path);

  if (doc.doctype().name() == "KFLogTask") {
    QDomNodeList nl = doc.elementsByTagName("Task");

    for (uint i = 0; i < nl.count(); i++) {
      QDomNodeList childNodes = nl.item(i).childNodes();
      QDomNamedNodeMap nmTask =  nl.item(i).attributes();

      wpList.clear();
      for (uint childIdx = 0; childIdx < childNodes.count(); childIdx++) {
        QDomNamedNodeMap nm =  childNodes.item(childIdx).attributes();

        Waypoint *w = new Waypoint;

        w->name = nm.namedItem("Name").toAttr().value().left(6).upper();
        w->description = nm.namedItem("Description").toAttr().value();
        w->icao = nm.namedItem("ICAO").toAttr().value().upper();
        w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
        w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
        w->projP = _globalMapMatrix.wgsToMap(w->origP);
        w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
        w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
        w->isLandable = nm.namedItem("Landable").toAttr().value().toInt();
        w->runway = nm.namedItem("Runway").toAttr().value().toInt();
        w->length = nm.namedItem("Length").toAttr().value().toInt();
        w->surface = nm.namedItem("Surface").toAttr().value().toInt();
        w->comment = nm.namedItem("Comment").toAttr().value();

        if (w->runway == 0 && w->length == 0) {
          w->runway = w->length = -1;
        }
        
        wpList.append(w);
      }
      f = new FlightTask(wpList, false, genTaskName());
      f->setPlanningType(nmTask.namedItem("PlanningType").toAttr().value().toInt());
      f->setPlanningDirection(nmTask.namedItem("PlanningDirection").toAttr().value().toInt());
      // remember first task in file
      if (firstTask == 0) {
        firstTask = f;
      }
      flightList.append(f);
      emit newTaskAdded(f);
    }

    if (firstTask) {
      slotSetFlight(firstTask);
    }
    return true;
  }
  else {
    KMessageBox::error(0, i18n("wrong doctype ") + doc.doctype().name(), i18n("Error occurred!"));
    return false;
  }
}

QString MapContents::genTaskName()
{
  static int tCount = 1;
  QString tmp;

  tmp.sprintf("TASK%03d", tCount++);
  return tmp;
}


/***********************************************
 * isoListEntry
 ***********************************************/

isoListEntry::isoListEntry(QRegion* region, int height) {
  this->region=region;
  this->height=height;
}
  
isoListEntry::~isoListEntry() {
  if(region) delete region;
}

