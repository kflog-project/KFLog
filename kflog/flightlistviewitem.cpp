/***********************************************************************
**
**   flightlistviewitem.cpp
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

#include "flightlistviewitem.h"
#include "tasklistviewitem.h"
#include "flight.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

FlightListViewItem::FlightListViewItem(Q3ListViewItem * parent, Flight * flight):Q3ListViewItem(parent){
  this->flight=flight;

  createChildren();
  
}

FlightListViewItem::~FlightListViewItem(){
}

/**
 * Called to make the item update itself, for example because the flight was optimized.
 */
void FlightListViewItem::update(){
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

/** Creates the childnodes for this flightnode. */
void FlightListViewItem::createChildren(){
  Q3Url url(flight->getFileName());

  setText(0, url.fileName());
  setText(1, flight->getDate().toString(Qt::LocalDate) + ": " + flight->getPilot());
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  Q3ListViewItem * subItem=new Q3ListViewItem((Q3ListViewItem*)this,QObject::tr("Pilot"),flight->getPilot());
  subItem->setSelectable(false);
  subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Date"), flight->getDate().toString(Qt::LocalDate));
  subItem->setSelectable(false);
  subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Glider"),flight->getType() + ", " + flight->getID());
  subItem->setSelectable(false);
  subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Points"),flight->getPoints(true));
  subItem->setSelectable(false);
  if (flight->isOptimized()){
    subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Points (optimized)"),flight->getPoints(false));
    subItem->setSelectable(false);
  }
  subItem=new Q3ListViewItem((Q3ListViewItem*)this,subItem,QObject::tr("Total distance"),flight->getDistance(true));
  subItem->setSelectable(false);
  subItem=new TaskListViewItem((Q3ListViewItem*)this, &flight->getTask(true), subItem);
  QPixmap taskPic = _mainWindow->getPixmap("task_16.png");
  subItem->setPixmap(0, taskPic);
  subItem->setSelectable(false);
  if (flight->isOptimized()){
    subItem=new TaskListViewItem((Q3ListViewItem*)this, &flight->getTask(false), subItem);
    subItem->setPixmap(0, taskPic);
    subItem->setSelectable(false);
  }
}
