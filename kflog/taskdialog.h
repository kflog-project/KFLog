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
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASK_DIALOG_H
#define TASK_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QErrorMessage>
#include <q3dict.h>
#include <QLabel>
#include <QLineEdit>
#include <q3listbox.h>
#include <QRadioButton>

#include "guicontrols/kfloglistview.h"
#include "flighttask.h"
#include "waypoint.h"

/**
  *@author Harald Maier
  */

class TaskDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskDialog )

public:

   TaskDialog( QWidget *parent=0 );

   virtual ~TaskDialog();

   void setTask(FlightTask *orig);

   FlightTask *getTask() { return pTask; }

private:

  /** No descriptions */
  void __initDialog();
  void fillWaypoints();

private slots:

  /** No descriptions */
  void enableWaypointButtons();
  /** No descriptions */
  void slotSetPlanningType( const QString & text );
  /** No descriptions */
  void slotSetPlanningDirection(int);
  void polish();
  void slotMoveUp();
  void slotMoveDown();
  void slotReplaceWaypoint();
  void slotAddWaypoint();
  void slotRemoveWaypoint();

private:
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

  QErrorMessage* errorFai;
  QErrorMessage* errorRoute;

  int colType;
  int colWaypoint;
  int colDist;
  int colCourse;

  unsigned int getCurrentPosition();
  void setSelected(unsigned int position);
};

#endif
