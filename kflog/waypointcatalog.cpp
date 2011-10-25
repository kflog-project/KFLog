/***********************************************************************
**
**   waypointcatalog.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifdef QT_4.6
#define DATA_STREAM QDataStream::Qt_4_6
#else
#define DATA_STREAM QDataStream::Qt_4_7
#endif

#include <cmath>
#include <unistd.h>

#include <QtGui>
#include <QtXml>

#include "altitude.h"
#include "runway.h"
#include "da4record.h"
#include "kfrgcs/vlapi2.h"
#include "mainwindow.h"
#include "mapdefaults.h"
#include "target.h"
#include "waypointcatalog.h"

#define KFLOG_FILE_MAGIC    0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50
#define FILE_FORMAT_ID      100
#define FILE_FORMAT_ID_2    101
#define FILE_FORMAT_ID_3    103 // waypoint list size added

// Center point definition, also used by waypoint import filter.
#define CENTER_POS      0
#define CENTER_HOMESITE 1
#define CENTER_MAP      2
#define CENTER_AIRFIELD 3

extern MainWindow *_mainWindow;
extern QSettings  _settings;

QSet<QString> WaypointCatalog::catalogSet;

WaypointCatalog::WaypointCatalog(const QString& name) :
  modified(false),
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
  showOtherSites = false;
  showObstacles = false;
  showLandmarks = false;
  showOutlandings = false;
  showStations = false;

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
          w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
          w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
          w->isLandable = nm.namedItem("Landable").toAttr().value().toInt();
          w->runway.first = nm.namedItem("Runway").toAttr().value().toUShort();

          if( w->runway.first > 36 )
            {
              w->runway.first = 0;
            }

          if( w->runway.first > 0 )
            {
              w->runway.second = w->runway.first <= 18 ? w->runway.first + 18 : w->runway.first - 18;
            }
          else
            {
              // No runways defined
              w->runway.second = 0;
            }

          w->length = nm.namedItem("Length").toAttr().value().toInt();
          w->surface = (enum Runway::SurfaceType)nm.namedItem("Surface").toAttr().value().toInt();
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
  root.setAttribute( "Creator", getlogin() );
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
    child.setAttribute("Frequency", w->frequency);
    child.setAttribute("Landable", w->isLandable);
    child.setAttribute("Runway", w->runway.first);
    child.setAttribute("Length", w->length);
    child.setAttribute("Surface", w->surface);
    child.setAttribute("Comment", w->comment);
    child.setAttribute("Importance", w->importance);
    child.setAttribute( "Country", w->country );

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
  float wpFrequency;
  qint8 wpLandable;
  qint16 wpRunway;
  float wpLength;
  qint8 wpSurface;
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

      //write file header
      out << quint32( KFLOG_FILE_MAGIC );
      out << qint8( FILE_TYPE_WAYPOINTS );
      out << quint16( FILE_FORMAT_ID_3 );
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
          wpFrequency = w->frequency;
          wpLandable = w->isLandable;
          wpRunway = (w->runway.first * 256) + w->runway.second;
          wpLength = w->length;
          wpSurface = w->surface;
          wpComment = w->comment;
          wpImportance = w->importance;

          out << wpName;
          out << wpDescription;
          out << wpICAO;
          out << wpType;
          out << wpLatitude;
          out << wpLongitude;
          out << wpElevation;
          out << wpFrequency;
          out << wpLandable;
          out << wpRunway;
          out << wpLength;
          out << wpSurface;
          out << wpComment;
          out << wpImportance;
          out << w->country;
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
bool WaypointCatalog::importVolkslogger(const QString& filename)
{
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
  int lineCount;
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
                  w->surface = Runway::Asphalt;
                }
              else
                {
                  w->surface = Runway::Grass;
                }

              if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_A)
                {
                  w->type = BaseMapElement::Airfield;
                }

              w->isLandable = true;
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
      filter.append(QObject::tr("KFLog waypoints") + " (*.kflogwp *.KFLOGWP);;");
      filter.append(QObject::tr("Cumulus waypoints") + " (*.kwp *.KWP);;");
      filter.append(QObject::tr("Filser txt waypoints") + " (*.txt *.TXT);;");
      filter.append(QObject::tr("Filser da4 waypoints") + " (*.da4 *.DA4);;");
      filter.append(QObject::tr("Cambrigde waypoints") + " (*.dat *.DAT);;");
      filter.append(QObject::tr("SeeYou waypoints") + " (*.cup *.CUP);;");
      filter.append(QObject::tr("All") + " (*.*)");

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
  else
    {
      fName += ".kflogwp";
      return writeBinary();
    }
}

/**
 * This function calls either read or readBinary depending on the filename
 * of the catalog.
 */
