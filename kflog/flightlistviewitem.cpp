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
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

FlightListViewItem::FlightListViewItem(QListViewItem * parent, Flight * flight):QListViewItem(parent){
  this->flight=flight;
  QUrl url(flight->getFileName());
  KLocale loc("");
  
  setText(0,url.fileName());
  setText(1,loc.formatDate(flight->getDate(),true) + ": " + flight->getPilot());
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  QListViewItem * subItem=new QListViewItem((QListViewItem*)this,i18n("Pilot"),flight->getPilot());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Date"),loc.formatDate(flight->getDate(),false));
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Glider"),flight->getType() + ", " + flight->getID());
  subItem->setSelectable(false);
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Points"),flight->getPoints(true));
  subItem->setSelectable(false);
  if (flight->isOptimized()){
    subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Points (optimized)"),flight->getPoints(false));
    subItem->setSelectable(false);
  }
  subItem=new QListViewItem((QListViewItem*)this,subItem,i18n("Total distance"),flight->getDistance(true));
  subItem->setSelectable(false);
  subItem=new TaskListViewItem((QListViewItem*)this, &flight->getTask(true), subItem);
  subItem->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("task", KIcon::NoGroup, KIcon::SizeSmall));
  subItem->setSelectable(false);
  if (flight->isOptimized()){
    subItem=new TaskListViewItem((QListViewItem*)this, &flight->getTask(false), subItem);
    subItem->setSelectable(false);
  }
  
  
}

FlightListViewItem::~FlightListViewItem(){
}
