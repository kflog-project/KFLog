/***************************************************************************
                          waypoints.cpp  -  description
                             -------------------
    begin                : Fri Nov 30 2001
    copyright            : (C) 2001 by Harald Maier
    email                : harry@kflog.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "waypoints.h"
#include "wp.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "airport.h"
#include "glidersite.h"
#include "kflog.h"

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <qcursor.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qpushbutton.h>
#include <qregexp.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kapp.h>

extern MapContents _globalMapContents;
extern MapMatrix _globalMapMatrix;

Waypoints::Waypoints(QWidget *parent, const char *name, QString *catalog)
  : QFrame(parent, name)
{
  addWaypointWindow(this);
  addPopupMenu();

  waypointCatalogs.setAutoDelete(true);

  if (catalog == 0) {
    slotNewWaypointCatalog();
  }
  else {
    openCatalog(*catalog);
  }

  waypointDlg = new WaypointDialog(this);
  connect(waypointDlg, SIGNAL(addWaypoint()), SLOT(slotAddWaypoint()));

  importFilterDlg = new WaypointImpFilterDialog(this);
}

Waypoints::~Waypoints()
{
}

/** No descriptions */

void Waypoints::addWaypointWindow(QWidget *parent)
{
  waypoints =  new KFLogListView("Waypoints", parent, "waypoints");
  waypoints->setShowSortIndicator(true);
  waypoints->setAllColumnsShowFocus(true);

  colName = waypoints->addColumn(i18n("Name"));
  colDesc = waypoints->addColumn(i18n("Description"));
  colICAO = waypoints->addColumn(i18n("ICAO"));
  colType = waypoints->addColumn(i18n("Type"));
  colLat = waypoints->addColumn(i18n("Latitude"));
  colLong = waypoints->addColumn(i18n("Longitude"));
  colElev = waypoints->addColumn(i18n("Elevation (m)"));
  colFrequency = waypoints->addColumn(i18n("Frequency"));
  colLandable = waypoints->addColumn(i18n("Landable"));
  colRunway = waypoints->addColumn(i18n("Runway"));
  colLength = waypoints->addColumn(i18n("Length (m)"));
  colSurface = waypoints->addColumn(i18n("Surface"));
  colComment = waypoints->addColumn(i18n("Comment"));

  waypoints->setColumnAlignment(colElev, AlignRight);
  waypoints->setColumnAlignment(colFrequency, AlignRight);
  waypoints->setColumnAlignment(colRunway, AlignRight);
  waypoints->setColumnAlignment(colLength, AlignRight);

  waypoints->loadConfig();

  connect(waypoints, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
    SLOT(showWaypointPopup(QListViewItem *, const QPoint &, int)));
  connect(waypoints, SIGNAL(doubleClicked(QListViewItem *)),
    SLOT(slotEditWaypoint()));

  // header
  QHBoxLayout *header = new QHBoxLayout(5);
  QLabel *l = new QLabel(i18n("%1:").arg("Waypoints"), parent);
  l->setMaximumWidth(l->sizeHint().width() + 5);

  catalogName = new QComboBox(false, parent);
  connect(catalogName, SIGNAL(activated(int)), SLOT(slotSwitchWaypointCatalog(int)));

  QPushButton *fileOpen = new QPushButton(parent);
  fileOpen->setPixmap(BarIcon("fileopen"));
  QSizePolicy sp = fileOpen->sizePolicy();
  sp.setHorData(QSizePolicy::Fixed);
  fileOpen->setSizePolicy(sp);
  connect(fileOpen, SIGNAL(clicked()), SLOT(slotOpenWaypointCatalog()));

  QPushButton *filter = new QPushButton(i18n("Filter"), parent);
  sp = filter->sizePolicy();
  sp.setHorData(QSizePolicy::Fixed);
  filter->setSizePolicy(sp);
  connect(filter, SIGNAL(clicked()), SLOT(slotFilterWaypoints()));

  header->addWidget(l);
  header->addWidget(catalogName);
  header->addWidget(fileOpen);
  header->addWidget(filter);

  QVBoxLayout *layout = new QVBoxLayout(parent, 5, 5);

  layout->addLayout(header);
  layout->addWidget(waypoints);
}