bool WaypointCatalog::load(const QString& catalog)
{
  if (catalog.right(8).toLower() == ".kflogwp")
    return readXml(catalog);
  else if (catalog.right(12).toLower() == "welt2000.txt")
    return readWelt2000(catalog);
  else if (catalog.right(4).toLower() == ".txt")
    return readFilserTXT(catalog);
  else if (catalog.right(4).toLower() == ".da4")
    return readFilserDA4(catalog);
  else if (catalog.right(4).toLower() == ".cup")
    return readCup(catalog);
  else if (catalog.right(4).toLower() == ".dat")
    return readDat(catalog);
  else
    return readBinary(catalog);
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
                  w->name = list [1];
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
                  w->origP.setLat((int)(list[3].toDouble() * 600000.0));
                  w->origP.setLon((int)(list[4].toDouble() * 600000.0));
                  w->elevation = (int)(list[5].toInt() * 0.3048); // don't we have conversion constants ?
                  w->frequency = list[6].toDouble() / 1000.0;
                  w->isLandable = false;
                  w->length = list[7].toInt(); // length ?!
                  w->runway.first = list[8].toInt(); // direction ?!
                  QChar surface = list[9].toUpper()[0];
                  switch (surface.toAscii())
                    {
                    case 'G':
                      w->surface = Runway::Grass;
                      break;
                    case 'C':
                      w->surface = Runway::Concrete;
                      break;
                    default:
                      w->surface = Runway::Unknown;
                      break;
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
      out << "*,TpName,Type,Latitude,Longitude,Altitude,Frequency,RWY,RWYdir,RWYtype,TCA,TC" << endl;

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
        out << (int)(w->frequency*1000) << ",";
        out << w->length << ",";
        out << w->runway.first << ",";

        switch (w->surface)
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

        out << "3,I,,," << endl;
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
          // read values from file
          in >> wpName;
          in >> wpDescription;
          in >> wpICAO;
          in >> wpType;
          in >> wpLatitude;
          in >> wpLongitude;

          if( fileFormat < FILE_FORMAT_ID_3 )
            {
              in >> wpElevation;
              in >> wpFrequency;
            }
          else
            {
              in >> wpElevation3;
              in >> wpFrequency3;
            }

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
          in >> wpComment;
          in >> wpImportance;

          if( fileFormat >= FILE_FORMAT_ID_3 )
            {
              in >> wpCountry;
            }

          //create new waypoint object and set the correct properties
          Waypoint *w = new Waypoint;

          w->name = wpName.left(8).toUpper();
          w->description = wpDescription;
          w->icao = wpICAO;
          w->type = wpType;
          w->origP.setLat(wpLatitude);
          w->origP.setLon(wpLongitude);
          w->isLandable = wpLandable;
          w->runway.first = wpRunway/256;
          w->runway.second = wpRunway & 256;
          w->surface = (enum Runway::SurfaceType) wpSurface;
          w->comment = wpComment;
          w->importance = wpImportance;
          w->country = wpCountry;

          if( fileFormat < FILE_FORMAT_ID_3 )
            {
              w->elevation = wpElevation;
              w->frequency = wpFrequency;
              w->length = wpLength;
            }
          else
            {
              w->elevation = wpElevation3;
              w->frequency = wpFrequency3;
              w->length = wpLength3;
            }

          //qDebug("Waypoint read: %s (%s - %s) offset %d-%d",w->name.toLatin1().data(),w->description.toLatin1().data(),w->icao.toLatin1().data(), startoffset, f.at());

          if (!insertWaypoint(w))
            {
              qDebug("odd... error reading waypoints");
              delete w;
              break;
            }
        }

      onDisc = true;
      path = catalog;
      ok = true;

      if( fileFormat < FILE_FORMAT_ID_3 )
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

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  while (!in.atEnd())
    {
      QString line = in.readLine();

      lineNo++;

      if( line.size() == 0 )
        {
          continue;
        }

      bool ok;

      QList<QString> list = splitCupLine( line, ok );

      if( list[0] == "-----Related Tasks-----" )
        {
          // Task part starts, we will ignore it and break up reading
          break;
        }

      // 10 elements are mandatory, element 11 description is optional
      if( list.count() < 10 ||
          list[0].toLower() == "name" ||
          list[1].toLower() == "code" ||
          list[2].toLower() == "country" )
        {
          // too less elements or a description line, ignore this
          continue;
        }

      // A cup line consists of the following elements:
      //
      // Name,Code,Country,Latitude,Longitude,Elevation,Style,Direction,Length,Frequency,Description
      //
      // See here for more info: http://download.naviter.com/docs/cup_format.pdf
      Waypoint *w = new Waypoint;

      w->isLandable = false;
      w->importance = 0;

      if( list[0].length() ) // long name of waypoint
        {
          w->description = list[0].replace( QRegExp("\""), "" );
        }
      else
        {
          w->description = "";
        }

      if( list[1].isEmpty() )
        {
          // If no code is set, we assign the long name as code to have a workaround.
          list[1] = list[0];
        }

      // short name of a waypoint has only 8 characters and upper cases
      w->name = list[1].replace( QRegExp("\""), "" ).left(8).toUpper();
      w->country = list[2].left(2).toUpper();
      w->icao = "";
      w->surface = Runway::Unknown;

      // waypoint type
      uint wpType = list[6].toUInt(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Invalid waypoint type '%s'. Ignoring it.",
                   lineNo, list[6].toAscii().data() );
          delete w;
          continue;
        }

      switch( wpType )
        {
        case 1:
          w->type = BaseMapElement::Landmark;
          break;
        case 2:
          w->type = BaseMapElement::Airfield;
          w->surface = Runway::Grass;
          w->isLandable = true;
          w->importance = 1;
          break;
        case 3:
          w->type = BaseMapElement::Outlanding;
          w->isLandable = true;
          w->importance = 1;
          break;
        case 4:
          w->type = BaseMapElement::Gliderfield;
          w->isLandable = true;
          w->importance = 1;
          break;
        case 5:
          w->type = BaseMapElement::Airfield;
          w->surface = Runway::Concrete;
          w->isLandable = true;
          w->importance = 1;
          break;
        case 9:
          w->type = BaseMapElement::Ndb;
          break;
        case 10:
          w->type = BaseMapElement::Vor;
          break;
        default:
          w->type = BaseMapElement::Landmark;
          break;
        }

      // latitude as ddmm.mmm(N|S)
      double degree = list[3].left(2).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (N/S) (1)", lineNo);
          delete w;
          continue;
        }

      double minutes = list[3].mid(2,6).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (N/S) (2)", lineNo);
          delete w;
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
          delete w;
          continue;
        }

      minutes = list[4].mid(3,6).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (E/W) (2)", lineNo);
          delete w;
          continue;
        }

      double lonTmp = (degree * 600000.) + (minutes * 10000.0);


      if( list[4].right(1).toUpper() == "W" )
        {
          lonTmp = -lonTmp;
        }

      w->origP.setLat((int) rint(latTmp));
      w->origP.setLon((int) rint(lonTmp));

      // two units are possible:
      // o meter: m
      // o feet:  ft
      if( list[5].length() ) // elevation in meter or feet
        {
          QString unit;
          int uStart = list[5].indexOf( QRegExp("[mf]") );

          if( uStart == -1 )
            {
              qWarning("CUP Read (%d): Error reading elevation unit '%s'.", lineNo,
                       list[5].toLatin1().data());
              delete w;
              continue;
            }

          unit = list[5].mid( uStart ).toLower();

          float tmpElev = (list[5].left(list[5].length() - unit.length())).toFloat(&ok);

          if( ! ok )
            {
              qWarning("CUP Read (%d): Error reading elevation value '%s'.", lineNo,
                       list[5].left(list[5].length() - unit.length()).toLatin1().data());
              delete w;
              continue;
            }

          if( unit == "m" )
            {
              w->elevation = tmpElev;
            }
          else if( unit == "ft" )
            {
              w->elevation = tmpElev * 0.3048;
            }
          else
            {
              qWarning("CUP Read (%d): Unknown elevation value '%s'.", lineNo,
                       unit.toLatin1().data());
              delete w;
              continue;
            }
        }

      if( list[9].trimmed().length() ) // airport frequency
        {
          float frequency = list[9].replace( QRegExp("\""), "" ).toFloat(&ok);

          if( ok )
            {
              w->frequency = frequency;
            }
          else
            {
              w->frequency = 0.0;
            }
        }

      if( list[7].trimmed().length() ) // runway direction 010...360
        {
          uint rdir = list[7].toInt(&ok);

          if( ok )
            {
              w->runway.first = rdir/10;
              w->runway.second = w->runway.first <= 18 ? w->runway.first + 18 : w->runway.first - 18;
            }
        }

      if( list[8].trimmed().length() ) // runway length in meters
        {
          // three units are possible:
          // o meter: m
          // o nautical mile: nm
          // o statute mile: ml
          QString unit;
          int uStart = list[8].indexOf( QRegExp("[lmn]") );

          if( uStart != -1 )
            {
              unit = list[8].mid( uStart ).toLower();
              float length = list[8].left( list[8].length()-unit.length() ).toFloat(&ok);

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

                  w->length = length;
                }
            }
        }

      if( list.count() == 11 && list[10].trimmed().length() ) // description, optional
        {
          w->comment += list[10].replace( QRegExp("\""), "" );
        }

      // We do check, if the waypoint name is already in use because cup
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
          qWarning("CUP Read (%d): Error inserting waypoint in catalog", lineNo);
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

