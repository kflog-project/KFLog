/***********************************************************************
**
**   waypointcatalog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pwd.h>
#include <unistd.h>
#include <cmath>

#include "waypointcatalog.h"
#include "airport.h"
#include "kfrgcs/vlapi2.h"
#include "da4record.h"

#include <qdom.h>
#include <qapplication.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>


#define KFLOG_FILE_MAGIC    0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50
#define FILE_FORMAT_ID      100
#define FILE_FORMAT_ID_2    101


WaypointCatalog::WaypointCatalog(const QString& name)
  : modified(false), onDisc(false)
{
  static int catalogNr = 1;

  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory", getpwuid(getuid())->pw_dir);
  if (name == QString::null) {
    QString t;
    t.setNum(catalogNr++);
    catalogName = i18n("unnamed") + t;
  }
  else
    catalogName = name;
  path = wayPointDir + "/" + catalogName + ".kflogwp";

  showAll = true;
  showAirports = showGliderSites = showOtherSites = showObstacle = showLandmark = showOutlanding =
  showStation = false;

  areaLat1 = areaLat2 = areaLong1 = areaLong2 = radiusLat = radiusLong = 1;
}

WaypointCatalog::~WaypointCatalog()
{
}

/** read a catalog from file */
bool WaypointCatalog::read(const QString& catalog)
{
  bool ok = false;
  bool needConvert = false;
  QFile f(catalog);

  if (f.exists()) {
    if (f.open(IO_ReadOnly)) {
      QDomDocument doc;

      QApplication::setOverrideCursor(Qt::waitCursor);
      doc.setContent(&f);

      if (doc.doctype().name() == "KFLogWaypoint") {
        QDomNodeList nl = doc.elementsByTagName("Waypoint");

        for (uint i = 0; i < nl.count(); i++) {
          QDomNamedNodeMap nm =  nl.item(i).attributes();
          Waypoint *w = new Waypoint;

          w->name = nm.namedItem("Name").toAttr().value().left(6).upper();
          w->description = nm.namedItem("Description").toAttr().value();
          w->icao = nm.namedItem("ICAO").toAttr().value().upper();
          w->type = nm.namedItem("Type").toAttr().value().toInt();
          w->origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
          w->origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
          w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
          w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
          w->isLandable = nm.namedItem("Landable").toAttr().value().toInt();
          w->runway = nm.namedItem("Runway").toAttr().value().toInt();
          w->length = nm.namedItem("Length").toAttr().value().toInt();
          w->surface = nm.namedItem("Surface").toAttr().value().toInt();
          w->comment = nm.namedItem("Comment").toAttr().value();
          w->importance = nm.namedItem("Importance").toAttr().value().toInt();

          if (w->runway == 0 && w->length == 0) {
            // old format, convert it to new
            w->runway = w->length = -1;
            needConvert = true;
          }

          if (!wpList.insertItem(w))
          {
            delete w;
            break;
          }
        }

        onDisc = true;
        path = catalog;

        ok = true;
      }
      else {
        KMessageBox::error(0, i18n("wrong doctype ") + doc.doctype().name(), i18n("Error occurred!"));
      }

      f.close();

      if (needConvert) {
        write();
      }

      QApplication::restoreOverrideCursor();

    }
    else {
      KMessageBox::error(0, QString("<qt><B>%1</B><BR>").arg(catalog) + i18n ("permission denied!") + "</qt>", i18n("Error occurred!"));
    }
  }
  else {
    KMessageBox::error(0, QString("<qt><B>%1</B><BR>").arg(catalog) + i18n("not found!") + "</qt>", i18n("Error occurred!"));
  }

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
  QDictIterator<Waypoint> it(wpList);
/*
  if (!onDisc) {
    fName = KFileDialog::getSaveFileName(path, "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)", 0, i18n("Save waypoint catalog"));
    if(!fName.isEmpty()) {
      if (fName.right(8) != ".kflogwp") {
        fName += ".kflogwp";
      }
    }
    else {
      return false;
    }
  }
*/
  QApplication::setOverrideCursor( Qt::waitCursor );

  doc.appendChild(root);

  for (w = it.current(); w != 0; w = ++it) {
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
    child.setAttribute("Runway", w->runway);
    child.setAttribute("Length", w->length);
    child.setAttribute("Surface", w->surface);
    child.setAttribute("Comment", w->comment);
    child.setAttribute("Importance", w->importance);

    root.appendChild(child);
  }

  f.setName(fName);
  if (f.open(IO_WriteOnly)) {
    QString txt = doc.toString();
    f.writeBlock(txt, txt.length());
    f.close();
    path = fName;
    modified = false;
    onDisc = true;
  }
  else {
    KMessageBox::error(0, QString ("<qt><B>%1</B><BR>").arg(fName) + i18n("permission denied!") + "</qt>", i18n("Error occurred!"));
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
  Q_INT8 wpType;
  Q_INT32 wpLatitude;
  Q_INT32 wpLongitude;
  Q_INT16 wpElevation;
  double wpFrequency;
  Q_INT8 wpLandable;
  Q_INT16 wpRunway;
  Q_INT16 wpLength;
  Q_INT8 wpSurface;
  QString wpComment="";
  Q_UINT8 wpImportance;

  Waypoint *w;
  QFile f;
  QString fName = path;
  QDictIterator<Waypoint> it(wpList);

  f.setName(fName);
  if (f.open(IO_WriteOnly)) {
    QDataStream out(& f);

    //write fileheader
    out << Q_UINT32(KFLOG_FILE_MAGIC);
    out << Q_INT8(FILE_TYPE_WAYPOINTS);
    out << Q_UINT16(FILE_FORMAT_ID_2); //use the new format with importance field.

    for (w = it.current(); w != 0; w = ++it) {
      wpName=w->name;
      wpDescription=w->description;
      wpICAO=w->icao;
      wpType=w->type;
      wpLatitude=w->origP.lat();
      wpLongitude=w->origP.lon();
      wpElevation=w->elevation;
      wpFrequency=w->frequency;
      wpLandable=w->isLandable;
      wpRunway=w->runway;
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
    KMessageBox::error(0, QString ("<qt><B>%1</B><BR>").arg(fName) + i18n("permission denied!") + "</qt>", i18n("Error occurred!"));
  }
  return ok;
}

/** No descriptions */
bool WaypointCatalog::importVolkslogger(const QString& filename){
 QFileInfo fInfo(filename);
 QFile f(filename);

  if(!fInfo.exists())
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>does not exist!").arg(filename) + "</qt>");
      return false;
    }
  if(!fInfo.size())
    {
      KMessageBox::sorry(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is empty!").arg(filename) + "</qt>");
      return false;
    }
  //
  // We need a better format-identification then only the extension ...
  //
  if(((QString)fInfo.extension()).lower() != "dbt")
    {
      KMessageBox::error(0,
          "<qt>" + i18n("The selected file<BR><B>%1</B><BR>is not a Volkslogger-file!").arg(filename) + "</qt>");
      return false;
    }

  if(!f.open(IO_ReadOnly))
    {
      KMessageBox::error(0,
          "<qt>" + i18n("You don't have permission to access file<BR><B>%1</B>").arg(filename) + "</qt>",
          i18n("No permission"));
      return false;
    }

  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Import file ..."));
  importProgress.setLabelText(
      "<qt>" + i18n("Please wait while loading file<BR><B>%1</B>").arg(filename) + "</qt>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setTotalSteps(200);
  importProgress.show();
  importProgress.setMinimumDuration(0);

  importProgress.setProgress(0);

  unsigned int fileLength = fInfo.size();
  unsigned int filePos = 0;

  QStringList s;
  QString line;
  QTextStream stream(&f);

  int lat, lon, latTemp, lonTemp, latmin, lonmin;
  char latChar, lonChar;
  int lineCount;
  QChar flag;

  while (!stream.eof())
    {
      if(importProgress.wasCancelled()) return false;

      lineCount++;
      line = stream.readLine();
      s = QStringList::split (",", line, true);
      filePos += line.length();
      importProgress.setProgress(( filePos * 200 ) / fileLength);
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

      if (s.count() == 5) {
        sscanf(s[2], "%2d%5d", &lat, &latmin);
        sscanf(s[3], "%3d%5d", &lon, &lonmin);

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

        w->name = s[1].stripWhiteSpace();
        //w->description = "<unknown>";
        //w->icao = "<unknown>";

        if (flag & VLAPI_DATA::WPT::WPTTYP_L) {
          if (flag & VLAPI_DATA::WPT::WPTTYP_H) {
            w->surface = Airport::Asphalt;
          }
          else {
            w->surface = Airport::Grass;
          }

          if (flag & VLAPI_DATA::WPT::WPTTYP_A) {
            w->type = BaseMapElement::Airfield;
          }

          w->isLandable = true;
        }
        w->origP = WGSPoint(latTemp, lonTemp);

        if (!wpList.insertItem(w))
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
bool WaypointCatalog::save(bool alwaysAskName){
  QString fName = path;

  // check for unsupported file types - currently none
  /*
  if (fName.right(4).lower() == ".da4")
    if (KMessageBox::warningYesNoCancel(
                NULL,
                i18n("<qt>This type is not supported.<br>Save in KFLog format ?<BR><B>%1</B></qt>").arg(fName),
                i18n("Save changes?"),
                i18n("Save"),
                i18n("Discard")) == KMessageBox::Yes)
              alwaysAskName = true;
  */

  if (!onDisc || alwaysAskName) {
    fName = KFileDialog::getSaveFileName(path, "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)\n"
                                               "*.kwp *.KWP|Cumulus and KFLogEmbedded waypoints (*.kwp)\n"
                                               "*.txt *.TXT|Filser txt waypoints (*.txt)\n"
                                               "*.da4 *.DA4|Filser da4 waypoints (*.da4)",
                                               0, i18n("Save waypoint catalog"));
    if(!fName.isEmpty()) {
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
bool WaypointCatalog::load(const QString& catalog){
  if (catalog.right(8).lower() == ".kflogwp")
    return read(catalog);
  else if (catalog.right(4).lower() == ".txt")
    return readFilserTXT (catalog);
  else if (catalog.right(4).lower() == ".da4")
    return readFilserDA4 (catalog);
  else
    return readBinary(catalog);
}

/** read a waypoint catalog from a filser txt file */
bool WaypointCatalog::readFilserTXT (const QString& catalog)
{
  qDebug ("WaypointCatalog::readFilserTXT (%s)", catalog.latin1());
  QFile f(catalog);

  if (f.exists())
  {
    if (f.open(IO_ReadOnly))
    {
      while (!f.atEnd())
      {
        QString line;
        Q_LONG result = f.readLine (line, 256);
        if (result > 0)
        {
          QStringList list = QStringList::split (",", line, true);
          if (list[0] == "*") // comment/header line
            continue;
          Waypoint *w = new Waypoint;
          w->name = list [1];
          w->description = "";
          w->icao = "";
//        why don't we have type "turnpoint" ?
          if (list[2].upper() == "TP")
            w->type = BaseMapElement::Landmark;
          else if (list[2].upper() == "APT")
            w->type = BaseMapElement::Airfield;
          else if (list[2].upper() == "OUTLAN")
            w->type = BaseMapElement::Outlanding;
          else if (list[2].upper() == "MARKER")
            w->type = BaseMapElement::Landmark;
          else
            w->type = BaseMapElement::Landmark;
          w->origP.setLat((int)(list[3].toDouble() * 600000.0));
          w->origP.setLon((int)(list[4].toDouble() * 600000.0));
          w->elevation = (int)(list[5].toInt() * 0.3048); // don't we have conversion constants ?
          w->frequency = list[6].toDouble() / 1000.0;
          w->isLandable = false;
          w->length = list[7].toInt(); // length ?!
          w->runway = list[8].toInt(); // direction ?!
          QChar surface = list[9].upper()[0];
          switch (surface)
          {
            case 'G': w->surface = Airport::Grass;
                  break;
            case 'C': w->surface = Airport::Concrete;
                  break;
            default:  w->surface = Airport::Unknown;
          }
          w->comment = i18n("Imported from %1").arg(catalog);
          w->importance = 3;

          if (!wpList.insertItem(w))
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
  qDebug ("WaypointCatalog::writeFilserTXT (%s)", catalog.latin1());
  QFile f(catalog);

  if (f.open(IO_WriteOnly))
  {
    QTextStream out (&f);
    out << "*,TpName,Type,Latitiude,Longitude,Altitude,Frequency,RWY,RWYdir,RWYtype,TCA,TC" << endl;
    QDictIterator<Waypoint> it(wpList);
    for (Waypoint* w = it.current(); w != 0; w = ++it)
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
      out << w->runway << ",";
      switch (w->surface)
      {
        case Airport::Grass:
          out << "G,";
          break;
        case Airport::Concrete:
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
  qDebug ("WaypointCatalog::readFilserDA4 (%s)", catalog.latin1());
  QFile f(catalog);

  if (f.exists())
  {
    if (f.open(IO_ReadOnly))
    {
      QDataStream in(&f);
      DA4Buffer buffer;
      in.readRawBytes ((char*)&buffer, sizeof (DA4Buffer));
      for (int RecordNumber = 0; RecordNumber < WAYPOINT_MAX; RecordNumber++)
      {
        DA4WPRecord record (&buffer.waypoints[RecordNumber]);

        if (record.type() != BaseMapElement::NotSelected)
        {
          if (record.name().stripWhiteSpace().isEmpty())
            continue;
          Waypoint *w = record.newWaypoint();

          if (!wpList.insertItem(w))
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
  qDebug ("WaypointCatalog::writeFilserDA4 (%s)", catalog.latin1());
  QFile f(catalog);

  if (f.open(IO_WriteOnly))
  {
    QDataStream out (&f);
    QDictIterator<Waypoint> it(wpList);
    DA4Buffer buffer;
    int RecordNumber = 0;
    for (Waypoint* w = it.current(); w != 0; w = ++it)
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

    out.writeRawBytes ((char*)&buffer, sizeof(DA4Buffer));

    // fill buffer with empty task names
    char buf [MAXTSKNAME] = "                                    ";
    for (RecordNumber = 0; RecordNumber < TASK_MAX; RecordNumber++)
      out.writeRawBytes (buf, MAXTSKNAME);
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
  Q_INT8 wpType;
  Q_INT32 wpLatitude;
  Q_INT32 wpLongitude;
  Q_INT16 wpElevation;
  double wpFrequency;
  Q_INT8 wpLandable;
  Q_INT16 wpRunway;
  Q_INT16 wpLength;
  Q_INT8 wpSurface;
  QString wpComment="";
  Q_UINT8 wpImportance;

  Q_UINT32 fileMagic;
  Q_INT8 fileType;
  Q_UINT16 fileFormat;


  QFile f(catalog);
  if (f.exists()) {
    if (f.open(IO_ReadOnly)) {

      QDataStream in(&f);
      in.setVersion(2);

      //check if the file has the correct format
      in >> fileMagic;
      if (fileMagic != KFLOG_FILE_MAGIC) {
        qDebug("Waypoint file not recognized as KFLog filetype.");
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

      while(!in.eof()) {
        // read values from file
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
          w->runway =wpRunway;
          w->length = wpLength;
          w->surface = wpSurface;
          w->comment = wpComment;
          w->importance = wpImportance;
          //qDebug("Waypoint read: %s (%s - %s)",w->name.latin1(),w->description.latin1(),w->icao.latin1());

          if (!wpList.insertItem(w))
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
      KMessageBox::error(0, QString("<qt><B>%1</B><BR>").arg(catalog) + "permission denied!" + "</qt>", i18n("Error occurred!"));
    }
  }
  else {
    //KMessageBox::error(0, i18n("<qt><B>%1</B><BR>not found!</qt>").arg(catalog), i18n("Error occurred!"));
    qDebug("Waypoint catalog not found.");
  }

  return true;
}