/** No descriptions */
void Waypoints::addPopupMenu()
{
  wayPointPopup = new KPopupMenu(waypoints);
  wayPointPopup->insertTitle(SmallIcon("waypoint"), "Waypoint's", 0);
  wayPointPopup->insertItem(SmallIcon("waypoint"), i18n("&New catalog"), this,
    SLOT(slotNewWaypointCatalog()));
  wayPointPopup->insertItem(SmallIcon("fileopen"), i18n("&Open catalog"), this,
    SLOT(slotOpenWaypointCatalog()));
  idWaypointCatalogImport = wayPointPopup->insertItem(i18n("&Import catalog"), this,
    SLOT(slotImportWaypointCatalog()));
  idWaypointImportFromMap = wayPointPopup->insertItem(i18n("Import from &map"), this,
    SLOT(slotImportWaypointFromMap()));
  idWaypointImportFromFile = wayPointPopup->insertItem(i18n("Import from &file"), this,
    SLOT(slotImportWaypointFromFile()));
  idWaypointCatalogSave = wayPointPopup->insertItem(SmallIcon("filesave"), i18n("&Save catalog"), this,
    SLOT(slotSaveWaypointCatalog()));
  idWaypointCatalogClose = wayPointPopup->insertItem(SmallIcon("fileclose"), i18n("&Close catalog"), this,
    SLOT(slotCloseWaypointCatalog()));
  wayPointPopup->insertSeparator();
  wayPointPopup->insertItem(i18n("Down&load from logger"), this,
    SLOT(slotNotHandledItem()));
  wayPointPopup->insertItem(i18n("&Upload to logger"), this,
    SLOT(slotNotHandledItem()));
  wayPointPopup->insertSeparator();
  idWaypointNew = wayPointPopup->insertItem(SmallIcon("filenew"), i18n("New &waypoint"), this,
    SLOT(slotNewWaypoint()));
  idWaypointEdit = wayPointPopup->insertItem(SmallIcon("wizard"), i18n("&Edit waypopint"), this,
    SLOT(slotEditWaypoint()));
  idWaypointDelete = wayPointPopup->insertItem(SmallIcon("editdelete"), i18n("&Delete waypoint"), this,
    SLOT(slotDeleteWaypoint()));
  wayPointPopup->insertSeparator();
  idWaypointCopy2Task = wayPointPopup->insertItem(SmallIcon("editcopy"), i18n("Copy to &task"), this,
    SLOT(slotCopyWaypoint2Task()));
  idWaypointCenterMap = wayPointPopup->insertItem(SmallIcon("centerwaypoint"), i18n("Center map on waypoint"), this,
    SLOT(slotCenterMap()));
}

/** open a catalog and set it active */
void Waypoints::slotOpenWaypointCatalog()
{
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);

  QString fName = KFileDialog::getOpenFileName(wayPointDir, "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)", this, i18n("Open waypoint catalog"));

  openCatalog(fName);
}

/** No descriptions */
void Waypoints::slotNotHandledItem()
{
  KMessageBox::sorry(this, "This function is not yet available!", "ooops");
}

void Waypoints::showWaypointPopup(QListViewItem *it, const QPoint &, int)
{
  wayPointPopup->setItemEnabled(idWaypointCatalogSave, waypointCatalogs.count() && waypointCatalogs.current()->modified);
  wayPointPopup->setItemEnabled(idWaypointCatalogClose, waypointCatalogs.count() > 1);
  wayPointPopup->setItemEnabled(idWaypointCatalogImport, waypointCatalogs.count());
  wayPointPopup->setItemEnabled(idWaypointImportFromMap,waypointCatalogs.count());

  wayPointPopup->setItemEnabled(idWaypointNew,waypointCatalogs.count());
  wayPointPopup->setItemEnabled(idWaypointEdit, it != 0);
  wayPointPopup->setItemEnabled(idWaypointDelete, it != 0);
  wayPointPopup->setItemEnabled(idWaypointCopy2Task, it != 0);

  wayPointPopup->exec(QCursor::pos());
}

void Waypoints::slotNewWaypoint()
{
  waypointDlg->clear();

  if (waypointDlg->exec() == QDialog::Accepted) {
    slotAddWaypoint();
  }
}

/** create a new catalog */
void Waypoints::slotNewWaypointCatalog()
{
  int newItem = catalogName->count();
  WaypointCatalog *w = new WaypointCatalog;

  waypointCatalogs.append(w);
  catalogName->insertItem(w->path);

  catalogName->setCurrentItem(newItem);
  slotSwitchWaypointCatalog(newItem);
}

