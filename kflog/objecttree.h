/***********************************************************************
**
**   objecttree.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef OBJECTTREE_H
#define OBJECTTREE_H

#include <q3listview.h>
#include <q3popupmenu.h>
#include <QWidget>

#include "flight.h"
#include "flightgroup.h"
#include "flighttask.h"

/**
  * @short KFLogBrowser - Displays all currently loaded objects.
  * @author André Somers
  *
  * This object is used to give the user an overview of the objects he has
  * currently loaded. These objects include Flights, Tasks, Flightgroups and
  * possibly more in the future. The loaded objects are represented in a
  * treeview, giving easy access to each object to the user.
  */

class ObjectTree : public Q3ListView  {
   Q_OBJECT
public:
  /**
   * Constructor. Uses the default @ref QWidget arguments
   */
  ObjectTree(QWidget *parent=0, const char *name=0);
  /**
   * Destructor
   */
  ~ObjectTree();
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);

private:
  /**
   * Contains the ID of the Name column
   */
  int colName;
  /**
   * Contains the ID of the Description column
   */
  int colDesc;

  /**
   * Contains a reference to the rootnode for flights
   */
  Q3ListViewItem * FlightRoot;
  /**
   * Contains a reference to the rootnode for flightgroups
   */
  //QListViewItem * FlightGroupRoot;
  /**
   * Contains a reference to the rootnode for tasks
   */
  Q3ListViewItem * TaskRoot;

  Q3PopupMenu *taskPopup;
  /**
   * References for task-related items in the popupmenu
   */
  int idTaskEdit;
  int idTaskDelete;
  int idTaskSave;
  int idTaskSaveAll;
  /**
   * References for flight-related items in the popupmenu
   */
  int idFlightGroupEdit;
  int idFlightClose;
  int idFlightOptimize;
  int idFlightOptimizeOLC;

  BaseFlightElement* currentFlightElement;
  int currentFlightElementType();
  QString path;
   
public slots: // Public slots
  /**
   * Called when a new flight has been added.
   */
  void slotNewFlightAdded(Flight *);
  /**
   * Called when a new flightgroup has been added.
   */
  void slotNewFlightGroupAdded(FlightGroup *);
  /**
   * Called when a new task has been created or loaded.
   */
  void slotNewTaskAdded(FlightTask *);
  /**
   * This slot is called if the currently selected flight has changed.
   */
  void slotSelectedFlightChanged();
  /** Signaled if the current flight was somehow changed.  */
  void slotFlightChanged();
  /** Signaled if a flightelement is going to be closed. Used to remove the item from the list. */
  void slotCloseFlight(BaseFlightElement*);

private slots: // Private slots
  /** No descriptions */
  void slotEditTask();
  void slotDeleteTask();
  void slotSaveTask();
  void slotSaveAllTask();
  //void slotSelectTask(QListViewItem *item);
  void showTaskPopup(Q3ListViewItem *it, const QPoint &, int);
  /**
   * Called if the selection has changed.
   */
  void slotSelected(Q3ListViewItem *);

signals: // Signals
  /**
   * Send out whenever the user selects a flight, task, or flightgroup
   */
  void selectedFlight(BaseFlightElement *);
  /**
   * indicate that a new task should be created
   */
  void newTask();
  /**
   * indicate that a task should be opened. Is used to show the OpenTask dialog
   */
  void openTask();
  /**
   * indicate that a baseflightelement should be closed
   */
  void closeTask();
  /**
   * indicate that a new flight group should be created
   */
  void newFlightGroup();
  /**
   * indicate that the current flight group must be edited
   */
  void editFlightGroup();
  /**
   * indicate that a flight should be opened. Is used to show the OpenFlight dialog
   */
  void openFlight();
  /**
   * Indicate that a file should be opened
   */
  void openFile(const char*);
  /**
   * Indicate that the current flight should be optimized
   */
  void optimizeFlight();
  /**
   * Indicate that the current flight should be optimized for OLC declaration
   */
  void optimizeFlightOLC();

protected: // Protected methods
  /**
   * Searches the objecttree for the node representing the baseflightelement
   * given as an argument.
   * @returns a pointer to the QListViewItem if found, 0 otherwise.
   */
  Q3ListViewItem * findFlightElement(BaseFlightElement * bfe);
  /** No descriptions */
  void addTaskWindow(QWidget *parent);
  void addPopupMenu();
};

#endif
