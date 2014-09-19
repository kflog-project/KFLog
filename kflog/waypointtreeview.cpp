/***************************************************************************

   waypointtreeview.cpp

   This file is part of KFLog.

                             -------------------
    begin                : Fri Nov 30 2001
    copyright            : (C) 2001 by Harald Maier
                               2011-2014 by Axel Pauli

    This file is distributed under the terms of the General Public
    License. See the file COPYING for more information.

***************************************************************************/

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "airfield.h"
#include "kflogconfig.h"
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

  listItems = new QLabel( tr("Total Items: 0") );

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

  ActionWaypointOpenDefaultCatalog =
    wayPointMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
					       QObject::tr("&Open default catalog"),
					       this,
					       SLOT(slotOpenDefaultWaypointCatalog()) );

  wayPointMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
                                             QObject::tr("&Open catalog"),
                                             this,
                                             SLOT(slotOpenWaypointCatalog()) );


  ActionWaypointCatalogImport = wayPointMenu->addAction( tr("&Import from catalog"),
                                                         this,
                                                         SLOT(slotImportWaypointCatalog()) );

  ActionWaypointImportFromMap = wayPointMenu->addAction( tr("Import from &map"),
                                                         this,
                                                         SLOT(slotImportWaypointFromMap()) );

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

void WaypointTreeView::slotOpenDefaultWaypointCatalog()
{
  if( KFLogConfig::existsDefaultWaypointCatalog() == true )
    {
      QString catalog = KFLogConfig::getDefaultWaypointCatalog();
      openCatalog( catalog );
    }
}

/** open a catalog and set it active */
void WaypointTreeView::slotOpenWaypointCatalog()
{
  QString wayPointDir = _settings.value( "/Path/DefaultWaypointDirectory",
                                         _mainWindow->getApplicationDataDirectory() ).toString();

  QString filter;
  filter.append(tr("All formats") + " (WELT2000.TXT *.da4 *.DA4 *.dat *.DAT *.dbt *.DBT *.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP *.txt *.TXT);;");
  filter.append(tr("KFLog") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Cumulus") + " (*.kwp *.KWP);;");
  filter.append(tr("Cambridge") + " (*.dat *.DAT);;");
  filter.append(tr("Filser txt") + " (*.txt *.TXT);;");
  filter.append(tr("Filser da4") + " (*.da4 *.DA4);;");
  filter.append(tr("SeeYou") + " (*.cup *.CUP);;");
  filter.append(tr("Volkslogger") + " (*.dbt *.DBT);;" );
  filter.append(tr("Welt2000") + " (WELT2000.TXT)");

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

  // enable and disable the correct menu items
  ActionWaypointOpenDefaultCatalog->setEnabled( KFLogConfig::existsDefaultWaypointCatalog() );
  ActionWaypointCatalogSave->setEnabled(waypointCatalogs.count() && currentWaypointCatalog->modified);
  ActionWaypointCatalogSaveAs->setEnabled(waypointCatalogs.count() > 0);
  ActionWaypointCatalogClose->setEnabled( waypointCatalogs.count() > 0 );
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
  WaypointDialog* waypointDlg = new WaypointDialog(this);

  connect( waypointDlg, SIGNAL(addWaypoint(Waypoint *)),
           this, SLOT(slotAddWaypoint(Waypoint *)) );

  waypointDlg->exec();

  delete waypointDlg;
}

/** create a new catalog */
void WaypointTreeView::slotNewWaypointCatalog()
{
  int newItem = catalogBox->count();
  WaypointCatalog *wc = new WaypointCatalog;

  // Set filter data in catalog
  setFilterDataInCatalog( wc );

  waypointCatalogs.append(wc);
  catalogBox->addItem(wc->path);

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
  if( w == 0 )
    {
      return;
    }

  WaypointDialog* waypointDlg = new WaypointDialog(this);

  connect( waypointDlg, SIGNAL(addWaypoint(Waypoint *)),
           this, SLOT(slotAddWaypoint(Waypoint *)) );

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
  waypointDlg->setElevation(w->elevation);
  waypointDlg->icao->setText(w->icao);

  tmp = QString("%1").arg(w->frequency, 3, 'f', 3, QChar('0'));

  while( tmp.size() < 7 )
    {
      // add leading zeros
      tmp.insert(0, "0");
    }

  waypointDlg->frequency->setText( tmp );

  Runway rwy;

  if( w->rwyList.size() > 0 )
    {
      rwy = w->rwyList[0];
    }

  waypointDlg->runway->setCurrentIndex( rwy.m_heading.first );
  waypointDlg->length->setText(QString("%1").arg(rwy.m_length, 0, 'f', 0) );

  // translate to id
  waypointDlg->setSurface(rwy.m_surface);
  waypointDlg->comment->setText(w->comment);
  waypointDlg->isLandable->setChecked(rwy.m_isOpen);
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
          w->elevation = waypointDlg->getElevation();
          w->icao = waypointDlg->icao->text().toUpper();
          w->frequency = waypointDlg->frequency->text().toFloat();

          rwy.m_heading.first = waypointDlg->runway->currentIndex();

          if( rwy.m_heading.first > 0 )
            {
              int rw1 = rwy.m_heading.first;

              rwy.m_heading.second = ((rw1 > 18) ? rw1 - 18 : rw1 + 18);
            }
          else
            {
              rwy.m_heading = QPair<ushort, ushort> ( 0, 0 );
            }

          tmp = waypointDlg->length->text();

          if( !tmp.isEmpty() )
            {
              rwy.m_length = tmp.toFloat();
            }
          else
            {
              rwy.m_length = 0.0;
            }

          rwy.m_surface = (enum Runway::SurfaceType) waypointDlg->getSurface();
          w->comment = waypointDlg->comment->text();
          rwy.m_isOpen = waypointDlg->isLandable->isChecked();

          if( w->rwyList.size() > 0 )
            {
              w->rwyList.removeFirst();
              w->rwyList.insert(0, rwy);
            }

          currentWaypointCatalog->modified = true;
          slotFillWaypoints();
        }
    }

  delete waypointDlg;
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

