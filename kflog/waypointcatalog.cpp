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

#include <cmath>

#include <QtGui>
#include <QtXml>

#include "runway.h"
#include "da4record.h"
#include "kfrgcs/vlapi2.h"
#include "mainwindow.h"
#include "waypointcatalog.h"

#define KFLOG_FILE_MAGIC    0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50
#define FILE_FORMAT_ID      100
#define FILE_FORMAT_ID_2    101

extern MainWindow *_mainWindow;

WaypointCatalog::WaypointCatalog(const QString& name) :
  modified(false),
  onDisc(false)
{
  static int catalogNr = 1;

  extern QSettings _settings;
  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();
  if( name == QString::null )
    {
      QString t;
      t.setNum( catalogNr++ );
      catalogName = QObject::tr( "unnamed" ) + t;
    }
  else
    {
      catalogName = name;
    }

  path = wayPointDir + "/" + catalogName + ".kflogwp";

  showAll = true;
  showAirfields = false;
  showGliderfields = false;
  showOtherSites = false;
  showObstacles = false;
  showLandmarks = false;
  showOutlandings = false;
  showStations = false;

  areaLat1 = areaLat2 = areaLong1 = areaLong2 = radiusLat = radiusLong = 0;

  // Default is 500 Km
  radiusSize = 500;

  // Center homesite
  centerRef = 1;

  // Reset airfield name reference.
  airfieldRef = "";
}

WaypointCatalog::~WaypointCatalog()
{
  qDeleteAll( wpList );
}

