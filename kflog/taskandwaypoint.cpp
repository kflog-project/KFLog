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
#define COL_WAYPOINT_FREQ 6
#define COL_WAYPOINT_ELEV 7
#define COL_WAYPOINT_RUNWAY 8
#define COL_WAYPOINT_LENGTH 9
#define COL_WAYPOINT_SURFACE 10
#define COL_WAYPOINT_COMMENT 11

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
  TranslationElement *te;
  surfaces.setAutoDelete(true);

  surfaces.append(new TranslationElement(Airport::NotSet, i18n("Unknown")));
  surfaces.append(new TranslationElement(Airport::Grass, i18n("Grass")));
  surfaces.append(new TranslationElement(Airport::Asphalt, i18n("Asphalt")));
  surfaces.append(new TranslationElement(Airport::Concrete, i18n("Concrete")));

  surfaces.sort();
  // index by id
  for (te = surfaces.first(); te != 0; te = surfaces.next()) {
    surfacesById.insert(te->id, te);
  }
}

void TaskAndWaypoint::initTypes()
{
  TranslationElement *te;
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
  // index by id
  for (te = waypointTypes.first(); te != 0; te = waypointTypes.next()) {
    waypointTypesById.insert(te->id, te);
  }
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
  waypoints =  new KFLogTable(0, 12, w, "waypoints");

  QHeader *h = waypoints->horizontalHeader();
  h->setLabel(0, i18n("Name"));
  h->setLabel(1, i18n("Description"));
  h->setLabel(2, i18n("ICAO"));
  h->setLabel(3, i18n("Type"));
  h->setLabel(4, i18n("Latitude"));
  h->setLabel(5, i18n("Longitude"));
  h->setLabel(6, i18n("Frequency"));
  h->setLabel(7, i18n("Elevation (m)"));
  h->setLabel(8, i18n("Runway"));
  h->setLabel(9, i18n("Length (m)"));
  h->setLabel(10, i18n("Surface"));
  h->setLabel(11, i18n("Comment"));

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

    wayPointPopup->setItemEnabled(idWaypointNew, waypointCatalogs.count());
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
  int idx, comboIdx;
  WaypointList *wl = waypointCatalogs.current()->wpList;

  if (!waypointDlg->name->text().isEmpty()) {
    // insert a new waypoint to current catalog
    WaypointElement *w = new WaypointElement;
    w->name = waypointDlg->name->text().upper();
    w->description = waypointDlg->description->text();
    comboIdx = waypointDlg->waypointType->currentItem();
    // translate to id
    w->type = comboIdx != -1 ? waypointTypes.at(comboIdx)->id : -1;
    w->pos.setX(waypointDlg->longitude->seconds());
    w->pos.setY(waypointDlg->latitude->seconds());
    w->elevation = waypointDlg->elevation->text().toInt();
    w->icao = waypointDlg->icao->text().upper();
    w->frequency = waypointDlg->frequency->text().toDouble();
    w->runway = waypointDlg->runway->text().toInt();
    w->length = waypointDlg->length->text().toInt();
    comboIdx = waypointDlg->surface->currentItem();
    // translate to id
    w->surface = comboIdx != -1 ? surfaces.at(comboIdx)->id : -1;
    w->comment = waypointDlg->comment->text();

    idx = wl->insertItem(w);
    if (idx != -1) {
      waypointCatalogs.current()->modified = true;
      // check if we have a new item
      if ((int)wl->count() > waypoints->numRows()) {
        waypoints->insertRow(idx);
      }

      fillWaypoints(idx, idx);
    }
  }
}

/** No descriptions */
void TaskAndWaypoint::slotEditWaypoint()
{
  int idx = waypoints->currentRow();
  int comboIdx;
  QString tmp;

  if (idx >= 0) {
    WaypointList *wl = waypointCatalogs.current()->wpList;
    WaypointElement *w = wl->at(idx);

    // initialize dialg
    waypointDlg->name->setText(w->name);
    waypointDlg->description->setText(w->description);
    // translate id to index
    waypointDlg->waypointType->setCurrentItem(waypointTypes.findRef(waypointTypesById[w->type]));
    waypointDlg->longitude->setSeconds(w->pos.x());
    waypointDlg->latitude->setSeconds(w->pos.y());
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
    waypointDlg->surface->setCurrentItem(surfaces.findRef(surfacesById[w->surface]));
    waypointDlg->comment->setText(w->comment);

    if (waypointDlg->exec() == Accepted) {
      if (!waypointDlg->name->text().isEmpty()) {
        w->name = waypointDlg->name->text().upper();
        w->description = waypointDlg->description->text();
        comboIdx = waypointDlg->waypointType->currentItem();
        // translate to id
        w->type = comboIdx != -1 ? waypointTypes.at(comboIdx)->id : -1;
        w->pos.setX(waypointDlg->longitude->seconds());
        w->pos.setY(waypointDlg->latitude->seconds());
        w->elevation = waypointDlg->elevation->text().toInt();
        w->icao = waypointDlg->icao->text().upper();
        w->frequency = waypointDlg->frequency->text().toDouble();
        w->runway = waypointDlg->runway->text().toInt();
        w->length = waypointDlg->length->text().toInt();
        comboIdx = waypointDlg->surface->currentItem();
        // translate to id
        w->surface = comboIdx != -1 ? surfaces.at(comboIdx)->id : -1;
        w->comment = waypointDlg->comment->text();

        waypointCatalogs.current()->modified = true;
        fillWaypoints(idx, idx);
      }
    }
  }
}

