/***********************************************************************
**
**   waypointcatalog.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#define DATA_STREAM QDataStream::Qt_4_8

#ifdef _MSC_VER
    #pragma comment (lib, "advapi32.lib")
#endif

#ifdef _WIN32
#include <qt_windows.h>
#include <Lmcons.h>
#endif

#include <cmath>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <QtWidgets>
#include <QtXml>

#include "altitude.h"
#include "Frequency.h"
#include "runway.h"
#include "da4record.h"

#ifndef _WIN32
#include "kfrgcs/vlapi2.h"
#endif

#include "mainwindow.h"
#include "mapdefaults.h"
#include "target.h"
#include "waypointcatalog.h"

#define KFLOG_FILE_MAGIC    0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50
#define FILE_FORMAT_ID      100
#define FILE_FORMAT_ID_2    101
#define FILE_FORMAT_ID_3    103 // waypoint list size added
#define FILE_FORMAT_ID_4    104 // runway list added
#define FILE_FORMAT_ID_5    105 // runway length stored as float to avoid rounding issues between ft - m
#define FILE_FORMAT_ID_8    108 // runway and frequencies handled as lists, compatible to Cumulus

// Center point definition, also used by waypoint import filter.
#define CENTER_POS      0
#define CENTER_HOMESITE 1
#define CENTER_MAP      2
#define CENTER_AIRFIELD 3

extern MainWindow *_mainWindow;
extern QSettings  _settings;

QSet<QString> WaypointCatalog::catalogSet;

#ifdef _WIN32
class UserNameCache
{
private:
    QString m_UserName;
public:
    UserNameCache()
    {
        DWORD BuffSize = UNLEN + 1;
        char* pUserName = new char[BuffSize];
        if (!GetUserNameA(pUserName,&BuffSize))
        {
            strcpy(pUserName,"<Unknown user>");
        }
        m_UserName.fromLatin1(pUserName, BuffSize);
        delete[] pUserName;
    }
    char* getUserName()
    {
        return m_UserName.toLatin1().data();
    }
};

#endif

char* getLogin()
{
#ifndef _WIN32
    return getlogin();
#else
    static UserNameCache LoginCache;
    return LoginCache.getUserName();
#endif
}

WaypointCatalog::WaypointCatalog(const QString& name) :
  modified(false),
  activatedFilter(None),
  onDisc(false)
{
  static int catalogNr = 1;

  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();
  if( name.isEmpty() )
    {
      // Create an unique catalog name.
      for( int i = 0; i < 1000; i++ )
        {
          catalogName = QObject::tr("unnamed") + QString::number(catalogNr);
          catalogNr++;

          path = wayPointDir + "/" + catalogName + ".kflogwp";

          // Check, if file does not exist. Otherwise take a new filename.
          if( QFile::exists(path) == false && catalogSet.contains(path) == false )
            {
              break;
            }
        }
    }
  else
    {
      catalogName = name;

      QFileInfo fi(catalogName);

      if( fi.suffix().isEmpty() )
        {
          // Add the default suffix to the filename, if no one exists.
          catalogName += ".kflogwp";
          fi.setFile( catalogName );
        }

      if( fi.fileName() == catalogName )
        {
          // Add the waypoint directory to the pure catalog name
          path = wayPointDir + "/" + catalogName;
        }
      else
        {
          path = catalogName;
        }
    }

  catalogSet.insert( path );

  qDebug() << "WaypointCatalog(): New WaypointCatalog" << path << "created";

  showAll = true;
  showAirfields = false;
  showGliderfields = false;
  showNavaids = false;
  showReportings = false;
  showObstacles = false;
  showLandmarks = false;
  showOutlandings = false;
  showHotspots = false;

  areaLat1 = areaLat2 = areaLong1 = areaLong2 = 0;

  // Default is 500 Km
  radiusSize = 500;

  // Center homesite
  centerRef = CENTER_HOMESITE;

  // Reset airfield name reference.
  airfieldRef = "";
}

WaypointCatalog::~WaypointCatalog()
{
  qDeleteAll( wpList );
  catalogSet.remove( path );
}

WGSPoint WaypointCatalog::getCenterPoint()
{
  // Check the kind of center point. If it set to homesite, we should
  // update it because the user could change it in the meantime.
  if( centerRef == CENTER_HOMESITE )
    {
      int hLat = _settings.value("/Homesite/Latitude", HOME_DEFAULT_LAT).toInt();
      int hLon = _settings.value("/Homesite/Longitude", HOME_DEFAULT_LON).toInt();
      centerPoint = QPoint( hLat, hLon);
    }

  return centerPoint;
}

void WaypointCatalog::setCenterPoint( const WGSPoint& center )
{
  centerPoint = center;
}

/** read a catalog from file */
bool WaypointCatalog::readXml(const QString& catalog)
{
  qDebug() << "WaypointCatalog::readXml():" << catalog;

  QFile file(catalog);

  if( ! file.exists() )
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString("<html><B>%1</B><BR>").arg(catalog) +
                             QObject::tr("not found!") +
                             "</html>",
                             QMessageBox::Ok );
      return false;
    }

  if( ! file.open( QIODevice::ReadOnly ) )
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString("<html><B>%1</B><BR>").arg(catalog) +
                             QObject::tr("permission denied!") +
                             "</html>", QMessageBox::Ok );
      return false;
    }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QString errorMsg;
  int errorLine;
  int errorColumn;
  QDomDocument doc;

  bool ok = doc.setContent( &file, false, &errorMsg, &errorLine, &errorColumn );

  if( ! ok )
    {
      QApplication::restoreOverrideCursor();

      qWarning() << "WaypointCatalog::readXml(): XML parse error in File="
    		 << catalog
                 << "Error=" << errorMsg
                 << "Line=" << errorLine
                 << "Column=" << errorColumn;

      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error in %1").arg(QFileInfo(catalog).fileName()),
                             QString("<html>XML Error at line %1 column %2:<br><br>%3</html>").arg(errorLine).arg(errorColumn).arg(errorMsg),
                             QMessageBox::Ok );
      file.close();
      return false;
    }

  if (doc.doctype().name() == "KFLogWaypoint")
    {
      QDomNodeList nl = doc.elementsByTagName("Waypoint");

      for( int i = 0; i < nl.count(); i++ )
        {
          QDomNamedNodeMap nm =  nl.item(i).attributes();
          Waypoint *w = new Waypoint;

          w->name = nm.namedItem("Name").toAttr().value().left(8).toUpper();
          w->description = nm.namedItem("Description").toAttr().value();
          w->icao = nm.namedItem("ICAO").toAttr().value().toUpper();

          if( w->icao == "-1" )
            {
              w->icao = "";
            }

          w->type = nm.namedItem("Type").toAttr().value().toInt();
          w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
          w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
          w->elevation = nm.namedItem("Elevation").toAttr().value().toFloat();

          float fqval = nm.namedItem("Frequency").toAttr().value().toDouble();
          Frequency fq;
          fq.setUnit( Frequency::MHz );
          fq.setValue( fqval );
          w->addFrequency( fq );

          ushort rwyHeading = nm.namedItem("Runway").toAttr().value().toUShort();
          ushort rwyHd[2];
          rwyHd[0] = rwyHeading >> 8;
          rwyHd[1] = rwyHeading & 0xff;
          bool isLandable = nm.namedItem("Landable").toAttr().value().toInt();
          int rwyLength = nm.namedItem("Length").toAttr().value().toInt();
          enum Runway::SurfaceType rwySfc = (enum Runway::SurfaceType) nm.namedItem("Surface").toAttr().value().toInt();

          for( int i=0; i < 2; i++ )
            {
              if( rwyHd[i] > 0 )
                {
                  // only runway headings > 0 are valid and taken into account
                  Runway rwy;
                  rwy.setName( QString( "%1").arg( rwyHd[i], 2, 10, QChar('0') ) );
                  rwy.setHeading( rwyHd[i] * 10 );
                  rwy.setLength( rwyLength );
                  rwy.setSurface( rwySfc );

                  if( isLandable == true )
                    {
                      rwy.setOperations( Runway::Active );
                    }
                  else
                    {
                      rwy.setOperations( Runway::Closed );
                    }

                  w->addRunway( rwy );
                }
            }

          w->comment = nm.namedItem("Comment").toAttr().value();
          w->importance = nm.namedItem("Importance").toAttr().value().toInt();

          if( nm.contains("Country") )
            {
              w->country = nm.namedItem("Country").toAttr().value();
            }

          if( !insertWaypoint( w ) )
            {
              break;
            }
        }

      onDisc = true;
      path = catalog;

      ok = true;
    }
  else
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QObject::tr("wrong doctype ") + doc.doctype().name(),
                             QMessageBox::Ok );
    }

  file.close();
  QApplication::restoreOverrideCursor();

  return ok;
}

