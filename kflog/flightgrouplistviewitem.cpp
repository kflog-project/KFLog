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
**   Licence. See the file COPYING for more information.
**
***********************************************************************/

#include "flightlistviewitem.h"
#include "flightgrouplistviewitem.h"
#include "flight.h"
#include "flightgroup.h"

FlightGroupListViewItem::FlightGroupListViewItem(QListViewItem * parent, FlightGroup * flightGroup):QListViewItem(parent){
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
  QListViewItem * itm = firstChild();
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

  QPtrList<Flight::Flight> flights = flightGroup->getFlightList();

  setText(0,flightGroup->getFileName());
  setText(1,tmp.sprintf("%d ", flights.count())+QObject::tr("flights"));
  //setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));

  QListViewItem * subItem;
  for(unsigned int i=0;i<flights.count();i++)
  {
    subItem = new FlightListViewItem((QListViewItem*)this,flights.at(i));
    subItem->setSelectable(false);
  }
}
