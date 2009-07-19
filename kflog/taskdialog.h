/***********************************************************************
**
**   taskdialog.h
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

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qdict.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qwidget.h>

#include "guicontrols/kfloglistview.h"
#include "flighttask.h"
#include "waypoint.h"

/**
  *@author Harald Maier
  */

class TaskDialog : public QDialog  {
   Q_OBJECT
public: 
   TaskDialog(QWidget *parent=0, const char *name=0);
   ~TaskDialog();
   void setTask(FlightTask *orig);
   FlightTask *getTask() { return pTask; }
private: // Private methods
  /** No descriptions */
  void __initDialog();
  void fillWaypoints();
public: // Public attributes
private slots: // Private slots
  /** No descriptions */
  void enableWaypointButtons();
  /** No descriptions */
  void slotSetPlanningType(int);
  /** No descriptions */
  void slotSetPlanningDirection(int);
  void polish();
  void slotMoveUp();
  void slotMoveDown();
  void slotReplaceWaypoint();
  void slotAddWaypoint();
  void slotRemoveWaypoint();
private: // Private attributes
  /**  */
  QListBox *waypoints;
  QDict<Waypoint> waypointDict;
  QPtrList<Waypoint> wpList;
  FlightTask *pTask;
  /**  */
  QLineEdit *name;
  QLabel *taskType;

  QComboBox *planningTypes;
  QCheckBox *left;
  QCheckBox *right;

  KFLogListView *route;
  QPushButton *back;
  QPushButton *forward;

  int colType;
  int colWaypoint;
  int colDist;
  int colCourse;
};

#endif
