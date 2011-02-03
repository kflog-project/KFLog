/***************************************************************************
                          waypoints.h  -  description
                             -------------------
    begin                : Fri Nov 30 2001
    copyright            : (C) 2001 by Harald Maier
                               2011 by Axel Pauli

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

/**
 * \class Waypoints
 *
 * \author Harald Maier, Axel Pauli
 *
 * \short Waypoint widget for display an interface
 *
 * \date 2001-2011
 *
 * \version $Id$
 */

#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include "guicontrols/kfloglistview.h"
#include "waypoint.h"
#include "waypointcatalog.h"
#include "waypointdialog.h"
#include "waypointimpfilterdialog.h"

#include <QAction>
#include <QComboBox>
#include <QMenu>
#include <QSplitter>
#include <QWidget>

class Waypoints : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( Waypoints )

public:

  Waypoints(QWidget *parent = 0, const QString& catalog = QString::null);

  ~Waypoints();
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
  void addWaypointWindow(QWidget *parent);

  /** Create menus. */
  void createMenu();

  /** No descriptions */
  void fillWaypoints();
  /**
   * read filter and store in catalog
   */
  void getFilterData();

  /**
   * set filter data to values from catalog
   */
  void setFilterData();

  void openCatalog(QString &catalog);

 private: // Private attributes

  /** menu for waypoint's */
  QMenu *wayPointMenu;
  QMenu *catalogCopySubMenu;
  QMenu *catalogMoveSubMenu;

  /**  */
  QComboBox *catalogName;
  KFLogListView *waypoints;

  /** actions of menu items */
  QAction *ActionWaypointCatalogSave;
  QAction *ActionWaypointCatalogSaveAs;
  QAction *ActionWaypointCatalogImport;
  QAction *ActionWaypointCatalogClose;
  QAction *ActionWaypointImportFromMap;
  QAction *ActionWaypointImportFromFile;
  QAction *ActionWaypointNew;
  QAction *ActionWaypointEdit;
  QAction *ActionWaypointDelete;
  QAction *ActionWaypointCopy2Task;
  QAction *ActionWaypointCenterMap;
  QAction *ActionWaypointSetHome;

  /** all loaded catalogs */
  QList<WaypointCatalog*> waypointCatalogs;

  /** The current used waypoint catalog. */
  WaypointCatalog *currentWaypointCatalog;

  /** Wayoint dialog. */
  WaypointDialog *waypointDlg;

  /** Waypoint import filter dialog */
  WaypointImpFilterDialog *importFilterDlg;

  /** column index for waypoints */
  int colName;
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

  void slotDeleteWaypoint();
  void slotEditWaypoint();
  /** create a new catalog */
  void slotNewWaypointCatalog();
  /** open a catalog and set it active */
  void slotOpenWaypointCatalog();
  void slotSaveWaypointCatalog();
  void slotSaveWaypointCatalogAs();
  void slotCloseWaypointCatalog();
  void slotImportWaypointCatalog();
  void slotSwitchWaypointCatalog(int idx);
  void showWaypointMenu(Q3ListViewItem *, const QPoint &, int);
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

  public slots: // Public slots
  /**
   * A waypoint has been added to the catalog
   */
  void slotNewWaypoint();
  /**
   * A waypoint has been deleted
   */
  void slotDeleteWaypoint(Waypoint*);
  /**
   * filter waypoints to display
   */
  void slotEditWaypoint(Waypoint*);
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
   * No description
   */
  void slotImportWaypointFromFile();
  /**
   * No description
   */
   void slotSetWaypointCatalogName(QString catalog);

 signals:
  /**
   * Request to copy the referenced waypoint into the current task
   */
  void copyWaypoint2Task(Waypoint *);
  /**
   * signal to emit when current waypoint catalog has changed.
   */
  void waypointCatalogChanged(WaypointCatalog *);
  /**
   * Request to center the map on the given coordinates
   */
  void centerMap(int, int);
};

#endif
