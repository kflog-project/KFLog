/***************************************************************************
                          waypoints.cpp  -  description
                             -------------------
    begin                : Fri Nov 30 2001
    copyright            : (C) 2001 by Harald Maier
    email                : harry@kflog.org

    $Id$

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "airport.h"
#include "glidersite.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "translationlist.h"
#include "waypoints.h"

#include <pwd.h>

#include <qcursor.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qsizepolicy.h>
#include <qregexp.h>

extern MapContents _globalMapContents;
extern MapMatrix _globalMapMatrix;

Waypoints::Waypoints(QWidget *parent, const char *name, const QString& /*catalog*/)
  : QFrame(parent, name)
{
  addWaypointWindow(this);
  addPopupMenu();

  waypointCatalogs.setAutoDelete(true);
  /*
    if (catalog.isEmpty() == 0) {
    slotNewWaypointCatalog();
    }
    else {
    openCatalog(*catalog);
    }
  */
  waypointDlg = new WaypointDialog(this);
  connect(waypointDlg, SIGNAL(addWaypoint(Waypoint *)), SLOT(slotAddWaypoint(Waypoint *)));

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

  colName = waypoints->addColumn(tr("Name"));
  colDesc = waypoints->addColumn(tr("Description"));
  colICAO = waypoints->addColumn(tr("ICAO"));
  colType = waypoints->addColumn(tr("Type"));
  colLat = waypoints->addColumn(tr("Latitude"));
  colLong = waypoints->addColumn(tr("Longitude"));
  colElev = waypoints->addColumn(tr("Elevation (m)"));
  colFrequency = waypoints->addColumn(tr("Frequency"));
  colLandable = waypoints->addColumn(tr("Landable"));
  colRunway = waypoints->addColumn(tr("Runway"));
  colLength = waypoints->addColumn(tr("Length (m)"));
  colSurface = waypoints->addColumn(tr("Surface"));
  colComment = waypoints->addColumn(tr("Comment"));

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
  QLabel *l = new QLabel(QString("%1:").arg(tr("Waypoints")), parent);
  l->setMaximumWidth(l->sizeHint().width() + 5);

  catalogName = new QComboBox(false, parent);
  connect(catalogName, SIGNAL(activated(int)), SLOT(slotSwitchWaypointCatalog(int)));

  QPushButton *fileOpen = new QPushButton(parent);
  fileOpen->setPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_fileopen_16.png");
  QSizePolicy sp = fileOpen->sizePolicy();
  sp.setHorData(QSizePolicy::Fixed);
  fileOpen->setSizePolicy(sp);
  connect(fileOpen, SIGNAL(clicked()), SLOT(slotOpenWaypointCatalog()));

  QPushButton *filter = new QPushButton(tr("Filter"), parent);
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
  wayPointPopup = new QPopupMenu(waypoints);
  catalogCopySubPopup = new QPopupMenu(waypoints);
  catalogMoveSubPopup = new QPopupMenu(waypoints);


//  wayPointPopup->insertTitle(SmallIcon("waypoint"), "Waypoint's", 0);
  wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/waypoint_16.png"), tr("&New catalog"), this, SLOT(slotNewWaypointCatalog()));
  wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_fileopen_16.png"), tr("&Open catalog"), this, SLOT(slotOpenWaypointCatalog()));
  idWaypointCatalogImport = wayPointPopup->insertItem(tr("&Import catalog"), this, SLOT(slotImportWaypointCatalog()));
  idWaypointImportFromMap = wayPointPopup->insertItem(tr("Import from &map"), this, SLOT(slotImportWaypointFromMap()));
  idWaypointImportFromFile = wayPointPopup->insertItem(tr("Import from &file"), this, SLOT(slotImportWaypointFromFile()));
  idWaypointCatalogSave = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_filesave_16.png"), tr("&Save catalog"), this, SLOT(slotSaveWaypointCatalog()));
  idWaypointCatalogSaveAs = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_filesaveas_16.png"), tr("&Save catalog as..."), this, SLOT(slotSaveWaypointCatalogAs()));
  idWaypointCatalogClose = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_fileclose_16.png"), tr("&Close catalog"), this, SLOT(slotCloseWaypointCatalog()));
  wayPointPopup->insertSeparator();
  idWaypointNew = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_filenew_16.png"), tr("New &waypoint"), this, SLOT(slotNewWaypoint()));
  idWaypointEdit = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_wizard_16.png"), tr("&Edit waypoint"), this, SLOT(slotEditWaypoint()));
  idWaypointDelete = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_editdelete_16.png"), tr("&Delete waypoint"), this, SLOT(slotDeleteWaypoint()));
  idWaypointCopy2Catalog = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_editcopy_16.png"), tr("Copy to &catalog"), catalogCopySubPopup);
  idWaypointMove2Catalog = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_move_16.png"), tr("Move to &catalog"), catalogMoveSubPopup);

  wayPointPopup->insertSeparator();
  idWaypointCopy2Task = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_editcopy_16.png"), tr("Copy to &task"), this, SLOT(slotCopyWaypoint2Task()));
  idWaypointCenterMap = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/centerwaypoint_16.png"), tr("Center map on waypoint"), this, SLOT(slotCenterMap()));
  idWaypointSetHome = wayPointPopup->insertItem(QPixmap(QDir::homeDirPath() + "/.kflog/pics/kde_gohome_16.png"), tr("Set Homesite"), this, SLOT(slotSetHome()));

  connect(catalogCopySubPopup, SIGNAL(activated(int)), this, SLOT(slotCopy2Catalog(int)));
  connect(catalogMoveSubPopup, SIGNAL(activated(int)), this, SLOT(slotMove2Catalog(int)));
}

