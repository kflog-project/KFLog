/***************************************************************************
                          waypoints.cpp  -  description
                             -------------------
    begin                : Fri Nov 30 2001
    copyright            : (C) 2001 by Harald Maier
                               2011 by Axel Pauli

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

#include <QtGui>

#include "airfield.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "runway.h"
#include "waypoints.h"
#include "wgspoint.h"
#include "mainwindow.h"

extern MainWindow  *_mainWindow;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

Waypoints::Waypoints(QWidget *parent, const QString& catalog) :
  QWidget(parent),
  currentWaypointCatalog(0)
{
  Q_UNUSED( catalog )

  setObjectName( "Waypoints" );

  addWaypointWindow(this);
  createMenu();

  waypointDlg = new WaypointDialog(this);

  connect( waypointDlg, SIGNAL(addWaypoint(Waypoint *)),
           this, SLOT(slotAddWaypoint(Waypoint *)) );

  importFilterDlg = new WaypointImpFilterDialog(this);
}

Waypoints::~Waypoints()
{
  qDeleteAll( waypointCatalogs );
}

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
          SLOT(showWaypointMenu(Q3ListViewItem *, const QPoint &, int)));
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

/** Create menus. */
void Waypoints::createMenu()
{
  wayPointMenu = new QMenu(waypoints);

  wayPointMenu->setTitle( tr("Waypoints"));

  wayPointMenu->addAction( _mainWindow->getPixmap("waypoint_16.png"),
                                             QObject::tr("&New catalog"),
                                             this,
                                             SLOT(slotNewWaypointCatalog()) );

  wayPointMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
                                             QObject::tr("&Open catalog"),
                                             this,
                                             SLOT(slotOpenWaypointCatalog()) );


  ActionWaypointCatalogImport = wayPointMenu->addAction( tr("&Import catalog"),
                                                         this,
                                                         SLOT(slotImportWaypointCatalog()) );

  ActionWaypointImportFromMap = wayPointMenu->addAction( tr("Import from &map"),
                                                         this,
                                                         SLOT(slotImportWaypointFromMap()) );

  ActionWaypointImportFromFile = wayPointMenu->addAction( tr("Import from &file"),
                                                          this,
                                                          SLOT(slotImportWaypointFromFile()) );

  ActionWaypointCatalogSave = wayPointMenu->addAction( _mainWindow->getPixmap("kde_filesave_16.png"),
                                                       tr("&Save catalog"),
                                                       this,
                                                       SLOT(slotSaveWaypointCatalog()) );

  ActionWaypointCatalogSaveAs = wayPointMenu->addAction( _mainWindow->getPixmap("kde_filesaveas_16.png"),
                                                         tr("&Save catalog as..."),
                                                         this,
                                                         SLOT(slotSaveWaypointCatalogAs()) );

  ActionWaypointCatalogClose = wayPointMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
                                                        tr("&Close catalog"),
                                                        this,
                                                        SLOT(slotCloseWaypointCatalog()) );
  wayPointMenu->addSeparator();

  ActionWaypointNew = wayPointMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                                               tr("New &waypoint"),
                                               this,
                                               SLOT(slotNewWaypoint()) );

  ActionWaypointEdit = wayPointMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                                tr("&Edit waypoint"),
                                                this,
                                                SLOT(slotEditWaypoint()) );

  ActionWaypointDelete = wayPointMenu->addAction( _mainWindow->getPixmap("kde_editdelete_16.png"),
                                                  tr("&Delete waypoint"),
                                                  this,
                                                  SLOT(slotDeleteWaypoint()) );

  catalogCopySubMenu = wayPointMenu->addMenu( _mainWindow->getPixmap("kde_editcopy_16.png"),
                                              tr("Copy to &catalog" ) );

  catalogMoveSubMenu = wayPointMenu->addMenu( _mainWindow->getPixmap("kde_move_16.png"),
                                              tr("Move to &catalog") );
  wayPointMenu->addSeparator();

  ActionWaypointCopy2Task = wayPointMenu->addAction( _mainWindow->getPixmap("kde_editcopy_16.png"),
                                                     tr("Copy to &task"),
                                                     this,
                                                     SLOT(slotCopyWaypoint2Task()) );

  ActionWaypointCenterMap = wayPointMenu->addAction( _mainWindow->getPixmap("centerwaypoint_16.png"),
                                                     tr("Center map on waypoint"),
                                                     this,
                                                     SLOT(slotCenterMap()) );

  ActionWaypointSetHome = wayPointMenu->addAction( _mainWindow->getPixmap("kde_gohome_16.png"),
                                                   tr("Set Homesite"),
                                                   this,
                                                   SLOT(slotSetHome()) );

  connect( catalogCopySubMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotCopy2Catalog(QAction *)) );
  connect( catalogMoveSubMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotMove2Catalog(QAction *)) );
}

