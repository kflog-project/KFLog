/***************************************************************************
                          taskandwaypoint.cpp  -  description
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

#include "taskandwaypoint.h"
#include "basemapelement.h"
#include "airport.h"
#include "waypointelement.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "airport.h"
#include "glidersite.h"
#include "waypointimpfilterdialog.h"

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#define COL_WAYPOINT_NAME 0
#define COL_WAYPOINT_DESCRIPTION 1
#define COL_WAYPOINT_ICAO 2
#define COL_WAYPOINT_TYPE 3
#define COL_WAYPOINT_LAT 4
#define COL_WAYPOINT_LONG 5
#define COL_WAYPOINT_ELEV 6
#define COL_WAYPOINT_FREQ 7
#define COL_WAYPOINT_IS_LANDABLE 8
#define COL_WAYPOINT_RUNWAY 9
#define COL_WAYPOINT_LENGTH 10
#define COL_WAYPOINT_SURFACE 11
#define COL_WAYPOINT_COMMENT 12

extern MapContents _globalMapContents;
extern MapMatrix _globalMapMatrix;

TaskAndWaypoint::TaskAndWaypoint(QWidget *parent, const char *name )
  : KDialog(parent, name, true)
{
  TranslationElement *te;

  setCaption(i18n("Task & Waypoints"));

  QVBoxLayout *layout = new QVBoxLayout(this);
  QHBoxLayout *buttons = new QHBoxLayout(10);
  QSplitter *split = new QSplitter(this);
  split->setOrientation(Vertical);

  initSurfaces();
  initTypes();
  addTaskWindow(split);
  addWaypointWindow(split);
  addPopupMenu();

  QPushButton* close = new QPushButton(i18n("Close"), this);
  connect(close, SIGNAL(clicked()), SLOT(reject()));

  buttons->addStretch();
  buttons->addWidget(close);
  buttons->addSpacing(5);

  layout->addWidget(split);
  layout->addLayout(buttons);
  layout->addSpacing(5);
  setMinimumSize(800, 500);

  waypointCatalogs.setAutoDelete(true);
  slotNewWaypointCatalog();

  waypointDlg = new WaypointDialog(this);
  connect(waypointDlg, SIGNAL(addWaypoint()), SLOT(slotAddWaypoint()));

  // init comboboxes
  for (te = waypointTypes.first(); te != 0; te = waypointTypes.next()) {
    waypointDlg->waypointType->insertItem(te->text);
  }

  for (te = surfaces.first(); te != 0; te = surfaces.next()) {
    waypointDlg->surface->insertItem(te->text);
  }

}

TaskAndWaypoint::~TaskAndWaypoint()
{
}

void TaskAndWaypoint::initSurfaces()
{
  surfaces.setAutoDelete(true);

  surfaces.append(new TranslationElement(Airport::NotSet, i18n("Unknown")));
  surfaces.append(new TranslationElement(Airport::Grass, i18n("Grass")));
  surfaces.append(new TranslationElement(Airport::Asphalt, i18n("Asphalt")));
  surfaces.append(new TranslationElement(Airport::Concrete, i18n("Concrete")));

  surfaces.sort();
}

void TaskAndWaypoint::initTypes()
{
  waypointTypes.setAutoDelete(true);

  // don't know if we really need all of them
  waypointTypes.append(new TranslationElement(BaseMapElement::AerialRailway, i18n("Aerial railway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Airfield, i18n("Airfield")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Airport, i18n("Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::AmbHeliport, i18n("Ambul. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Ballon, i18n("Ballon")));
  waypointTypes.append(new TranslationElement(BaseMapElement::City, i18n("City")));
  waypointTypes.append(new TranslationElement(BaseMapElement::CivHeliport, i18n("Civil Heliport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::IntAirport, i18n("Int. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::MilAirport, i18n("Mil. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::CivMilAirport, i18n("Civil/Mil. Airport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::ClosedAirfield, i18n("Closed Airfield")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Glidersite, i18n("Glider site")));
  waypointTypes.append(new TranslationElement(BaseMapElement::HangGlider, i18n("Hang glider")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Highway, i18n("Highway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Landmark, i18n("Landmark")));
  waypointTypes.append(new TranslationElement(BaseMapElement::MilHeliport, i18n("Mil. Heliport")));
  waypointTypes.append(new TranslationElement(BaseMapElement::UltraLight, i18n("Ultralight")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Parachute, i18n("Parachute")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Outlanding, i18n("Outlanding")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Obstacle, i18n("Obstacle")));
  waypointTypes.append(new TranslationElement(BaseMapElement::ObstacleGroup, i18n("Obstacle group")));
  waypointTypes.append(new TranslationElement(BaseMapElement::LightObstacleGroup, i18n("Obstacle group (lighted)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::LightObstacle, i18n("Obstacle (lighted)")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Railway, i18n("Railway")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Road, i18n("Road")));
  waypointTypes.append(new TranslationElement(BaseMapElement::Village, i18n("Village")));

  waypointTypes.sort();
}

/** No descriptions */
void TaskAndWaypoint::addTaskWindow(QSplitter *s)
{
  QWidget *w = new QWidget(s);
  KFLogTable *t =  new KFLogTable(1, 1, w, "tasks");
  QVBoxLayout *layout = new QVBoxLayout(w, 5, 5);

  connect(t, SIGNAL(clicked(int, int, int, const QPoint &)),
    SLOT(showTaskPopup(int, int, int, const QPoint &)));
  connect(t, SIGNAL(doubleClicked(int, int, int, const QPoint &)),
    SLOT(slotNotHandledItem()));

  layout->addWidget(new QLabel(i18n("%1:").arg("Tasks"), w));
  layout->addWidget(t);
}