/** No descriptions */
bool WaypointCatalog::writeXml()
{
  bool ok = true;
  QDomDocument doc("KFLogWaypoint");
  QDomElement root = doc.createElement("KFLogWaypoint");
  QDomElement child;
  Waypoint *w;
  QFile file;
  QString fName = path;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  root.setAttribute( "Application", "KFLog" );
  root.setAttribute( "Creator", getLogin() );
  root.setAttribute( "Time", QTime::currentTime().toString( "HH:mm:mm" ) );
  root.setAttribute( "Date", QDate::currentDate().toString( Qt::ISODate ) );
  root.setAttribute( "Version", "1.0" );
  root.setAttribute( "Entries", wpList.size() );

  doc.appendChild(root);

  foreach(w, wpList)
  {
#if 0
    qDebug( "writing waypoint %s (%s - %s)",
            w->name.toLatin1().data(),
            w->description.toLatin1().data(),
            w->icao.toLatin1().data() );
#endif

    child = doc.createElement("Waypoint");

    child.setAttribute("Name", w->name.left(8).toUpper());
    child.setAttribute("Description", w->description);
    child.setAttribute("ICAO", w->icao);
    child.setAttribute("Type", w->type);
    child.setAttribute("Latitude", w->origP.lat());
    child.setAttribute("Longitude", w->origP.lon());
    child.setAttribute("Elevation", w->elevation);
    child.setAttribute("Comment", w->comment);
    child.setAttribute("Importance", w->importance);
    child.setAttribute("Country", w->country);

    Frequency fq;

    if( w->frequencyList.size() == 0 ) {
      child.setAttribute("Frequency", 0.0);
    }
    else {
      Frequency::getMainFrequency( w->frequencyList, fq );
    }

    child.setAttribute("Frequency", fq.getValue());

    // Only one runway is stored under XML
    const Runway& rwy = Runway::getMainRunway( w->rwyList );

    child.setAttribute("Landable", rwy.isOpen() );
    child.setAttribute("Runway", (rwy.getHeading() << 8 ) );
    child.setAttribute("Length", rwy.getLength() );
    child.setAttribute("Surface", rwy.getSurface() );

    root.appendChild(child);
  }

  file.setFileName(fName);

  if( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      const int IndentSize = 4;

      QTextStream out( &file );
      doc.save( out, IndentSize );
      file.close();

      path = fName;
      modified = false;
      onDisc = true;
    }
  else
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString ("<html><B>%1</B><BR>").arg(fName) +
                             QObject::tr("permission denied!") +
                             "</html>", QMessageBox::Ok );
    }

  QApplication::restoreOverrideCursor();
  return ok;
}

bool WaypointCatalog::writeBinary()
{
  bool ok = true;
  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  qint8 wpType;
  qint32 wpLatitude;
  qint32 wpLongitude;
  float wpElevation;
  QString wpComment="";
  quint8 wpImportance;

  Waypoint *w;
  QFile f;
  QString fName = path;

  f.setFileName(fName);

  if (f.open(QIODevice::WriteOnly))
    {
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      QDataStream out(& f);
      out.setVersion( DATA_STREAM );

      // write file header
      out << quint32( KFLOG_FILE_MAGIC );
      out << qint8( FILE_TYPE_WAYPOINTS );
      out << quint16( FILE_FORMAT_ID_8 );
      out << qint32( wpList.size() );

      foreach(w, wpList)
        {
          wpName = w->name.left(8).toUpper();
          wpDescription = w->description;
          wpICAO = w->icao;
          wpType = w->type;
          wpLatitude = w->origP.lat();
          wpLongitude = w->origP.lon();
          wpElevation = w->elevation;
          wpComment = w->comment;
          wpImportance = w->importance;

          out << wpName;
          out << wpDescription;
          out << wpICAO;
          out << wpType;
          out << wpLatitude;
          out << wpLongitude;
          out << wpElevation;

          // The frequency list is saved
          Frequency::saveFrequencies( out, w->getFrequencyList() );

          out << wpComment;
          out << wpImportance;
          out << w->country;

          // The runway list is saved
          Runway::saveRunways( out, w->getRunwayList() );
        }

      f.close();
      path = fName;
      modified = false;
      onDisc = true;

      QApplication::restoreOverrideCursor();
    }
  else
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString ("<html><B>%1</B><BR>").arg(fName) +
                             QObject::tr("permission denied!") + "</html>",
                             QMessageBox::Ok );
    }

  return ok;
}

/** No descriptions */
bool WaypointCatalog::readVolkslogger(const QString& filename)
{
#ifndef _WIN32
  QFileInfo fInfo(filename);
  QFile f(filename);

  if(!fInfo.exists())
    {
      QMessageBox::critical(_mainWindow, QObject::tr("Error occurred!"),
                            "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(filename) + "</html>", QMessageBox::Ok );
      return false;
    }

  if(!fInfo.size())
    {
      QMessageBox::warning(_mainWindow, QObject::tr("Error occurred!"),
                           "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is empty!").arg(filename) + "</html>", QMessageBox::Ok );
      return false;
    }
  //
  // We need a better format-identification then only the extension ...
  //
  if( fInfo.suffix().toLower() != "dbt")
    {
      QMessageBox::critical(_mainWindow, QObject::tr("Error occurred!"),
                            "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is not a Volkslogger-file!").arg(filename) + "</html>",
                            QMessageBox::Ok );
      return false;
    }

  if(!f.open(QIODevice::ReadOnly))
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("No permission"),
                             "<html>" +
                             QObject::tr("You don't have permission to access file<BR><B>%1</B>").arg(filename) +
                             "</html>",
                             QMessageBox::Ok );
      return false;
    }

  QProgressDialog importProgress( _mainWindow );
  importProgress.setWindowModality(Qt::WindowModal);
  importProgress.setWindowTitle(QObject::tr("Import file ..."));
  importProgress.setLabelText( "<html>" +
                               QObject::tr("Please wait while loading file<BR><B>%1</B>").arg(filename) + "</html>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setRange(0, 200);
  importProgress.setVisible(true);
  importProgress.setMinimumDuration(0);
  importProgress.setValue(0);

  unsigned int fileLength = fInfo.size();
  unsigned int filePos = 0;

  QStringList s;
  QString line;
  QTextStream stream(&f);

  int lat, lon, latTemp, lonTemp, latmin, lonmin;
  char latChar, lonChar;
  int lineCount = 0;
  QChar flag;

  while (!stream.atEnd())
    {
      if( importProgress.wasCanceled() )
        {
          return false;
        }

      lineCount++;
      line = stream.readLine();
      s = line.split (",");
      filePos += line.length();
      importProgress.setValue(( filePos * 200 ) / fileLength);
      Waypoint *w = new Waypoint;

      //
      // File is a collection of WPs.
      //
      latChar = 'N';
      lonChar = 'E';

      // line format
      // 001,name(max. 6 chars),lat,long,flags
      // flags = XN
      // N = OR'd digit of
      // 1 = landable
      // 2 = hard surface (we use asphalt) if set else gras
      // 4 = airport if set else glider site
      // 8 = checkpoint (ignored)

      if (s.count() == 5)
        {
          // sscanf(s[2], "%2d%5d", &lat, &latmin);
          lat    = s[2].left(2).toInt();
          latmin = s[2].mid(2,5).toInt();

          // sscanf(s[3], "%3d%5d", &lon, &lonmin);
          lon    = s[3].left(3).toInt();
          lonmin = s[3].mid(3,5).toInt();

          latTemp = lat * 600000 + latmin * 10;
          lonTemp = lon * 600000 + lonmin * 10;

          if(latChar == 'S') latTemp = -latTemp;
          if(lonChar == 'W') lonTemp = -lonTemp;

          if (s[4].length() == 2)
            {
              flag = s[4][1];
            }
          else
            {
              flag = 0;
            }

          w->name = s[1].trimmed();

          if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_L)
            {
              if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_H)
                {
                  // w->surface = Runway::Asphalt;
                }
              else
                {
                  // w->surface = Runway::Grass;
                }

              if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_A)
                {
                  w->type = BaseMapElement::Airfield;
                }

              // w->isLandable = true;
            }

          w->origP = WGSPoint(latTemp, lonTemp);

          if (!insertWaypoint(w))
            {
              delete w;
              break;
            }
        }
    }

  // close the import dialog, clean up and add the FlightRoute we just created
  importProgress.close();
