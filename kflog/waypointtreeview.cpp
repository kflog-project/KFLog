/***************************************************************************
                          waypointtreeview.cpp
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
#include "map.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "runway.h"
#include "wgspoint.h"
#include "waypointtreeview.h"
#include "mainwindow.h"

extern MainWindow  *_mainWindow;
extern Map         *_globalMap;
extern MapConfig   *_globalMapConfig;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;
extern QSettings   _settings;

WaypointTreeView::WaypointTreeView(QWidget *parent, const QString& catalog) :
  QWidget(parent),
  currentWaypointCatalog(0)
{
  Q_UNUSED( catalog )

  setObjectName( "WaypointTreeView" );
  setToolTip( tr("<html>Press right mouse button to open the waypoint menu.<br><br>"
                 "Press middle mouse button to open the table columns menu.</html>") );

  createWaypointWindow();
  createMenu();

  waypointDlg = new WaypointDialog(this);

  connect( waypointDlg, SIGNAL(addWaypoint(Waypoint *)),
           this, SLOT(slotAddWaypoint(Waypoint *)) );

  importFilterDlg = new WaypointImpFilterDialog(this);
}

WaypointTreeView::~WaypointTreeView()
{
  qDeleteAll( waypointCatalogs );
}

void WaypointTreeView::createWaypointWindow()
{
  waypointTree = new KFLogTreeWidget( "WaypointTreeView", this );
  waypointTree->setSortingEnabled( true );
  waypointTree->setAllColumnsShowFocus( true );
  waypointTree->setFocusPolicy( Qt::StrongFocus );
  waypointTree->setRootIsDecorated( false );
  waypointTree->setItemsExpandable( false );
  waypointTree->setSelectionMode( QAbstractItemView::ExtendedSelection );
  waypointTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  waypointTree->setAlternatingRowColors( true );
  waypointTree->addRowSpacing( 5 );
  waypointTree->setColumnCount( 14 );

  QStringList headerLabels;

  headerLabels  << tr("Name")
                << tr("Description")
                << tr("Country")
                << tr("ICAO")
                << tr("Type")
                << tr("Latitude")
                << tr("Longitude")
                << tr("Elevation")
                << tr("Frequency")
                << tr("Landable")
                << tr("Runway")
                << tr("Length")
                << tr("Surface")
                << tr("Comment");

  waypointTree->setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = waypointTree->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );
  headerItem->setTextAlignment( 5, Qt::AlignCenter );
  headerItem->setTextAlignment( 6, Qt::AlignCenter );
  headerItem->setTextAlignment( 7, Qt::AlignCenter );
  headerItem->setTextAlignment( 8, Qt::AlignCenter );
  headerItem->setTextAlignment( 9, Qt::AlignCenter );
  headerItem->setTextAlignment( 10, Qt::AlignCenter );
  headerItem->setTextAlignment( 11, Qt::AlignCenter );
  headerItem->setTextAlignment( 12, Qt::AlignCenter );
  headerItem->setTextAlignment( 13, Qt::AlignCenter );

  colName = 0;
  colDesc = 1;
  colCountry = 2;
  colICAO = 3;
  colType = 4;
  colLat = 5;
  colLong = 6;
  colElev = 7;
  colFrequency = 8;
  colLandable = 9;
  colRunway = 10;
  colLength = 11;
  colSurface = 12;
  colComment = 13;

  // Try to load a stored header configuration.
  waypointTree->loadConfig();

  connect( waypointTree,
          SIGNAL(rightButtonPressed(QTreeWidgetItem*, const QPoint&)),
          SLOT(slotShowWaypointMenu(QTreeWidgetItem*, const QPoint&)) );

  connect( waypointTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
           SLOT(slotEditWaypoint()) );

  // header
  QHBoxLayout *header = new QHBoxLayout;
  header->setSpacing(10);

  QLabel *label = new QLabel(QString("%1:").arg(tr("Waypoints")));
  label->setMaximumWidth(label->sizeHint().width() + 5);

  catalogBox = new QComboBox;
  connect(catalogBox, SIGNAL(activated(int)), SLOT(slotSwitchWaypointCatalog(int)));

  listItems = new QLabel( tr("Items: 0") );

  QPushButton *fileOpen = new QPushButton;
  fileOpen->setIcon(_mainWindow->getPixmap("kde_fileopen_16.png"));
  fileOpen->setToolTip( tr("Open a waypoint catalog.") );
  QSizePolicy sp = fileOpen->sizePolicy();
  sp.setHorizontalPolicy(QSizePolicy::Fixed);
  fileOpen->setSizePolicy(sp);
  connect(fileOpen, SIGNAL(clicked()), SLOT(slotOpenWaypointCatalog()));

  QPushButton *filter = new QPushButton(tr("Filter"));
  sp = filter->sizePolicy();
  sp.setHorizontalPolicy(QSizePolicy::Fixed);
  filter->setSizePolicy(sp);
  connect(filter, SIGNAL(clicked()), SLOT(slotFilterWaypoints()));

  header->addWidget(label);
  header->addWidget(catalogBox);
  header->addWidget(listItems);
  header->addWidget(fileOpen);
  header->addWidget(filter);

  QVBoxLayout *layout = new QVBoxLayout;

  layout->setSpacing( 5 );
  layout->addLayout(header);
  layout->addWidget(waypointTree);

  setLayout( layout );
}

/** Create menus. */
void WaypointTreeView::createMenu()
{
  wayPointMenu = new QMenu(waypointTree);

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
                                                  tr("&Delete selected waypoints"),
                                                  this,
                                                  SLOT(slotDeleteWaypoints()) );

  catalogCopySubMenu = wayPointMenu->addMenu( _mainWindow->getPixmap("kde_editcopy_16.png"),
                                              tr("Copy selected waypoints to &catalog" ) );

  catalogMoveSubMenu = wayPointMenu->addMenu( _mainWindow->getPixmap("kde_move_16.png"),
                                              tr("Move selected waypoints to &catalog") );
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
 * Copies all selected waypoints to the selected catalog.
 */
