/***********************************************************************
**
**   tasklistviewitem.cpp
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

#include <QtGui>

#include "tasklistviewitem.h"
#include "flighttask.h"
#include "waypoint.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

TaskListViewItem::TaskListViewItem( QTreeWidgetItem* parent,
                                    FlightTask* task,
                                    QTreeWidgetItem* insertAfter ) :
  QTreeWidgetItem( parent, insertAfter, TASK_LIST_VIEW_ITEM_TYPEID ),
  task(task)
{
  createChildren();
}

TaskListViewItem::~TaskListViewItem(){
}

/**
 * Called to make the item update itself, for example because the flight was
 * optimized.
 */
void TaskListViewItem::update()
{
  /* This function updates the task node after something has changed. It would be better
     to check what was changed, and react accordingly. This is pretty complex though, and
     even just resetting the text for each child node is more work than just deleting them
     and then re-creating them. f*/

  // first, delete all child nodes
  if( childCount() )
    {
      QList<QTreeWidgetItem *> children = takeChildren();
      qDeleteAll(children);
    }

  // now, recreate them
  createChildren();
}

/** This function populates the node with data from the task */
void TaskListViewItem::createChildren()
{
  QString wpName;
  
  setText( 0, QFileInfo(task->getFileName()).fileName() );
  setText( 1, task->getPlanningTypeString());

  QStringList sl = (QStringList() << QObject::tr("Type")
                                  << task->getPlanningTypeString() );

  QTreeWidgetItem* subItem = new QTreeWidgetItem( this, sl );
  subItem->setFlags( Qt::ItemIsEnabled );

  sl = (QStringList() << QObject::tr("Distance") << task->getTaskDistanceString() );
  subItem = new QTreeWidgetItem( this, sl );
  subItem->setFlags( Qt::ItemIsEnabled );

  sl = (QStringList() << QObject::tr("Points") << task->getPointsString() );
  subItem = new QTreeWidgetItem( this, sl );
  subItem->setFlags( Qt::ItemIsEnabled );

  if( task->getWPList().isEmpty() )
    {
      return;
    }

  int wpCount = task->getWPList().count();

  sl = (QStringList() << QObject::tr("Waypoints")
                      << QObject::tr("%1 waypoints in task").arg(wpCount) );
  QTreeWidgetItem* wpSubItem = new QTreeWidgetItem( this, sl );
  wpSubItem->setFlags( Qt::ItemIsEnabled );
  wpSubItem->setIcon( 0, _mainWindow->getPixmap("waypoint_16.png") );

  for( int i = 0; i < wpCount; i++ )
    {
      wpName = QObject::tr( "Turnpoint" );

      if( i == 0 )
        {
          wpName = QObject::tr( "Takeoff" );
        }

      if( i == 1 )
        {
          wpName = QObject::tr( "Start" );
        }

      if( i == wpCount - 2 )
        {
          wpName = QObject::tr( "Finish" );
        }

      if( i == wpCount - 1 )
        {
          wpName = QObject::tr( "Landing" );
        }

      sl = (QStringList() << wpName << task->getWPList().at( i )->name );
      subItem = new QTreeWidgetItem( wpSubItem, sl );
      subItem->setFlags( Qt::ItemIsEnabled );
      subItem->setIcon( 0, _mainWindow->getPixmap( "centerwaypoint_16.png" ) );
    }
}