#endif
  return true;
}

/** Checks if the file exists on disk, and if not asks the user for it.
  * It then calls either write() or writeBinary(), depending on the
  * selected format.
  */
bool WaypointCatalog::save(bool alwaysAskName)
{
  QString fName = path;

  if( !onDisc || alwaysAskName )
    {
      QString filter;
      filter.append(QObject::tr("All formats") + " (*.dat *.DAT *.dbt *.DBT *.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP *.txt *.TXT);;");
      filter.append(QString("KFLog") + " (*.kflogwp *.KFLOGWP);;");
      filter.append(QString("KFLog/Cumulus") + " (*.kwp *.KWP);;");
      filter.append(QString("Cambrigde") + " (*.dat *.DAT);;");
      filter.append(QString("Filser txt") + " (*.txt *.TXT);;");
      filter.append(QString("Filser da4") + " (*.da4 *.DA4);;");
      filter.append(QString("SeeYou") + " (*.cup *.CUP);;");
      // filter.append(QString("Any file") + " (*.*)");

      fName = QFileDialog::getSaveFileName( _mainWindow,
                                            QObject::tr("Save waypoint catalog"),
                                            path,
                                            filter );
      if( fName.isEmpty() )
        {
          return false;
        }

      path = fName;
    }

  if (fName.right(8).toLower() == ".kflogwp")
    return writeXml();
  else if (fName.right(4).toLower() == ".txt")
    return writeFilserTXT(fName);
  else if (fName.right(4).toLower() == ".da4")
    return writeFilserDA4(fName);
  else if (fName.right(4).toLower() == ".dat")
    return writeDat(fName);
  else if (fName.right(4).toLower() == ".cup")
    return writeCup(fName);
  else if (fName.toLower().endsWith( ".kwp" ))
      return writeBinary();
  else
    qWarning() << "WaypointCatalog::load(): unknown file type" << fName;

  return false;

}

/**
 * This function calls either read or readBinary depending on the filename
 * of the catalog.
 */
bool WaypointCatalog::load(const QString& catalog)
{
  qDebug() << "load catalog" << catalog;

  if (catalog.right(8).toLower() == ".kflogwp")
    return readXml(catalog);
  else if (catalog.right(4).toLower() == ".txt")
    return readFilserTXT(catalog);
  else if (catalog.right(4).toLower() == ".da4")
    return readFilserDA4(catalog);
  else if (catalog.right(4).toLower() == ".cup")
    return readCup(catalog);
  else if (catalog.right(4).toLower() == ".dat")
    return readDat(catalog);
  else if (catalog.right(4).toLower() == ".dbt")
    return readVolkslogger(catalog);
  else if (catalog.toLower().endsWith( ".kwp" ))
    return readBinary(catalog);
  else
    qWarning() << "WaypointCatalog::load(): unknown file type" << catalog;

  return false;
}

/** read a waypoint catalog from a filser txt file */
bool WaypointCatalog::readFilserTXT(const QString& catalog)
{
  qDebug() << "WaypointCatalog::readFilserTXT: " << catalog;

  QFile f(catalog);

  if (f.exists())
    {
      if (f.open(QIODevice::ReadOnly))
        {
          while (!f.atEnd())
            {
              char line[256];

              qint64 result = f.readLine (line, sizeof(line));

              if (result > 0)
                {
                  QStringList list = QString(line).split (",");

                  if(list.size() == 0 || list[0] == "*") // comment/header line
                    {
                      continue;
                    }

                  if( list.size() < 9 )
                    {
                      // That will prevent a crash, if a wrong file is read!
                      continue;
                    }

                  Waypoint *w = new Waypoint;
                  w->name = list[1];
                  w->description = "";
                  w->icao = "";

                  if (list[2].toUpper() == "TP")
                    w->type = BaseMapElement::Landmark;
                  else if (list[2].toUpper() == "APT")
                    w->type = BaseMapElement::Airfield;
                  else if (list[2].toUpper() == "OUTLAN")
                    w->type = BaseMapElement::Outlanding;
                  else if (list[2].toUpper() == "MARKER")
                    w->type = BaseMapElement::Landmark;
                  else
                    w->type = BaseMapElement::Landmark;
                  w->origP.setLat((int)(rint(list[3].toDouble() * 600000.0)));
                  w->origP.setLon((int)(rint(list[4].toDouble() * 600000.0)));
                  w->elevation = (int)(rint(list[5].toInt() * 0.3048)); // don't we have conversion constants ?

                  float fq = list[6].toFloat() / 1000.0;
                  Frequency freq;
                  freq.setValue( fq );
                  freq.setUnit( Frequency::MHz );
                  freq.setType( Frequency::Unknown );
                  w->addFrequency( freq );

                  ushort rwyHeading = list[8].toUShort(); // direction ?!

                  if( rwyHeading > 0 )
                    {
                      Runway rwy;
                      rwy.setHeading( rwyHeading * 10 );
                      rwy.setName( QString( "%1").arg( rwyHeading, 2, 10, QChar('0')) );
                      rwy.setLength( list[7].toUInt() ); // length ?!

                      QChar surface = list[9].toUpper()[0];

                      switch (surface.toLatin1())
                        {
                        case 'G':
                          rwy.setSurface( Runway::Grass );
                          break;
                        case 'C':
                          rwy.setSurface( Runway::Concrete );
                          break;
                        default:
                          rwy.setSurface( Runway::Unknown );
                          break;
                        }

                      w->rwyList.append(rwy);
                    }

                  w->comment = QObject::tr("Imported from %1").arg(catalog);
                  w->importance = 1;

                  if (!insertWaypoint(w))
                    {
                      delete w;
                      break;
                    }
                }
            }

          f.close();
          onDisc = true;
          path = catalog;
          return true;
        }
    }
  return false;
}

/** write a waypoint catalog into a filser txt file */
bool WaypointCatalog::writeFilserTXT (const QString& catalog)
{
  qDebug ("WaypointCatalog::writeFilserTXT (%s)", catalog.toLatin1().data());

  QFile f(catalog);

  if (f.open(QIODevice::WriteOnly))
    {
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      QTextStream out (&f);
      out << "*,TpName,Type,Latitude,Longitude,Altitude,Frequency,RWY,RWYdir,RWYtype,TCA,TC" << Qt::endl;

      foreach(Waypoint* w, wpList)
      {
        out << "," << w->name << ",";

        switch (w->type)
          {
          case BaseMapElement::Landmark:
            out << "TP,";
            break;
          case BaseMapElement::Airfield:
            out << "APT,";
            break;
          case BaseMapElement::Outlanding:
            out << "OUTLAN,";
            break;
          default:
            out << "MARKER,";
            break;
          }

        out << w->origP.lat()/600000.0 << ",";
        out << w->origP.lon()/600000.0 << ",";
        out << (int)(w->elevation/0.3048) << ",";

        Frequency fq;

        Frequency::getMainFrequency( w->frequencyList, fq );
        out << (int)(fq.getValue() * 1000) << ",";

        Runway rwy;

        if( w->rwyList.size() > 0 )
          {
            rwy = w->rwyList[0];
          }

        out << (int) rwy.getLength() << ",";
        out << rwy.getHeading() << ",";

        switch (rwy.getSurface() )
          {
          case Runway::Grass:
            out << "G,";
            break;
          case Runway::Concrete:
            out << "C,";
            break;
          default:
            out << "U,";
            break;
          }

        out << "3,I,,," << Qt::endl;
      }

      f.close();
      QApplication::restoreOverrideCursor();
      return true;
    }

  return false;
}

