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

#include <qdom.h>
#include <qapplication.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

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
          WaypointElement *w = new WaypointElement;

          w->name = nm.namedItem("Name").toAttr().value().left(6).upper();
          w->description = nm.namedItem("Description").toAttr().value();
          w->icao = nm.namedItem("ICAO").toAttr().value().upper();
          w->type = nm.namedItem("Type").toAttr().value().toInt();
          w->pos.setY(nm.namedItem("Latitude").toAttr().value().toInt());
          w->pos.setX(nm.namedItem("Longitude").toAttr().value().toInt());
          w->elevation = nm.namedItem("Elevation").toAttr().value().toInt();
          w->frequency = nm.namedItem("Frequency").toAttr().value().toDouble();
          w->isLandable = nm.namedItem("Landable").toAttr().value().toInt();
          w->runway = nm.namedItem("Runway").toAttr().value().toInt();
          w->length = nm.namedItem("Length").toAttr().value().toInt();
          w->surface = nm.namedItem("Surface").toAttr().value().toInt();
          w->comment = nm.namedItem("Comment").toAttr().value();

          if (!wpList.insertItem(w)) {
            break;
          }
        }

        ok = true;
      }
      else {
        KMessageBox::error(0, i18n("wrong doctype ") + doc.doctype().name(), i18n("Error occurred!"));
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
  WaypointElement *w;
  QFile f;
  QString fName = path;
  QDictIterator<WaypointElement> it(wpList);

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
    child.setAttribute("Latitude", w->pos.y());
    child.setAttribute("Longitude", w->pos.x());
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

