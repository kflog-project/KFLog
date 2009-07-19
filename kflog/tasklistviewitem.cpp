/***********************************************************************
**
**   tasklistviewitem.cpp
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

#include "tasklistviewitem.h"
#include "flighttask.h"
#include "waypoint.h"

#include <qurl.h>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

TaskListViewItem::TaskListViewItem(QListViewItem * parent,
                                   FlightTask * task,
                                   QListViewItem* insertAfter):QListViewItem(parent, insertAfter){
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
  QListViewItem * itm = firstChild();
  while (itm!=0) {
    delete itm;
    itm=firstChild();
  }

  //now, recreate them
  createChildren();

}

/** This function populates the node with data from the task */
void TaskListViewItem::createChildren(){
  QUrl url(task->getFileName());
  KLocale loc("");
  QString wpName;
  
  setText(0,url.fileName());
  setText(1,task->getPlanningTypeString());
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  QListViewItem * subItem=new QListViewItem((QListViewItem*)this,i18n("Type"),task->getPlanningTypeString());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Distance"),task->getTaskDistanceString());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Points"),task->getPointsString());
  subItem->setSelectable(false);

  if (!task->getWPList().isEmpty()) {
    int wpCount=task->getWPList().count();
    QListViewItem * wpSubItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Waypoints"),i18n("%1 waypoints in task").arg(wpCount));
    wpSubItem->setSelectable(false);
    wpSubItem->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("waypoint", KIcon::NoGroup, KIcon::SizeSmall));

    for (int i=0;i<wpCount;i++) {
      wpName=i18n("Turnpoint");
      if (i==0) wpName=i18n("Takeoff");
      if (i==1) wpName=i18n("Start");
      if (i==wpCount-2) wpName=i18n("Finish");
      if (i==wpCount-1) wpName=i18n("Landing");

      subItem=new QListViewItem(wpSubItem,subItem,wpName,task->getWPList().at(i)->name);
      subItem->setSelectable(false);
      subItem->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("centerwaypoint", KIcon::NoGroup, KIcon::SizeSmall));
    }
  }
}