/** read a catalog from file */
bool WaypointCatalog::read(const QString& catalog)
{
  bool ok = false;
  QFile f(catalog);

  if( !f.exists() )
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString("<html><B>%1</B><BR>").arg(catalog) +
                             QObject::tr("not found!") +
                             "</html>",
                             QMessageBox::Ok );
      return false;
    }

  if( !f.open( QIODevice::ReadOnly ) )
    {
       QMessageBox::critical( _mainWindow,
                              QObject::tr("Error occurred!"),
                              QString("<html><B>%1</B><BR>").arg(catalog) +
                              QObject::tr("permission denied!") +
                              "</html>", QMessageBox::Ok );
       return false;
    }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QDomDocument doc;
  doc.setContent(&f);

  if (doc.doctype().name() == "KFLogWaypoint")
    {
      QDomNodeList nl = doc.elementsByTagName("Waypoint");

      for( int i = 0; i < nl.count(); i++ )
        {
          QDomNamedNodeMap nm =  nl.item(i).attributes();
          Waypoint *w = new Waypoint;

          w->name = nm.namedItem("Name").toAttr().value().left(6).toUpper();
          w->description = nm.namedItem("Description").toAttr().value();
          w->icao = nm.namedItem("ICAO").toAttr().value().toUpper();
          w->type = nm.namedItem("Type").toAttr().value().toInt();
          w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
          w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
          w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
          w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
          w->isLandable = nm.namedItem("Landable").toAttr().value().toInt();
          w->runway.first = nm.namedItem("Runway").toAttr().value().toInt();
          w->runway.second = w->runway.first <= 18 ? w->runway.first + 18 : w->runway.first - 18;
          w->length = nm.namedItem("Length").toAttr().value().toInt();
          w->surface = (enum Runway::SurfaceType)nm.namedItem("Surface").toAttr().value().toInt();
          w->comment = nm.namedItem("Comment").toAttr().value();
          w->importance = nm.namedItem("Importance").toAttr().value().toInt();

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

  f.close();
  QApplication::restoreOverrideCursor();

  return ok;
}

/** No descriptions */
bool WaypointCatalog::write()
{
  bool ok = true;
  QDomDocument doc("KFLogWaypoint");
  QDomElement root = doc.createElement("KFLogWaypoint");
  QDomElement child;
  Waypoint *w;
  QFile f;
  QString fName = path;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

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

      child.setAttribute("Name", w->name);
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

      root.appendChild(child);
    }

  f.setFileName(fName);

  if (f.open(QIODevice::WriteOnly)) {
    QString txt = doc.toString();
    f.write(txt.toAscii().data(), txt.length());
    f.close();
    path = fName;
    modified = false;
    onDisc = true;
  }
  else {
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
  qint16 wpElevation;
  double wpFrequency;
  qint8 wpLandable;
  qint16 wpRunway;
  qint16 wpLength;
  qint8 wpSurface;
  QString wpComment="";
  quint8 wpImportance;

  Waypoint *w;
  QFile f;
  QString fName = path;

  f.setFileName(fName);

  if (f.open(QIODevice::WriteOnly)) {
    QDataStream out(& f);

    //write fileheader
    out << quint32(KFLOG_FILE_MAGIC);
    out << qint8(FILE_TYPE_WAYPOINTS);
    out << quint16(FILE_FORMAT_ID_2); //use the new format with importance field.

    foreach(w, wpList) {
      wpName=w->name;
      wpDescription=w->description;
      wpICAO=w->icao;
      wpType=w->type;
      wpLatitude=w->origP.lat();
      wpLongitude=w->origP.lon();
      wpElevation=w->elevation;
      wpFrequency=w->frequency;
      wpLandable=w->isLandable;
      wpRunway=w->runway.first;
      wpLength=w->length;
      wpSurface=w->surface;
      wpComment=w->comment;
      wpImportance=w->importance;

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
    }
    f.close();
    path = fName;
    modified = false;
    onDisc = true;
  }
  else {
    QMessageBox::critical(_mainWindow, QObject::tr("Error occurred!"), QString ("<html><B>%1</B><BR>").arg(fName) + QObject::tr("permission denied!") + "</html>", QMessageBox::Ok );
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

      if (s.count() == 5){
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

        if (s[4].length() == 2) {
          flag = s[4][1];
        }
        else {
          flag = 0;
        }

        w->name = s[1].trimmed();
        //w->description = "<unknown>";
        //w->icao = "<unknown>";

        if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_L) {
          if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_H) {
            w->surface = Runway::Asphalt;
          }
          else {
            w->surface = Runway::Grass;
          }

          if (flag.digitValue() & VLAPI_DATA::WPT::WPTTYP_A) {
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

  // check for unsupported file types - currently cup

  if (fName.right(4).toLower() == ".cup")
    {
    QMessageBox saveBox(QObject::tr("Save changes?"),
                        QObject::tr("<html>Saving in the current file format is not supported.<br>Save in another format? <BR><B>%1</B></html>").arg(fName),
                        QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
    saveBox.setButtonText(QMessageBox::Yes, "Save");
    saveBox.setButtonText(QMessageBox::No, "Discard");

    if(saveBox.exec()==QMessageBox::Yes)
      alwaysAskName = true;
  }

  if (!onDisc || alwaysAskName)
    {
       QString filter;
       filter.append(QObject::tr("KFLog waypoints") + " (*.kflogwp *.KFLOGWP);;");
       filter.append(QObject::tr("Cumulus waypoints") + " (*.kwp *.KWP);;");
       filter.append(QObject::tr("Filser txt waypoints") + " (*.txt *.TXT);;");
       filter.append(QObject::tr("Filser da4 waypoints") + " (*.da4 *.DA4)");

       fName = QFileDialog::getSaveFileName( _mainWindow,
                                             QObject::tr("Save waypoint catalog"),
                                             path,
                                             filter );

    if(!fName.isEmpty())
      {
      if ((fName.right(8) != ".kflogwp") &&
          (fName.right(4) != ".kwp") &&
          (fName.right(4) != ".da4") &&
          (fName.right(4) != ".txt"))
      {
        fName += ".kflogwp";
      }
      path = fName;
    }
    else {
      return false;
    }
  }

  if (fName.right(8) == ".kflogwp")
    return write();
  else if (fName.right(4) == ".txt")
    return writeFilserTXT (fName);
  else if (fName.right(4) == ".da4")
    return writeFilserDA4 (fName);
  else
    return writeBinary();
}

/** This function calls either read or readBinary depending on the filename of the catalog. */
bool WaypointCatalog::load(const QString& catalog)
{
  if (catalog.right(8).toLower() == ".kflogwp")
    return read(catalog);
  else if (catalog.right(4).toLower() == ".txt")
    return readFilserTXT (catalog);
  else if (catalog.right(4).toLower() == ".da4")
    return readFilserDA4 (catalog);
  else if (catalog.right(4).toLower() == ".cup")
    return readCup (catalog);
  else
    return readBinary(catalog);
}

/** read a waypoint catalog from a filser txt file */
bool WaypointCatalog::readFilserTXT (const QString& catalog)
{
  qDebug("WaypointCatalog::readFilserTXT (%s)", catalog.toLatin1().data());

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

          if (list[0] == "*") // comment/header line
            {
              continue;
            }

          Waypoint *w = new Waypoint;
          w->name = list [1];
          w->description = "";
          w->icao = "";
//        why don't we have type "turnpoint" ?
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
            case 'G': w->surface = Runway::Grass;
                  break;
            case 'C': w->surface = Runway::Concrete;
                  break;
            default:  w->surface = Runway::Unknown;
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
    QTextStream out (&f);
    out << "*,TpName,Type,Latitiude,Longitude,Altitude,Frequency,RWY,RWYdir,RWYtype,TCA,TC" << endl;

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
      }
      out << "3,I,,," << endl;
    }
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
      onDisc = true;
      path = catalog;
      return true;
    }
  }
  return false;
}

