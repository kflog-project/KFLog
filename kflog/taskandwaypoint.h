/***************************************************************************
                          taskandwaypoint.h  -  description
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

#ifndef TASKANDWAYPOINT_H
#define TASKANDWAYPOINT_H

#include "waypointdialog.h"
#include "waypointcatalog.h"
#include "translationlist.h"
#include "guicontrols/kflogtable.h"

#include <qwidget.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qlist.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kpopupmenu.h>

/**
  *@author Harald Maier
  */

class TaskAndWaypoint : public KDialog  {
   Q_OBJECT
public: 
	TaskAndWaypoint(QWidget *parent=0, const char *name=0);
	~TaskAndWaypoint();
private: // Private methods
  /** No descriptions */
  void initSurfaces();
  /** No descriptions */
  void initTypes();
  /** No descriptions */
  void addTaskWindow(QSplitter *s);
  /** No descriptions */
  void addWaypointWindow(QSplitter *s);
  /** No descriptions */
  void addPopupMenu();
  /** No descriptions */
  void fillWaypoints();
private: // Private attributes
  TranslationList surfaces;
  TranslationList waypointTypes;
  /** popup menu for tasks's */
  KPopupMenu *taskPointPopup;
  /** popup menu for waypoint's */
  KPopupMenu *wayPointPopup;
  /**  */
  QComboBox *catalogName;
  KFLogTable *waypoints;
  /** some menu items */
  int idWaypointCatalogSave;
  int idWaypointCatalogImport;
  int idWaypointCatalogClose;
  int idWaypointNew;
  int idWaypointEdit;
  int idWaypointDelete;
  int idWaypointCopy2Task;
  /** all loaded catalogs */
  QList<WaypointCatalog> waypointCatalogs;
  /**  */
  WaypointDialog *waypointDlg;
private slots: // Private slots
  void slotNotHandledItem();
  void slotNewWaypoint();
  void slotDeleteWaypoint();
  void slotEditWaypoint();
  /** insert waypoint from waypoint dialog */
  void slotAddWaypoint();
  void reject();
  /** create a new catalog */
  void slotNewWaypointCatalog();
  /** open a catalog and set it active */
  void slotOpenWaypointCatalog();
  void slotSaveWaypointCatalog();
  void slotCloseWaypointCatalog();
  void slotImportWaypointCatalog();
  void slotSwitchWaypointCatalog(int idx);
  void showTaskPopup(int row, int col, int button, const QPoint &mousePos);
  void showWaypointPopup(int row, int col, int button, const QPoint &mousePos);
public slots: // Public slots
};

#endif
