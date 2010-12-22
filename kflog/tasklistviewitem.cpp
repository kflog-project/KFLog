/***********************************************************************
**
**   tasklistviewitem.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Andr� Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>
#include <Qt3Support>

#include "tasklistviewitem.h"
#include "flighttask.h"
#include "waypoint.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

TaskListViewItem::TaskListViewItem(Q3ListViewItem * parent,
                                   FlightTask * task,
                                   Q3ListViewItem* insertAfter):Q3ListViewItem(parent, insertAfter){
  this->task=task;
  createChildren();
}

TaskListViewItem::~TaskListViewItem(){
}

/**
 * Called to make the item update itself, for example because the flight was optimized.
 */
void TaskListViewItem::update(){
  /* This funtion updates the flightnode after something has changed. It would be better
     to check what was changed, and react accordingly. This is pretty complex though, and
     even just resetting the text for each childnode is more work than just deleting them
     and then re-creating them. f*/


  //first, delete all childnodes
  Q3ListViewItem * itm = firstChild();
  while (itm!=0) {
    delete itm;
    itm=firstChild();
  }

  //now, recreate them
  createChildren();

}

/** This function populates the node with data from the task */
void TaskListViewItem::createChildren(){
  Q3Url url(task->getFileName());
  QString wpName;
  
  setText(0,url.fileName());
  setText(1,task->getPlanningTypeString());
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  Q3ListViewItem * subItem=new Q3ListViewItem((Q3ListViewItem*)this,QObject::tr("Type"),task->getPlanningTypeString());
  subItem->setSelectable(false);
  subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Distance"),task->getTaskDistanceString());
  subItem->setSelectable(false);
  subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Points"),task->getPointsString());
  subItem->setSelectable(false);

  if (!task->getWPList().isEmpty()) {
    int wpCount=task->getWPList().count();
    Q3ListViewItem * wpSubItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Waypoints"),QObject::tr("%1 waypoints in task").arg(wpCount));
    wpSubItem->setSelectable(false);
    wpSubItem->setPixmap(0, _mainWindow->getPixmap("waypoint_16.png"));

    for (int i=0;i<wpCount;i++) {
      wpName=QObject::tr("Turnpoint");
      if (i==0) wpName=QObject::tr("Takeoff");
      if (i==1) wpName=QObject::tr("Start");
      if (i==wpCount-2) wpName=QObject::tr("Finish");
      if (i==wpCount-1) wpName=QObject::tr("Landing");

      subItem=new Q3ListViewItem(wpSubItem,subItem,wpName,task->getWPList().at(i)->name);
      subItem->setSelectable(false);
      subItem->setPixmap(0, _mainWindow->getPixmap("centerwaypoint_16.png"));
    }
  }
}
