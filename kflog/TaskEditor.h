/***********************************************************************
**
**   TaskEditor.h
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

#ifndef TASK_EDITOR_H
#define TASK_EDITOR_H

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QVariant>

#include "flighttask.h"
#include "kflogtreewidget.h"
#include "waypoint.h"

/**
 * \class TaskEditor
 *
 * \brief Dialog widget to create or edit a flight task.
 *
 * Dialog widget to create or edit a flight task.
 *
 * \author Harald Maier, Axel Pauli
 *
 * \date 2002-2014
 *
 * \version 1.2
 */

class TaskEditor : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskEditor )

public:

  TaskEditor( QWidget *parent=0 );

  virtual ~TaskEditor();

  void setTask( FlightTask *orig );

  FlightTask *getTask() { return m_editedTask; }

protected:

 void showEvent( QShowEvent *event );

private:

  /** creates the widget elements used by the dialog. */
  void createDialog();

  /** Loads the route waypoints in the list view. */
  void loadRouteWaypoints();

public slots:

  /**
   * Called, if the right mouse button is pressed in the route tree widget.
   *
   * \param item Selected item in the route tree widget.
   *
   * \param position Position of selected item in the route tree widget.
   */
  void slotRightButtonPressed( QTreeWidgetItem *item, const QPoint &position );

private slots:

  /** Called to open the help window. */
  void slotHelp();

  /** Called, if the point source selection is changed. */
  void slotLoadSelectableWaypoints( int index );

  /** Called, if the input in m_pointSearchInput has to be cleared. */
  void slotClearSearchInput();

  /** Called, if the input in m_pointSearchInput has been changed. */
  void slotSearchInputEdited( const QString& text );

  /**
   * Called, if return key is pressed and m_pointSearchInput has the focus.
   * The currently selected waypoint item is taken over in the task list.
   */
  void slotTakeFoundItem();

  /** Called, if the search column selection is changed. */
  void slotSearchColumnIndexChanged( int index );

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
  void slotDuplicateWayoint();

  /** Called if the OK button is pressed. */
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
  QList<Waypoint*> m_taskWpList;

  /* Flight task to be edited. */
  FlightTask *m_editedTask;

  /* Fall back task, if no task is setup. */
  FlightTask m_task;

  /** Initial name of task. */
  QString    m_taskInitName;
  QLineEdit *m_taskNameEditor;
  QLabel    *m_taskType;

  QComboBox *m_planningTypes;
  QCheckBox *m_left;
  QCheckBox *m_right;

  /** overview with task points */
  KFLogTreeWidget *m_route;

  /** Column variables used in route tree display. */
  int colRouteType;
  int colRouteWaypoint;
  int colRouteDist;
  int colRouteCourse;
  int colRouteLeg;

  /** Waypoint list view. */
  KFLogTreeWidget *m_wpListView;

  /** Combo box for waypoint source selection. */
  QComboBox *m_pointSourceBox;

  /** Combo box for search column selection in m_wpListView. */
  QComboBox *m_pointColumnSelector;

  /** Line editor for text search input executed in m_wpListView. */
  QLineEdit *m_pointSearchInput;

  /** Clears the input of m_pointSearchInput. */
  QPushButton *m_clearPointSearchInput;

  /** Column variables used in waypoint tree display. */
  int colWpName;
  int colWpDescription;
  int colWpCountry;
  int colWpIcao;

  QPushButton *m_addCmd;
  QPushButton *m_removeCmd;
  QPushButton *m_upCmd;
  QPushButton *m_downCmd;
  QPushButton *m_invertCmd;

  /** Actions used in route tree mouse popup menu. */
  QAction *m_actionMoveUp;
  QAction *m_actionMoveDown;
  QAction *m_actionDuplicate;
  QAction *m_actionRemove;
  QAction *m_actionInvert;
};

#endif
