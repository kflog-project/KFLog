/***********************************************************************
**
**   objecttree.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
  * \class ObjectTree
  *
  * \author André Somers, Axel Pauli
  *
  * \brief Object browser - Displays all currently loaded objects.
  *
  * This object is used to give the user an overview of the objects he has
  * currently loaded. These objects include Flights, Tasks, Flight groups and
  * possibly more in the future. The loaded objects are represented in a
  * tree view, giving easy access to each object to the user.
  *
  * \date 2003-2023
  *
  * \version 1.2
  */

#pragma once

#include "flight.h"
#include "flightgroup.h"
#include "flighttask.h"
#include "kflogtreewidget.h"

class QAction;
class QMenu;
class QWidget;
class QTreeWidgetItem;

class ObjectTree : public KFLogTreeWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( ObjectTree )

public:

  /**
   * Constructor. Uses the default @ref QWidget arguments
   */
  ObjectTree( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~ObjectTree();

protected:

  void dragMoveEvent( QDragMoveEvent * event );

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
   * Contains a reference to the root node for flights
   */
  QTreeWidgetItem* FlightRoot;

  /**
   * Contains a reference to the root node for flight groups
   */
  QTreeWidgetItem* FlightGroupRoot;

  /**
   * Contains a reference to the root node for tasks
   */
  QTreeWidgetItem* TaskRoot;

  /**
   * The popup flight menu of the object tree.
   */
  QMenu *flightMenu;

  /**
   * The popup flight group menu of the object tree.
   */
  QMenu *flightGroupMenu;

  /**
   * The popup task menu of the object tree.
   */
  QMenu *taskMenu;

  /**
   * The popup airspace menu of the object tree.
   */
  QMenu *airspaceMenu;

  /**
   * References for task-related items in the popup menu
   */
  QAction* actionTaskNew;
  QAction* actionTaskOpen;
  QAction* actionTaskEdit;
  QAction* actionTaskClose;
  QAction* actionTaskSave;
  QAction* actionTaskSaveAll;

  /**
   * References for flight-related items in the popup menu
   */
  QAction* actionFlightOpen;
  QAction* actionFlightClose;
  QAction* actionFlightSetQNH;
  QAction* actionFlightOptimize;
  QAction* actionFlightOptimizeOLC;
  QAction* actionFlightCheckAirspaces;

  QAction* actionFlightGroupNew;
  QAction* actionFlightGroupEdit;
  QAction* actionFlightGroupClose;

  BaseFlightElement* currentFlightElement;
  QString path;

  int currentFlightElementType();

  /** Creates the popup menus provided by this widget. */
  void createMenus();

  /** Sets the what's that help text. */
  void setHelpText();

public slots:

  /**
   * Called when a new flight has been added.
   */
  void slotNewFlightAdded(Flight *);
  /**
   * Called when a new flight group has been added.
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
  /**
   * Signaled if the current flight was somehow changed.
   */
  void slotFlightChanged();
  /**
   * Signaled if a flight element is going to be closed. Used to
   * remove the item from the list.
   */
  void slotCloseFlight(BaseFlightElement*);
  /**
   * Called, if all flight nodes shall be updated.
   */
  void slotUpdateAllFlights();

private slots:

  /** No descriptions */
  void slotEditTask();

  void slotCloseFlightElement();

  void slotSaveTask();

  void slotSaveAllTask();

  //void slotSelectTask(QListViewItem *item);

  void slotShowObjectTreeMenu( QTreeWidgetItem *item, const QPoint &position );
  /**
   * Called if the selection has been changed.
   */
  void slotSelectionChanged( QTreeWidgetItem *item, int column );

  void slotItemDoubleClicked( QTreeWidgetItem *item, int column );

  /**
   * Called if an item is expanded.
   */
  void slotItemExpanded( QTreeWidgetItem* item );

signals:
  /**
   * Send out whenever the user selects a flight, task, or flight group
   */
  void newFlightSelected(BaseFlightElement *);
  /**
   * indicate that a new task should be created
   */
  void newTask();

  /**
   * indicate that a task should be opened. Is used to show the OpenTask dialog
   */
  void openTask();

  /**
   * indicate that a base flight element should be closed
   */
  void closeFlightElement();

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
  void openFile(const QUrl&);

  /**
   * Sets the current flight's QNH
   */
  void setFlightQNH();

  /**
   * Request for updating of all flight windows..
   */
  void updateFlightWindows();

  /**
   * Indicate that the current flight should be optimized
   */
  void optimizeFlight();

  /**
   * Indicate that the current flight should be optimized for OLC declaration
   */
  void optimizeFlightOLC();

  /**
   * Emitted to draw the current selected flight at the map.
   */
  void showCurrentFlight();

protected:

  /**
   * Searches the object tree for the node representing the base flight element
   * given as an argument.
   *
   * @returns a pointer to the QTreeWidgetItem if found, otherwise 0.
   */
  QTreeWidgetItem* findFlightElement( BaseFlightElement* bfe );

  /** No descriptions */
  void addTaskWindow(QWidget *parent);
};