/* save changes in catalogs, return success */
bool Waypoints::saveChanges()
{
  WaypointCatalog *w;
  for (w = waypointCatalogs.first(); w != 0; w = waypointCatalogs.next()) {
    if (w->modified) {
      switch(KMessageBox::warningYesNoCancel(this, i18n("Save changes to<BR><B>%1</B>").arg(w->path))) {
      case KMessageBox::Yes:
        if (!w->write()) {
          return false;
        }
        break;
      case KMessageBox::Cancel:
        return false;
      }
    }
  }

  return true;
}

/** insert waypoint from waypoint dialog */
void Waypoints::slotAddWaypoint()
{
  if (!waypointDlg->name->text().isEmpty()) {
    // insert a new waypoint to current catalog
    wayPoint *w = new wayPoint;
    w->name = waypointDlg->name->text().upper();
    w->description = waypointDlg->description->text();
    w->type = waypointDlg->getWaypointType();
    w->origP.setLat(_globalMapContents.degreeToNum(waypointDlg->latitude->text()));
    w->origP.setLon(_globalMapContents.degreeToNum(waypointDlg->longitude->text()));
    w->elevation = waypointDlg->elevation->text().toInt();
    w->icao = waypointDlg->icao->text().upper();
    w->frequency = waypointDlg->frequency->text().toDouble();
    w->runway = waypointDlg->runway->text().toInt();
    w->length = waypointDlg->length->text().toInt();
    w->surface = waypointDlg->getSurface();
    w->isLandable = waypointDlg->isLandable->isChecked();
    w->comment = waypointDlg->comment->text();

    if (waypointCatalogs.current()->wpList.insertItem(w)) {
      waypointCatalogs.current()->modified = true;
      fillWaypoints();
    }
  }
}

/** No descriptions */
void Waypoints::slotEditWaypoint()
{
  QListViewItem *item = waypoints->currentItem();
  QString tmp, oldName;

  if (item != 0) {
    WaypointList *wl = &waypointCatalogs.current()->wpList;
    oldName = item->text(colName);
    wayPoint *w = wl->find(oldName);

    // initialize dialg
    waypointDlg->name->setText(w->name);
    waypointDlg->description->setText(w->description);
    // translate id to index
    waypointDlg->setWaypointType(w->type);
    waypointDlg->longitude->setText(printPos(w->origP.lon(), false));
    waypointDlg->latitude->setText(printPos(w->origP.lat(), true));
    tmp.sprintf("%d", w->elevation);
    waypointDlg->elevation->setText(tmp);
    waypointDlg->icao->setText(w->icao);
    tmp.sprintf("%.3f", w->frequency);
    waypointDlg->frequency->setText(tmp);
    tmp.sprintf("%d", w->runway);
    waypointDlg->runway->setText(tmp);
    tmp.sprintf("%d", w->length);
    waypointDlg->length->setText(tmp);
    // translate to id
    waypointDlg->setSurface(w->surface);
    waypointDlg->comment->setText(w->comment);
    waypointDlg->isLandable->setChecked(w->isLandable);

    if (waypointDlg->exec() == QDialog::Accepted) {
      if (!waypointDlg->name->text().isEmpty()) {
        w->name = waypointDlg->name->text().left(6).upper();
        w->description = waypointDlg->description->text();
        w->type = waypointDlg->getWaypointType();
        w->origP.setLat(_globalMapContents.degreeToNum(waypointDlg->latitude->text()));
        w->origP.setLon(_globalMapContents.degreeToNum(waypointDlg->longitude->text()));
        w->elevation = waypointDlg->elevation->text().toInt();
        w->icao = waypointDlg->icao->text().upper();
        w->frequency = waypointDlg->frequency->text().toDouble();
        w->runway = waypointDlg->runway->text().toInt();
        w->length = waypointDlg->length->text().toInt();
        w->surface = waypointDlg->getSurface();
        w->comment = waypointDlg->comment->text();
        w->isLandable = waypointDlg->isLandable->isChecked();

        /* name has changed, remove old key and insert a new key */
        if (oldName != w->name) {
          wl->take(oldName);
          wl->insertItem(w);
        }

        waypointCatalogs.current()->modified = true;
        fillWaypoints();
      }
    }
  }
}

/** No descriptions */
void Waypoints::slotDeleteWaypoint()
{
  QListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    QString tmp = item->text(colName);
    waypointCatalogs.current()->wpList.remove(tmp);
    waypointCatalogs.current()->modified = true;
    delete item;
  }
}