void WaypointTreeView::slotFillWaypoints()
{
  QString tmp;
  Waypoint *w;

  bool filterRadius = false;
  bool filterArea = false;

  waypointTree->clear();

  if( currentWaypointCatalog == 0 )
    {
      // There is no waypoint catalog defined.
      return;
    }

  // Retrieve filter values from catalog
  setFilterDataFromCatalog();

  enum WaypointCatalog::FilterType ft = currentWaypointCatalog->getFilter();

  if( ft == WaypointCatalog::Radius )
    {
      filterRadius = ( currentWaypointCatalog->getCenterPoint().lat() != 0  ||
		       currentWaypointCatalog->getCenterPoint().lon() != 0);
    }
  else if( ft == WaypointCatalog::Area )
    {
      filterArea   = ( currentWaypointCatalog->areaLat2 != 0 &&
                       currentWaypointCatalog->areaLong2 != 0 && !filterRadius);
    }

  Altitude::altitudeUnit altUnit = Altitude::getUnit();

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
    else if (filterRadius && currentWaypointCatalog->radiusSize > 0.0 )
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

    if( altUnit == Altitude::feet )
      {
        item->setText(colElev,
            QString::number( Altitude(w->elevation).getFeet(), 'f', 0) + " " + Altitude::getUnitText());
      }
    else
      {
        // The default is always meters
        item->setText(colElev,
            QString::number(w->elevation, 'f', 0) + " " + Altitude::getUnitText());
      }

    w->frequency > 0 ? tmp.sprintf("%.3f", w->frequency) : tmp = "";

    item->setText(colFrequency, tmp);

    Runway rwy;

    if( w->rwyList.size() > 0 )
      {
        rwy = w->rwyList[0];
      }

    item->setText(colLandable, rwy.m_isOpen == true ? tr("Yes") : "");

    if( rwy.m_heading.first > 0 )
      {
        tmp.sprintf( "%02d/%02d", rwy.m_heading.first, rwy.m_heading.second );
        item->setText(colRunway, tmp);
      }

    if( rwy.m_length > 0 )
      {
        tmp.sprintf( "%.0f m", rwy.m_length );
        item->setText(colLength, tmp);
      }

    if( rwy.m_heading.first > 0 )
      {
        item->setText( colSurface, Runway::item2Text( rwy.m_surface ) );
      }

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

void WaypointTreeView::slotSwitchWaypointCatalog(int idx)
{
  currentWaypointCatalog = waypointCatalogs.value(idx);

  slotFillWaypoints();
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
  filter.append(tr("All formats") + " (WELT2000.TXT *.dat *.DAT *.dbt *.DBT *.cup *.CUP *.kflogwp *.KFLOGWP *.kwp *.KWP *.txt *.TXT);;");
  filter.append(tr("KFLog") + " (*.kflogwp *.KFLOGWP);;");
  filter.append(tr("Cumulus") + " (*.kwp *.KWP);;");
  filter.append(tr("Cambridge") + " (*.dat *.DAT);;");
  filter.append(tr("Filser txt") + " (*.txt *.TXT);;");
  filter.append(tr("Filser da4") + " (*.da4 *.DA4);;");
  filter.append(tr("SeeYou") + " (*.cup *.CUP);;");
  filter.append(tr("Volkslogger") + " (*.dbt *.DBT);;" );
  filter.append(tr("Welt2000") + " (WELT2000.TXT)");

  QString fName = QFileDialog::getOpenFileName( this,
                                                tr("Import waypoints from catalog"),
                                                wayPointDir,
                                                filter );
  if( ! fName.isEmpty() )
    {
      // Update filter of catalog

      // read from disk
      bool ok = currentWaypointCatalog->load(fName);
      currentWaypointCatalog->modified = ok;
      slotFillWaypoints();
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

  currentWaypointCatalog = 0;

  catalogBox->removeItem(idx);

  // activate new catalog
  cnt = catalogBox->count();

  if( idx >= cnt )
    {
      idx = cnt - 1;
    }

  if( idx < 0 )
    {
      // last catalog has been removed
      updateWpListItems();

      // Clear waypoint tree
      waypointTree->clear();

      emit waypointCatalogChanged(0);
      return;
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

  bool filterRadius = false;
  bool filterArea = false;

  if (importFilterDlg->exec() == QDialog::Accepted)
    {
      setFilterDataInCatalog( currentWaypointCatalog );

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

      enum WaypointCatalog::FilterType ft = importFilterDlg->getFilter();

      if( ft == WaypointCatalog::Radius )
	{
	  filterRadius = ( currentWaypointCatalog->getCenterPoint().lat() != 0  ||
			   currentWaypointCatalog->getCenterPoint().lon() != 0);
	}
      else if( ft == WaypointCatalog::Area )
      	{
	  filterArea = ( currentWaypointCatalog->areaLat2 != 0 &&
			 currentWaypointCatalog->areaLong2 != 0 && !filterRadius );
      	}

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
        else if ( filterRadius && currentWaypointCatalog->radiusSize > 0.0 )
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
        w->type = s->getTypeID();
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
          a = dynamic_cast<Airfield*>(s); // try casting to an airfield

          if( a )
            {
              w->rwyList = a->getRunwayList();
            }

          break;

        default:

          break;
        }

        currentWaypointCatalog->wpList.append(w);
      }
    }

    currentWaypointCatalog->modified = true;
    slotFillWaypoints();
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
      setFilterDataInCatalog( currentWaypointCatalog );
      slotFillWaypoints();
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

  int idx;
  int loop = 1;

  if( w->name.isEmpty() || currentWaypointCatalog->findWaypoint( w->name, idx ) )
    {
      w->name.sprintf( "WPT_%04d", loop );

      while( currentWaypointCatalog->findWaypoint( w->name, idx ) && loop < 10000 )
        {
          w->name.sprintf( "WPT%04d", ++loop );
        }

      if( w->description.isEmpty() )
        {
          w->description = w->name;
        }
    }

  currentWaypointCatalog->wpList.append( w );
  currentWaypointCatalog->modified = true;
  slotFillWaypoints();
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
      _globalMapContents->slotReloadAirfieldData();
      slotFillWaypoints();
  }
}

