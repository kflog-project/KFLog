/***********************************************************************
**
**   tasklistviewitem.h
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

#ifndef TASKLISTVIEWITEM_H
#define TASKLISTVIEWITEM_H

#include <qlistview.h>
#define TASKLISTVIEWITEM_TYPEID 10002

class FlightTask;

/**Represents a Task in a listview.
  *@author André Somers
  */

class TaskListViewItem : public QListViewItem  {
public: 
  /**
   * Constructor.
   * @param parent Reference to parent @ref QListViewItem
   * @param task Reference to @ref FlightTask object to display
   * @param insertAfter Reference to @ref QListViewItem after which this item should
   *                    be inserted
   */
  TaskListViewItem(QListViewItem * parent, FlightTask * task, QListViewItem* insertAfter=0);
  /**
   * Destructor
   */
  ~TaskListViewItem();
  /**
   * Contains reference to the @ref FlightTask this @ref QListViewItem is representing
   */
  FlightTask * task;
  /**
   * @returns an identifier with the value TASKLISTVIEWITEM_TYPEID for runtime typechecking
   */
  inline virtual int rtti() const{return TASKLISTVIEWITEM_TYPEID;};
  /**
   * Called to make the item update itself.
   */
  void update();

protected: // Protected methods
  /**
   * Creates the childnodes for this flightnode.
   */
  void createChildren();
};

#endif