/** read a waypoint catalog from a filser da4 file */
bool WaypointCatalog::readFilserDA4 (const QString& catalog)
{
  qDebug ("WaypointCatalog::readFilserDA4 (%s)", catalog.toLatin1().data());

  QFile f(catalog);

  if (f.exists())
    {
      if (f.open(QIODevice::ReadOnly))
        {
          QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

          QDataStream in(&f);
          DA4Buffer buffer;
          in.readRawData ((char*)&buffer, sizeof (DA4Buffer));

          for (int RecordNumber = 0; RecordNumber < WAYPOINT_MAX; RecordNumber++)
            {
              DA4WPRecord record (&buffer.waypoints[RecordNumber]);

              if (record.type() != BaseMapElement::NotSelected)
                {
                  if (record.name().trimmed().isEmpty())
                    continue;
                  Waypoint *w = record.newWaypoint();

                  if (!insertWaypoint(w))
                    {
                      delete w;
                      break;
                    }
                }
            }

          f.close();
          onDisc = true;
          path = catalog;
          QApplication::restoreOverrideCursor();
          return true;
        }
    }

  return false;
}

/** write a waypoint catalog into a filser da4 file */
bool WaypointCatalog::writeFilserDA4 (const QString& catalog)
{
  qDebug() << "WaypointCatalog::writeFilserDA4:" << catalog << "with"
           << wpList.size() << "item(s)";

  if( wpList.size() > WAYPOINT_MAX )
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("To much waypoints!"),
                             QObject::tr("A DA4 waypoint file can only contain up to 600 waypoints. ") +
                             QString(QObject::tr("Your file contains %1.")).arg(wpList.size()),
                             QMessageBox::Ok );
      return false;
    }

  QFile f(catalog);

  if (f.open(QIODevice::WriteOnly))
    {
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      QDataStream out (&f);
      DA4Buffer buffer;
      int RecordNumber = 0;

      foreach(Waypoint* w, wpList)
      {
        DA4WPRecord record (&buffer.waypoints[RecordNumber++]);
        record.setWaypoint(w);
      }

      // fill rest of waypoints
      while (RecordNumber < WAYPOINT_MAX)
        {
          DA4WPRecord record (&buffer.waypoints[RecordNumber++]);
          record.clear();
        }

      // write empty tasks
      RecordNumber = 0;

      while (RecordNumber < TASK_MAX)
        {
          DA4TaskRecord record (&buffer.tasks[RecordNumber++]);
          record.clear();
        }

      out.writeRawData ((char*)&buffer, sizeof(DA4Buffer));

      // fill buffer with empty task names
      char buf [MAXTSKNAME] = "                                    ";

      for (RecordNumber = 0; RecordNumber < TASK_MAX; RecordNumber++)
        {
          out.writeRawData (buf, MAXTSKNAME);
        }

      f.close();
      QApplication::restoreOverrideCursor();
      return true;
    }

  return false;
}

/** read a catalog from file */
bool WaypointCatalog::readBinary(const QString &catalog)
{
  qDebug() << "WaypointCatalog::readBinary" << catalog;

  bool ok = false;

  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  qint8 wpType;
  qint32 wpLatitude;
  qint32 wpLongitude;
  qint16 wpElevation;
  double wpFrequency;
  qint8 wpLandable;
  qint16 wpRunway;
  qint16 wpLength;
  qint8 wpSurface;
  QString wpComment="";
  QString wpCountry="";
  quint8 wpImportance;

  quint32 fileMagic;
  qint8 fileType;
  quint16 fileFormat;
  qint32 wpListSize = 0;

  // new variables from format version 3
  float wpFrequency3;
  float wpElevation3;
  float wpLength3;

  // new element from format 8
  QList<Frequency> frequencyList;

  // new element from format version 4
  QList<Runway> rwyList;

  QFile f(catalog);

  if (! f.exists())
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QObject::tr("<html><B>Catalog %1</B><BR>not found!</html>").arg(catalog),
                             QMessageBox::Ok );

      return false;
    }

  if (f.open(QIODevice::ReadOnly))
    {

      QDataStream in(&f);

      //check if the file has the correct format
      in >> fileMagic;

      if (fileMagic != KFLOG_FILE_MAGIC)
        {
          qDebug("Waypoint file not recognized as KFLog file type.");
          return false;
        }

      in >> fileType;

      if (fileType != FILE_TYPE_WAYPOINTS)
        {
          qDebug("Waypoint file is a KFLog file, but not for waypoints.");
          return false;
        }

      in >> fileFormat;

      if( fileFormat < FILE_FORMAT_ID_2 )
        {
          qWarning() << "Wrong waypoint file format! Read format Id"
                     << fileFormat
                     << ". Expecting" << FILE_FORMAT_ID_3 << ".";

          return false;
        }

      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      // from here on, we assume that the file has the correct format.
      if( fileFormat == FILE_FORMAT_ID_2 )
        {
          in.setVersion( QDataStream::Qt_2_0 );
        }
      else
        {
          in.setVersion( DATA_STREAM );
          in >> wpListSize;
        }

      while( !in.atEnd() )
        {
          frequencyList.clear();
          rwyList.clear();

          // read values from file
          in >> wpName;
          in >> wpDescription;
          in >> wpICAO;
          in >> wpType;
          in >> wpLatitude;
          in >> wpLongitude;

          qDebug() << "WpName" << wpName << "wpDescription" << wpDescription;

          if( fileFormat < FILE_FORMAT_ID_3 )
            {
              in >> wpElevation;
              in >> wpFrequency;
            }
          else if( fileFormat < FILE_FORMAT_ID_8 )
            {
              in >> wpElevation3;
              in >> wpFrequency3;
            }
          else
            {
              in >> wpElevation3;
              Frequency::loadFrequencies( in, frequencyList );
            }

          if( fileFormat < FILE_FORMAT_ID_4 )
            {
              in >> wpLandable;
              in >> wpRunway;

              if( fileFormat < FILE_FORMAT_ID_3 )
                {
                  in >> wpLength;
                }
              else
                {
                  in >> wpLength3;
                }

              in >> wpSurface;
            }

          in >> wpComment;
          in >> wpImportance;

          if( fileFormat >= FILE_FORMAT_ID_3 )
            {
              in >> wpCountry;
            }

          if( fileFormat >= FILE_FORMAT_ID_4 && fileFormat < FILE_FORMAT_ID_8 )
            {
              // The runway list has to be read
              quint8 listSize;
              quint16 ilength;
              float   flength;
              quint16 iwidth;
              float   fwidth;
              quint16 heading;
              quint8 surface;
              quint8 isOpen;
              quint8 isBidirectional;

              in >> listSize;

              for( int i = 0; i < (int) listSize; i++ )
                {
                  if( fileFormat >= FILE_FORMAT_ID_5 )
                    {
                      in >> flength;
                      in >> fwidth;
                    }
                  else
                    {
                      in >> ilength;
                      flength = static_cast<float>(ilength);
                      in >> iwidth;
                      fwidth = static_cast<float>(iwidth);
                    }

                  in >> heading;
                  in >> surface;
                  in >> isOpen;
                  in >> isBidirectional; // not used in KFLog

                  QPair<ushort, ushort> headings;
                  headings.first = heading >> 8;
                  headings.second = heading & 0xff;

                  Runway rwy;
                  rwy.setLength( flength );
                  rwy.setWidth( fwidth );
                  rwy.setHeading( headings.first * 10 );
                  rwy.setName( QString( "%1").arg(headings.first, 2, 10, QChar('0')) );
                  rwy.setSurface( surface );
                  if( isOpen )
                    rwy.setOperations( Runway::Active );
                  else
                    rwy.setOperations( Runway::Closed );

                  rwyList.append( rwy );
                }
            }

          if( fileFormat >= FILE_FORMAT_ID_8 )
            {
              Runway::loadRunways( in, rwyList );
            }

          //create new waypoint object and set the correct properties
          Waypoint *w = new Waypoint;

          w->name = wpName.left(8).toUpper();
          w->description = wpDescription;
          w->icao = wpICAO;
          w->type = wpType;
          w->origP.setLat(wpLatitude);
          w->origP.setLon(wpLongitude);
          w->comment = wpComment;
          w->importance = wpImportance;
          w->country = wpCountry;

          if( fileFormat < FILE_FORMAT_ID_3 )
            {
              w->elevation = wpElevation;
              Frequency fq;
              fq.setValue( wpFrequency );
              fq.setUnit( Frequency::MHz );
              w->addFrequency( fq );
            }
          else if( fileFormat < FILE_FORMAT_ID_8 )
            {
              w->elevation = wpElevation3;
              Frequency fq;
              fq.setValue( wpFrequency3 );
              fq.setUnit( Frequency::MHz );
              w->addFrequency( fq );
            }
          else
            {
              w->elevation = wpElevation3;
              w->setFequencyList( frequencyList );
            }

          if( fileFormat >= FILE_FORMAT_ID_4 )
            {
              // We have a runway list
              if( rwyList.size() )
                {
                  w->rwyList = rwyList;
                }
            }

          if( ! insertWaypoint(w) )
            {
              qDebug("odd... error reading waypoints");
              delete w;
              break;
            }
        }

      onDisc = true;
      path = catalog;
      ok = true;

      if( fileFormat < FILE_FORMAT_ID_5 )
        {
          // write file back in newer format
          writeBinary();
        }

      f.close();
      QApplication::restoreOverrideCursor();
    }
  else
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString("<html><B>%1</B><BR>").arg(catalog) +
                             "permission denied!" +
                             "</html>", QMessageBox::Ok );

      ok = false;
    }

  return ok;
}

