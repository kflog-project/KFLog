/***********************************************************************
**
**   taskdialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef TASK_DIALOG_H
#define TASK_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QErrorMessage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QVariant>

#include "flighttask.h"
#include "kflogtreewidget.h"
#include "waypoint.h"

/**
 * \class TaskDialog
 *
 * \brief Dialog widget to create or edit a flight task.
 *
 * Dialog widget to create or edit a flight task.
 *
 * \author Harald Maier, Axel Pauli
 *
 * \date 2002-2014
 *
 * \version $Id$
 */

class TaskDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskDialog )

public:

   TaskDialog( QWidget *parent=0 );

   virtual ~TaskDialog();

   void setTask( FlightTask *orig );

   FlightTask *getTask() { return pTask; }

private:

  /** creates the widget elements used by the dialog. */
  void createDialog();

  void loadRouteWaypoints();

private slots:

  /** Called, if the point source selection is changed. */
  void slotLoadSelectableWaypoints( int index );

  /** Called if an item is clicked in the route tree view. */
  void slotItemClicked( QTreeWidgetItem * item, int column );

  /** No descriptions */
  void slotSetPlanningType( const QString & text );

  /** No descriptions */
  void slotSetPlanningDirection(int);

  void slotMoveUp();
  void slotMoveDown();
  void slotInvertWaypoints();
  void slotAddWaypoint();
  void slotRemoveWaypoint();

  /** Called if the ok button is pressed. */
  void slotAccept();

private:

  /**
   * Sets the selection entries in m_pointSourceBox according to the
   * available data.
   */
  void setEntriesInPointSourceBox();

  int getCurrentPosition();

  void setSelected( int position );

  void enableCommandButtons();

private:

  /** Enumeration types for point source selection. */
  enum PointSource
    {
      None,
      Waypoints,
      Airfields,
      Outlandings,
      Navaids,
      Hotspots
    };

  /** Waypoint list of task. */
  QList<Waypoint*> wpList;

  /* Flight task to be modified. */
  FlightTask *pTask;

  /* Fall back task, if no task is setup. */
  FlightTask _task;

  QString    startName;
  QLineEdit *name;
  QLabel    *taskType;

  QComboBox *planningTypes;
  QCheckBox *left;
  QCheckBox *right;

  /** overview with task points */
  KFLogTreeWidget *route;

  /** Columns used by route display. */
  int colRouteType;
  int colRouteWaypoint;
  int colRouteDist;
  int colRouteCourse;
  int colRouteDummy;

  /** Waypoint list view. */
  KFLogTreeWidget *waypoints;

  /** Combo box for point source selection. */
  QComboBox *m_pointSourceBox;

  int colWpName;
  int colWpDescription;
  int colWpCountry;
  int colWpIcao;
  int colWpDummy;

  QPushButton *addCmd;
  QPushButton *removeCmd;
  QPushButton *upCmd;
  QPushButton *downCmd;
  QPushButton *invertCmd;

  QErrorMessage* errorFai;
  QErrorMessage* errorRoute;
};

#endif