/** No descriptions */
void Waypoints::fillWaypoints()
{
  QString tmp;
  wayPoint *w;
  WaypointCatalog *c = waypointCatalogs.current();
  QListViewItem *item;
  QDictIterator<wayPoint> it(c->wpList);
  bool filterRadius, filterArea;
  extern TranslationList surfaces;
  extern TranslationList waypointTypes;

  waypoints->clear();

  filterRadius = (c->radiusLat != 1  || c->radiusLong != 1);
  filterArea = (c->areaLat2 != 1 && c->areaLong2 != 1 && !filterRadius);

  for (w = it.toFirst(); w != 0; w = ++it) {
    if (!c->showAll) {
      switch(w->type) {
      case BaseMapElement::IntAirport:
      case BaseMapElement::Airport:
      case BaseMapElement::MilAirport:
      case BaseMapElement::CivMilAirport:
      case BaseMapElement::Airfield:
        if (!c->showAirports) {
          continue;
        }
        break;
      case BaseMapElement::Glidersite:
        if (!c->showGliderSites) {
          continue;
        }
        break;
      case BaseMapElement::UltraLight:
      case BaseMapElement::HangGlider:
      case BaseMapElement::Parachute:
      case BaseMapElement::Ballon:
        if (!c->showOtherSites) {
          continue;
        }
        break;
      }
    }

     if (filterArea) {
       if (w->origP.lat() < c->areaLat1 || w->origP.lat() > c->areaLat2 ||
           w->origP.lon() < c->areaLong1 || w->origP.lon() > c->areaLong2) {
           continue;
       }
     }
     else if (filterRadius) {
       if (dist(c->radiusLat, c->radiusLong, w->origP.lat(), w->origP.lon()) > c->radiusSize) {
         continue;
       }
     }

    item = new QListViewItem(waypoints);
    item->setText(colName, w->name);
    item->setText(colDesc, w->description);
    item->setText(colICAO, w->icao);
    item->setText(colType, w->type == -1 ? QString::null : waypointTypes.itemById(w->type)->text);
    item->setText(colLat, printPos(w->origP.lat(), true));
    item->setText(colLong, printPos(w->origP.lon(), false));
    tmp.sprintf("%d", w->elevation);
    item->setText(colElev, tmp);
    tmp.sprintf("%.3f", w->frequency);
    item->setText(colFrequency, tmp);
    item->setText(colLandable, w->isLandable == true ? i18n("Yes") : QString::null);
    tmp.sprintf("%02d", w->runway);
    item->setText(colRunway, tmp);
    tmp.sprintf("%d", w->length);
    item->setText(colLength, tmp);
    item->setText(colSurface, w->surface == -1 ? QString::null : surfaces.itemById(w->surface)->text);
    item->setText(colComment, w->comment);
  }

  emit waypointCatalogChanged( waypointCatalogs.current() );
}

/** No descriptions */
void Waypoints::slotSwitchWaypointCatalog(int idx)
{
  waypointCatalogs.at(idx);
  fillWaypoints();
}

void Waypoints::slotSaveWaypointCatalog()
{
  WaypointCatalog *w = waypointCatalogs.current();
  if (w->write()) {
    catalogName->changeItem(w->path, catalogName->currentItem());
  }
}

void Waypoints::slotImportWaypointCatalog()
{
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);

  QString fName = KFileDialog::getOpenFileName(wayPointDir, "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)\n*|All files", this, i18n("Import waypoint catalog"));

  if(!fName.isEmpty()) {
    WaypointCatalog *w = waypointCatalogs.current();

    // read from disk
    w->read(fName);
    w->modified = true;
    fillWaypoints();
  }
}

void Waypoints::slotCloseWaypointCatalog()
{
  WaypointCatalog *w = waypointCatalogs.current();
  int idx = waypointCatalogs.at();
  int cnt;

  if (w->modified) {
    switch(KMessageBox::warningYesNoCancel(this, i18n("Save changes to<BR><B>%1</B>").arg(w->path))) {
    case KMessageBox::Yes:
      if (!w->write()) {
        return;
      }
      break;
    case KMessageBox::Cancel:
      return;
    }
  }

  waypointCatalogs.removeRef(w);
  catalogName->removeItem(idx);

  // activate new catalog
  cnt = catalogName->count();
  if (idx >= cnt) {
    idx = cnt -1;
  }

  catalogName->setCurrentItem(idx);
  slotSwitchWaypointCatalog(idx);
}