/** read a waypoint catalog from a SeeYou cup file, only waypoint part */
bool WaypointCatalog::readCup (const QString& catalog)
{
  qDebug() << "WaypointCatalog::readCupFile" << catalog;

  QFile file(catalog);

  if(!file.exists())
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(catalog) + "</html>",
                             QMessageBox::Ok );
      return false;
    }

  if(file.size() == 0)
    {
      QMessageBox::warning( _mainWindow,
                            QObject::tr("Error occurred!"),
                            "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is empty!").arg(catalog) + "</html>",
                            QMessageBox::Ok );
      return false;
    }

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      return false;
    }

  QSet<QString> names;

  for( int i = 0; i < wpList.size(); i++ )
    {
      // Store all used names of the waypoint list in a set.
      names.insert( wpList.at(i)->name );
    }

  int lineNo = 0;

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  QString cupFormatMinimal = "name,code,country,lat,lon,elev,style";
  QString cupFormatOld = cupFormatMinimal + ",rwdir,rwlen,freq,desc";
  QString cupFormat = cupFormatMinimal + ",rwdir,rwlen,rwwidth,freq,desc";

  int old = 0;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  while (!in.atEnd())
    {
      QString line = in.readLine().trimmed();

      if( line[0] == '#' )
        {
          // ignore comments at the beginning of the file
          continue;
        }

      lineNo++;

      if( lineNo == 1 )
        {
          // Check first line, describing cup format
          if( line.size() > cupFormatMinimal.size() &&
              line.startsWith( cupFormat ) == true )
            {
              old = 0;
            }
          else if( line.size() > cupFormatMinimal.size() &&
                   line.startsWith( cupFormatOld ) == true )
            {
              old = 1;
            }
          else if( line != cupFormatMinimal )
            {
              qWarning() << "WaypointCatalog::readCup(): File"
                         << catalog
                         << "is not a supported cup file according"
                         << "to specification of 2022!";

              file.close();
              QApplication::restoreOverrideCursor();
              return false;
            }

          continue;
        }

      bool ok;

      QList<QString> list = splitCupLine( line, ok );

      if( list[0] == "-----Related Tasks-----" )
        {
          // Task part starts, we will ignore it and break up reading
          break;
        }

      // 7 elements are mandatory, the rest is optional
      if( list.size() < 7 )
        {
          // too less elements, ignore this entry
          continue;
        }

      // A cup line consists of the following elements in 2022:
      //
      // 0     1     2        3    4    5     6      7      8      9        10    11    12        13
      // name, code, country, lat, lon, elev, style, rwdir, rwlen, rwwidth, freq, desc, userdata, pics
      //
      // Older versions of CUP has no item rwwidth.
      //
      // Columns after style field may be missing and can be removed from header and data.
      //
      // See here for more info: https://downloads.naviter.com/docs/SeeYou_CUP_file_format.pdf

      Waypoint wp;
      Runway rwy;

      if( list[0].length() ) // long name of waypoint
        {
          wp.description = list[0].replace( QRegExp("\""), "" );
        }
      else
        {
          qWarning( "CUP Read (%d): No name is defined, ignoring record.",
                    lineNo );
          continue;
         }

      if( list[1].isEmpty() )
        {
          // If no code (short name) is set, we assign the long name as code to have a workaround.
          list[1] = list[0];
        }

      // short name of a waypoint limited to 8 characters
      wp.name = list[1].replace( QRegExp("\""), "" ).left(8).toUpper();
      wp.country = list[2].left(2).toUpper();
      wp.icao = "";

      // waypoint type aka style
      uint wpType = list[6].toUInt(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Invalid waypoint type '%s'. Ignoring it.",
                   lineNo, list[6].toLatin1().data() );
          continue;
        }

      switch( wpType )
        {
        case 1:
          wp.type = BaseMapElement::Landmark;
          break;
        case 2:
          wp.type = BaseMapElement::Airfield;
          rwy.setSurface( Runway::Grass );
          break;
        case 3:
          wp.type = BaseMapElement::Outlanding;
          break;
        case 4:
          wp.type = BaseMapElement::Gliderfield;
          rwy.setSurface( Runway::Grass );
          break;
        case 5:
          wp.type = BaseMapElement::Airfield;
          rwy.setSurface( Runway::Concrete );
          break;
        case 9:
          wp.type = BaseMapElement::Vor;
          break;
        case 10:
          wp.type = BaseMapElement::Ndb;
          break;
        case 11:
          wp.type = BaseMapElement::Thermal;
          break;
        case 19:
          wp.type = BaseMapElement::CompPoint;
          break;
        case 20:
        case 21:
          wp.type = BaseMapElement::HangGlider;
          break;
        default:
          wp.type = BaseMapElement::Landmark;
          break;
        }

      // latitude as ddmm.mmm(N|S)
      double degree = list[3].left(2).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (N/S) (1)", lineNo);
          continue;
        }

      double minutes = list[3].mid(2,6).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (N/S) (2)", lineNo);
          continue;
        }

      double latTmp = (degree * 600000.) + (minutes * 10000.0);

      if( list[3].right(1).toUpper() == "S" )
        {
          latTmp = -latTmp;
        }

      // longitude dddmm.mmm(E|W)
      degree = list[4].left(3).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (E/W) (1)", lineNo);
          continue;
        }

      minutes = list[4].mid(3,6).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (E/W) (2)", lineNo);
          continue;
        }

      double lonTmp = (degree * 600000.) + (minutes * 10000.0);


      if( list[4].right(1).toUpper() == "W" )
        {
          lonTmp = -lonTmp;
        }

      wp.origP.setLat((int) rint(latTmp));
      wp.origP.setLon((int) rint(lonTmp));

      // read elevation
      // two units are possible:
      // o meter: m
      // o feet:  ft
      if( list[5].length() )
        {
          // elevation in meter or feet, if missing meter is assumed
          QString unit;
          list[5] = list[5].toLower();
          int uStart = list[5].indexOf( QRegExp("[mf]") );

          if( uStart == -1 )
            {
              unit = "m";
            }

          unit = list[5].mid( uStart ).toLower();

          float tmpElev = (list[5].left(list[5].length() - unit.length())).toFloat(&ok);

          if( ! ok )
            {
              qWarning("CUP Read (%d): Error reading elevation value '%s'.", lineNo,
                       list[5].left(list[5].length() - unit.length()).toLatin1().data());
              continue;
            }

          if( unit == "m" )
            {
              wp.elevation = tmpElev;
            }
          else if( unit == "ft" )
            {
              wp.elevation = tmpElev * 0.3048;
            }
          else
            {
              qWarning("CUP Read (%d): Unknown elevation value '%s'.", lineNo,
                       unit.toLatin1().data());
              continue;
            }
        }

      if( list.size() == 7 )
        {
          // no more data defined
          continue;
        }

      bool takeRwyData = false;

      // runway direction 010...360
      if( list.size() >= 8  && list[7].trimmed().size() > 0 )
        {
          uint rdir = list[7].toInt(&ok);

          if( ok )
            {
              rwy.setHeading( rdir );

              // set runway designator
              QString name;

              int rd = ( rdir + 5 ) / 10;

              if( rd == 0 ) {
                  rd = 36;
              }

              if( rd < 10 ) {
                  name += '0';
              }

              rwy.setName( name + QString::number( rd ) );
              takeRwyData = true;
            }
        }

      // runway length with unit
      if( list.size() >= 9 && list[8].trimmed().size() > 0 )
        {
          // three units are possible:
          // o meter: m
          // o nautical mile: nm
          // o statute mile: ml
          // o feet: ft, @AP: Note that is not conform to the SeeYou specification
          //                  but I saw it in an south African file.
          // If unit is missing, meter is assumed
          QString unit;
          list[8] = list[8].toLower();
          int uStart = list[8].indexOf( QRegExp("[fmn]") );
          float length = 0.0;

          if( uStart != -1 )
            {
              unit = list[8].mid( uStart ).toLower();
              length = list[8].left( list[8].length()-unit.length() ).toFloat(&ok);
            }
          else
            {
              unit = "m";
              length = list[8].toFloat( &ok );
            }

          if( ok )
            {
              if( unit == "nm" ) // nautical miles
                {
                  length *= 1852;
                }
              else if( unit == "ml" ) // statute miles
                {
                  length *= 1609.34;
                }
              else if( unit == "ft" ) // feet
                {
                  length *= 0.3048;
                }

              rwy.setLength( length );
              rwy.setOperations( Runway::Active );
              rwy.setTurnDirection( Runway::Both );
              takeRwyData = true;
            }
        }

      // runway width with unit as new item
      if( old == 0 && list.size() >= 10 && list[9].trimmed().size() > 0 )
        {
          // three units are possible:
          // o meter: m
          // o nautical mile: nm
          // o statute mile: ml
          // o feet: ft, @AP: Note that is not conform to the SeeYou specification
          //                  but I saw it in an south African file.
          // If unit is missing, meter is assumed
          QString unit;
          list[9] = list[9].toLower();
          int uStart = list[9].indexOf( QRegExp("[fmn]") );
          float width = 0.0;

          if( uStart != -1 )
            {
              unit = list[9].mid( uStart ).toLower();
              width = list[9].left( list[9].length()-unit.length() ).toFloat(&ok);
            }
          else
            {
              unit = "m";
              width = list[9].toFloat( &ok );
            }

          if( ok )
            {
              if( unit == "nm" ) // nautical miles
                {
                  width *= 1852;
                }
              else if( unit == "ml" ) // statute miles
                {
                  width *= 1609.34;
                }
              else if( unit == "ft" ) // feet
                {
                  width *= 0.3048;
                }

              rwy.setWidth( width );
              takeRwyData = true;
            }
        }

      if( takeRwyData == true )
        {
          // Store runway in the runway list with the set data
          wp.rwyList.append( rwy );
        }

      // airport frequency as 123.500, can be enclosed in quotations marks
      if( list.size() >= 11 - old && list[10 -old].trimmed().size() )
        {
          float frequency = list[10 - old].replace( QRegExp("\""), "" ).toFloat(&ok);

          Frequency freq;

          if( ok )
            {
              freq.setValue( frequency );
              freq.setUnit( Frequency::MHz );
              freq.setType( Frequency::Info );
              wp.addFrequency( freq );
            }
          else
            {
              wp.addFrequency( freq );
            }
        }

      // description, optional
      if( list.count() >= 12 - old && list[11 - old].trimmed().length() )
        {
          wp.comment += list[11 - old ].replace( QRegExp("\""), "" );
        }

      // We do check, if the waypoint name is already in use because cup
      // short names are not always unique.
      if( names.contains( wp.name ) )
        {
          for( int i = 0; i < 100; i++ )
            {
              // Hope that not more as 100 same names will be exist.
              QString number = QString::number(i);
               wp.name = wp.name.left(wp.name.size() - number.size()) + number;

              if( names.contains( wp.name ) == false )
                {
                  break;
                }
            }
        }

      Waypoint* w = new Waypoint( wp );

      if( ! insertWaypoint( w ) )
        {
          qWarning("CUP Read (%d): Error inserting waypoint in catalog", lineNo );
          delete w;
          break;
        }

      // Store used waypoint name in set.
      names.insert( wp.name );
    }

  file.close();
  QApplication::restoreOverrideCursor();

  onDisc = true;
  path = catalog;
  return true;
}

