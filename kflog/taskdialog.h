/***********************************************************************
**
**   taskdialog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011 by Axel Pauli
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
 * \date 2002-2011
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

  void loadListWaypoints();

private slots:

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

  int getCurrentPosition();

  void setSelected( int position );

  void enableCommandButtons();

private:

  /** Waypoint list of task. */
  QList<Waypoint*> wpList;

  FlightTask *pTask;

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

/**
 * This template is used to store a pointer by using the class QVariant.
 *
 * I found it here:
 *
 * http://blog.bigpixel.ro/2010/04/23/storing-pointer-in-qvariant/
 *
 * So how do you use this? Assuming you have a class MyClass, and you want
 * to store a pointer to this class as a property of a QWidget, or any
 * QObject, or you want to convert it to QVariant, you can do the following:
 *
 * MyClass *p;
 * QVariant v = VPtr<MyClass>::asQVariant(p);
 *
 * MyClass *p1 = VPtr<MyClass>::asPtr(v);
 *
 */
template <class T> class VPtr
{
  public:

    static T* asPtr(QVariant v)
      {
        return  (T *) v.value<void *>();
      };

    static QVariant asQVariant(T* ptr)
      {
        return qVariantFromValue((void *) ptr);
      };
};

#endif