/**
 * Copies the current waypoint to the selected catalog
 */
void Waypoints::slotCopy2Catalog( QAction* action )
{
  int id = action->data().toInt();

  Q3ListViewItem *item = waypoints->currentItem();
  Waypoint *wpt;

  if( item != 0 )
    {
      QString tmp = item->text( colName );
      wpt = currentWaypointCatalog->findWaypoint( tmp );
      waypointCatalogs.at( id )->wpList.append( new Waypoint( wpt ) );
      waypointCatalogs.at( id )->modified = true;
    }
}

/**
 * Moves the current waypoint to the selected catalog
 */
void Waypoints::slotMove2Catalog( QAction* action )
{
  int id = action->data().toInt();

  Q3ListViewItem *item = waypoints->currentItem();
  Waypoint * wpt;

  if( item != 0 )
    {
      QString tmp = item->text( colName );
      wpt = currentWaypointCatalog->findWaypoint( tmp );
      waypointCatalogs.at( id )->wpList.append( new Waypoint( wpt ) );
      currentWaypointCatalog->removeWaypoint( tmp );
      currentWaypointCatalog->modified = true;
      waypointCatalogs.value( id )->modified = true;
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
  filter.append(tr("All supported waypoint formats") + " (*.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP *.txt *.TXT);;");
  filter.append(tr("KFLog waypoints") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Cumulus waypoints") + " (*.kwp *.KWP);;");
  filter.append(tr("Filser txt waypoints") + " (*.txt *.TXT);;");
  filter.append(tr("Filser da4 waypoints") + " (*.da4 *.DA4);;");
  filter.append(tr("SeeYou cup waypoints") + " (*.cup *.CUP)");

  QString fName = QFileDialog::getOpenFileName( this,
                                                tr("Open waypoint catalog"),
                                                wayPointDir,
                                                filter );

  if( ! fName.isEmpty() )
    {
      openCatalog( fName );
    }
}

void Waypoints::showWaypointMenu(Q3ListViewItem *it, const QPoint &, int)
{
  //enable and disable the correct menu items
  ActionWaypointCatalogSave->setEnabled(waypointCatalogs.count() && currentWaypointCatalog->modified);
  ActionWaypointCatalogSaveAs->setEnabled(waypointCatalogs.count() > 0);
  ActionWaypointCatalogClose->setEnabled(waypointCatalogs.count() > 1);
  ActionWaypointCatalogImport->setEnabled(waypointCatalogs.count());
  ActionWaypointImportFromMap->setEnabled(waypointCatalogs.count());

  ActionWaypointNew->setEnabled(waypointCatalogs.count());
  ActionWaypointEdit->setEnabled(it != 0);
  ActionWaypointDelete->setEnabled(it != 0);
  ActionWaypointCopy2Task->setEnabled(it != 0);
  ActionWaypointCenterMap->setEnabled(it != 0);
  ActionWaypointSetHome->setEnabled(it != 0);

  // fill the submenus for the move & copy to catalog
  catalogCopySubMenu->clear();
  catalogMoveSubMenu->clear();

  // get current catalog index
  int curCat = waypointCatalogs.indexOf( currentWaypointCatalog );

  for( int i = 0; i < waypointCatalogs.count(); i++ )
    {
      if( curCat != i )
        {
          // only insert if this catalog is NOT the current catalog...
          QAction* action = catalogCopySubMenu->addAction( waypointCatalogs.at( i )->path );
          action->setData( i );

          action = catalogMoveSubMenu->addAction( waypointCatalogs.at( i )->path );
          action->setData( i );
        }
    }

  // make sure we go back to the original catalog...
  waypointCatalogs.at(curCat);
  wayPointMenu->exec( QCursor::pos() );
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
              tr("<html>The waypoint file has been modified.<br>Save changes to<BR><B>%1</B></html>").arg(w->path),
              QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
          saveBox.setButtonText(QMessageBox::Yes, tr("Save"));
          saveBox.setButtonText(QMessageBox::No, tr("Discard"));
          switch(saveBox.exec())
            {
            case QMessageBox::Yes:
              // Hier zwischenzeitlich auf binär format umgestellt ...
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

    // initialize dialog
    waypointDlg->setWindowTitle( tr( "Edit Waypoint" ) );
    waypointDlg->name->setText(w->name);
    waypointDlg->description->setText(w->description);
    // translate id to index
    waypointDlg->setWaypointType(w->type);
    waypointDlg->longitude->setKFLogDegree(w->origP.lon());
    waypointDlg->latitude->setKFLogDegree(w->origP.lat());
    tmp.sprintf("%d", w->elevation);
    waypointDlg->elevation->setText(tmp);
    waypointDlg->icao->setText(w->icao);
    tmp.sprintf("%.3f", w->frequency);
    waypointDlg->frequency->setText(tmp);

    if( w->runway.first > 0 )
        {
          tmp.sprintf( "%02d", w->runway.first );
        }
      else
        {
          tmp = "";
        }

      waypointDlg->runway->setText( tmp );

      if( w->length != -1 )
        {
          tmp.sprintf( "%d", w->length );
        }
      else
        {
          tmp = "";
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
        w->origP.setLat(waypointDlg->latitude->KFLogDegree());
        w->origP.setLon(waypointDlg->longitude->KFLogDegree());
        w->elevation = waypointDlg->elevation->text().toInt();
        w->icao = waypointDlg->icao->text().upper();
        w->frequency = waypointDlg->frequency->text().toDouble();

        tmp = waypointDlg->runway->text();

        if (!tmp.isEmpty()) {
          w->runway.first = tmp.toInt();

          int rw1 = w->runway.first;

          w->runway.second = ((rw1 > 18) ? rw1 - 18 : rw1 + 18 );

        }
        else {
          w->runway = QPair<ushort, ushort>(0, 0);
        }

        tmp = waypointDlg->length->text();
        if (!tmp.isEmpty()) {
          w->length = tmp.toInt();
        }
        else {
          w->length = -1;
        }
        w->surface = (enum Runway::SurfaceType) waypointDlg->getSurface();
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
                            tr("<html>Waypoint <b>%1</b> will be deleted.<br>Are you sure?</html>").arg(wp->name),
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
                            tr("<html>Waypoint <b>%1</b> will be deleted.<br>Are you sure?</html>").arg(tmp),
                            QMessageBox::Warning, QMessageBox::Ok, QMessageBox::Cancel, 0);
    waypointBox.setButtonText(QMessageBox::Ok, tr("&Delete"));

    if (waypointBox.exec() == QMessageBox::Ok)
    {
      currentWaypointCatalog->removeWaypoint(tmp);
      currentWaypointCatalog->modified = true;
      delete item;

      // eggert@kflog.org
      // this must be done to make the deletion effective immediately (map display)
      fillWaypoints();
    }
  }
}

/** No descriptions */
void Waypoints::fillWaypoints()
{
  qDebug() << "Waypoints::fillWaypoints()";

  QString tmp;
  Waypoint *w;
  Q3ListViewItem *item;
  bool filterRadius, filterArea;
  extern MapConfig *_globalMapConfig;

  waypoints->clear();

  filterRadius = (currentWaypointCatalog->radiusLat != 0  || currentWaypointCatalog->radiusLong != 0);
  filterArea   = (currentWaypointCatalog->areaLat2 != 0 && currentWaypointCatalog->areaLong2 != 0 && !filterRadius);

  foreach(w, currentWaypointCatalog->wpList)
      {
        if( !currentWaypointCatalog->showAll )
          {
            switch( w->type )
              {
              case BaseMapElement::IntAirport:
              case BaseMapElement::Airport:
              case BaseMapElement::MilAirport:
              case BaseMapElement::CivMilAirport:
              case BaseMapElement::Airfield:

                if( !currentWaypointCatalog->showAirfields )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Gliderfield:

                if( !currentWaypointCatalog->showGliderfields )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::UltraLight:
              case BaseMapElement::HangGlider:
              case BaseMapElement::Parachute:
              case BaseMapElement::Balloon:

                if( !currentWaypointCatalog->showOtherSites )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Outlanding:

                if( !currentWaypointCatalog->showOutlandings )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Obstacle:

                if( !currentWaypointCatalog->showObstacles )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Landmark:

                if( !currentWaypointCatalog->showLandmarks )
                  {
                    continue;
                  }
                break;
              }
          }

    if (filterArea)
      {
        if (w->origP.lat() < currentWaypointCatalog->areaLat1 || w->origP.lat() > currentWaypointCatalog->areaLat2 ||
            w->origP.lon() < currentWaypointCatalog->areaLong1 || w->origP.lon() > currentWaypointCatalog->areaLong2)
          {
            continue;
          }
      }
    else if (filterRadius)
      {
        // We have to consider the user chosen distance unit.
        double catalogDist = Distance::convertToMeters( currentWaypointCatalog->radiusSize ) / 1000.;

        // This distance is calculated im kilometers.
        double radiusDist = dist( currentWaypointCatalog->radiusLat,
                                  currentWaypointCatalog->radiusLong,
                                  w->origP.lat(),
                                  w->origP.lon() );

        if ( radiusDist > catalogDist )
          {
            continue;
          }
    }

    item = new Q3ListViewItem(waypoints);
    item->setText(colName, w->name);
    item->setText(colDesc, w->description);
    item->setText(colICAO, w->icao);
    item->setText(colType, BaseMapElement::item2Text(w->type, tr("unknown")));
    item->setText(colLat, WGSPoint::printPos(w->origP.lat(), true));
    item->setText(colLong, WGSPoint::printPos(w->origP.lon(), false));
    tmp.sprintf("%d", w->elevation);
    item->setText(colElev, tmp);
    w->frequency > 1 ? tmp.sprintf("%.3f", w->frequency) : tmp=QString::null;
    item->setText(colFrequency, tmp);
    item->setText(colLandable, w->isLandable == true ? tr("Yes") : QString::null);
    if (w->runway.first > 0) {
      tmp.sprintf("%02d", w->runway.first);
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
  filter.append(tr("KFLog waypoints") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("All files") + " (*.*)");

  QString fName = QFileDialog::getOpenFileName( this,
                                                tr("Import waypoint catalog"),
                                                wayPointDir,
                                                filter );
  if( ! fName.isEmpty() )
    {
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
           "<html>" + tr("Save changes to<BR><B>%1</B>").arg(currentWaypointCatalog->path) + "</html>",
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
  QList<int> searchList;

  bool filterRadius, filterArea;

  if (importFilterDlg->exec() == QDialog::Accepted)
    {
      getFilterData();

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showAirfields )
        {
          searchList.append( MapContents::AirfieldList );
        }

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showGliderfields )
        {
          searchList.append( MapContents::GliderfieldList );
        }

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showOtherSites )
        {
          searchList.append( MapContents::AddSitesList );
        }

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showObstacles )
        {
          searchList.append( MapContents::ObstacleList );
        }

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showLandmarks )
        {
          searchList.append( MapContents::LandmarkList );
        }

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showOutlandings )
        {
          searchList.append( MapContents::OutLandingList );
        }

      if( currentWaypointCatalog->showAll || currentWaypointCatalog->showStations )
        {
          searchList.append( MapContents::StationList );
        }

      filterRadius = (currentWaypointCatalog->radiusLat != 0  || currentWaypointCatalog->radiusLong != 0);

      filterArea = (currentWaypointCatalog->areaLat2 != 0 && currentWaypointCatalog->areaLong2 != 0 && !filterRadius);

    for( int k = 0; k < searchList.size(); k++ )
      {
      for (int i = 0; i < _globalMapContents->getListLength(searchList.at(k) ); i++)
        {

        s = (SinglePoint *)_globalMapContents->getElement(searchList.at(k), i);
        p = s->getWGSPosition();

        // check area
        if (filterArea)
          {
          if (p.lon() < currentWaypointCatalog->areaLong1 || p.lon() > currentWaypointCatalog->areaLong2 ||
              p.lat() < currentWaypointCatalog->areaLat1 || p.lat() > currentWaypointCatalog->areaLat2)
            {
             continue;
            }
          }
        else if (filterRadius)
          {
            // We have to consider the user chosen distance unit.
            double catalogDist = Distance::convertToMeters( currentWaypointCatalog->radiusSize ) / 1000.;

            // This distance is calculated im kilometers.
            double radiusDist = dist( currentWaypointCatalog->radiusLat,
                                      currentWaypointCatalog->radiusLong,
                                      p.lat(),
                                      p.lon());

            if ( radiusDist > catalogDist )
              {
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
                      w->runway = runway->getRunwayDirection();
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
  // dump will occur.
  if( !currentWaypointCatalog )
    {
      return; // no catalog loaded
    }

  if( importFilterDlg->exec() == QDialog::Accepted )
    {
      getFilterData();
      fillWaypoints();
    }
}

/** add a new waypoint from outside */
void Waypoints::slotAddWaypoint(Waypoint *w)
{
  if (!currentWaypointCatalog) {
    //let's make sure we have a waypoint catalog
    WaypointCatalog* wpc = new WaypointCatalog(tr("unnamed"));
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
    _settings.setValue("/Homesite/Name", w->name);
    _settings.setValue("/Homesite/Latitude", w->origP.lat());
    _settings.setValue("/Homesite/Longitude", w->origP.lon());

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
  currentWaypointCatalog->showAirfields = importFilterDlg->airfields->isChecked();
  currentWaypointCatalog->showGliderfields = importFilterDlg->gliderfields->isChecked();
  currentWaypointCatalog->showOtherSites = importFilterDlg->otherSites->isChecked();
  currentWaypointCatalog->showObstacles = importFilterDlg->obstacles->isChecked();
  currentWaypointCatalog->showLandmarks = importFilterDlg->landmarks->isChecked();
  currentWaypointCatalog->showOutlandings = importFilterDlg->outlandings->isChecked();
  currentWaypointCatalog->showStations = importFilterDlg->stations->isChecked();

  currentWaypointCatalog->areaLat1 = importFilterDlg->fromLat->KFLogDegree();
  currentWaypointCatalog->areaLat2 = importFilterDlg->toLat->KFLogDegree();
  currentWaypointCatalog->areaLong1 = importFilterDlg->fromLong->KFLogDegree();
  currentWaypointCatalog->areaLong2 = importFilterDlg->toLong->KFLogDegree();

  switch ( importFilterDlg->getCenterRef() )
  {
    case CENTER_POS:
      currentWaypointCatalog->radiusLat  = importFilterDlg->radiusLat->KFLogDegree();
      currentWaypointCatalog->radiusLong = importFilterDlg->radiusLong->KFLogDegree();
      break;
    case CENTER_HOMESITE:
      currentWaypointCatalog->radiusLat  = _settings.value("/Homesite/Latitude").toInt();
      currentWaypointCatalog->radiusLong = _settings.value("/Homesite/Longitude").toInt();
      break;
    case CENTER_MAP:
      p = _globalMapMatrix->getMapCenter(false);
      currentWaypointCatalog->radiusLat  = p.lat();
      currentWaypointCatalog->radiusLong = p.lon();
      break;
    case CENTER_AIRFIELD:
      currentWaypointCatalog->airfieldRef = importFilterDlg->airfieldRefTxt;
      currentWaypointCatalog->radiusLat   = importFilterDlg->getAirfieldRef().lat();
      currentWaypointCatalog->radiusLong  = importFilterDlg->getAirfieldRef().lon();
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

void Waypoints::setFilterData()
{
  importFilterDlg->useAll->setChecked(currentWaypointCatalog->showAll);
  importFilterDlg->airfields->setChecked(currentWaypointCatalog->showAirfields);
  importFilterDlg->gliderfields->setChecked(currentWaypointCatalog->showGliderfields);
  importFilterDlg->otherSites->setChecked(currentWaypointCatalog->showOtherSites);
  importFilterDlg->obstacles->setChecked(currentWaypointCatalog->showObstacles);
  importFilterDlg->landmarks->setChecked(currentWaypointCatalog->showLandmarks);
  importFilterDlg->outlandings->setChecked(currentWaypointCatalog->showOutlandings);
  importFilterDlg->stations->setChecked(currentWaypointCatalog->showStations);

  importFilterDlg->fromLat->setKFLogDegree(currentWaypointCatalog->areaLat1);
  importFilterDlg->toLat->setKFLogDegree(currentWaypointCatalog->areaLat2);
  importFilterDlg->fromLong->setKFLogDegree(currentWaypointCatalog->areaLong1);
  importFilterDlg->toLong->setKFLogDegree(currentWaypointCatalog->areaLong2);

  importFilterDlg->radiusLat->setKFLogDegree(currentWaypointCatalog->radiusLat);
  importFilterDlg->radiusLong->setKFLogDegree(currentWaypointCatalog->radiusLong);
  importFilterDlg->selectRadius( currentWaypointCatalog->centerRef );

  QString radTxt = QString::number(currentWaypointCatalog->radiusSize);

  int idx = importFilterDlg->radius->findText( radTxt );

  if( idx != -1 )
    {
      importFilterDlg->radius->setCurrentIndex( idx );
    }
  else
    {
      importFilterDlg->radius->setCurrentIndex( 0 );
    }

  importFilterDlg->airfieldRefTxt = currentWaypointCatalog->airfieldRef;
}

/** No descriptions */
void Waypoints::slotImportWaypointFromFile()
{
  extern QSettings _settings;

  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

   // we should not include types we don't support (yet). Also, the strings should be translated.
  QString filter;
  filter.append( tr("Volkslogger format") + " (*.dbt *.DBT)" );

  QString fName = QFileDialog::getOpenFileName( this,
                                                tr("Import waypoints from file"),
                                                wayPointDir,
                                                filter );
  if( !fName.isEmpty() )
    {
      // read from disk
      currentWaypointCatalog->modified = true;

      if( fName.right( 4 ).toLower() == ".dbt" )
        {
          currentWaypointCatalog->importVolkslogger( fName );
        }
      else
        {
          currentWaypointCatalog->modified = false;
        }

      fillWaypoints();
    }
}

void Waypoints::openCatalog( QString &catalog )
{
  if( !catalog.isEmpty() )
    {
      int newItem = catalogName->count();

      WaypointCatalog *wc = new WaypointCatalog( "" );
      QFile f( catalog );

      if( f.exists() )
        {
          // read from disk
          if( ! wc->load( catalog ) )
            {
              delete wc;
            }
          else
            {
              waypointCatalogs.append( wc );
              currentWaypointCatalog = wc;
              catalogName->addItem( wc->path );

              catalogName->setCurrentIndex( newItem );
              qDebug() << "New Waypoint Catalog" << wc->path
                       << "added with" << wc->wpList.size() << "items";

              slotSwitchWaypointCatalog( newItem );
            }
        }
      else
        {
          delete wc;
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