bool WaypointCatalog::insertWaypoint(Waypoint *newWaypoint)
{
  bool result = true;
  int index;

  Waypoint *existingWaypoint = findWaypoint( newWaypoint->name, index );

  if( existingWaypoint != 0 )
    {
      qDebug() << "FoundWP" << existingWaypoint->name;

      if( existingWaypoint->name != newWaypoint->name ||
          existingWaypoint->angle != newWaypoint->angle ||
          existingWaypoint->comment != newWaypoint->comment ||
          existingWaypoint->description != newWaypoint->description ||
          existingWaypoint->distance != newWaypoint->distance ||
          existingWaypoint->elevation != newWaypoint->elevation ||
          existingWaypoint->fixTime != newWaypoint->fixTime ||
          existingWaypoint->icao != newWaypoint->icao ||
          existingWaypoint->importance != newWaypoint->importance ||
          existingWaypoint->origP != newWaypoint->origP ||
          existingWaypoint->type != newWaypoint->type ||
          existingWaypoint->country != newWaypoint->country )
        {
          switch( QMessageBox::warning( _mainWindow,
                                        QObject::tr("Waypoint exists"),
                                        "<html>" + QObject::tr("A waypoint with the name<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Do you want to replace the existing waypoint?").arg(newWaypoint->name) + "</html>",
                                        QMessageBox::Yes|QMessageBox::No|QMessageBox::Abort,
                                        QMessageBox::Yes ) )
            {
            case QMessageBox::Abort:
              delete newWaypoint;
              result = false;
              break;

            case QMessageBox::No:
              delete newWaypoint;
              break;

            case QMessageBox::Yes:
            default:
              delete wpList.takeAt(index);
              wpList.append(newWaypoint);
              break;
            }
        }
    }
  else
    {
      wpList.append( newWaypoint );
    }

  return result;
}

Waypoint *WaypointCatalog::findWaypoint( const QString& name, int &index )
{
  for( int i=0; i < wpList.size(); i++ )
    {
      if( wpList.at(i)->name == name )
        {
          index = i;
          return wpList.at(i);
        }
    }

  index = -1;

  return static_cast<Waypoint *> (0);
}

bool WaypointCatalog::removeWaypoint( const QString& name )
{
  for( int i = 0; i < wpList.size(); i++ )
    {
      if( wpList.at(i)->name == name )
        {
          delete wpList.takeAt(i);
          return true;
        }
    }

  return false;
}

QList<QString> WaypointCatalog::splitCupLine( QString& line, bool &ok )
{
  // A cup line consists of elements separated by commas. String elements
  // are enclosed in quotation marks. Inside such a string element, a
  // comma is allowed and is not to interpret as separator!
  QList<QString> list;

  int start, pos, len, idx;
  start = pos = len = idx = 0;

  line = line.trimmed();

  len = line.size();

  while( true )
    {
      if( line[pos] == QChar('"') )
        {
          // Handle quoted string
          pos++;

          if( pos >= len )
            {
              ok = false;
              return list;
            }

          // Search the end quote
          idx = line.indexOf( QChar('"'), pos );

          if( idx == -1 )
            {
              // Syntax error, abort split
              ok = false;
              return list;
            }

          pos = idx; // set current position to index of quote sign
        }

      idx = line.indexOf( QChar(','), pos );

      if( idx == -1 )
        {
          // No comma found, maybe we are at the end
          if( start < len )
            {
              list.append( line.mid( start, len-start ) );
            }
          else
            {
              list.append( QString("") );
            }

          ok = true;
          return list;
        }

      if( start < idx )
        {
          list.append( line.mid( start, idx-start ) );
        }
      else
        {
          list.append( QString("") );
        }

      if( (idx + 1) >= len )
        {
          // No more data available
          ok = true;
          return list;
        }

      pos = start = idx + 1;
    }

  return list;
}

