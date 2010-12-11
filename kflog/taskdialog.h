/***********************************************************************
**
**   taskdialog.h
**
**   This file is part of KFLog4.
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

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <q3dict.h>
#include <QLabel>
#include <QLineEdit>
#include <q3listbox.h>
#include <QRadioButton>
#include <QWidget>

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
  Q3ListBox *waypoints;
  Q3Dict<Waypoint> waypointDict;
  QList<Waypoint*> wpList;
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

  unsigned int getCurrentPosition();
  void setSelected(unsigned int position);
};

#endif