void Waypoints::slotImportWaypointFromMap()
{
  SinglePoint *s;
  wayPoint *w;
  WaypointCatalog *c = waypointCatalogs.current();
  WaypointList *wl = &(c->wpList);
  unsigned int i;
  int type, loop;
  WGSPoint p;
  QString tmp;
  QRegExp blank("[ ]");
  QValueList<int> searchLists;
  QValueList<int>::Iterator searchListsIt;
  KConfig* config = KGlobal::config();
  config->setGroup("Map Data");
  bool filterRadius, filterArea;

  if (importFilterDlg->exec() == QDialog::Accepted) {
    getFilterData();

    if (c->showAll || c->showAirports) {
      searchLists.append(MapContents::AirportList);
    }
    if (c->showAll || c->showGliderSites) {
      searchLists.append(MapContents::GliderList);
    }
    if (c->showAll || c->showOtherSites) {
      searchLists.append(MapContents::AddSitesList);
    }
    if (c->showAll || c->showObstacle) {
      searchLists.append(MapContents::ObstacleList);
    }
    if (c->showAll || c->showLandmark) {
      searchLists.append(MapContents::LandmarkList);
    }
    if (c->showAll || c->showOutlanding) {
      searchLists.append(MapContents::OutList);
    }
    if (c->showAll || c->showStation) {
      searchLists.append(MapContents::StationList);
    }

    filterRadius = (c->radiusLat != 1  || c->radiusLong != 1);
    filterArea = (c->areaLat2 != 1 && c->areaLong2 != 1 && !filterRadius);

    for (searchListsIt =searchLists.begin(); searchListsIt != searchLists.end(); ++searchListsIt) {
      for (i = 0; i < _globalMapContents.getListLength(*searchListsIt); i++) {

        s = (SinglePoint *)_globalMapContents.getElement(*searchListsIt, i);
        p = s->getWGSPosition();

        // check area
        if (filterArea) {
          if (p.lon() < c->areaLong1 || p.lon() > c->areaLong2 ||
              p.lat() < c->areaLat1 || p.lat() > c->areaLat2) {
              continue;
          }
        }
        else if (filterRadius) {
          if (dist(c->radiusLat, c->radiusLong, p.lat(), p.lon()) > c->radiusSize) {
            continue;
          }
        }

        w = new wayPoint;

        w->name = s->getName().replace(blank, QString::null).left(6).upper();
        loop = 0;
        while (wl->find(w->name) && loop < 100000) {
          tmp.setNum(loop++);
          w->name = w->name.left(6 - tmp.length()) + tmp;
        }
        w->description = s->getName();
        type = s->getTypeID();
        w->type = type;
        w->runway = 0;
        w->length = 0;
        w->surface = -1;
        w->frequency = 0.0;

        w->origP = p;

        w->elevation = s->getElevation();

        switch(type) {
        case BaseMapElement::IntAirport:
        case BaseMapElement::Airport:
        case BaseMapElement::MilAirport:
        case BaseMapElement::CivMilAirport:
        case BaseMapElement::Airfield:
        case BaseMapElement::Glidersite:
          w->icao = ((RadioPoint *) s)->getICAO();
          w->frequency = ((RadioPoint *) s)->getFrequency().toDouble();
          w->isLandable = true;
//        if (a->getRunwayNumber()) {
//          runway r = a->getRunway(0);
//          w->runway = r.direction;
//          w->length = r.length;
//          w->surface = r.surface;
//        }
          break;
        default:
          w->isLandable = false;
        }

        if (!wl->insertItem(w)) {
          break;
        }
      }
    }

    waypointCatalogs.current()->modified = true;
    fillWaypoints();
  }
}
/** filter waypoints to display */
void Waypoints::slotFilterWaypoints()
{
  if (importFilterDlg->exec() == QDialog::Accepted) {
    getFilterData();
    fillWaypoints();
  }
}
/** add a new waypoint from outside */
void Waypoints::slotAddWaypoint(wayPoint *w)
{
  WaypointList *wl = &waypointCatalogs.current()->wpList;
  int loop = 1;
  if (w->name.isEmpty()) {
    w->name.sprintf("WPT%03d", loop);
    while (wl->find(w->name) && loop < 1000) {
      w->name.sprintf("WPT%03d", ++loop);
    }
  }

  if (wl->insertItem(w)) {
    waypointCatalogs.current()->modified = true;
    fillWaypoints();
  }
}

void Waypoints::slotCopyWaypoint2Task()
{
  QListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    WaypointList *wl = &waypointCatalogs.current()->wpList;
    wayPoint *w = wl->find(item->text(colName));

    emit copyWaypoint2Task(w);
  }
}