/** Reads a Cambridge Aero Instruments turnpoint file. */
bool WaypointCatalog::readDat(const QString &catalog)
{
  // Found a file format description here:
  // http://www.gregorie.org/gliding/pna/cai_format.html

  qDebug() << "WaypointCatalog::readDatFile" << catalog;

  QFile file(catalog);

  if(!file.exists())
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(catalog) + "</html>",
                             QMessageBox::Ok );
      return false;
    }

  if(file.size() == 0)
    {
      QMessageBox::warning( _mainWindow,
                            QObject::tr("Error occurred!"),
                            "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is empty!").arg(catalog) + "</html>",
                            QMessageBox::Ok );
      return false;
    }

  if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QMessageBox::warning( _mainWindow,
                            QObject::tr("Error occurred!"),
                            "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is not readable!").arg(catalog) + "</html>",
                            QMessageBox::Ok );

      return false;
    }

  QSet<QString> names;

  for( int i = 0; i < wpList.size(); i++ )
    {
      // Store all used names of the waypoint list in a set.
      names.insert( wpList.at(i)->name );
    }

  int lineNo = 0;

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  while (!in.atEnd())
    {
      QString line = in.readLine().trimmed();

      lineNo++;

      if( line.size() == 0 || line.startsWith("*") )
        {
          // Filter out empty and comment lines
          continue;
        }

      bool ok;
      QStringList list = line.split( "," );

      /*
      Example turnpoints, two possible coordinate formats seems to be in use.
      0 ,1         ,2          ,3   ,4,5           ,6
      31,57:04.213N,002:47.239W,450F,T,AB1 AboynBrg,RdBroverRDee

      0,1        ,2         ,3  ,4  ,5           ,6
      1,52:08:39N,012:40:06E,66M,HAS,SP1 LUESSE  ,EDOJ
      */

      // Lines defining a turnpoint contain 7 fields, separated by commas.
      // The final field is terminated by the newline. Field 7 is optional.
      if( list.size() < 6 )
        {
          qWarning() << "Line" << lineNo
                     << "is ignored, contains too less elements!";
          continue;
        }

      Waypoint *w = new Waypoint;

      w->importance = 0; // low
      w->country = _settings.value( "/Homesite/Country", "" ).toString();

      if( list[1].size() < 9 )
        {
          qWarning("DAT Read (%d): Format error latitude", lineNo);
          continue;
        }

      // latitude as 57:04.213N|S or 52:08:39N|S
      double degree = list[1].left(2).toDouble(&ok);

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error latitude degree", lineNo);
          delete w;
          continue;
        }

      double minutes = 0.0;
      double seconds = 0.0;

      if( list[1][5] == QChar('.') )
        {
          minutes = list[1].mid(3, 6).toDouble(&ok);
        }
      else if( list[1][5] == QChar(':') )
        {
          minutes = list[1].mid(3, 2).toDouble(&ok);

          if( ok )
            {
              seconds = list[1].mid(6, 2).toDouble(&ok);
            }
        }

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error latitude minutes/seconds", lineNo);
          delete w;
          continue;
        }

      double latTmp = (degree * 600000.) + (10000. * (minutes + seconds / 60. ));

      if( list[1].right(1).toUpper() == "S" )
        {
          latTmp = -latTmp;
        }

      // longitude as 002:47.239E|W or 012:40:06E|W
      if( list[2].size() < 10 )
        {
          qWarning("DAT Read (%d): Format error longitude", lineNo);
          continue;
        }

      degree = list[2].left(3).toDouble(&ok);

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error longitude degree", lineNo);
          delete w;
          continue;
        }

      minutes = 0.0;
      seconds = 0.0;

      if( list[2][6] == QChar('.') )
        {
          minutes = list[2].mid(4, 6).toDouble(&ok);
        }
      else if( list[2][6] == QChar(':') )
        {
          minutes = list[2].mid(4, 2).toDouble(&ok);

          if( ok )
            {
              seconds = list[2].mid(7, 2).toDouble(&ok);
            }
        }

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error longitude minutes/seconds", lineNo);
          delete w;
          continue;
        }

      double lonTmp = (degree * 600000.) + (10000. * (minutes + seconds / 60. ));


      if( list[2].right(1).toUpper() == "W" )
        {
          lonTmp = -lonTmp;
        }

      w->origP.setLat((int) rint(latTmp));
      w->origP.setLon((int) rint(lonTmp));

      // Height AMSL 9{1,5}[FM] 9=height, F=feet, M=metres.
      // two units are possible:
      // o meter: m
      // o feet:  ft
      if( list[3].size() ) // elevation in meter or feet
        {
          QString unit = list[3].right(1).toUpper();

          if( unit != "F" && unit != "M" )
            {
              qWarning("DAT Read (%d): Error reading elevation unit '%s'.",
                       lineNo, list[3].toLatin1().data());
              delete w;
              continue;
            }

          float tmpElev = list[3].left(list[3].size() - 1).toFloat(&ok);

          if( ! ok )
            {
              qWarning("DAT Read (%d): Error reading elevation value '%s'.",
                        lineNo,
                        list[3].left(list[3].size() - 1).toLatin1().data());
              delete w;
              continue;
            }

          if( unit == "M" )
            {
              w->elevation = tmpElev;
            }
          else if( unit == "F" )
            {
              // Convert feet to meters
              w->elevation = tmpElev * 0.3048;
            }
          else
            {
              qWarning("DAT Read (%d): Unknown elevation value '%s'.", lineNo,
                       unit.toLatin1().data());
              delete w;
              continue;
            }
        }

      /*
      Turnpoint attributes

      Cambridge documentation defines the following:
      Code    Meaning
      A       Airfield (not necessarily landable). All turnpoints marked 'A' in the UK are landable.
      L       Landable Point. Not necessarily an airfield.
      S       Start Point
      F       Finish Point
      H       Home Point
      M       Markpoint
      R       Restricted Point
      T       Turnpoint
      W       Waypoint
      */

      if( list[4].isEmpty() )
        {
          qWarning("DAT Read (%d): Missing turnpoint attributes", lineNo );
          delete w;
          continue;
        }

      // That is the default
      w->type = BaseMapElement::Landmark;

      list[4] = list[4].toUpper();

      if( list[4].contains("T") )
        {
          w->type = BaseMapElement::Turnpoint;
        }

      if( list[4].contains("A") )
        {
          w->type = BaseMapElement::Airfield;
        }

      if( list[4].contains("L") )
        {
          // w->isLandable = true;
        }

      if( list[5].isEmpty() )
        {
          qWarning("DAT Read (%d): Missing turnpoint name", lineNo );
          delete w;
          continue;
        }

      // Short name of a waypoint has only 8 characters and upper cases in KFLog.
      // That is handled in another way by Cambridge.
      w->name = list[5].left(8).toUpper().trimmed();
      w->description = list[5].trimmed();

      if( list.size() >= 7 )
        {
          QString comment = list[6].trimmed();

          if( ! comment.isEmpty() )
            {
              // A description is optional by Cambridge.
              w->comment += comment;
            }
        }

#if 0
      if( ! w->comment.isEmpty() )
        {
          w->comment.append("; ");
        }

      w->comment.append( "TP attributes: ").append(list[4]);
      w->comment.append( "; WP-No: ").append(list[0]);
#endif

      // We do check, if the waypoint name is already in use because DAT
      // short names are not always unique.
      if( names.contains( w->name ) )
        {
          for( int i = 0; i < 100; i++ )
            {
              // Hope that not more as 100 same names will be exist.
              QString number = QString::number(i);
               w->name = w->name.left(w->name.size() - number.size()) + number;

              if( names.contains( w->name ) == false )
                {
                  break;
                }
            }
        }

      if( !insertWaypoint(w) )
        {
          qWarning("DAT Read (%d): Error inserting waypoint in catalog", lineNo);
          break;
        }

      // Store used waypoint name in set.
      names.insert( w->name );
    }

  file.close();
  QApplication::restoreOverrideCursor();

  onDisc = true;
  path = catalog;
  return true;
}

