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
#include <pwd.h>
#include <unistd.h>

#include "waypointcatalog.h"
#include "airport.h"
#include "kfrgcs/vlapi2.h"

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


WaypointCatalog::WaypointCatalog()
  : modified(false), onDisc(false)
{
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory", getpwuid(getuid())->pw_dir);
  path = wayPointDir + "/" + i18n("unnamed.kflogwp");

  showAll = true;
  showAirports = showGliderSites = showOtherSites = showObstacle = showLandmark = showOutlanding =
  showStation = false;

  areaLat1 = areaLat2 = areaLong1 = areaLong2 = radiusLat = radiusLong = 1;
}

WaypointCatalog::~WaypointCatalog()
{
}

/** read a catalog from file */
bool WaypointCatalog::read(QString &catalog)
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
          wayPoint *w = new wayPoint;

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

          if (w->runway == 0 && w->length == 0) {
            // old format, convert it to new
            w->runway = w->length = -1;
            needConvert = true;
          }

          if (!wpList.insertItem(w)) {
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
      KMessageBox::error(0, i18n("<B>%1</B><BR>permission denied!").arg(catalog), i18n("Error occurred!"));
    }
  }
  else {
    KMessageBox::error(0, i18n("<B>%1</B><BR>not found!").arg(catalog), i18n("Error occurred!"));
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
  wayPoint *w;
  QFile f;
  QString fName = path;
  QDictIterator<wayPoint> it(wpList);

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
    KMessageBox::error(0, i18n("<B>%1</B><BR>permission denied!").arg(fName), i18n("Error occurred!"));
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

  wayPoint *w;
  QFile f;
  QString fName = path;
  QDictIterator<wayPoint> it(wpList);

  if (!onDisc)
    {
      //temporary, just create a waypointfile
      fName="/home/heiner/cumulus.kwp";
    }

  f.setName(fName);
  if (f.open(IO_WriteOnly))
    {
      QDataStream out(&f);

      out << (Q_UINT32)KFLOG_FILE_MAGIC;
      out << (Q_INT8)FILE_TYPE_WAYPOINTS;
      out << (Q_INT16)FILE_FORMAT_ID;

      for (w = it.current(); w != 0; w = ++it)
        {

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
        }

      f.close();
      path = fName;
      modified = false;
      onDisc = true;
    }
  else
    {
      KMessageBox::error(0, i18n("<B>%1</B><BR>permission denied!").arg(fName), i18n("Error occurred!"));
    }

  return ok;
}
/** No descriptions */
bool WaypointCatalog::importVolkslogger(QString & filename){
 QFileInfo fInfo(filename);
 QFile f(filename);

  if(!fInfo.exists())
    {
      KMessageBox::error(0,
          i18n("The selected file<BR><B>%1</B><BR>does not exist!").arg(filename));
      return false;
    }
  if(!fInfo.size())
    {
      KMessageBox::sorry(0,
          i18n("The selected file<BR><B>%1</B><BR>is empty!").arg(filename));
      return false;
    }
  //
  // We need a better format-identification then only the extension ...
  //
  if(((QString)fInfo.extension()).lower() != "dbt")
    {
      KMessageBox::error(0,
          i18n("The selected file<BR><B>%1</B><BR>is not a Volkslogger-file!").arg(filename));
      return false;
    }

  if(!f.open(IO_ReadOnly))
    {
      KMessageBox::error(0,
          i18n("You don't have permission to access file<BR><B>%1</B>").arg(filename),
          i18n("No permission"));
      return false;
    }

  QProgressDialog importProgress(0,0,true);

  importProgress.setCaption(i18n("Import file ..."));
  importProgress.setLabelText(
      i18n("Please wait while loading file<BR><B>%1</B>").arg(filename));
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
      wayPoint *w = new wayPoint;

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
          else {
            w->type = -1;
          }

          w->isLandable = true;
        }
        else {
          w->type = -1;
          w->isLandable = false;
          w->surface = 0;
        }
		// use WGSPoint() instead
        // w->origP.setLat(latTemp);
        //w->origP.setLon(lonTemp);
		w->origP = WGSPoint(latTemp, lonTemp);

		w->elevation =0;
        w->frequency = 0;
        w->runway = -1;
        w->length = -1;

        //w->comment = "<no comment>";

        if (!wpList.insertItem(w)) {
          break;
        }
      }
   }

  // close the import dialog, clean up and add the FlightRoute we just created
  importProgress.close();

  return true;
}

