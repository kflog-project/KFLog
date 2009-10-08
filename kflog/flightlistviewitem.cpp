/***********************************************************************
**
**   flightlistviewitem.cpp
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

#include "flightlistviewitem.h"
#include "tasklistviewitem.h"
#include "flight.h"
#include <qurl.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>

FlightListViewItem::FlightListViewItem(QListViewItem * parent, Flight * flight):QListViewItem(parent){
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
  QListViewItem * itm = firstChild();
  while (itm!=0) {
    delete itm;
    itm=firstChild();
  }

  //now, recreate them
  createChildren();

}

/** Creates the childnodes for this flightnode. */
void FlightListViewItem::createChildren(){
  QUrl url(flight->getFileName());
  KLocale loc("");

  setText(0,url.fileName());
  setText(1,loc.formatDate(flight->getDate(),true) + ": " + flight->getPilot());
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  QListViewItem * subItem=new QListViewItem((QListViewItem*)this,QObject::tr("Pilot"),flight->getPilot());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,QObject::tr("Date"),loc.formatDate(flight->getDate(),false));
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,QObject::tr("Glider"),flight->getType() + ", " + flight->getID());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,QObject::tr("Points"),flight->getPoints(true));
  subItem->setSelectable(false);
  if (flight->isOptimized()){
    subItem=new QListViewItem((QListViewItem*)this,subItem,QObject::tr("Points (optimized)"),flight->getPoints(false));
    subItem->setSelectable(false);
  }
  subItem=new QListViewItem((QListViewItem*)this,subItem,QObject::tr("Total distance"),flight->getDistance(true));
  subItem->setSelectable(false);
  subItem=new TaskListViewItem((QListViewItem*)this, &flight->getTask(true), subItem);
  subItem->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("task", KIcon::NoGroup, KIcon::SizeSmall));
  subItem->setSelectable(false);
  if (flight->isOptimized()){
    subItem=new TaskListViewItem((QListViewItem*)this, &flight->getTask(false), subItem);
    subItem->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("task", KIcon::NoGroup, KIcon::SizeSmall));
    subItem->setSelectable(false);
  }
}
