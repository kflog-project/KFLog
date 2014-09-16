/***************************************************************************

   waypointtreeview.h

   This file is part of KFLog.

                             -------------------
    begin                : Fri Nov 30 2001
    copyright            : (C) 2001 by Harald Maier
                               2011-2014 by Axel Pauli

    This file is distributed under the terms of the General Public
    License. See the file COPYING for more information.

***************************************************************************/

/**
 * \class WaypointTreeView
 *
 * \author Harald Maier, Axel Pauli
 *
 * \short Waypoint tree widget for display of waypoints and their management.
 *
 * Waypoint tree widget for display of waypoints and their management.
 *
 * \date 2001-2014
 *
 * \version $Id$
 */

#ifndef WAYPOINT_TREE_VIEW_H
#define WAYPOINT_TREE_VIEW_H

#include "kflogtreewidget.h"
#include "waypoint.h"
#include "waypointcatalog.h"
#include "waypointdialog.h"
#include "waypointimpfilterdialog.h"

#include <QAction>
#include <QComboBox>
#include <QMenu>
#include <QWidget>

class WaypointTreeView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WaypointTreeView )

public:

  WaypointTreeView(QWidget *parent = 0, const QString& catalog = QString::null);

  ~WaypointTreeView();
  /**
   * save changes in catalogs, return success
   */
  bool saveChanges();
  /**
   * @return the current waypoint catalog
   */
  WaypointCatalog *getCurrentCatalog();

private: // Private methods

  /**
   * No descriptions
   */
  void createWaypointWindow();

  /** Create menus. */
  void createMenu();

  /**
   * read filter and store the values in the passed catalog
   */
  void setFilterDataInCatalog( WaypointCatalog* catalog );

  /**
   * set filter data to values from catalog
   */
  void setFilterDataFromCatalog();

  void openCatalog(QString &catalog);

  /** Update label about the number of waypoint items. */
  void updateWpListItems();

 private: // Private attributes

  /** Combobox with waypoint catalogs */
  QComboBox *catalogBox;

  /** Number of list items. */
  QLabel *listItems;

  /** Waypoint tree view widget */
  KFLogTreeWidget *waypointTree;

  /** menus for waypoint management */
  QMenu *wayPointMenu;
  QMenu *catalogCopySubMenu;
  QMenu *catalogMoveSubMenu;

  /** actions of menu items */
  QAction *ActionWaypointOpenDefaultCatalog;
  QAction *ActionWaypointCatalogSave;
  QAction *ActionWaypointCatalogSaveAs;
  QAction *ActionWaypointCatalogImport;
  QAction *ActionWaypointCatalogClose;
  QAction *ActionWaypointImportFromMap;
  QAction *ActionWaypointNew;
  QAction *ActionWaypointEdit;
  QAction *ActionWaypointDelete;
  QAction *ActionWaypointCopy2Task;
  QAction *ActionWaypointCenterMap;
  QAction *ActionWaypointSetHome;

  /** All loaded catalogs */
  QList<WaypointCatalog*> waypointCatalogs;

  /** The current used waypoint catalog. */
  WaypointCatalog *currentWaypointCatalog;

  /** Waypoint import filter dialog */
  WaypointImpFilterDialog *importFilterDlg;

  /** Column indexes of waypoint tree view */
  int colName;
  int colCountry;
  int colDesc;
  int colICAO;
  int colType;
  int colLat;
  int colLong;
  int colElev;
  int colFrequency;
  int colLandable;
  int colRunway;
  int colLength;
  int colSurface;
  int colComment;

 private slots:

 /**
  * Removes all selected waypoints from the tree view and from the current
  * catalog.
  */
  void slotDeleteWaypoints();

  /** Called if a waypoint should be edited. */
  void slotEditWaypoint();

  /** create a new catalog */
  void slotNewWaypointCatalog();
  /** open a catalog and set it active */
  void slotOpenDefaultWaypointCatalog();
  void slotOpenWaypointCatalog();
  void slotSaveWaypointCatalog();
  void slotSaveWaypointCatalogAs();
  void slotCloseWaypointCatalog();
  void slotImportWaypointCatalog();
  void slotSwitchWaypointCatalog(int idx);

  /** Called by tree view if right mouse button was pressed. */
  void slotShowWaypointMenu(QTreeWidgetItem* item, const QPoint& position);

  void slotImportWaypointFromMap();
  void slotCopyWaypoint2Task();
  void slotCenterMap();
  void slotSetHome();
 /**
  * Moves the current waypoint to the selected catalog
  */
  void slotMove2Catalog( QAction* action );
 /**
  * Copies the current waypoint to the selected catalog
  */
  void slotCopy2Catalog( QAction* action );

 public slots:
  /**
   * A waypoint has been added to the catalog
   */
  void slotNewWaypoint();
  /**
   * A waypoint has been deleted
   */
  void slotDeleteWaypoint(Waypoint*);
  /**
   * A waypoint should be edited.
   */
  void slotEditWaypoint(Waypoint*);
  /**
   * filter waypoints to display
   */
  void slotFilterWaypoints();
  /**
   * add a new waypoint
   */
  void slotAddWaypoint(Waypoint *w);
  /**
   * add a new waypoint from outside
   */
  void slotAddCatalog(WaypointCatalog *w);
  /**
   * Called when a new waypoint catalog was selected.
   */
   void slotSetWaypointCatalogName(QString& catalog);
   /**
    * Called to fill the waypoint list.
    */
   void slotFillWaypoints();

 signals:

  /**
   * Request to copy the referenced waypoint into the current task.
   */
  void copyWaypoint2Task(Waypoint *);
  /**
   * Signal is emitted when current waypoint catalog has been changed.
   */
  void waypointCatalogChanged(WaypointCatalog *);
  /**
   * Request to center the map on the given coordinates.
   */
  void centerMap(int, int);
};

#endif
