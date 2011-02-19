/***********************************************************************
**
**   objecttree.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * \class ObjectTree
  *
  * \author André Somers, Axel Pauli
  *
  * \brief KFLogBrowser - Displays all currently loaded objects.
  *
  * This object is used to give the user an overview of the objects he has
  * currently loaded. These objects include Flights, Tasks, Flight groups and
  * possibly more in the future. The loaded objects are represented in a
  * tree view, giving easy access to each object to the user.
  *
  * \date 2003-2011
  *
  * \version $Id$
  */

#ifndef OBJECT_TREE_H
#define OBJECT_TREE_H

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
  //QTreeWidgetItem* FlightGroupRoot;
  /**
   * Contains a reference to the root node for tasks
   */
  QTreeWidgetItem* TaskRoot;

  QMenu *objectTreeMenu;
  /**
   * References for task-related items in the popup menu
   */
  QAction* actionTaskEdit;
  QAction* actionTaskDelete;
  QAction* actionTaskSave;
  QAction* actionTaskSaveAll;
  /**
   * References for flight-related items in the popup menu
   */
  QAction* actionFlightGroupNew;
  QAction* actionFlightGroupEdit;
  QAction* actionFlightClose;
  QAction* actionFlightOptimize;
  QAction* actionFlightOptimizeOLC;

  BaseFlightElement* currentFlightElement;
  QString path;

  int currentFlightElementType();

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

private slots:

  /** No descriptions */
  void slotEditTask();

  void slotDeleteFlightElement();

  void slotSaveTask();

  void slotSaveAllTask();

  //void slotSelectTask(QListViewItem *item);

  void slotShowObjectTreeMenu( QTreeWidgetItem *item, const QPoint &position );
  /**
   * Called if the selection has been changed.
   */
  void slotSelectionChanged( QTreeWidgetItem * item, int column );

signals:
  /**
   * Send out whenever the user selects a flight, task, or flight group
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
   * Indicate that the current flight should be optimized
   */
  void optimizeFlight();
  /**
   * Indicate that the current flight should be optimized for OLC declaration
   */
  void optimizeFlightOLC();

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

  /** Creates the popup menu provided by this widget. */
  void createMenu();
};

#endif