/**
 * Copies the current waypoint to the selected catalog
 */
void Waypoints::slotCopy2Catalog(int id){
  QListViewItem *item = waypoints->currentItem();
  Waypoint * wpt;

  if (item != 0) {
    QString tmp = item->text(colName);
    wpt=waypointCatalogs.current()->wpList.find(tmp);
    if (waypointCatalogs.at(id)->wpList.insertItem(new Waypoint(wpt)))
      waypointCatalogs.at(id)->modified=true;
  }

}

/**
 * Moves the current waypoint to the selected catalog
 */
void Waypoints::slotMove2Catalog(int id){
  QListViewItem *item = waypoints->currentItem();
  Waypoint * wpt;

  if (item != 0) {
    QString tmp = item->text(colName);
    wpt=waypointCatalogs.current()->wpList.find(tmp);
    if (waypointCatalogs.at(id)->wpList.insertItem(new Waypoint(wpt))) {
      waypointCatalogs.current()->wpList.remove(tmp);
      waypointCatalogs.current()->modified = true;
      waypointCatalogs.at(id)->modified = true;
      delete item;
    }
  }

}

/** open a catalog and set it active */
void Waypoints::slotOpenWaypointCatalog()
{
  extern QSettings _settings;
  QString wayPointDir = _settings.readEntry("/KFLog/Path/DefaultWaypointDirectory",
                                          getpwuid(getuid())->pw_dir);

  QString filter;
  filter.append(tr("All supported waypoint formats")+" (*.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP *.txt *.TXT);;");
  filter.append(tr("KFLog waypoints")+" (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Cumulus and KFLogEmbedded waypoints")+" (*.kwp *.KWP);;");
  filter.append(tr("Filser txt waypoints")+" (*.txt *.TXT);;");
  filter.append(tr("Filser da4 waypoints")+" (*.da4 *.DA4);;");
  filter.append(tr("SeeYou cup waypoints")+" (*.cup *.CUP)");
  QString fName = QFileDialog::getOpenFileName(wayPointDir, filter, this, tr("Open waypoint catalog"));

  openCatalog(fName);
}

void Waypoints::showWaypointPopup(QListViewItem *it, const QPoint &, int)
{
  //enable and disable the correct menuitems
  wayPointPopup->setItemEnabled(idWaypointCatalogSave, waypointCatalogs.count() && waypointCatalogs.current()->modified);
  wayPointPopup->setItemEnabled(idWaypointCatalogSaveAs, waypointCatalogs.count() > 0);
  wayPointPopup->setItemEnabled(idWaypointCatalogClose, waypointCatalogs.count() > 1);
  wayPointPopup->setItemEnabled(idWaypointCatalogImport, waypointCatalogs.count());
  wayPointPopup->setItemEnabled(idWaypointImportFromMap,waypointCatalogs.count());

  wayPointPopup->setItemEnabled(idWaypointNew,waypointCatalogs.count());
  wayPointPopup->setItemEnabled(idWaypointEdit, it != 0);
  wayPointPopup->setItemEnabled(idWaypointDelete, it != 0);
  wayPointPopup->setItemEnabled(idWaypointCopy2Task, it != 0);
  wayPointPopup->setItemEnabled(idWaypointCenterMap, it != 0);
  wayPointPopup->setItemEnabled(idWaypointSetHome, it != 0);

  wayPointPopup->setItemEnabled(idWaypointCopy2Catalog, waypointCatalogs.count() > 1 && it != 0);
  wayPointPopup->setItemEnabled(idWaypointMove2Catalog, waypointCatalogs.count() > 1 && it != 0);

  //fill the submenus for the move & copy to catalog
  catalogCopySubPopup->clear();
  catalogMoveSubPopup->clear();
  //store current catalog index
  uint curCat=waypointCatalogs.at();
  for (unsigned int i=0;i<waypointCatalogs.count();i++) {
    if (curCat!=i) { //only insert if this catalog is NOT the current catalog...
      catalogCopySubPopup->insertItem(waypointCatalogs.at(i)->path,i);
      catalogMoveSubPopup->insertItem(waypointCatalogs.at(i)->path,i);
    }
  }
  //make sure we go back to the original catalog...
  waypointCatalogs.at(curCat);
  wayPointPopup->exec(QCursor::pos());
}