void WaypointTreeView::setFilterDataInCatalog( WaypointCatalog* catalog )
{
  if( ! catalog )
    {
      return;
    }

  WGSPoint p;

  catalog->showAll = importFilterDlg->useAll->isChecked();
  catalog->showAirfields = importFilterDlg->airfields->isChecked();
  catalog->showGliderfields = importFilterDlg->gliderfields->isChecked();
  catalog->showOtherSites = importFilterDlg->otherSites->isChecked();
  catalog->showObstacles = importFilterDlg->obstacles->isChecked();
  catalog->showLandmarks = importFilterDlg->landmarks->isChecked();
  catalog->showOutlandings = importFilterDlg->outlandings->isChecked();
  catalog->showStations = importFilterDlg->stations->isChecked();

  catalog->setFilter( importFilterDlg->getFilter() );
  catalog->areaLat1 = importFilterDlg->fromLat->KFLogDegree();
  catalog->areaLat2 = importFilterDlg->toLat->KFLogDegree();
  catalog->areaLong1 = importFilterDlg->fromLong->KFLogDegree();
  catalog->areaLong2 = importFilterDlg->toLong->KFLogDegree();
  catalog->centerRef = importFilterDlg->getCenterRef();

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
      catalog->airfieldRef = importFilterDlg->airfieldRefTxt;
      lat = importFilterDlg->getAirfieldRef().lat();
      lon = importFilterDlg->getAirfieldRef().lon();
      break;

  default:
      lat=0;
      lon=0;
      break;
  }

  catalog->setCenterPoint( QPoint(lat, lon) );
  catalog->radiusSize = importFilterDlg->getCenterRadius();

  // normalize coordinates
  if (catalog->areaLat1 > catalog->areaLat2)
    {
      int tmp = catalog->areaLat1;
      catalog->areaLat1 = catalog->areaLat2;
      catalog->areaLat2 = tmp;
    }

  if (catalog->areaLong1 > catalog->areaLong2)
    {
      int tmp = catalog->areaLong1;
      catalog->areaLong1 = catalog->areaLong2;
      catalog->areaLong2 = tmp;
    }
}

