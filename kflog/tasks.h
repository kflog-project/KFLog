/***********************************************************************
**
**   tasks.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASKS_H
#define TASKS_H

#include <qwidget.h>
#include <qframe.h>
#include <qdict.h>

#include <kpopupmenu.h>

#include "guicontrols/kfloglistview.h"
#include "flighttask.h"

/**
  *@author Harald Maier
  */

class Tasks : public QFrame  {
   Q_OBJECT
public: 
	Tasks(QWidget *parent=0, const char *name=0);
	~Tasks();
private: // Private attributes
  /**  */
  KFLogListView *tasks;
  /** popup menu for waypoint's */
  KPopupMenu *taskPopup;

  int colName;
  int colDesc;
  int colTaskDist;
  int colTotalDist;

  int idTaskEdit;
  int idTaskDelete;
  int idTaskSave;
  int idTaskSaveAll;

  QDict <FlightTask> taskList;

  QString path;
  QListViewItem *oldItem;
private: // Private methods
  /** No descriptions */
  void addTaskWindow(QWidget *parent);
  void addPopupMenu();
private slots: // Private slots
  /** No descriptions */
  void slotNotHandledItem();
  void slotNewTask();
  void slotOpenTask();
  void slotEditTask();
  void slotDeleteTask();
  void slotSaveTask();
  void slotSaveAllTask();
  void slotSelectTask(QListViewItem *item);
  void showTaskPopup(QListViewItem *it, const QPoint &, int);
public slots: // Public slots
  /** No descriptions */
  void slotAppendTask(FlightTask *f);
  /** No descriptions */
  void setCurrentTask();
  /** No descriptions */
  void slotUpdateTask();
signals: // Signals
  /** indicate that a new task should be created */
  void newTask();
  /** No descriptions */
  void openTask();
  void flightSelected(BaseFlightElement *);
  /** No descriptions */
  void closeTask();
};

#endif
