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

#include <pwd.h>

#include <QtGui>
#include <Qt3Support>

#include "airfield.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "runway.h"
#include "translationlist.h"
#include "waypoints.h"
#include "wgspoint.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

Waypoints::Waypoints(QWidget *parent, const char *name, const QString& /*catalog*/)
  : QFrame(parent, name)
{
  currentWaypointCatalog = 0;
  addWaypointWindow(this);
  addPopupMenu();

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
    while(!waypointCatalogs.isEmpty())
        delete waypointCatalogs.takeFirst();
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

  waypoints->setColumnAlignment(colElev, Qt::AlignRight);
  waypoints->setColumnAlignment(colFrequency, Qt::AlignRight);
  waypoints->setColumnAlignment(colRunway, Qt::AlignRight);
  waypoints->setColumnAlignment(colLength, Qt::AlignRight);

  waypoints->loadConfig();

  connect(waypoints, SIGNAL(rightButtonPressed(Q3ListViewItem *, const QPoint &, int)),
          SLOT(showWaypointPopup(Q3ListViewItem *, const QPoint &, int)));
  connect(waypoints, SIGNAL(doubleClicked(Q3ListViewItem *)),
          SLOT(slotEditWaypoint()));

  // header
  QHBoxLayout *header = new QHBoxLayout(5);
  QLabel *l = new QLabel(QString("%1:").arg(tr("Waypoints")), parent);
  l->setMaximumWidth(l->sizeHint().width() + 5);

  catalogName = new QComboBox(false, parent);
  connect(catalogName, SIGNAL(activated(int)), SLOT(slotSwitchWaypointCatalog(int)));

  QPushButton *fileOpen = new QPushButton(parent);
  fileOpen->setIcon(_mainWindow->getPixmap("kde_fileopen_16.png"));
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
  wayPointPopup = new Q3PopupMenu(waypoints);
  catalogCopySubPopup = new Q3PopupMenu(waypoints);
  catalogMoveSubPopup = new Q3PopupMenu(waypoints);


//  wayPointPopup->insertTitle(SmallIcon("waypoint"), "Waypoint's", 0);
  wayPointPopup->insertItem(_mainWindow->getPixmap("waypoint_16.png"), tr("&New catalog"), this, SLOT(slotNewWaypointCatalog()));
  wayPointPopup->insertItem(_mainWindow->getPixmap("kde_fileopen_16.png"), tr("&Open catalog"), this, SLOT(slotOpenWaypointCatalog()));
  idWaypointCatalogImport = wayPointPopup->insertItem(tr("&Import catalog"), this, SLOT(slotImportWaypointCatalog()));
  idWaypointImportFromMap = wayPointPopup->insertItem(tr("Import from &map"), this, SLOT(slotImportWaypointFromMap()));
  idWaypointImportFromFile = wayPointPopup->insertItem(tr("Import from &file"), this, SLOT(slotImportWaypointFromFile()));
  idWaypointCatalogSave = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_filesave_16.png"), tr("&Save catalog"), this, SLOT(slotSaveWaypointCatalog()));
  idWaypointCatalogSaveAs = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_filesaveas_16.png"), tr("&Save catalog as..."), this, SLOT(slotSaveWaypointCatalogAs()));
  idWaypointCatalogClose = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_fileclose_16.png"), tr("&Close catalog"), this, SLOT(slotCloseWaypointCatalog()));
  wayPointPopup->insertSeparator();
  idWaypointNew = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_filenew_16.png"), tr("New &waypoint"), this, SLOT(slotNewWaypoint()));
  idWaypointEdit = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_wizard_16.png"), tr("&Edit waypoint"), this, SLOT(slotEditWaypoint()));
  idWaypointDelete = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_editdelete_16.png"), tr("&Delete waypoint"), this, SLOT(slotDeleteWaypoint()));
  idWaypointCopy2Catalog = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_editcopy_16.png"), tr("Copy to &catalog"), catalogCopySubPopup);
  idWaypointMove2Catalog = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_move_16.png"), tr("Move to &catalog"), catalogMoveSubPopup);

  wayPointPopup->insertSeparator();
  idWaypointCopy2Task = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_editcopy_16.png"), tr("Copy to &task"), this, SLOT(slotCopyWaypoint2Task()));
  idWaypointCenterMap = wayPointPopup->insertItem(_mainWindow->getPixmap("centerwaypoint_16.png"), tr("Center map on waypoint"), this, SLOT(slotCenterMap()));
  idWaypointSetHome = wayPointPopup->insertItem(_mainWindow->getPixmap("kde_gohome_16.png"), tr("Set Homesite"), this, SLOT(slotSetHome()));

  connect(catalogCopySubPopup, SIGNAL(activated(int)), this, SLOT(slotCopy2Catalog(int)));
  connect(catalogMoveSubPopup, SIGNAL(activated(int)), this, SLOT(slotMove2Catalog(int)));
}