void WaypointTreeView::setFilterDataFromCatalog()
{
  importFilterDlg->useAll->setChecked(currentWaypointCatalog->showAll);
  importFilterDlg->airfields->setChecked(currentWaypointCatalog->showAirfields);
  importFilterDlg->gliderfields->setChecked(currentWaypointCatalog->showGliderfields);
  importFilterDlg->otherSites->setChecked(currentWaypointCatalog->showOtherSites);
  importFilterDlg->obstacles->setChecked(currentWaypointCatalog->showObstacles);
  importFilterDlg->landmarks->setChecked(currentWaypointCatalog->showLandmarks);
  importFilterDlg->outlandings->setChecked(currentWaypointCatalog->showOutlandings);
  importFilterDlg->stations->setChecked(currentWaypointCatalog->showStations);

  importFilterDlg->setFilter( currentWaypointCatalog->getFilter() );
  importFilterDlg->fromLat->setKFLogDegree(currentWaypointCatalog->areaLat1);
  importFilterDlg->toLat->setKFLogDegree(currentWaypointCatalog->areaLat2);
  importFilterDlg->fromLong->setKFLogDegree(currentWaypointCatalog->areaLong1);
  importFilterDlg->toLong->setKFLogDegree(currentWaypointCatalog->areaLong2);

  importFilterDlg->centerLat->setKFLogDegree(currentWaypointCatalog->getCenterPoint().lat());
  importFilterDlg->centerLong->setKFLogDegree(currentWaypointCatalog->getCenterPoint().lon());
  importFilterDlg->selectRadius( currentWaypointCatalog->centerRef );

  QString radTxt = QString::number(currentWaypointCatalog->radiusSize);

  importFilterDlg->setCenterRadius( radTxt );
  importFilterDlg->airfieldRefTxt = currentWaypointCatalog->airfieldRef;
}

void WaypointTreeView::openCatalog( QString &catalog )
{
  if( ! catalog.isEmpty() )
    {
      int newItem = catalogBox->count();

      WaypointCatalog *wc = new WaypointCatalog( catalog );

      waypointCatalogs.append( wc );
      currentWaypointCatalog = wc;

      // set filter data in current catalog
      setFilterDataInCatalog( wc );

      catalogBox->addItem( wc->path );
      catalogBox->setCurrentIndex( newItem );

      // store catalog as last opened one.
      KFLogConfig::setLastUsedWaypointCatalog( catalog );

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
  if( ! catalog.isEmpty() )
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

  listItems->setText( tr("Total Items: ") + QString::number( items ) + " - " +
                      tr("Filtered Items: ") +
                      QString::number( waypointTree->topLevelItemCount()) );
}