/** No descriptions */
void TaskAndWaypoint::addWaypointWindow(QSplitter *s)
{
  QWidget *w = new QWidget(s);
  waypoints =  new KFLogTable(0, 13, w, "waypoints");

  QHeader *h = waypoints->horizontalHeader();
  h->setLabel(COL_WAYPOINT_NAME, i18n("Name"));
  h->setLabel(COL_WAYPOINT_DESCRIPTION, i18n("Description"));
  h->setLabel(COL_WAYPOINT_ICAO, i18n("ICAO"));
  h->setLabel(COL_WAYPOINT_TYPE, i18n("Type"));
  h->setLabel(COL_WAYPOINT_LAT, i18n("Latitude"));
  h->setLabel(COL_WAYPOINT_LONG, i18n("Longitude"));
  h->setLabel(COL_WAYPOINT_ELEV, i18n("Elevation (m)"));
  h->setLabel(COL_WAYPOINT_FREQ, i18n("Frequency"));
  h->setLabel(COL_WAYPOINT_IS_LANDABLE, i18n("Landable"));
  h->setLabel(COL_WAYPOINT_RUNWAY, i18n("Runway"));
  h->setLabel(COL_WAYPOINT_LENGTH, i18n("Length (m)"));
  h->setLabel(COL_WAYPOINT_SURFACE, i18n("Surface"));
  h->setLabel(COL_WAYPOINT_COMMENT, i18n("Comment"));

  connect(waypoints, SIGNAL(pressed(int, int, int, const QPoint &)),
    SLOT(showWaypointPopup(int, int, int, const QPoint &)));
  connect(waypoints, SIGNAL(doubleClicked(int, int, int, const QPoint &)),
    SLOT(slotEditWaypoint()));

  // header
  QHBoxLayout *header = new QHBoxLayout(5);
  QLabel *l = new QLabel(i18n("%1:").arg("Waypoints"), w);
  l->setMaximumWidth(l->sizeHint().width() + 5);

  catalogName = new QComboBox(false, w);
  connect(catalogName, SIGNAL(activated(int)), SLOT(slotSwitchWaypointCatalog(int)));

  QPushButton *fileOpen = new QPushButton(w);
  fileOpen->setPixmap(BarIcon("fileopen"));
  QSizePolicy sp = fileOpen->sizePolicy();
  sp.setHorData(QSizePolicy::Fixed);
  fileOpen->setSizePolicy(sp);
  connect(fileOpen, SIGNAL(clicked()), SLOT(slotOpenWaypointCatalog()));

  header->addWidget(l);
  header->addWidget(catalogName);
  header->addWidget(fileOpen);

  QVBoxLayout *layout = new QVBoxLayout(w, 5, 5);

  layout->addLayout(header);
  layout->addWidget(waypoints);
}

