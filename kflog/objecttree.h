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

#include <qwidget.h>
#include <qlistview.h>

class BaseFlightElement;
class Flight;
class FlightTask;

/**
  * @short Displays all currently loaded objects.
  * @author André Somers
  *
  * This object is used to give the user an overview of the objects he has
  * currently loaded. These objects include Flights, Tasks, Flightgroups and
  * possibly more in the future. The loaded objects are represented in a
  * treeview, giving easy access to each object to the user.
  */

class ObjectTree : public QListView  {
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
  QListViewItem * FlightRoot;
  /**
   * Contains a reference to the rootnode for flightgroups
   */
  QListViewItem * FlightGroupRoot;
  /**
   * Contains a reference to the rootnode for tasks
   */
  QListViewItem * TaskRoot;
  
public slots: // Public slots
  /**
   * Called if a new flight has been added.
   */
  void slotNewFlightAdded(Flight *);
  /**
   * Called if a new task has been created or loaded.
   */
  void slotNewTaskAdded(FlightTask *);
  /**
   * This slot is called if the currently selected flight has changed.
   */
  void slotSelectedFlightChanged(BaseFlightElement *);
  /** Signaled if the current flight was somehow changed.  */
  void slotFlightChanged();

signals: // Signals
  /**
   * Send out whenever the user selects a flight, task, or flightgroup
   */
  void selectedFlight(BaseFlightElement *);

private slots: // Private slots
  /**
   * Called if the selection has changed.
   */
  void slotSelected(QListViewItem *);

protected: // Protected methods
  /**
   * Searches the objecttree for the node representing the baseflightelement
   * given as an argument.
   * @returns a pointer to the QListViewItem if found, 0 otherwise.
   */
  QListViewItem * findFlightElement(BaseFlightElement * bfe);
};

#endif