void Waypoints::slotNewWaypoint()
{
  waypointDlg->clear();
  waypointDlg->exec();
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
  for (w = waypointCatalogs.first(); w != 0; w = waypointCatalogs.next())
    {
      if (w->modified)
        {
          QMessageBox saveBox(tr("Save changes?"),
              tr("<qt>The waypoint file has been modified.<br>Save changes to<BR><B>%1</B></qt>").arg(w->path),
              QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
          saveBox.setButtonText(QMessageBox::Yes, tr("Save"));
          saveBox.setButtonText(QMessageBox::No, tr("Discard"));
          switch(saveBox.exec())
            {
            case QMessageBox::Yes:
              // Hier zwischenzeitlich auf binärformat umgestellt ...
              if (!w->save()) //Binary())
                return false;
              break;
            case QMessageBox::Cancel:
              return false;
            }
        }
    }

  return true;
}

/** No descriptions */
void Waypoints::slotEditWaypoint()
{
  QListViewItem *item = waypoints->currentItem();
  QString tmp, oldName;

  if (item != 0) {
    WaypointDict *wl = &waypointCatalogs.current()->wpList;
    oldName = item->text(colName);
    Waypoint *w = wl->find(oldName);
    slotEditWaypoint (w);
  }
}

/** No descriptions */
void Waypoints::slotEditWaypoint(Waypoint* w)
{
  if (w)
  {
    QString tmp, oldName;

    oldName = w->name;
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
    if (w->runway != -1) {
      tmp.sprintf("%d", w->runway);
    }
    else {
      tmp = QString::null;
    }
    waypointDlg->runway->setText(tmp);
    if (w->length != -1) {
      tmp.sprintf("%d", w->length);
    }
    else {
      tmp = QString::null;
    }
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
        tmp = waypointDlg->runway->text();
        if (!tmp.isEmpty()) {
          w->runway = tmp.toInt();
        }
        else {
          w->runway = -1;
        }
        tmp = waypointDlg->length->text();
        if (!tmp.isEmpty()) {
          w->length = tmp.toInt();
        }
        else {
          w->length = -1;
        }
        w->surface = waypointDlg->getSurface();
        w->comment = waypointDlg->comment->text();
        w->isLandable = waypointDlg->isLandable->isChecked();

        /* name has changed, remove old key and insert a new key */
        if (oldName != w->name) {
          WaypointDict *wl = &waypointCatalogs.current()->wpList;
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
void Waypoints::slotDeleteWaypoint(Waypoint* wp)
{
  if (wp)
  {
    QMessageBox waypointBox(tr("Delete waypoint?"),
                            tr("<qt>Waypoint <b>%1</b> will be deleted.<br>Are you sure?</qt>").arg(wp->name),
                            QMessageBox::Warning, QMessageBox::Ok, QMessageBox::Cancel, 0);
    waypointBox.setButtonText(QMessageBox::Ok, tr("&Delete"));
    if (waypointBox.exec()== QMessageBox::Ok)
    {
      waypointCatalogs.current()->wpList.remove(wp->name);
      waypointCatalogs.current()->modified = true;
      fillWaypoints();
    }
  }
}

/** No descriptions */
void Waypoints::slotDeleteWaypoint()
{
  QListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    QString tmp = item->text(colName);
    QMessageBox waypointBox(tr("Delete waypoint?"),
                            tr("<qt>Waypoint <b>%1</b> will be deleted.<br>Are you sure?</qt>").arg(tmp),
                            QMessageBox::Warning, QMessageBox::Ok, QMessageBox::Cancel, 0);
    waypointBox.setButtonText(QMessageBox::Ok, tr("&Delete"));
    if (waypointBox.exec() == QMessageBox::Ok)
    {
      waypointCatalogs.current()->wpList.remove(tmp);
      waypointCatalogs.current()->modified = true;
      delete item;

      // eggert@kflog.org
      // this must be done to make the deletion effective immediatly (map display)
      fillWaypoints();
    }
  }
}

/** No descriptions */
void Waypoints::fillWaypoints()
{
  QString tmp;
  Waypoint *w;
  WaypointCatalog *c = waypointCatalogs.current();
  QListViewItem *item;
  QDictIterator<Waypoint> it(c->wpList);
  bool filterRadius, filterArea;
  extern TranslationList surfaceTypes;
  extern TranslationList waypointTypes;
  extern MapConfig _globalMapConfig;

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
      case BaseMapElement::Balloon:
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
    item->setText(colType, w->type == -1 ? QString::null : waypointTypes.itemText(w->type));
    item->setText(colLat, printPos(w->origP.lat(), true));
    item->setText(colLong, printPos(w->origP.lon(), false));
    tmp.sprintf("%d", w->elevation);
    item->setText(colElev, tmp);
    w->frequency > 1 ? tmp.sprintf("%.3f", w->frequency) : tmp=QString::null;
    item->setText(colFrequency, tmp);
    item->setText(colLandable, w->isLandable == true ? tr("Yes") : QString::null);
    if (w->runway > -1) {
      tmp.sprintf("%02d", w->runway);
    }
    else {
      tmp = QString::null;
    }
    item->setText(colRunway, tmp);
    if (w->length > -1) {
      tmp.sprintf("%02d", w->length);
    }
    else {
      tmp = QString::null;
    }
    item->setText(colLength, tmp);

    item->setText(colSurface, w->surface == -1 ? QString::null : surfaceTypes.itemText(w->surface));
    item->setText(colComment, w->comment);
    item->setPixmap(colName, _globalMapConfig.getPixmap(w->type,false,true));
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
  if (w->save()) {
    catalogName->changeItem(w->path, catalogName->currentItem());
  }
}

void Waypoints::slotSaveWaypointCatalogAs()
{
  WaypointCatalog *w = waypointCatalogs.current();
  if (w->save(true)) {
    catalogName->changeItem(w->path, catalogName->currentItem());
  }
}

void Waypoints::slotImportWaypointCatalog()
{
  extern QSettings _settings;
  QString wayPointDir = _settings.readEntry("/KFLog/Path/DefaultWaypointDirectory",
                                          getpwuid(getuid())->pw_dir);

  QString filter;
  filter.append(tr("KFLog waypoints")+" (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("All files")+" (*.*)");
  QString fName = QFileDialog::getOpenFileName(wayPointDir, filter, this, tr("Import waypoint catalog"));

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
    switch(QMessageBox::warning(this, tr("Save"),
           "<qt>" + tr("Save changes to<BR><B>%1</B>").arg(w->path) + "</qt>",
           QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel)) {
    case QMessageBox::Yes:
      if (!w->save()) {
        return;
      }
      break;
    case QMessageBox::Cancel:
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
  Airport *a;
  GliderSite *gs;
  Waypoint *w;
  WaypointCatalog *c = waypointCatalogs.current();
  WaypointDict *wl = &(c->wpList);
  unsigned int i;
  int type, loop;
  WGSPoint p;
  QString tmp;
  QRegExp blank("[ ]");
  QValueList<int> searchLists;
  QValueList<int>::Iterator searchListsIt;
  bool filterRadius, filterArea;

  if (importFilterDlg->exec() == QDialog::Accepted) {
    getFilterData();

    if (c->showAll || c->showAirports) {
      searchLists.append(MapContents::AirportList);
    }
    if (c->showAll || c->showGliderSites) {
      searchLists.append(MapContents::GliderSiteList);
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

        w = new Waypoint;

        w->name = s->getName().replace(blank, QString::null).left(6).upper();
        loop = 0;
        while (wl->find(w->name) && loop < 100000) {
          tmp.setNum(loop++);
          w->name = w->name.left(6 - tmp.length()) + tmp;
        }
        w->description = s->getName();
        type = s->getTypeID();
        w->type = type;

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
          a = dynamic_cast<Airport*>(s); //try casting to an airfield
          if (a) {
            if (a->getRunwayNumber()) {
              runway* r = a->getRunway(0);
              if (r) {
                w->runway = r->direction;
                w->length = r->length;
                w->surface = r->surface;
              }
            }
          } else { //try casting to an airfield did not work, try glider site
            gs = dynamic_cast<GliderSite*>(s);
            if (gs) {
              if (gs->getRunwayNumber()) {
                runway* r = gs->getRunway(0);
                if (r) {
                  w->runway = r->direction;
                  w->length = r->length;
                  w->surface = r->surface;
                }
              }
            }
          }
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
  // @AP: check, if a WaypointCatalog is available, otherwise a core
  // dump will occure

  if (!waypointCatalogs.current()) {
    return; // no catalog loaded
  }

  if (importFilterDlg->exec() == QDialog::Accepted) {
    getFilterData();
    fillWaypoints();
  }
}

/** add a new waypoint from outside */
void Waypoints::slotAddWaypoint(Waypoint *w)
{
  if (!waypointCatalogs.current()) { //let's make sure we have a waypointcatalog
    WaypointCatalog * wpc=new WaypointCatalog(tr("unnamed"));
    slotAddCatalog(wpc);
  }

  WaypointDict *wl = &waypointCatalogs.current()->wpList;
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
    WaypointDict *wl = &waypointCatalogs.current()->wpList;
    Waypoint *w = wl->find(item->text(colName));

    emit copyWaypoint2Task(w);
  }
}

void Waypoints::slotCenterMap()
{
  QListViewItem *item = waypoints->currentItem();

  if (item != 0)
    {
      WaypointDict *wl = &waypointCatalogs.current()->wpList;
      Waypoint *w = wl->find(item->text(colName));

      emit centerMap(w->origP.lat(), w->origP.lon());
    }
}

void Waypoints::slotSetHome()
{
  QListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    WaypointDict *wl = &waypointCatalogs.current()->wpList;
    Waypoint *w = wl->find(item->text(colName));

    extern QSettings _settings;
    _settings.writeEntry("/KFLog/MapData/Homesite", w->name);
    _settings.writeEntry("/KFLog/MapData/HomesiteLatitude", w->origP.lat());
    _settings.writeEntry("/KFLog/MapData/HomesiteLongitude", w->origP.lon());

    // update airfield lists from Welt2000 if home site changes:
    extern MapContents  _globalMapContents;
    _globalMapContents.slotReloadMapData();
  }
}

void Waypoints::getFilterData()
{
  WGSPoint p;
  extern QSettings _settings;
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
    c->radiusLat = _settings.readNumEntry("/KFLog/MapData/HomesiteLatitude");
    c->radiusLong = _settings.readNumEntry("/KFLog/MapData/HomesiteLongitude");
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
  extern QSettings _settings;
  QString wayPointDir = _settings.readEntry("/KFLog/Path/DefaultWaypointDirectory",
                                          getpwuid(getuid())->pw_dir);

   // we should not include types we don't support (yet). Also, the strings should be translated.
  QString filter;
  filter.append(tr("Volkslogger format")+" (*.dbt *.DBT)");//;;");
//  filter.append(tr("Garmin format")+" (*.gdn *.GDN);;");
//  filter.append(tr("All files")+" (*.*)");
  QString fName = QFileDialog::getOpenFileName(wayPointDir, filter, this, tr("Import waypoints from file"));

  if(!fName.isEmpty()) {
    WaypointCatalog *w = waypointCatalogs.current();

    // read from disk
    w->modified = true;
    if (fName.right(4).lower() == ".dbt"){
            w->importVolkslogger(fName);
      //    } else if (fName.right(4).lower() == "*.gdn"){
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
    WaypointCatalog *w = new WaypointCatalog("");
    QFile f(catalog);

    if (f.exists()) {
      // read from disk
      if (!w->load(catalog)) {
        delete w;
      }
      else {
        waypointCatalogs.append(w);
        catalogName->insertItem(w->path);

        catalogName->setCurrentItem(newItem);
        qDebug(".....");
        slotSwitchWaypointCatalog(newItem);
      }
    }
    else {
      delete w;
    }
  }
}

/* slot to set name of catalog and open it without a file selection dialog */
void Waypoints::slotSetWaypointCatalogName(QString catalog)
{
  if (!catalog.isEmpty()) {
    openCatalog(catalog);
  }
  else {
    slotNewWaypointCatalog();
  }
}

/** return the current waypoint catalog */
WaypointCatalog * Waypoints::getCurrentCatalog()
{
  return waypointCatalogs.current();
}

void Waypoints::slotAddCatalog(WaypointCatalog *w)
{
  int newItem = catalogName->count();

  waypointCatalogs.append(w);
  catalogName->insertItem(w->path);

  catalogName->setCurrentItem(newItem);
  slotSwitchWaypointCatalog(newItem);
}