/**
 * Copies the current waypoint to the selected catalog
 */
void Waypoints::slotCopy2Catalog(int id){
  Q3ListViewItem *item = waypoints->currentItem();
  Waypoint *wpt;

  if(item != 0) {
    QString tmp = item->text(colName);
    wpt=currentWaypointCatalog->findWaypoint(tmp);
    waypointCatalogs.at(id)->wpList.append(new Waypoint(wpt));
    waypointCatalogs.at(id)->modified=true;
  }

}

/**
 * Moves the current waypoint to the selected catalog
 */
void Waypoints::slotMove2Catalog(int id){
  Q3ListViewItem *item = waypoints->currentItem();
  Waypoint * wpt;

  if(item != 0) {
    QString tmp = item->text(colName);
    wpt=currentWaypointCatalog->findWaypoint(tmp);
    waypointCatalogs.at(id)->wpList.append(new Waypoint(wpt));
    currentWaypointCatalog->removeWaypoint(tmp);
    currentWaypointCatalog->modified = true;
    waypointCatalogs.value(id)->modified = true;
    delete item;
  }

}

/** open a catalog and set it active */
void Waypoints::slotOpenWaypointCatalog()
{
  extern QSettings _settings;
  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

  QString filter;
  filter.append(tr("All supported waypoint formats")+" (*.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP *.txt *.TXT);;");
  filter.append(tr("KFLog waypoints")+" (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Cumulus and KFLogEmbedded waypoints")+" (*.kwp *.KWP);;");
  filter.append(tr("Filser txt waypoints")+" (*.txt *.TXT);;");
  filter.append(tr("Filser da4 waypoints")+" (*.da4 *.DA4);;");
  filter.append(tr("SeeYou cup waypoints")+" (*.cup *.CUP)");
  QString fName = Q3FileDialog::getOpenFileName(wayPointDir, filter, this, tr("Open waypoint catalog"));

  openCatalog(fName);
}

void Waypoints::showWaypointPopup(Q3ListViewItem *it, const QPoint &, int)
{
  //enable and disable the correct menuitems
  wayPointPopup->setItemEnabled(idWaypointCatalogSave, waypointCatalogs.count() && currentWaypointCatalog->modified);
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
  int curCat=waypointCatalogs.indexOf(currentWaypointCatalog);
  for (int i=0;i<waypointCatalogs.count();i++) {
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
  foreach(w, waypointCatalogs)
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
  Q3ListViewItem *item = waypoints->currentItem();
  QString oldName;

  if (item != 0) {
    oldName = item->text(colName);
    Waypoint *w = currentWaypointCatalog->findWaypoint(oldName);
    slotEditWaypoint (w);
  }
}

/** No descriptions */
void Waypoints::slotEditWaypoint(Waypoint* w)
{
  if (w)
  {
    QString tmp;

    // initialize dialg
    waypointDlg->name->setText(w->name);
    waypointDlg->description->setText(w->description);
    // translate id to index
    waypointDlg->setWaypointType(w->type);
    waypointDlg->longitude->setText(WGSPoint::printPos(w->origP.lon(), false));
    waypointDlg->latitude->setText(WGSPoint::printPos(w->origP.lat(), true));
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
        w->origP.setLat(WGSPoint::degreeToNum(waypointDlg->latitude->text()));
        w->origP.setLon(WGSPoint::degreeToNum(waypointDlg->longitude->text()));
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

        currentWaypointCatalog->modified = true;
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
      currentWaypointCatalog->removeWaypoint(wp->name);
      currentWaypointCatalog->modified = true;
      fillWaypoints();
    }
  }
}

