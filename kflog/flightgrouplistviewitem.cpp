/***********************************************************************
**
**   flightgrouplistviewitem.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2008 by Constantijn Neeteson
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include "flightlistviewitem.h"
#include "flightgrouplistviewitem.h"
#include "flight.h"
#include "flightgroup.h"

FlightGroupListViewItem::FlightGroupListViewItem( QTreeWidgetItem* parent,
                                                  FlightGroup* flightGroup ) :
  QTreeWidgetItem( parent, FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID ),
  flightGroup(flightGroup)
{
  createChildren();
}

FlightGroupListViewItem::~FlightGroupListViewItem()
{
}

/**
 * Called to make the item update itself, for example because the flight was
 * optimized.
 */
void FlightGroupListViewItem::update()
{
  /* This function updates the flight node after something has changed. It would be better
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

/** Creates the child nodes for this flight group node. */
void FlightGroupListViewItem::createChildren()
{
  QList<class Flight*> flights = flightGroup->getFlightList();

  setText( 0,flightGroup->getFileName() );
  setText( 1, QString::number(flights.count()) + " " + QObject::tr("Flights") );

  for( int i = 0; i < flights.count(); i++ )
    {
      QTreeWidgetItem* subItem = new FlightListViewItem( this, flights.at( i ) );
      subItem->setFlags( Qt::ItemIsEnabled );
    }
}
