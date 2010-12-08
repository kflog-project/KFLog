/***********************************************************************
**
**   flightgrouplistviewitem.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2008 by Constantijn Neeteson
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "flightlistviewitem.h"
#include "flightgrouplistviewitem.h"
#include "flight.h"
#include "flightgroup.h"

FlightGroupListViewItem::FlightGroupListViewItem(Q3ListViewItem * parent, FlightGroup * flightGroup):Q3ListViewItem(parent){
  this->flightGroup=flightGroup;

  createChildren();

}

FlightGroupListViewItem::~FlightGroupListViewItem(){
}

/**
 * Called to make the item update itself, for example because the flight was optimized.
 */
void FlightGroupListViewItem::update(){
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
void FlightGroupListViewItem::createChildren(){
  QString tmp;

  QList<Flight::Flight*> flights = flightGroup->getFlightList();

  setText(0,flightGroup->getFileName());
  setText(1,tmp.sprintf("%d ", flights.count())+QObject::tr("flights"));
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  Q3ListViewItem * subItem;
  for(int i=0;i<flights.count();i++)
  {
    subItem = new FlightListViewItem((Q3ListViewItem*)this,flights.at(i));
    subItem->setSelectable(false);
  }
}