/** No descriptions */
void TaskAndWaypoint::slotDeleteWaypoint()
{
  int idx = waypoints->currentRow();
  WaypointList *wl = waypointCatalogs.current()->wpList;
  waypoints->deleteRow(idx);
  wl->remove(idx);
}

/** No descriptions */
void TaskAndWaypoint::fillWaypoints(int start, int end)
{
  int degree, min, sec;
  div_t divRes;
  QString tmp;
  WaypointList *wl = waypointCatalogs.current()->wpList;
  WaypointElement *w;

  if (end == -1) {
    end = wl->count() - 1;
  }

  while (start <= end) {
    w = wl->at(start);

    waypoints->setItem(start, COL_WAYPOINT_NAME, new QTableItem(waypoints, QTableItem::Never, w->name));
    waypoints->setItem(start, COL_WAYPOINT_DESCRIPTION, new QTableItem(waypoints, QTableItem::Never, w->description));
    waypoints->setItem(start, COL_WAYPOINT_ICAO, new QTableItem(waypoints, QTableItem::Never, w->icao));

    waypoints->setItem(start, COL_WAYPOINT_TYPE, new QTableItem(waypoints, QTableItem::Never,
      w->type == -1 ? QString::null : waypointTypesById[w->type]->text));

    divRes = div(w->pos.y(), 3600);
    degree = divRes.quot;
    divRes = div(divRes.rem, 60);
    min = divRes.quot;
    sec = divRes.rem;
    tmp.sprintf("%c%02d°%02d'%02d\"", w->pos.y() >= 0 ? 'N' : 'S', degree, min, sec);
    waypoints->setItem(start, COL_WAYPOINT_LAT, new QTableItem(waypoints, QTableItem::Never, tmp));

    divRes = div(w->pos.x(), 3600);
    degree = divRes.quot;
    divRes = div(divRes.rem, 60);
    min = divRes.quot;
    sec = divRes.rem;
    tmp.sprintf("%c%03d°%02d'%02d\"", w->pos.x() >= 0 ? 'E' : 'W', degree, min, sec);
    waypoints->setItem(start, COL_WAYPOINT_LONG, new QTableItem(waypoints, QTableItem::Never, tmp));

    tmp.sprintf("%.3f", w->frequency);
    waypoints->setItem(start, COL_WAYPOINT_FREQ, new QTableItem(waypoints, QTableItem::Never, tmp));
    tmp.sprintf("%d", w->elevation);
    waypoints->setItem(start, COL_WAYPOINT_ELEV, new QTableItem(waypoints, QTableItem::Never, tmp));
    tmp.sprintf("%02d", w->runway);
    waypoints->setItem(start, COL_WAYPOINT_RUNWAY, new QTableItem(waypoints, QTableItem::Never, tmp));
    tmp.sprintf("%d", w->length);
    waypoints->setItem(start, COL_WAYPOINT_LENGTH, new QTableItem(waypoints, QTableItem::Never, tmp));

    waypoints->setItem(start, COL_WAYPOINT_SURFACE, new QTableItem(waypoints, QTableItem::Never,
      w->surface == -1 ? QString::null : surfacesById[w->surface]->text));

    waypoints->setItem(start, COL_WAYPOINT_COMMENT, new QTableItem(waypoints, QTableItem::Never, w->comment));

    start++;
  }
}

/** No descriptions */
void TaskAndWaypoint::slotSwitchWaypointCatalog(int idx)
{
  WaypointList *wl = waypointCatalogs.at(idx)->wpList;

  waypoints->setNumRows(wl->count());
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
    waypoints->setNumRows(w->wpList-> count());
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