/** No descriptions */
void TaskAndWaypoint::addPopupMenu()
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
    SLOT(slotNotHandledItem()));
}

/** open a catalog and set it active */
void TaskAndWaypoint::slotOpenWaypointCatalog()
{
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString wayPointDir = config->readEntry("DefaultWaypointDirectory",
      getpwuid(getuid())->pw_dir);

  QString fName = KFileDialog::getOpenFileName(wayPointDir, "*.kflogwp *.KFLOGWP|KFLog waypoints (*.kflogwp)", this, i18n("Open waypoint catalog"));

  if(!fName.isEmpty()) {
    int newItem = catalogName->count();
    WaypointCatalog *w = new WaypointCatalog;

    // read from disk
    if (!w->read(fName)) {
      delete w;
    }
    else {
      w->onDisc = true;
      w->path = fName;

      waypointCatalogs.append(w);
      catalogName->insertItem(w->path);

      catalogName->setCurrentItem(newItem);
      slotSwitchWaypointCatalog(newItem);
    }
  }
}

/** No descriptions */
void TaskAndWaypoint::slotNotHandledItem()
{
  KMessageBox::sorry(this, "This function is not yet available!", "ooops");
}

void TaskAndWaypoint::showTaskPopup(int row, int col, int button, const QPoint &)
{
  slotNotHandledItem();
}

void TaskAndWaypoint::showWaypointPopup(int row, int col, int button, const QPoint &)
{
  if (button == RightButton) {
    wayPointPopup->setItemEnabled(idWaypointCatalogSave, waypointCatalogs.count() && waypointCatalogs.current()->modified);
    wayPointPopup->setItemEnabled(idWaypointCatalogClose, waypointCatalogs.count());
    wayPointPopup->setItemEnabled(idWaypointCatalogImport, waypointCatalogs.count());
    wayPointPopup->setItemEnabled(idWaypointImportFromMap,waypointCatalogs.count());

    wayPointPopup->setItemEnabled(idWaypointNew,waypointCatalogs.count());
    wayPointPopup->setItemEnabled(idWaypointEdit, row != -1);
    wayPointPopup->setItemEnabled(idWaypointDelete, row != -1);
    wayPointPopup->setItemEnabled(idWaypointCopy2Task, row != -1);

    if (row != -1) {
      waypoints->setCurrentCell(row, COL_WAYPOINT_NAME);
    }
    wayPointPopup->exec(QCursor::pos());
  }
}

void TaskAndWaypoint::slotNewWaypoint()
{
  waypointDlg->clear();

  if (waypointDlg->exec() == Accepted) {
    slotAddWaypoint();
  }
}

/** create a new catalog */
void TaskAndWaypoint::slotNewWaypointCatalog()
{
  int newItem = catalogName->count();
  WaypointCatalog *w = new WaypointCatalog;

  waypointCatalogs.append(w);
  catalogName->insertItem(w->path);

  catalogName->setCurrentItem(newItem);
  slotSwitchWaypointCatalog(newItem);
}

void TaskAndWaypoint::reject()
{
  WaypointCatalog *w;
  for (w = waypointCatalogs.first(); w != 0; w = waypointCatalogs.next()) {
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
  }

  KDialog::reject();
}

