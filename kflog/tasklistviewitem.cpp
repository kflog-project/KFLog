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
#include <qurl.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

TaskListViewItem::TaskListViewItem(QListViewItem * parent,
                                   FlightTask * task,
                                   QListViewItem* insertAfter):QListViewItem(parent, insertAfter){
  this->task=task;
  QUrl url(task->getFileName());
  KLocale loc("");

  setText(0,url.fileName());
  setText(1,task->getPlanningTypeString());
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  QListViewItem * subItem=new QListViewItem((QListViewItem*)this,i18n("Type"),task->getPlanningTypeString());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Distance"),task->getTaskDistanceString());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Points"),task->getPointsString());
  subItem->setSelectable(false);
 
}

TaskListViewItem::~TaskListViewItem(){
}