/** creates a waypoint catalog from a Welt2000 file. */
bool WaypointCatalog::readWelt2000(const QString& catalog)
{
  qDebug() << "WaypointCatalog::readWelt2000" << catalog;

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

  bool ok;

  QString lastInput = _settings.value( "/Waypoints/Welt2000Filter", "").toString();

  QString cFilter = QInputDialog::getText( _mainWindow,
                                           QObject::tr("Enter countries to be read"),
                                           QObject::tr("Two letter Country codes:"),
                                           QLineEdit::Normal,
                                           lastInput,
                                           &ok );
  if( ! ok || cFilter.isEmpty() )
    {
      return false;
    }

  QStringList clist = cFilter.split( QRegExp("[,; ]"), QString::SkipEmptyParts );

  QStringList countryList;

  for( int i = 0; i < clist.count(); i++ )
    {
      QString e = clist[i].trimmed().toUpper();

      if( e.length() != 2 || countryList.contains( e ) )
        {
          continue;
        }

      countryList += e;
    }

  countryList.sort();

   if( countryList.size() == 0 )
    {
       QMessageBox::warning( _mainWindow,
                             QObject::tr("Error occurred!"),
                             "<html>" + QObject::tr("Your country entries were wrong! "
                             "Two letter codes are only allowed. Use spaces to separate "
                             "them from each other.") + "</html>",
                             QMessageBox::Ok );
       return false;
    }

  // Save the last user filter input
  _settings.setValue( "/Waypoints/Welt2000Filter", cFilter );

  // put all entries of country list into a dictionary for faster access
  QSet<QString> countryDict;

  for( int i = 0; i < countryList.count(); i++ )
    {
      // populate country dictionary
      countryDict.insert( countryList[i] );
    }

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  uint lineNo = 0;

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

      // Extract country sign. It is coded according to ISO 3166.
      QString country = line.mid( 60, 2 ).toUpper();

      if( ! countryDict.isEmpty() )
        {
          if( ! countryDict.contains(country) )
            {
              continue;
            }
        }

      Waypoint *wp = new Waypoint;

      wp->isLandable = false;
      wp->importance = 0;
      wp->country = country;
      wp->name = line.mid( 0, 6 ).trimmed().toUpper();

      // convert all to toUpper case
      line = line.toUpper();

      // look, what kind of line was read.
      // COL5 = 1 Airfield or also UL site
      // COL5 = 2 Outlanding, contains also UL places
      QString kind = line.mid( 5, 1 );

      bool ulField = false;
      bool glField = false;
      bool afField = false;
      bool olField = false;

      QString commentShort;
      QString commentLong;

      if( kind == "2" ) // can be an UL field or an outlanding
        {
          if( line.mid( 23, 4 ) == "*ULM" )
            {
              ulField = true;
            }
          else
            {
              olField = true;
              commentShort = line.mid( 24, 4 );
              commentShort.replace(QRegExp("[!?]+"), " " );
              commentShort = commentShort.toUpper().trimmed();

              if( commentShort.startsWith( "FL" ) )
                {
                  commentLong = QString( QObject::tr("Emergency Field No: ")) +
                                commentShort.mid( 2, 2 );
                  wp->comment = commentLong;
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
      else if( kind == "1" )
        {
          afField = true;
          wp->icao = line.mid( 24, 4 ).trimmed().toUpper();

          if( line.mid( 20, 4 ) == "GLD#" )
            {
              // other possibility for a glider field with ICAO code
              glField = true;
            }
        }

      // waypoint name long
      QString wpName = line.mid( 7, 16 );

      // remove special mark signs
      wpName.replace( QRegExp("[!?]+"), "" );

      // remove resp. replace white spaces against one space
      wpName = wpName.simplified();

      if( wpName.length() == 0 )
        {
          continue;
        }

      // waypoint type, landmark is the default.
      BaseMapElement::objectType afType = BaseMapElement::Landmark;

      // determine waypoint type so good as possible
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
          if( wp->icao.startsWith("ET") )
            {
              // German military airport
              afType = BaseMapElement::MilAirport;
            }
          else if( wpName.contains(QRegExp(" MIL$")) )
            {
              // should be an military airport but not 100% sure
              afType = BaseMapElement::MilAirport;
            }
          else if( wp->icao.startsWith("EDD") )
            {
              // German international airport
              afType = BaseMapElement::IntAirport;
            }
          else
            {
              afType = BaseMapElement::Airfield;
            }
        }

      if( afType != BaseMapElement::Landmark )
        {
          // That is an airfield or an outlanding
          wpName = wpName.toLower();

          QChar lastChar(' ');

          // convert airfield names to upper-lower
          for( int i=0; i < wpName.length(); i++ )
            {
              if( lastChar == ' ' )
                {
                  wpName.replace( i, 1, wpName.mid(i,1).toUpper() );
                }

              lastChar = wpName[i];
            }

          if( ulField  )
            {
              if( wpName.right(3) == " Ul" )
                {
                  // Convert lower l of Ul to upper case
                  wpName.replace( wpName.length()-1, 1, "L" );
                }
            }
        }
      else
        {
          // That is a real waypoint. Its name is longer.
          wpName = line.mid( 7, 34 );

          // remove special mark signs
          wpName.replace( QRegExp("[!?]+"), "" );

          // remove resp. replace white spaces against one space
          wpName = wpName.simplified();
        }

      wp->description = wpName;
      wp->type = afType;

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
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude minute value, ignoring entry!",
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong latitude second value, ignoring entry!",
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
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
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      m = min.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude minute value, ignoring entry!",
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      s = sec.toDouble(&ok);

      if( ! ok )
        {
          qWarning( "W2000, Line %d: %s (%s) wrong longitude second value, ignoring entry!",
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
          continue;
        }

      double lonTmp = (d * 600000.) + (10000. * (m + s / 60. ));

      lon = (qint32) rint(lonTmp);

      if( line[52] == 'W' )
        {
          lon = -lon;
        }

      wp->origP = WGSPoint(lat, lon);

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
                    lineNo, wpName.toLatin1().data(), country.toLatin1().data() );
          elevation = 0;
        }

      wp->elevation = elevation;

      if( afType != BaseMapElement::Landmark )
        {
          // frequency
          QString frequency = line.mid(36,3) + "." + line.mid(39,2).trimmed();

          float fFrequency = frequency.toFloat(&ok);

          if( ( !ok || fFrequency < 108 || fFrequency > 137.0 ) )
            {
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

          wp->frequency = fFrequency;

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
            }
          else
            {
              if( rwDir1 == rwDir2 || abs( rwDir1 - rwDir2 ) == 18 )
                {
                  // We have only one runway
                  rwDirections[0].first  = rwDir1;
                  rwDirections[0].second = rwDir2;
                }
              else
                {
                  // WE have two runways
                  rwDirections[0].first  = rwDir1;
                  rwDirections[0].second = ((rwDir1 > 18) ? rwDir1 - 18 : rwDir1 + 18 );
                  rwDirections[1].first  = rwDir2;
                  rwDirections[1].second = ((rwDir2 > 18) ? rwDir2 - 18 : rwDir2 + 18 );
                }
            }

          wp->runway = rwDirections[0];

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
              rwLen = 0;
            }
          else
            {
              rwLen *= 10;
            }

          wp->length = rwLen;

          // runway surface
          QChar rwType = line[28];

          if( rwType == 'A' )
            {
              wp->surface = Runway::Asphalt;
            }
          else if( rwType == 'C' )
            {
              wp->surface = Runway::Concrete;
            }
          else if( rwType == 'G' )
            {
              wp->surface = Runway::Grass;
            }
          else if( rwType == 'S' )
            {
              wp->surface = Runway::Sand;
            }
          else
            {
              wp->surface = Runway::Unknown;
            }
        }

      if( !insertWaypoint(wp) )
        {
          qWarning("Welt2000 Read (%d): Error inserting waypoint in catalog", lineNo);
          break;
        }
    } // End of while( ! in.atEnd() )

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
      // qDebug() << "FoundWP" << existingWaypoint->name;

      if( existingWaypoint->name != newWaypoint->name ||
          existingWaypoint->angle != newWaypoint->angle ||
          existingWaypoint->comment != newWaypoint->comment ||
          existingWaypoint->description != newWaypoint->description ||
          existingWaypoint->distance != newWaypoint->distance ||
          existingWaypoint->elevation != newWaypoint->elevation ||
          existingWaypoint->fixTime != newWaypoint->fixTime ||
          existingWaypoint->frequency != newWaypoint->frequency ||
          existingWaypoint->icao != newWaypoint->icao ||
          existingWaypoint->importance != newWaypoint->importance ||
          existingWaypoint->isLandable != newWaypoint->isLandable ||
          existingWaypoint->length != newWaypoint->length ||
          existingWaypoint->origP != newWaypoint->origP ||
          existingWaypoint->runway != newWaypoint->runway ||
          existingWaypoint->type != newWaypoint->type ||
          existingWaypoint->surface != newWaypoint->surface ||
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
          w->isLandable = true;
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

  char *user = getlogin();

  out << "*"
      << endl
      << "* Winpilot/CAI File generated by KFLog "
      << KFLOG_VERSION
      << " at "
      << QDate::currentDate().toString( Qt::ISODate )
      << " "
      << QTime::currentTime().toString( "HH:mm:mm" );

  if( user )
    {
      out << ", Creator is "  << getlogin();
    }

   out << endl
       << "*"
       << endl;

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

    if( wpList[i]->isLandable )
      {
        out << "L";
      }

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
        << endl;
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
  // Name,Code,Country,Latitude,Longitude,Elevation,Style,Direction,Length,Frequency,Description
  //
  // See here for more info: http://download.naviter.com/docs/cup_format.pdf
  //
  // "Aachen Merzbruc",AACHE,DE,5049.383N,00611.183E,189.0m,5,80,530.0m,"122.875",

  // That's the first line in a cup waypoint file
  out << "name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc"
      << endl;

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

      float elevation = wpList[i]->elevation;

      if( Altitude::getUnit() == Altitude::feet )
        {
          elevation = Altitude(elevation).getFeet();
          out << QString("%1").arg(elevation, 0, 'f', 1) << "ft,";
        }
      else
        {
          out << QString("%1").arg(elevation, 0, 'f', 1) << "m,";
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
            if( wpList[i]->surface == Runway::Concrete )
              {
                wpType = 5;
                break;
              }

            if( wpList[i]->surface == Runway::Grass )
              {
                wpType = 2;
                break;
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

      if( wpList[i]->runway.first > 0 )
        {
          out << (wpList[i]->runway.first * 10);
        }

      out << ",";

      if( wpList[i]->length > 0 )
        {
          out << QString("%1m").arg( wpList[i]->length, 1 );
        }

      out << ",";

      if( wpList[i]->frequency > 0.0 )
        {
          out << "\""
              << QString("%1").arg( wpList[i]->frequency, 0, 'f', 3, QChar('0') )
              << "\"";
        }

      out << ",";

      if( ! wpList[i]->comment.isEmpty() )
        {
          out << "\""
              << wpList[i]->comment
              << "\"";
        }

      out << endl;
    }

  out << "-----Related Tasks-----" << endl;

  file.close();
  QApplication::restoreOverrideCursor();
  return true;
}