/** insert waypoint from waypoint dialog */
void TaskAndWaypoint::slotAddWaypoint()
{
  int comboIdx;

  if (!waypointDlg->name->text().isEmpty()) {
    // insert a new waypoint to current catalog
    WaypointElement *w = new WaypointElement;
    w->name = waypointDlg->name->text().upper();
    w->description = waypointDlg->description->text();
    comboIdx = waypointDlg->waypointType->currentItem();
    // translate to id
    w->type = comboIdx != -1 ? waypointTypes.at(comboIdx)->id : -1;
    w->pos.setX(_globalMapContents.degreeToNum(waypointDlg->longitude->text()));
    w->pos.setY(_globalMapContents.degreeToNum(waypointDlg->latitude->text()));
    w->elevation = waypointDlg->elevation->text().toInt();
    w->icao = waypointDlg->icao->text().upper();
    w->frequency = waypointDlg->frequency->text().toDouble();
    w->runway = waypointDlg->runway->text().toInt();
    w->length = waypointDlg->length->text().toInt();
    comboIdx = waypointDlg->surface->currentItem();
    // translate to id
    w->surface = comboIdx != -1 ? surfaces.at(comboIdx)->id : -1;
    w->comment = waypointDlg->comment->text();

    if (waypointCatalogs.current()->wpList.insertItem(w)) {
      waypointCatalogs.current()->modified = true;
      fillWaypoints();
    }
  }
}