void WaypointTreeView::slotCopy2Catalog( QAction* action )
{
  int id = action->data().toInt();

  QList<QTreeWidgetItem *> items = waypointTree->selectedItems();

  if( items.size() == 0 )
    {
      return;
    }

  for( int i = 0; i < items.size(); i++ )
    {
      // qDebug() << "CopyItem:" << items.at(i)->text( colName );

      int idx;
      QString wpName = items.at(i)->text( colName );
      Waypoint *wpt = currentWaypointCatalog->findWaypoint( wpName, idx );

      waypointCatalogs.at( id )->wpList.append( new Waypoint( wpt ) );
      waypointCatalogs.at( id )->modified = true;
    }
}

/**
 * Moves all selected waypoints to the selected catalog.
 */
void WaypointTreeView::slotMove2Catalog( QAction* action )
{
  int id = action->data().toInt();

  QList<QTreeWidgetItem *> items = waypointTree->selectedItems();

  if( items.size() == 0 )
    {
      return;
    }

  for( int i = 0; i < items.size(); i++ )
    {
      // qDebug() << "MoveItem:" << items.at(i)->text( colName );

      int idx;
      QString wpName = items.at(i)->text( colName );
      Waypoint *wpt = currentWaypointCatalog->findWaypoint( wpName, idx );

      // Take waypoint from source list
      wpt = currentWaypointCatalog->wpList.takeAt( idx );

      // Append waypoint to new list
      waypointCatalogs.at( id )->wpList.append( wpt );

      delete waypointTree->takeTopLevelItem( waypointTree->indexOfTopLevelItem(items.at(i)) );
    }

  waypointTree->slotResizeColumns2Content();
  currentWaypointCatalog->modified = true;
  waypointCatalogs.value( id )->modified = true;
  updateWpListItems();
  emit waypointCatalogChanged(currentWaypointCatalog);
}

