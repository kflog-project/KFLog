/***************************************************************************
                          waypoints.h  -  description
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

#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include "waypointimpfilterdialog.h"
#include "waypointdialog.h"
#include "waypointcatalog.h"
#include "guicontrols/kfloglistview.h"
#include "waypoint.h"

#include <qframe.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qptrlist.h>

#include <kpopupmenu.h>

/**
 *@short Waypoint-frame for display in interface
 *
 *@author Harald Maier
 *@version (unkown)
 */

class Waypoints : public QFrame  {
  Q_OBJECT
    public:
  Waypoints(QWidget *parent = 0, const char *name = 0, const QString& catalog = QString::null);
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
  /** No descriptions */
  void addPopupMenu();
  /** No descriptions */
  void fillWaypoints();
  /**
   * read filter and store in catalog
   */
  void getFilterData();
  void openCatalog(QString &catalog);
 private: // Private attributes
  /** popup menu for waypoint's */
  KPopupMenu *wayPointPopup;
  KPopupMenu *catalogCopySubPopup;
  KPopupMenu *catalogMoveSubPopup;
  
  /**  */
  QComboBox *catalogName;
  KFLogListView *waypoints;
  /** some menu items */
  int idWaypointCatalogSave;
  int idWaypointCatalogSaveAs;
  int idWaypointCatalogImport;
  int idWaypointCatalogClose;
  int idWaypointImportFromMap;
  int idWaypointImportFromFile;
  int idWaypointNew;
  int idWaypointEdit;
  int idWaypointDelete;
  int idWaypointCopy2Task;
  int idWaypointCenterMap;
  int idWaypointSetHome;
  int idWaypointCopy2Catalog;
  int idWaypointMove2Catalog;

  /** all loaded catalogs */
  QPtrList<WaypointCatalog> waypointCatalogs;
  /**  */
  WaypointDialog *waypointDlg;
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

  private slots: // Private slots
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
  void showWaypointPopup(QListViewItem *, const QPoint &, int);
  void slotImportWaypointFromMap();
  void slotCopyWaypoint2Task();
  void slotCenterMap();
  void slotSetHome();
 /**
  * Moves the current waypoint to the selected catalog
  */
  void slotMove2Catalog(int);
 /**
  * Copies the current waypoint to the selected catalog
  */
  void slotCopy2Catalog(int);

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
 signals: // Signals
  /**
   * Request to copy the referenced waypoint into the current task
   */
  void copyWaypoint2Task(Waypoint *);
  /**
   * signal to emit when current waypointcatalog has changed.
   */
  void waypointCatalogChanged(WaypointCatalog *);
  /**
   * Request to center the map on the given coordinates
   */
  void centerMap(int, int);
};

#endif