/** No descriptions */
void TaskAndWaypoint::slotEditWaypoint()
{
  int row = waypoints->currentRow();
  int comboIdx;
  QString tmp, oldName;

  if (row >= 0) {
    WaypointList *wl = &waypointCatalogs.current()->wpList;
    oldName = waypoints->text(row, COL_WAYPOINT_NAME);
    WaypointElement *w = wl->find(oldName);

    // initialize dialg
    waypointDlg->name->setText(w->name);
    waypointDlg->description->setText(w->description);
    // translate id to index
    waypointDlg->waypointType->setCurrentItem(waypointTypes.idxById(w->type));
    waypointDlg->longitude->setText(printPos(w->pos.x(), false));
    waypointDlg->latitude->setText(printPos(w->pos.y(), true));
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
    waypointDlg->surface->setCurrentItem(surfaces.idxById(w->surface));
    waypointDlg->comment->setText(w->comment);
    waypointDlg->isLandable->setChecked(w->isLandable);

    if (waypointDlg->exec() == Accepted) {
      if (!waypointDlg->name->text().isEmpty()) {
        w->name = waypointDlg->name->text().left(6).upper();
        w->description = waypointDlg->description->text();
        comboIdx = waypointDlg->waypointType->currentItem();
        // translate to id
        w->type = comboIdx != -1 ? waypointTypes.at(comboIdx)->id : -1;
        w->pos.setX(_globalMapContents.degreeToNum(waypointDlg->longitude->text()));
        w->pos.setY(_globalMapContents.degreeToNum(waypointDlg->latitude->text()));
        w->elevation = waypointDlg->elevation->text().toInt();
        w->icao = waypointDlg->icao->text().upper();
        w->frequency = waypointDlg->frequency->text().toDouble();
        w->runway = waypointDlg->runway->text().toInt();
        w->length = waypointDlg->length->text().toInt();
        comboIdx = waypointDlg->surface->currentItem();
        // translate to id
        w->surface = comboIdx != -1 ? surfaces.at(comboIdx)->id : -1;
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
void TaskAndWaypoint::slotDeleteWaypoint()
{
  QString tmp = waypoints->text(waypoints->currentRow(), COL_WAYPOINT_NAME);
  waypointCatalogs.current()->wpList.remove(tmp);
  waypointCatalogs.current()->modified = true;
  fillWaypoints();
}

/** No descriptions */
void TaskAndWaypoint::fillWaypoints()
{
  QString tmp;
  WaypointElement *w;
  QDictIterator<WaypointElement> it(waypointCatalogs.current()->wpList);
  int row = 0;

  waypoints->setNumRows(it.count());

  for (w = it.current(); w != 0; w = ++it) {
    waypoints->setItem(row, COL_WAYPOINT_NAME, new QTableItem(waypoints, QTableItem::Never, w->name));
    waypoints->setItem(row, COL_WAYPOINT_DESCRIPTION, new QTableItem(waypoints, QTableItem::Never, w->description));
    waypoints->setItem(row, COL_WAYPOINT_ICAO, new QTableItem(waypoints, QTableItem::Never, w->icao));
    waypoints->setItem(row, COL_WAYPOINT_TYPE, new QTableItem(waypoints, QTableItem::Never,
      w->type == -1 ? QString::null : waypointTypes.itemById(w->type)->text));
    waypoints->setItem(row, COL_WAYPOINT_LAT, new QTableItem(waypoints, QTableItem::Never,
      printPos(w->pos.y(), true)));
    waypoints->setItem(row, COL_WAYPOINT_LONG, new QTableItem(waypoints, QTableItem::Never,
      printPos(w->pos.x(), false)));
    tmp.sprintf("%d", w->elevation);
    waypoints->setItem(row, COL_WAYPOINT_ELEV, new QTableItem(waypoints, QTableItem::Never, tmp));
    tmp.sprintf("%.3f", w->frequency);
    waypoints->setItem(row, COL_WAYPOINT_FREQ, new QTableItem(waypoints, QTableItem::Never, tmp));
    waypoints->setItem(row, COL_WAYPOINT_IS_LANDABLE, new QTableItem(waypoints, QTableItem::Never,
      w->isLandable == true ? i18n("Yes") : QString::null));
    tmp.sprintf("%02d", w->runway);
    waypoints->setItem(row, COL_WAYPOINT_RUNWAY, new QTableItem(waypoints, QTableItem::Never, tmp));
    tmp.sprintf("%d", w->length);
    waypoints->setItem(row, COL_WAYPOINT_LENGTH, new QTableItem(waypoints, QTableItem::Never, tmp));
    waypoints->setItem(row, COL_WAYPOINT_SURFACE, new QTableItem(waypoints, QTableItem::Never,
      w->surface == -1 ? QString::null : surfaces.itemById(w->surface)->text));
    waypoints->setItem(row, COL_WAYPOINT_COMMENT, new QTableItem(waypoints, QTableItem::Never, w->comment));

    row++;
  }
  waypoints->sort();
}

/** No descriptions */
void TaskAndWaypoint::slotSwitchWaypointCatalog(int idx)
{
  waypointCatalogs.at(idx);
  fillWaypoints();
}

void TaskAndWaypoint::slotSaveWaypointCatalog()
{
  WaypointCatalog *w = waypointCatalogs.current();
  if (w->write()) {
    catalogName->changeItem(w->path, catalogName->currentItem());
  }
}

void TaskAndWaypoint::slotImportWaypointCatalog()
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

void TaskAndWaypoint::slotCloseWaypointCatalog()
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
  if (!cnt) {
    waypoints->setNumRows(0);
  }
  else {
    if (idx >= cnt) {
      idx = cnt -1;
    }

    catalogName->setCurrentItem(idx);
    slotSwitchWaypointCatalog(idx);
  }
}

void TaskAndWaypoint::slotImportWaypointFromMap()
{
  Airport *a;
  GliderSite *g;
  WaypointElement *w;
  WaypointList *wl = &waypointCatalogs.current()->wpList;
  unsigned int i;
  int type, loop;
  bool useAll, useAirports, useGliderSites, useOtherSites, useObstacle;
  bool useLandmark, useOutlanding, useStation, useArea;
  int fromLat, fromLong, toLat, toLong;
  QPoint p;
  QString tmp;
  QRegExp blank("[ ]");

  WaypointImpFilterDialog d;

  if (d.exec() == Accepted) {
    useAll = d.useAll->isChecked();
    useAirports = d.airports->isChecked();
    useGliderSites = d.gliderSites->isChecked();
    useOtherSites = d.otherSites->isChecked();
    useObstacle = d.obstacle->isChecked();
    useLandmark = d.landmark->isChecked();
    useOutlanding = d.outlanding->isChecked();
    useStation = d.station->isChecked();

    fromLat = _globalMapContents.degreeToNum(d.fromLat->text());
    toLat = _globalMapContents.degreeToNum(d.toLat->text());
    fromLong = _globalMapContents.degreeToNum(d.fromLong->text());
    toLong = _globalMapContents.degreeToNum(d.toLong->text());

    // normalize coordinates
    if (fromLat > toLat) {
      int tmp = fromLat;
      fromLat = toLat;
      toLat = tmp;
    }

    if (fromLong > toLong) {
      int tmp = fromLong;
      fromLong = toLong;
      toLong = tmp;
    }

    useArea = (toLat > 1 && toLong > 1);

    if (useAll || useAirports) {
      for (i = 0; i < _globalMapContents.getListLength(MapContents::AirportList); i++) {
        a = _globalMapContents.getAirport(i);

        p = _globalMapMatrix.mapToWgs(_globalMapMatrix.map(a->getPosition()));

        // check area
        if (useArea) {
          if (p.x() < fromLong || p.x() > toLong ||
              p.y() < fromLat || p.y() > toLat) {
              continue;
          }
        }

        w = new WaypointElement;

        w->name = a->getName().replace(blank, QString::null).left(6).upper();
        loop = 0;
        while (wl->find(w->name) && loop < 100000) {
          tmp.setNum(loop++);
          w->name = w->name.left(6 - tmp.length()) + tmp;
        }
        w->description = a->getName();
        type = a->getTypeID();
        w->type = type;

        w->pos.setX(p.x());
        w->pos.setY(p.y());

        w->elevation = a->getElevation();
        w->icao = a->getICAO();
        w->frequency = a->getFrequency().toDouble();

        switch(type) {
        case BaseMapElement::Airport:
        case BaseMapElement::MilAirport:
        case BaseMapElement::CivMilAirport:
        case BaseMapElement::Airfield:
          w->isLandable = true;
          break;
        default:
          w->isLandable = false;
        }
    //    w->runway = ;
    //    w->length = ;
    //    w->surface = ;

        if (!wl->insertItem(w)) {
          break;
        }
      }
    }

    if (useAll || useGliderSites) {
      for (i = 0; i < _globalMapContents.getListLength(MapContents::GliderList); i++) {
        g = _globalMapContents.getGlidersite(i);

        p = _globalMapMatrix.mapToWgs(_globalMapMatrix.map(g->getPosition()));
        // check area
        if (useArea) {
          if (p.x() < fromLong || p.x() > toLong ||
              p.y() < fromLat || p.y() > toLat) {
              continue;
          }
        }

        w = new WaypointElement;

        w->name = g->getName().replace(blank, QString::null).left(6).upper();
        loop = 0;
        while (wl->find(w->name) && loop < 100000) {
          tmp.setNum(loop++);
          w->name = w->name.left(6 - tmp.length()) + tmp;
        }
        w->description = g->getName();
        w->type = BaseMapElement::Glidersite;

        w->pos.setX(p.x());
        w->pos.setY(p.y());

        w->elevation = g->getElevation();
        w->icao = g->getICAO();
        w->frequency = g->getFrequency().toDouble();

        w->isLandable = true;
    //    w->runway = ;
    //    w->length = ;
    //    w->surface = ;

        if (!wl->insertItem(w)) {
          break;
        }
      }
    }

    waypointCatalogs.current()->modified = true;
    fillWaypoints();
  }
}