void Waypoints::slotCenterMap()
{
  QListViewItem *item = waypoints->currentItem();

  if (item != 0)
    {
      WaypointList *wl = &waypointCatalogs.current()->wpList;
      wayPoint *w = wl->find(item->text(colName));

      emit centerMap(w->origP.lat(), w->origP.lon());
    }
}

void Waypoints::getFilterData()
{
  WGSPoint p;
  KConfig* config = KGlobal::config();
  config->setGroup("Map Data");
  WaypointCatalog *c = waypointCatalogs.current();

  c->showAll = importFilterDlg->useAll->isChecked();
  c->showAirports = importFilterDlg->airports->isChecked();
  c->showGliderSites = importFilterDlg->gliderSites->isChecked();
  c->showOtherSites = importFilterDlg->otherSites->isChecked();
  c->showObstacle = importFilterDlg->obstacle->isChecked();
  c->showLandmark = importFilterDlg->landmark->isChecked();
  c->showOutlanding = importFilterDlg->outlanding->isChecked();
  c->showStation = importFilterDlg->station->isChecked();

  c->areaLat1 = _globalMapContents.degreeToNum(importFilterDlg->fromLat->text());
  c->areaLat2 = _globalMapContents.degreeToNum(importFilterDlg->toLat->text());
  c->areaLong1 = _globalMapContents.degreeToNum(importFilterDlg->fromLong->text());
  c->areaLong2 = _globalMapContents.degreeToNum(importFilterDlg->toLong->text());

  switch (importFilterDlg->getCenterRef()) {
  case CENTER_POS:
    c->radiusLat = _globalMapContents.degreeToNum(importFilterDlg->posLat->text());
    c->radiusLong = _globalMapContents.degreeToNum(importFilterDlg->posLong->text());
    break;
  case CENTER_HOMESITE:
    c->radiusLat = config->readNumEntry("Homesite Latitude");
    c->radiusLong = config->readNumEntry("Homesite Longitude");
    break;
  case CENTER_MAP:
    p = _globalMapMatrix.getMapCenter(false);
    c->radiusLat = p.lat();
    c->radiusLong = p.lon();
    break;
  case CENTER_AIRPORT:
    p = importFilterDlg->getAirportRef();
    c->radiusLat = p.lat();
    c->radiusLong = p.lon();
    break;
  }

  c->radiusSize = importFilterDlg->radius->currentText().toDouble();

  // normalize coordinates
  if (c->areaLat1 > c->areaLat2) {
    int tmp = c->areaLat1;
    c->areaLat1 = c->areaLat2;
    c->areaLat2 = tmp;
  }

  if (c->areaLong1 > c->areaLong2) {
    int tmp = c->areaLong1;
    c->areaLong1 = c->areaLong2;
    c->areaLong2 = tmp;
  }
}
/** No descriptions */
void Waypoints::slotImportWaypointFromFile(){
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);

  QString fName = KFileDialog::getOpenFileName(wayPointDir, "*.dbt *.DBT|Waypoint file (Volkslogger format, *.dbt *:DBT) \n *.gdn *.GDN|Waypoint file (Garmin format, *.gdn *.GDN) \n *|All files", this, i18n("Import waypoints from file"));

  if(!fName.isEmpty()) {
    WaypointCatalog *w = waypointCatalogs.current();

    // read from disk
    w->modified = true;
    if (fName.right(4).lower() == ".dbt"){
  	  w->importVolkslogger(fName);
//    } else if (fName.right(4).lower() == "*.gdn"){
//    w->importGarmin(fName);
    } else {
      w->modified = false;
    }
    fillWaypoints();
  }
}

void Waypoints::openCatalog(QString &catalog)
{
  if(!catalog.isEmpty()) {
    int newItem = catalogName->count();
    WaypointCatalog *w = new WaypointCatalog;
    QFile f(catalog);

     if (f.exists()) {
       // read from disk
       if (!w->read(catalog)) {
         delete w;
       }
       else {
         w->onDisc = true;
         w->path = catalog;

         waypointCatalogs.append(w);
         catalogName->insertItem(w->path);

         catalogName->setCurrentItem(newItem);
         slotSwitchWaypointCatalog(newItem);
       }
     }  else {
		delete w;
     }
  }
}

/* slot to set name of catalog and open it without a file selection dialog */
void Waypoints::slotSetWaypointCatalogName(QString catalog){
	this->openCatalog(catalog);
}