/** Writes a Cambridge Aero Instruments turnpoint file. */
bool WaypointCatalog::writeDat(const QString& catalog)
{
  qDebug ("WaypointCatalog::writeDat (%s)", catalog.toLatin1().data());

  QFile file(catalog);

  if( ! file.open(QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString ("<html><B>%1</B><BR>").arg(file.fileName()) +
                             QObject::tr("permission denied!") +
                             "</html>", QMessageBox::Ok );
      return false;

    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QTextStream out (&file);
  out.setCodec( "ISO 8859-15" );

  char *user = getLogin();

  out << "*"
      << Qt::endl
      << "* Winpilot/CAI File generated by KFLog "
      << KFLOG_VERSION
      << " at "
      << QDate::currentDate().toString( Qt::ISODate )
      << " "
      << QTime::currentTime().toString( "HH:mm:mm" );

  if( user )
    {
      out << ", Creator is "  << getLogin();
    }

   out << Qt::endl
       << "*"
       << Qt::endl;

  for( int i = 0; i < wpList.size(); i++ )
  {
    int degree;
    double minutes;

    WGSPoint::calcPos( wpList[i]->origP.lat(), degree, minutes );

    out << i+1 << ","
        << QString("%1").arg( abs(degree), 2, 10, QChar('0') )
        << ":";

    QString min = QString("%1").arg( fabs(minutes), 0, 'f', 3, QChar('0') );

    if( fabs(minutes) < 10.0 )
      {
        // add missing leading zero
        min.insert(0, "0");
      }

    out << min
        << ( (degree >= 0) ? "N" : "S" )
        << ",";

    WGSPoint::calcPos( wpList[i]->origP.lon(), degree, minutes );

    out << QString("%1").arg( abs(degree), 3, 10, QChar('0') )
        << ":";

    min = QString("%1").arg( fabs(minutes), 0, 'f', 3, QChar('0') );

    if( fabs(minutes) < 10.0 )
      {
        // add missing leading zero
        min.insert(0, "0");
      }

    out << min
        << ( (degree >= 0) ? "E" : "W" )
        << ",";

    float elevation = wpList[i]->elevation;

    if( Altitude::getUnit() == Altitude::feet )
      {
        elevation = Altitude(elevation).getFeet();
        out << QString("%1").arg(elevation, 0, 'f', 0) << "F,";
      }
    else
      {
        out << QString("%1").arg(elevation, 0, 'f', 0) << "M,";
      }

#if 0
    if( wpList[i]->isLandable )
      {
        out << "L";
      }
#endif

    if(  wpList[i]->type == BaseMapElement::Airfield )
      {
        out << "A";
      }
    else if( wpList[i]->type == BaseMapElement::Turnpoint )
      {
        out << "T";
      }
    else
      {
        // All other is handled as waypoint here because KFLog has no waypoint
        // attributes.
        out << "W";
      }

    out << ","
        << wpList[i]->description.left(12)
        << ","
        << wpList[i]->comment.left(wpList[i]->comment.indexOf(QChar(';'))).replace( QChar(','), QChar('/'))
        << Qt::endl;
  }

  file.close();
  QApplication::restoreOverrideCursor();
  return true;
}

/** Writes a SeeYou cup file, only waypoint part */
bool WaypointCatalog::writeCup(const QString& catalog)
{
  qDebug() << "WaypointCatalog::writeCup:" << catalog;

  QFile file(catalog);

  if( ! file.open(QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString ("<html><B>%1</B><BR>").arg(file.fileName()) +
                             QObject::tr("permission denied!") +
                             "</html>", QMessageBox::Ok );
      return false;

    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QTextStream out (&file);
  out.setCodec( "ISO 8859-15" );

  // A cup line consists of the following elements:
  //
  // name,code,country,lat,lon,elev,style,rwdir,rwlen,rwwidth,freq,desc
  //
  // Format desciption, see here:
  //
  // https://downloads.naviter.com/docs/SeeYou_CUP_file_format.pdf
  //
  // "Lesce","LJBL",SI,4621.379N,01410.467E,504.0m,5,144,1130.0m,,123.500,"Home Airfield"
  //
  // That's the first line in a cup waypoint file
  out << "name,code,country,lat,lon,elev,style,rwdir,rwlen,rwwidth,freq,desc\r\n";

  for( int i = 0; i < wpList.size(); i++ )
    {
      out << "\"" << wpList[i]->description << "\","
          << "\"" << wpList[i]->name << "\","
          << wpList[i]->country << ",";

      int degree;
      double minutes;

      WGSPoint::calcPos( wpList[i]->origP.lat(), degree, minutes );

      out << QString("%1").arg( abs(degree), 2, 10, QChar('0') );

      QString min = QString("%1").arg( fabs(minutes), 0, 'f', 3, QChar('0') );

      if( fabs(minutes) < 10.0 )
        {
          // add missing leading zero
          min.insert(0, "0");
        }

      out << min
          << ( (degree >= 0) ? "N" : "S" )
          << ",";

      WGSPoint::calcPos( wpList[i]->origP.lon(), degree, minutes );

      out << QString("%1").arg( abs(degree), 3, 10, QChar('0') );

      min = QString("%1").arg( fabs(minutes), 0, 'f', 3, QChar('0') );

      if( fabs(minutes) < 10.0 )
        {
          // add missing leading zero
          min.insert(0, "0");
        }

      out << min
          << ( (degree >= 0) ? "E" : "W" )
          << ",";

      // elevation is always in meters
      float elevation = wpList[i]->elevation;

      if( Altitude::getUnit() == Altitude::feet )
        {
          elevation = Altitude(elevation).getFeet();
          out << QString("%1ft,").arg(elevation, 0, 'f', 1);
        }
      else
        {
          out << QString("%1m,").arg(elevation, 0, 'f', 1);
        }

      uint wpType = BaseMapElement::Landmark;

      switch( wpList[i]->type )
        {
        case BaseMapElement::Landmark:
          wpType = 1;
          break;

        case BaseMapElement::UltraLight:
        case BaseMapElement::Airfield:
        case BaseMapElement::Airport:
        case BaseMapElement::IntAirport:
        case BaseMapElement::MilAirport:
        case BaseMapElement::CivMilAirport:
          {
            if( wpList[i]->getRunwayList().size() > 0 )
              {
                const Runway& rwy = wpList[i]->getRunwayList().at(0);

                if( rwy.getSurface() == Runway::Concrete )
                  {
                    wpType = 5;
                    break;
                  }

                if( rwy.getSurface() == Runway::Grass )
                  {
                    wpType = 2;
                    break;
                  }
              }

            // default assumption is airfield grass
            wpType = 2;
            break;
          }

        case BaseMapElement::Outlanding:
          wpType = 3;
          break;

        case BaseMapElement::Gliderfield:
          wpType = 4;
         break;

        case BaseMapElement::Vor:
        case BaseMapElement::VorDme:
        case BaseMapElement::VorTac:
          wpType = 9;
         break;

        case BaseMapElement::Ndb:
          wpType = 10;
         break;

        default:
          wpType = 1;
          break;
        }

      out << wpType << ",";

      if( wpList[i]->rwyList.size() > 0 )
        {
          Runway rwy = wpList[i]->rwyList[0];

          // heading as 000...360
          out << QString( "%1,").arg( rwy.getHeading(), 3, 10, QChar('0') );

          if( rwy.getLength() > 0 )
            {
              // Runway length in meters
              out << QString("%1m").arg( rwy.getLength(), 1 );
            }

          out << ",";

          if( rwy.getWidth() > 0 )
            {
              // Runway width in meters
              out << QString("%1m").arg( rwy.getWidth(), 1 );
            }

          out << ",";
        }
      else
        {
          // no runway data available
          out << ",,";
        }

      QList<Frequency>& fqList = wpList[i]->getFrequencyList();

      if( fqList.size() > 0.0 )
        {
          int i = 0;
          int fqlIdx = -1;

          // Only a main frequency should be shown in the display
          for( i = 0; i < fqList.size(); i++ )
            {
              quint8 type = fqList[i].getType();

              if( type == Frequency::Tower || type == Frequency::Info ||
                  type == Frequency::Information ||
                  fqList[i].isPrimary() == true )
                {
                  fqlIdx = i;
                  break;
                }
            }

          if( fqlIdx == -1  )
            {
              // No assignment has been done, take first index.
              fqlIdx = 0;
            }

          out << "\""
              << QString("%1").arg( fqList[i].getValue(), 0, 'f', 3, QChar('0') )
              << "\"";
        }

      out << ",";

      if( ! wpList[i]->comment.isEmpty() )
        {
          out << "\""
              << wpList[i]->comment
              << "\"";
        }

      out << "\r\n";
    }

  out << "-----Related Tasks-----" << "\r\n";

  file.close();
  QApplication::restoreOverrideCursor();
  return true;
}