/** write a waypoint catalog into a filser da4 file */
bool WaypointCatalog::writeFilserDA4 (const QString& catalog)
{
  qDebug ("WaypointCatalog::writeFilserDA4 (%s)", catalog.toLatin1().data());

  QFile f(catalog);

  if (f.open(QIODevice::WriteOnly))
  {
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
      out.writeRawData (buf, MAXTSKNAME);
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
  quint8 wpImportance;

  quint32 fileMagic;
  qint8 fileType;
  quint16 fileFormat;


  QFile f(catalog);

  if (! f.exists())
    {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QObject::tr("<html><B>Catalog %1</B><BR>not found!</html>").arg(catalog),
                             QMessageBox::Ok );
    }

    if (f.open(QIODevice::ReadOnly)) {

      QDataStream in(&f);
      in.setVersion(2);

      //check if the file has the correct format
      in >> fileMagic;
      if (fileMagic != KFLOG_FILE_MAGIC) {
        qDebug("Waypoint file not recognized as KFLog file type.");
        return false;
      }

      in >> fileType;
      if (fileType != FILE_TYPE_WAYPOINTS) {
        qDebug("Waypoint file is a KFLog file, but not for waypoints.");
        return false;
      }

      in >> fileFormat;
      if (fileFormat != FILE_FORMAT_ID && fileFormat != FILE_FORMAT_ID_2) {
        qDebug("Waypoint file does not have the correct format. It returned %d, where %d was expected.", fileFormat, FILE_FORMAT_ID);
        return false;
      }
    //from here on, we assume that the file has the correct format.
      if (fileFormat==FILE_FORMAT_ID_2)

      while( !in.atEnd() ) {
        // read values from file
          //startoffset=f.at();
          in >> wpName;
          in >> wpDescription;
          in >> wpICAO;
          in >> wpType;
          in >> wpLatitude;
          in >> wpLongitude;
          in >> wpElevation;
          in >> wpFrequency;
          in >> wpLandable;
          in >> wpRunway;
          in >> wpLength;
          in >> wpSurface;
          in >> wpComment;
          if (fileFormat>=FILE_FORMAT_ID_2) {
            in >> wpImportance;
          } else {
            wpImportance=1; //normal importance
          };

       //create new waypoint object and set the correct properties
          Waypoint *w = new Waypoint;

          w->name = wpName;
          w->description = wpDescription;
          w->icao = wpICAO;
          w->type = wpType;
          w->origP.setLat(wpLatitude);
          w->origP.setLon(wpLongitude);
          w->elevation = wpElevation;
          w->frequency = wpFrequency;
          w->isLandable = wpLandable;
          w->runway.first =wpRunway;
          w->length = wpLength;
          w->surface = (enum Runway::SurfaceType) wpSurface;
          w->comment = wpComment;
          w->importance = wpImportance;
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
        if (fileFormat<FILE_FORMAT_ID_2) //write file back in newer format
          writeBinary();
    }
    else {
      QMessageBox::critical( _mainWindow,
                             QObject::tr("Error occurred!"),
                             QString("<html><B>%1</B><BR>").arg(catalog) +
                             "permission denied!" +
                             "</html>", QMessageBox::Ok );
    }

  return true;
}

/** read a waypoint catalog from a SeeYou cup file, only waypoint part */
bool WaypointCatalog::readCup (const QString& catalog)
{
  qDebug ("WaypointCatalog::readCup (%s)", catalog.toLatin1().data());

  QFile f(catalog);

  if (f.exists())
  {
    if (f.open(QIODevice::ReadOnly))
    {
      while (!f.atEnd())
      {
        bool ok;
        char buf[256];

        qint64 result = f.readLine( buf, sizeof(buf) );

        if (result > 0)
        {
          QString line(buf);
          line.replace( QRegExp("[\r\n]"), "" );
          QStringList list = line.split (",");

          if( list[0] == "-----Related Tasks-----" )
	    {
	       // Task part starts, we will ignore it and break up reading
	      break;
	    }

	  // 10 elements are mandatory, element 11 description is optional
	  if( list.count() < 10 ||
	      list[0] == "Title" || list[1] == "Code" || list[2] == "Country" )
	    {
	      // too less elements or a description line, ignore this
	      continue;
	    }

	  // A cup line consists of the following elements:
	  //
	  // Title,Code,Country,Latitude,Longitude,Elevation,Style,Direction,Length,Frequency,Description
	  //
	  // See here for more info: http://www.seeyou.ws/thankyou.php?fname=cup_format.pdf
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

    w->name = list[1].replace( QRegExp("\""), "" ); // short name of waypoint
	  w->comment = list[2] + ": ";
    w->icao = "";
	  w->surface = Runway::Unknown;

	  // waypoint type
	  uint wpType = list[6].toUInt(&ok);

	  if( ! ok )
	    {
        qDebug("Invalid waypoint type. Ignoring.");
	      delete w; continue;
	    }

	  switch( wpType )
	    {
	    case 0:
	      w->type = BaseMapElement::NotSelected;
	      break;
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
	    case 6:
	      w->type = BaseMapElement::Landmark;
	      break;
	    case 7:
	      w->type = BaseMapElement::Landmark;
	      break;
	    default:
	      w->type = BaseMapElement::NotSelected;
	      break;
	    }

	  // latitude as ddmm.mmm(N|S)
	  double degree = list[3].left(2).toDouble(&ok);

	  if( ! ok )
	    {
        qDebug("Error reading coordinate (N/S) (1)");
	      delete w; continue;
	    }

	  double minutes = list[3].mid(2,6).toDouble(&ok);

	  if( ! ok )
	    {
        qDebug("Error reading coordinate (N/S) (2)");
	      delete w; continue;
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
        qDebug("Error reading coordinate (E/W) (1)");
	      delete w; continue;
	    }

	  minutes = list[4].mid(3,6).toDouble(&ok);

	  if( ! ok )
	    {
        qDebug("Error reading coordinate (E/W) (2)");
	      delete w; continue;
	    }

	  double lonTmp = (degree * 600000.) + (minutes * 10000.0);


	  if( list[4].right(1).toUpper() == "W" )
	    {
	      lonTmp = -lonTmp;
	    }

          w->origP.setLat((int) rint(latTmp));
          w->origP.setLon((int) rint(lonTmp));

	  if( list[5].length() > 1 ) // elevation in meter or feet
	    {
	      double tmpElev = (int) rint((list[5].left(list[5].length()-1)).toDouble(&ok));

	      if( ! ok )
		{
			qDebug("Error reading elevation '%s'.",list[5].left(list[5].length()-1).toLatin1().data());
			delete w; continue;
		}

	      if( list[5].right(1).toLower() == "f" )
		{
			w->elevation = (int) rint(tmpElev * 0.3048);
		} else {
			w->elevation = (int) rint(tmpElev);
		}
	    }

	  if( list[9].trimmed().length() ) // airport frequency
	    {
	      double frequency = list[9].replace( QRegExp("\""), "" ).toDouble(&ok);

	      if( ok )
		{
		  w->frequency = frequency;
		}
	    }

	  if( list[7].trimmed().length() ) // runway direction
	    {
	      uint rdir = list[7].toInt(&ok);

	      if( ok )
		{
		  w->runway.first = rdir;
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
		  double length = list[8].left( list[8].length()-unit.length() ).toDouble(&ok);

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

		      w->length = (int) rint( length );
		    }
		}
	    }

	  if( list.count() == 11 &&
	      list[10].trimmed().length() ) // description, optional
	    {
	      w->comment += list[10].replace( QRegExp("\""), "" );
	    }

          if (!insertWaypoint(w))
          {
            //delete w; //even if inserting fails, the dict will delete the waypoint!
            qDebug("Error inserting waypoint in catalog");
            break;
          }
        }
      }
      onDisc = true;
      path = catalog;
      return true;
    }
  }
  return false;
}