/** No descriptions */
void Waypoints::slotDeleteWaypoint()
{
  Q3ListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    QString tmp = item->text(colName);
    QMessageBox waypointBox(tr("Delete waypoint?"),
                            tr("<qt>Waypoint <b>%1</b> will be deleted.<br>Are you sure?</qt>").arg(tmp),
                            QMessageBox::Warning, QMessageBox::Ok, QMessageBox::Cancel, 0);
    waypointBox.setButtonText(QMessageBox::Ok, tr("&Delete"));
    if (waypointBox.exec() == QMessageBox::Ok)
    {
      currentWaypointCatalog->removeWaypoint(tmp);
      currentWaypointCatalog->modified = true;
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
  Q3ListViewItem *item;
  bool filterRadius, filterArea;
  extern TranslationList waypointTypes;
  extern MapConfig *_globalMapConfig;

  waypoints->clear();

  filterRadius = (currentWaypointCatalog->radiusLat != 1  || currentWaypointCatalog->radiusLong != 1);
  filterArea = (currentWaypointCatalog->areaLat2 != 1 && currentWaypointCatalog->areaLong2 != 1 && !filterRadius);

  foreach(w, currentWaypointCatalog->wpList) {
    if (!currentWaypointCatalog->showAll) {
      switch(w->type) {
      case BaseMapElement::IntAirport:
      case BaseMapElement::Airport:
      case BaseMapElement::MilAirport:
      case BaseMapElement::CivMilAirport:
      case BaseMapElement::Airfield:
        if (!currentWaypointCatalog->showAirports) {
          continue;
        }
        break;
      case BaseMapElement::Gliderfield:
        if (!currentWaypointCatalog->showGliderSites) {
          continue;
        }
        break;
      case BaseMapElement::UltraLight:
      case BaseMapElement::HangGlider:
      case BaseMapElement::Parachute:
      case BaseMapElement::Balloon:
        if (!currentWaypointCatalog->showOtherSites) {
          continue;
        }
        break;
      }
    }

    if (filterArea) {
      if (w->origP.lat() < currentWaypointCatalog->areaLat1 || w->origP.lat() > currentWaypointCatalog->areaLat2 ||
          w->origP.lon() < currentWaypointCatalog->areaLong1 || w->origP.lon() > currentWaypointCatalog->areaLong2) {
        continue;
      }
    }
    else if (filterRadius) {
      if (dist(currentWaypointCatalog->radiusLat, currentWaypointCatalog->radiusLong, w->origP.lat(), w->origP.lon()) > currentWaypointCatalog->radiusSize) {
        continue;
      }
    }
    item = new Q3ListViewItem(waypoints);
    item->setText(colName, w->name);
    item->setText(colDesc, w->description);
    item->setText(colICAO, w->icao);
    item->setText(colType, w->type == -1 ? QString::null : waypointTypes.itemText(w->type));
    item->setText(colLat, WGSPoint::printPos(w->origP.lat(), true));
    item->setText(colLong, WGSPoint::printPos(w->origP.lon(), false));
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

    item->setText(colSurface, Runway::item2Text(w->surface) );
    item->setText(colComment, w->comment);
    item->setPixmap(colName, _globalMapConfig->getPixmap(w->type,false,true));
  }
  emit waypointCatalogChanged(currentWaypointCatalog);
}

/** No descriptions */
void Waypoints::slotSwitchWaypointCatalog(int idx)
{
  currentWaypointCatalog = waypointCatalogs.value(idx);
  fillWaypoints();
}

void Waypoints::slotSaveWaypointCatalog()
{
  if(currentWaypointCatalog->save())
    catalogName->changeItem(currentWaypointCatalog->path, catalogName->currentItem());
}

void Waypoints::slotSaveWaypointCatalogAs()
{
  if(currentWaypointCatalog->save(true))
    catalogName->changeItem(currentWaypointCatalog->path, catalogName->currentItem());
}

void Waypoints::slotImportWaypointCatalog()
{
  extern QSettings _settings;
  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

  QString filter;
  filter.append(tr("KFLog waypoints")+" (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("All files")+" (*.*)");
  QString fName = Q3FileDialog::getOpenFileName(wayPointDir, filter, this, tr("Import waypoint catalog"));

  if(!fName.isEmpty()) {
    // read from disk
    currentWaypointCatalog->read(fName);
    currentWaypointCatalog->modified = true;
    fillWaypoints();
  }
}

void Waypoints::slotCloseWaypointCatalog()
{
  int idx = waypointCatalogs.indexOf(currentWaypointCatalog);
  int cnt;

  if(currentWaypointCatalog->modified) {
    switch(QMessageBox::warning(this, tr("Save"),
           "<qt>" + tr("Save changes to<BR><B>%1</B>").arg(currentWaypointCatalog->path) + "</qt>",
           QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel)) {
    case QMessageBox::Yes:
      if (!currentWaypointCatalog->save()) {
        return;
      }
      break;
    case QMessageBox::Cancel:
      return;
    }
  }

  waypointCatalogs.removeOne(currentWaypointCatalog);
  delete currentWaypointCatalog;

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
  Airfield *a;
  Waypoint *w;
  QList<Waypoint*> wl = currentWaypointCatalog->wpList;
  int type, loop;
  WGSPoint p;
  QString tmp;
  QRegExp blank("[ ]");
  Q3ValueList<int> searchLists;
  Q3ValueList<int>::Iterator searchListsIt;
  bool filterRadius, filterArea;

  if (importFilterDlg->exec() == QDialog::Accepted) {
    getFilterData();

    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showAirports) {
      searchLists.append(MapContents::AirfieldList);
    }
    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showGliderSites) {
      searchLists.append(MapContents::GliderfieldList);
    }
    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showOtherSites) {
      searchLists.append(MapContents::AddSitesList);
    }
    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showObstacle) {
      searchLists.append(MapContents::ObstacleList);
    }
    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showLandmark) {
      searchLists.append(MapContents::LandmarkList);
    }
    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showOutlanding) {
      searchLists.append(MapContents::OutLandingList);
    }
    if (currentWaypointCatalog->showAll || currentWaypointCatalog->showStation) {
      searchLists.append(MapContents::StationList);
    }

    filterRadius = (currentWaypointCatalog->radiusLat != 1  || currentWaypointCatalog->radiusLong != 1);
    filterArea = (currentWaypointCatalog->areaLat2 != 1 && currentWaypointCatalog->areaLong2 != 1 && !filterRadius);

    for (searchListsIt =searchLists.begin(); searchListsIt != searchLists.end(); ++searchListsIt) {
      for (int i = 0; i < _globalMapContents->getListLength(*searchListsIt); i++) {

        s = (SinglePoint *)_globalMapContents->getElement(*searchListsIt, i);
        p = s->getWGSPosition();

        // check area
        if (filterArea) {
          if (p.lon() < currentWaypointCatalog->areaLong1 || p.lon() > currentWaypointCatalog->areaLong2 ||
              p.lat() < currentWaypointCatalog->areaLat1 || p.lat() > currentWaypointCatalog->areaLat2) {
            continue;
          }
        }
        else if (filterRadius) {
          if (dist(currentWaypointCatalog->radiusLat, currentWaypointCatalog->radiusLong, p.lat(), p.lon()) > currentWaypointCatalog->radiusSize) {
            continue;
          }
        }

        w = new Waypoint;

        QString name = s->getName();
        w->name = name.replace(blank, "").left(6).upper();
        loop = 0;

        while (currentWaypointCatalog->findWaypoint(w->name) && loop < 100000) {
          tmp.setNum(loop++);
          w->name = w->name.left(6 - tmp.length()) + tmp;
        }
        w->description = s->getName();
        type = s->getObjectType();
        w->type = type;

        w->origP = p;

        w->elevation = s->getElevation();

        switch(type) {
        case BaseMapElement::IntAirport:
        case BaseMapElement::Airport:
        case BaseMapElement::MilAirport:
        case BaseMapElement::CivMilAirport:
        case BaseMapElement::Airfield:
        case BaseMapElement::Gliderfield:

#warning "Check convertion here"

          w->icao = ((Airfield *) s)->getICAO();
          w->frequency = ((Airfield *) s)->getFrequency().toDouble();
          w->isLandable = true;
          a = dynamic_cast<Airfield*>(s); //try casting to an airfield

          if( a )
            {
              if( a->getRunwayNumber() )
                {
                  Runway* runway = a->getRunway( 0 );

                  if( runway )
                    {
                      w->runway = runway->getRunwayDirection().first;
                      w->length = runway->length;
                      w->surface = runway->surface;
                    }
                }
          }

          break;
        default:
          w->isLandable = false;
        }

        currentWaypointCatalog->wpList.append(w);
      }
    }

    currentWaypointCatalog->modified = true;
    fillWaypoints();
  }
}