/** open a catalog and set it active */
void WaypointTreeView::slotOpenWaypointCatalog()
{
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

void WaypointTreeView::slotShowWaypointMenu( QTreeWidgetItem* item, const QPoint& position )
{
  Q_UNUSED( position )

  //enable and disable the correct menu items
  ActionWaypointCatalogSave->setEnabled(waypointCatalogs.count() && currentWaypointCatalog->modified);
  ActionWaypointCatalogSaveAs->setEnabled(waypointCatalogs.count() > 0);
  ActionWaypointCatalogClose->setEnabled( waypointCatalogs.count() > 1 );
  ActionWaypointCatalogImport->setEnabled( waypointCatalogs.count() );
  ActionWaypointImportFromMap->setEnabled( waypointCatalogs.count() );

  ActionWaypointNew->setEnabled( waypointCatalogs.count() );
  ActionWaypointEdit->setEnabled( item != 0 );
  ActionWaypointCenterMap->setEnabled( item != 0 );
  ActionWaypointCopy2Task->setEnabled( item != 0 && _globalMap->getPlanningState() == 1 );
  ActionWaypointSetHome->setEnabled( item != 0 );

  if( item )
    {
      QString home = item->text( colDesc );

      if( home.isEmpty() )
        {
          home = item->text( colName );
        }

      QString text = tr("Set Homesite") + " -> " + home;
      ActionWaypointSetHome->setText( text );

      text = tr("Copy to &task") + " (" + home + ")";
      ActionWaypointCopy2Task->setText( text );
    }
  else
    {
      ActionWaypointSetHome->setText( tr("Set Homesite") );
      ActionWaypointCopy2Task->setText( tr("Copy to &task") );
    }

  ActionWaypointDelete->setEnabled( waypointTree->selectedItems().size() );

  catalogCopySubMenu->setEnabled( waypointCatalogs.count() > 1 );
  catalogMoveSubMenu->setEnabled( waypointCatalogs.count() > 1 );

  // fill the submenus for the move & copy to catalog
  catalogCopySubMenu->clear();
  catalogMoveSubMenu->clear();

  if( waypointCatalogs.count() > 1 )
    {
      // get current catalog index
      int curCat = waypointCatalogs.indexOf( currentWaypointCatalog );

      for( int i = 0; i < waypointCatalogs.count(); i++ )
        {
          if( curCat != i )
            {
              // only insert if this catalog is NOT the current catalog...
              QAction* action = catalogCopySubMenu->addAction( waypointCatalogs.at(i)->path );
              action->setData( i );

              action = catalogMoveSubMenu->addAction( waypointCatalogs.at(i)->path );
              action->setData( i );
            }
        }
    }

  wayPointMenu->exec( QCursor::pos() );
}

void WaypointTreeView::slotNewWaypoint()
{
  waypointDlg->clear();
  waypointDlg->exec();
}

/** create a new catalog */
void WaypointTreeView::slotNewWaypointCatalog()
{
  int newItem = catalogBox->count();
  WaypointCatalog *w = new WaypointCatalog;

  waypointCatalogs.append(w);
  catalogBox->addItem(w->path);

  catalogBox->setCurrentIndex(newItem);
  slotSwitchWaypointCatalog(newItem);
}

/* save changes in catalogs, return success */
bool WaypointTreeView::saveChanges()
{
  WaypointCatalog *wc;

  foreach(wc, waypointCatalogs)
    {
      if (wc->modified)
        {
          QMessageBox::StandardButton button =
              QMessageBox::question( this,
                                     tr("Save changes?"),
                                     tr("<html>The waypoint file has been modified.<br>Save changes to<BR><B>%1</B></html>").arg(wc->path),
                                     QMessageBox::Yes|QMessageBox::Discard,
                                     QMessageBox::Yes );
          switch( button )
            {
            case QMessageBox::Yes:
              if ( !wc->save() )
                return false;
              break;

            case QMessageBox::Cancel:
              return false;

            default:
              return false;
            }
        }
    }

  return true;
}

/** No descriptions */
void WaypointTreeView::slotEditWaypoint()
{
  QTreeWidgetItem *item = waypointTree->currentItem();

  if( item != 0 )
    {
      int idx;
      QString oldName = item->text( colName );
      Waypoint *wpt = currentWaypointCatalog->findWaypoint( oldName, idx );
      slotEditWaypoint( wpt );
    }
}

/** No descriptions */
void WaypointTreeView::slotEditWaypoint(Waypoint* w)
{
  if( w )
  {
    QString tmp;

    // initialize dialog
    waypointDlg->setWindowTitle( tr( "Edit Waypoint" ) );
    waypointDlg->name->setText(w->name);
    waypointDlg->country->setText(w->country);
    waypointDlg->description->setText(w->description);
    // translate id to index
    waypointDlg->setWaypointType(w->type);
    waypointDlg->longitude->setKFLogDegree(w->origP.lon());
    waypointDlg->latitude->setKFLogDegree(w->origP.lat());
    waypointDlg->elevation->setText(QString("%1").arg(w->elevation, 0, 'f', 0) );
    waypointDlg->icao->setText(w->icao);

    tmp = QString("%1").arg(w->frequency, 3, 'f', 3, QChar('0'));

    while( tmp.size() < 7 )
      {
        // add leading zeros
        tmp.insert(0, "0");
      }

    waypointDlg->frequency->setText( tmp );
    waypointDlg->runway->setCurrentIndex( w->runway.first );
    waypointDlg->length->setText(QString("%1").arg(w->length, 0, 'f', 0) );

    // translate to id
    waypointDlg->setSurface(w->surface);
    waypointDlg->comment->setText(w->comment);
    waypointDlg->isLandable->setChecked(w->isLandable);
    waypointDlg->edit = true;

    if( waypointDlg->exec() == QDialog::Accepted )
      {
        if( !waypointDlg->name->text().isEmpty() )
          {
            w->name = waypointDlg->name->text().toUpper();
            w->country = waypointDlg->country->text().toUpper();
            w->description = waypointDlg->description->text();
            w->type = waypointDlg->getWaypointType();
            w->origP.setLat( waypointDlg->latitude->KFLogDegree() );
            w->origP.setLon( waypointDlg->longitude->KFLogDegree() );
            w->elevation = waypointDlg->elevation->text().toInt();
            w->icao = waypointDlg->icao->text().toUpper();
            w->frequency = waypointDlg->frequency->text().toFloat();
            w->runway.first = waypointDlg->runway->currentIndex();

            if( w->runway.first > 0 )
              {
                int rw1 = w->runway.first;

                w->runway.second = ((rw1 > 18) ? rw1 - 18 : rw1 + 18);
              }
            else
              {
                w->runway = QPair<ushort, ushort> ( 0, 0 );
              }

            tmp = waypointDlg->length->text();

            if( !tmp.isEmpty() )
              {
                w->length = tmp.toFloat();
              }
            else
              {
                w->length = 0.0;
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

/** This slot is called from the Map class. */
void WaypointTreeView::slotDeleteWaypoint(Waypoint* wp)
{
  if (wp)
  {
    QMessageBox waypointBox(tr("Delete waypoint?"),
                            tr("<html>Waypoint <b>%1</b> will be deleted.<br>Are you sure?</html>").arg(wp->name),
                            QMessageBox::Warning, QMessageBox::Ok, QMessageBox::Cancel, 0);

    waypointBox.setButtonText(QMessageBox::Ok, tr("&Delete"));

    if( waypointBox.exec() == QMessageBox::Ok )
        {
          currentWaypointCatalog->removeWaypoint( wp->name );
          currentWaypointCatalog->modified = true;

          QList<QTreeWidgetItem *> items;

          items = waypointTree->findItems( wp->name, Qt::MatchExactly, colName );

          for( int i = 0; i < items.size(); i++ )
            {
              delete waypointTree->takeTopLevelItem( waypointTree->indexOfTopLevelItem(items.at(i)) );
            }

          emit waypointCatalogChanged(currentWaypointCatalog);
        }
  }
}

/** This slot is called by the waypoint tree menu. */
void WaypointTreeView::slotDeleteWaypoints()
{
  QList<QTreeWidgetItem *> items = waypointTree->selectedItems();

  if( items.size() == 0 )
    {
      return;
    }

  QMessageBox::StandardButton button =
  QMessageBox::warning( this,
                        tr( "Delete waypoints?" ),
                        tr( "<html>All selected waypoints will be deleted.<br>Are you sure?</html>" ),
                        QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel );

  if( button != QMessageBox::Ok )
    {
      return;
    }

  for( int i = 0; i < items.size(); i++ )
    {
      // qDebug() << "RemoveItem:" << items.at(i)->text( colName );

      currentWaypointCatalog->removeWaypoint( items.at(i)->text( colName ) );
      delete waypointTree->takeTopLevelItem( waypointTree->indexOfTopLevelItem(items.at(i)) );
    }

  waypointTree->slotResizeColumns2Content();
  currentWaypointCatalog->modified = true;
  updateWpListItems();
  emit waypointCatalogChanged(currentWaypointCatalog);
}

/** No descriptions */
void WaypointTreeView::fillWaypoints()
{
  QString tmp;
  Waypoint *w;

  bool filterRadius, filterArea;

  waypointTree->clear();

  filterRadius = ( currentWaypointCatalog->getCenterPoint().lat() != 0  ||
                   currentWaypointCatalog->getCenterPoint().lon() != 0);

  filterArea   = ( currentWaypointCatalog->areaLat2 != 0 &&
                   currentWaypointCatalog->areaLong2 != 0 && !filterRadius);

  foreach( w, currentWaypointCatalog->wpList )
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

        // This distance is calculated in kilometers.
        double radiusDist = dist( currentWaypointCatalog->getCenterPoint().lat(),
                                  currentWaypointCatalog->getCenterPoint().lon(),
                                  w->origP.lat(),
                                  w->origP.lon() );

        if ( radiusDist > catalogDist )
          {
            continue;
          }
    }

    QTreeWidgetItem *item = new QTreeWidgetItem;

    item->setText(colName, w->name);
    item->setText(colDesc, w->description);
    item->setText(colCountry, w->country);
    item->setText(colICAO, w->icao);
    item->setText(colType, BaseMapElement::item2Text(w->type, tr("unknown")));
    item->setText(colLat,  WGSPoint::printPos(w->origP.lat(), true));
    item->setText(colLong, WGSPoint::printPos(w->origP.lon(), false));
    item->setText(colElev, QString::number(w->elevation) + " m");

    w->frequency > 0 ? tmp.sprintf("%.3f", w->frequency) : tmp = "";

    item->setText(colFrequency, tmp);

    item->setText(colLandable, w->isLandable == true ? tr("Yes") : "");

    if( w->runway.first > 0 )
      {
        tmp.sprintf( "%02d", w->runway.first );
      }
    else
      {
        tmp = "";
      }

    item->setText(colRunway, tmp);

    if( w->length > 0 )
      {
        tmp.sprintf( "%.0f m", w->length );
      }
    else
      {
        tmp = "";
      }

    item->setText(colLength, tmp);

    item->setText(colSurface, Runway::item2Text(w->surface) );
    item->setText(colComment, w->comment);
    item->setIcon(colName, _globalMapConfig->getPixmap(w->type, false, true) );

    // Set alignments of text labels
    item->setTextAlignment(colCountry, Qt::AlignCenter);
    item->setTextAlignment(colElev, Qt::AlignRight|Qt::AlignVCenter);
    item->setTextAlignment(colFrequency, Qt::AlignRight|Qt::AlignVCenter);
    item->setTextAlignment(colRunway, Qt::AlignCenter);
    item->setTextAlignment(colLength, Qt::AlignRight|Qt::AlignVCenter);
    item->setTextAlignment(colLandable, Qt::AlignCenter);
    item->setTextAlignment(colLat, Qt::AlignCenter);
    item->setTextAlignment(colLong, Qt::AlignCenter);

    waypointTree->addTopLevelItem( item );
  }

  waypointTree->slotResizeColumns2Content();
  waypointTree->sortByColumn(colName, Qt::AscendingOrder);
  updateWpListItems();

  emit waypointCatalogChanged(currentWaypointCatalog);
}

/** No descriptions */
void WaypointTreeView::slotSwitchWaypointCatalog(int idx)
{
  currentWaypointCatalog = waypointCatalogs.value(idx);
  fillWaypoints();
}

void WaypointTreeView::slotSaveWaypointCatalog()
{
  if(currentWaypointCatalog->save())
    {
      catalogBox->setItemText( catalogBox->currentIndex(),
                               currentWaypointCatalog->path );
    }
}

void WaypointTreeView::slotSaveWaypointCatalogAs()
{
  if(currentWaypointCatalog->save(true))
    {
      catalogBox->setItemText( catalogBox->currentIndex(),
                               currentWaypointCatalog->path );
    }
}

void WaypointTreeView::slotImportWaypointCatalog()
{
  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

  QString filter;
  filter.append(tr("KFLog") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("All") + " (*.*)");

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

void WaypointTreeView::slotCloseWaypointCatalog()
{
  int idx = waypointCatalogs.indexOf(currentWaypointCatalog);
  int cnt;

  if(currentWaypointCatalog->modified)
    {
    switch( QMessageBox::warning(this, tr("Save"),
           "<html>" + tr("Save changes to<BR><B>%1</B>").arg(currentWaypointCatalog->path) + "</html>",
           QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel))
    {
      case QMessageBox::Yes:

        if ( !currentWaypointCatalog->save())
          {
            return;
          }
        break;

      case QMessageBox::Cancel:
        return;
    }
  }

  waypointCatalogs.removeOne(currentWaypointCatalog);
  delete currentWaypointCatalog;

  catalogBox->removeItem(idx);

  // activate new catalog
  cnt = catalogBox->count();

  if( idx >= cnt )
    {
      idx = cnt - 1;
    }

  catalogBox->setCurrentIndex(idx);
  updateWpListItems();
  slotSwitchWaypointCatalog(idx);
}

void WaypointTreeView::slotImportWaypointFromMap()
{
  SinglePoint *s;
  Airfield *a;
  Waypoint *w;
  QList<Waypoint*> wl = currentWaypointCatalog->wpList;
  int loop;
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

      filterRadius = ( currentWaypointCatalog->getCenterPoint().lat() != 0  ||
                       currentWaypointCatalog->getCenterPoint().lon() != 0);

      filterArea = ( currentWaypointCatalog->areaLat2 != 0 &&
                     currentWaypointCatalog->areaLong2 != 0 && !filterRadius );

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
            double radiusDist = dist( currentWaypointCatalog->getCenterPoint().lat(),
                                      currentWaypointCatalog->getCenterPoint().lon(),
                                      p.lat(),
                                      p.lon());

            if ( radiusDist > catalogDist )
              {
                continue;
              }
          }

        w = new Waypoint;

        QString name = s->getName();
        w->name = name.replace(blank, "").left(8).toUpper();
        loop = 0;
        int idx;

        while(currentWaypointCatalog->findWaypoint(w->name, idx) && loop < 100000)
          {
            tmp.setNum(loop++);
            w->name = w->name.left(w->name.size() - tmp.length()) + tmp;
          }

        w->description = s->getName();
        w->country = s->getCountry();
        w->type = s->getObjectType();
        w->origP = s->getWGSPosition();
        w->elevation = s->getElevation();
        w->comment = s->getComment();

        switch(w->type)
        {
        case BaseMapElement::IntAirport:
        case BaseMapElement::Airport:
        case BaseMapElement::MilAirport:
        case BaseMapElement::CivMilAirport:
        case BaseMapElement::Airfield:
        case BaseMapElement::Gliderfield:

          w->icao = ((Airfield *) s)->getICAO();
          w->frequency = ((Airfield *) s)->getFrequency();
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
void WaypointTreeView::slotFilterWaypoints()
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
void WaypointTreeView::slotAddWaypoint(Waypoint *w)
{
  if( !currentWaypointCatalog )
    {
      //let's make sure we have a waypoint catalog
      WaypointCatalog* wpc = new WaypointCatalog;
      slotAddCatalog( wpc );
    }

  int loop = 1;

  if( w->name.isEmpty() )
    {
      int idx;
      w->name.sprintf( "WPT%03d", loop );

      while( currentWaypointCatalog->findWaypoint( w->name, idx ) && loop < 1000 )
        {
          w->name.sprintf( "WPT%03d", ++loop );
        }
    }

  currentWaypointCatalog->wpList.append( w );
  currentWaypointCatalog->modified = true;
  fillWaypoints();
}

void WaypointTreeView::slotCopyWaypoint2Task()
{
  QTreeWidgetItem *item = waypointTree->currentItem();

  if( item != 0 )
    {
      int idx;

      Waypoint *w = currentWaypointCatalog->findWaypoint( item->text( colName ), idx );

      emit copyWaypoint2Task( w );
    }
}

void WaypointTreeView::slotCenterMap()
{
  QTreeWidgetItem *item = waypointTree->currentItem();

  if (item != 0)
    {
      int idx;
      Waypoint *w = currentWaypointCatalog->findWaypoint(item->text(colName), idx);

      emit centerMap(w->origP.lat(), w->origP.lon());
    }
}

void WaypointTreeView::slotSetHome()
{
  QTreeWidgetItem *item = waypointTree->currentItem();

  if( item != 0 )
    {
      int idx;
      Waypoint *w = currentWaypointCatalog->findWaypoint( item->text( colName ), idx );

      _settings.setValue("/Homesite/Name", w->name);
      _settings.setValue("/Homesite/Latitude", w->origP.lat());
      _settings.setValue("/Homesite/Longitude", w->origP.lon());
      _settings.setValue("/Homesite/Country", w->country);

      // update airfield lists from Welt2000 if home site changes:
      _globalMapContents->slotReloadWelt2000Data();
      fillWaypoints();
  }
}

void WaypointTreeView::getFilterData()
{
  WGSPoint p;

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
  currentWaypointCatalog->centerRef = importFilterDlg->getCenterRef();

  int lat, lon;

  switch ( importFilterDlg->getCenterRef() )
  {

    case CENTER_POS:
      lat = importFilterDlg->centerLat->KFLogDegree();
      lon = importFilterDlg->centerLong->KFLogDegree();
      break;

    case CENTER_HOMESITE:
      lat = _settings.value("/Homesite/Latitude").toInt();
      lon = _settings.value("/Homesite/Longitude").toInt();
      break;

    case CENTER_MAP:
      p = _globalMapMatrix->getMapCenter(false);
      lat = p.lat();
      lon = p.lon();
      break;

    case CENTER_AIRFIELD:
      currentWaypointCatalog->airfieldRef = importFilterDlg->airfieldRefTxt;
      lat = importFilterDlg->getAirfieldRef().lat();
      lon = importFilterDlg->getAirfieldRef().lon();
      break;
  }

  currentWaypointCatalog->setCenterPoint( QPoint(lat, lon) );
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

void WaypointTreeView::setFilterData()
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

  importFilterDlg->centerLat->setKFLogDegree(currentWaypointCatalog->getCenterPoint().lat());
  importFilterDlg->centerLong->setKFLogDegree(currentWaypointCatalog->getCenterPoint().lon());
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
void WaypointTreeView::slotImportWaypointFromFile()
{
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

void WaypointTreeView::openCatalog( QString &catalog )
{
  if( ! catalog.isEmpty() )
    {
      int newItem = catalogBox->count();

      WaypointCatalog *wc = new WaypointCatalog( catalog );

      waypointCatalogs.append( wc );
      currentWaypointCatalog = wc;
      catalogBox->addItem( wc->path );
      catalogBox->setCurrentIndex( newItem );

      QFile f( catalog );

      if( f.exists() )
        {
          // read from disk
          wc->load( catalog );
        }

      qDebug() << "New Waypoint Catalog" << wc->path
               << "added with" << wc->wpList.size() << "items";

      slotSwitchWaypointCatalog( newItem );
    }
}

/* slot to set name of catalog and open it without a file selection dialog */
void WaypointTreeView::slotSetWaypointCatalogName( QString& catalog )
{
  if( !catalog.isEmpty() )
    {
      openCatalog( catalog );
    }
  else
    {
      slotNewWaypointCatalog();
    }
}

/** return the current waypoint catalog */
WaypointCatalog *WaypointTreeView::getCurrentCatalog()
{
  return currentWaypointCatalog;
}

void WaypointTreeView::slotAddCatalog(WaypointCatalog *w)
{
  int newItem = catalogBox->count();

  waypointCatalogs.append(w);
  catalogBox->addItem(w->path);

  catalogBox->setCurrentIndex(newItem);
  slotSwitchWaypointCatalog(newItem);
}

void WaypointTreeView::updateWpListItems()
{
  int items = 0;

  if( currentWaypointCatalog != 0 )
    {
      items = currentWaypointCatalog->wpList.size();
    }

  listItems->setText( tr("Items: ") + QString::number( items ) );
}