bool WaypointCatalog::insertWaypoint(Waypoint *newWaypoint)
{
  // qDebug() << "WaypointCatalog::insertWaypoint:" << newWaypoint->name;

  bool ins = true;

  Waypoint *existingWaypoint = findWaypoint( newWaypoint->name );

  if( existingWaypoint != 0 )
    {
    if(existingWaypoint->name != newWaypoint->name ||
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
       existingWaypoint->surface != newWaypoint->surface )
    {
      switch( QMessageBox::warning( _mainWindow,
                                    "Waypoint exists",
                                    "<html>" + QObject::tr("A waypoint with the name<BR><BR><B>%1</B> \
                                     <BR><BR>is already in current catalog.<BR> \
                                     <BR>Do you want to overwrite the existing waypoint?").arg(newWaypoint->name) + "</html>",
                                     QObject::tr("Cancel"),
                                     QObject::tr("&Overwrite")))
        {
          case QMessageBox::Abort:   //cancel
            delete newWaypoint;
            ins = false;
            break;

          case QMessageBox::Ok:      //overwrite, old version
            wpList.removeOne(existingWaypoint);
            wpList.append(newWaypoint);
            break;
        }
     }
   }
  else
    {
      wpList.append( newWaypoint );
    }

  return ins;
}

Waypoint *WaypointCatalog::findWaypoint( const QString& name )
{
  foreach( Waypoint *waypoint, wpList )
    {
      if( waypoint->name == name )
          {
            return waypoint;
          }
    }

  return 0;
}

bool WaypointCatalog::removeWaypoint( const QString& name )
{
  foreach( Waypoint *waypoint, wpList )
      {
        if( waypoint->name == name )
          {
            wpList.removeOne( waypoint );
            delete waypoint;
            return true;
          }
      }

  return false;
}