/** filter waypoints to display */
void Waypoints::slotFilterWaypoints()
{
  // @AP: check, if a WaypointCatalog is available, otherwise a core
  // dump will occure

  if (!currentWaypointCatalog) {
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
  if (!currentWaypointCatalog) { //let's make sure we have a waypointcatalog
    WaypointCatalog * wpc=new WaypointCatalog(tr("unnamed"));
    slotAddCatalog(wpc);
  }

  int loop = 1;
  if (w->name.isEmpty()) {
    w->name.sprintf("WPT%03d", loop);
    while(currentWaypointCatalog->findWaypoint(w->name) && loop < 1000) {
      w->name.sprintf("WPT%03d", ++loop);
    }
  }

  currentWaypointCatalog->wpList.append(w);
  currentWaypointCatalog->modified = true;
  fillWaypoints();
}

void Waypoints::slotCopyWaypoint2Task()
{
  Q3ListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    Waypoint *w = currentWaypointCatalog->findWaypoint(item->text(colName));

    emit copyWaypoint2Task(w);
  }
}

void Waypoints::slotCenterMap()
{
  Q3ListViewItem *item = waypoints->currentItem();

  if (item != 0)
    {
      Waypoint *w = currentWaypointCatalog->findWaypoint(item->text(colName));

      emit centerMap(w->origP.lat(), w->origP.lon());
    }
}

void Waypoints::slotSetHome()
{
  Q3ListViewItem *item = waypoints->currentItem();

  if (item != 0) {
    Waypoint *w = currentWaypointCatalog->findWaypoint(item->text(colName));

    extern QSettings _settings;
    _settings.setValue("/MapData/Homesite", w->name);
    _settings.setValue("/MapData/HomesiteLatitude", w->origP.lat());
    _settings.setValue("/MapData/HomesiteLongitude", w->origP.lon());

    // update airfield lists from Welt2000 if home site changes:
    extern MapContents  *_globalMapContents;
    _globalMapContents->slotReloadMapData();
  }
}

void Waypoints::getFilterData()
{
  WGSPoint p;
  extern QSettings _settings;

  currentWaypointCatalog->showAll = importFilterDlg->useAll->isChecked();
  currentWaypointCatalog->showAirports = importFilterDlg->airports->isChecked();
  currentWaypointCatalog->showGliderSites = importFilterDlg->gliderSites->isChecked();
  currentWaypointCatalog->showOtherSites = importFilterDlg->otherSites->isChecked();
  currentWaypointCatalog->showObstacle = importFilterDlg->obstacle->isChecked();
  currentWaypointCatalog->showLandmark = importFilterDlg->landmark->isChecked();
  currentWaypointCatalog->showOutlanding = importFilterDlg->outlanding->isChecked();
  currentWaypointCatalog->showStation = importFilterDlg->station->isChecked();

  currentWaypointCatalog->areaLat1 = WGSPoint::degreeToNum(importFilterDlg->fromLat->text());
  currentWaypointCatalog->areaLat2 = WGSPoint::degreeToNum(importFilterDlg->toLat->text());
  currentWaypointCatalog->areaLong1 = WGSPoint::degreeToNum(importFilterDlg->fromLong->text());
  currentWaypointCatalog->areaLong2 = WGSPoint::degreeToNum(importFilterDlg->toLong->text());

  switch (importFilterDlg->getCenterRef()) {
  case CENTER_POS:
    currentWaypointCatalog->radiusLat = WGSPoint::degreeToNum(importFilterDlg->posLat->text());
    currentWaypointCatalog->radiusLong = WGSPoint::degreeToNum(importFilterDlg->posLong->text());
    break;
  case CENTER_HOMESITE:
    currentWaypointCatalog->radiusLat = _settings.readNumEntry("/MapData/HomesiteLatitude");
    currentWaypointCatalog->radiusLong = _settings.readNumEntry("/MapData/HomesiteLongitude");
    break;
  case CENTER_MAP:
    p = _globalMapMatrix->getMapCenter(false);
    currentWaypointCatalog->radiusLat = p.lat();
    currentWaypointCatalog->radiusLong = p.lon();
    break;
  case CENTER_AIRPORT:
    p = importFilterDlg->getAirportRef();
    currentWaypointCatalog->radiusLat = p.lat();
    currentWaypointCatalog->radiusLong = p.lon();
    break;
  }

  currentWaypointCatalog->radiusSize = importFilterDlg->radius->currentText().toDouble();

  // normalize coordinates
  if (currentWaypointCatalog->areaLat1 > currentWaypointCatalog->areaLat2) {
    int tmp = currentWaypointCatalog->areaLat1;
    currentWaypointCatalog->areaLat1 = currentWaypointCatalog->areaLat2;
    currentWaypointCatalog->areaLat2 = tmp;
  }

  if (currentWaypointCatalog->areaLong1 > currentWaypointCatalog->areaLong2) {
    int tmp = currentWaypointCatalog->areaLong1;
    currentWaypointCatalog->areaLong1 = currentWaypointCatalog->areaLong2;
    currentWaypointCatalog->areaLong2 = tmp;
  }
}
/** No descriptions */
void Waypoints::slotImportWaypointFromFile(){
  extern QSettings _settings;

  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

   // we should not include types we don't support (yet). Also, the strings should be translated.
  QString filter;
  filter.append(tr("Volkslogger format")+" (*.dbt *.DBT)");//;;");
//  filter.append(tr("Garmin format")+" (*.gdn *.GDN);;");
//  filter.append(tr("All files")+" (*.*)");
  QString fName = Q3FileDialog::getOpenFileName(wayPointDir, filter, this, tr("Import waypoints from file"));

  if(!fName.isEmpty()) {

    // read from disk
    currentWaypointCatalog->modified = true;
    if (fName.right(4).lower() == ".dbt"){
            currentWaypointCatalog->importVolkslogger(fName);
      //    } else if (fName.right(4).lower() == "*.gdn"){
      //    currentWaypointCatalog->importGarmin(fName);
    } else {
      currentWaypointCatalog->modified = false;
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
        currentWaypointCatalog = w;
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
WaypointCatalog *Waypoints::getCurrentCatalog()
{
  return currentWaypointCatalog;
}

void Waypoints::slotAddCatalog(WaypointCatalog *w)
{
  int newItem = catalogName->count();

  waypointCatalogs.append(w);
  catalogName->insertItem(w->path);

  catalogName->setCurrentItem(newItem);
  slotSwitchWaypointCatalog(newItem);
}
