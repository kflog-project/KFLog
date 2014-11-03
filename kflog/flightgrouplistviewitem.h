/***********************************************************************
**
**   flightgrouplistviewitem.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2008 by Constantijn Neeteson
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * \class FlightGroupListViewItem
  *
  * \author Constantijn Neeteson, Axel Pauli
  *
  * \brief List view item that contains a flight group
  *
  * This class represents a flight group in the object tree. It manages it's own
  * children, all you need to do is invoke it with the
  * @ref FlightGroupListViewItem(QTreeWidgetItem* parent, Flight* flight) constructor.
  *
  * \date 2003-2011
  *
  * \version $Id$
  */

#ifndef FLIGHT_GROUP_LIST_VIEW_ITEM_H
#define FLIGHT_GROUP_LIST_VIEW_ITEM_H

#include <QTreeWidgetItem>

#define FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID 10002

class FlightGroup;
class Flight;

class FlightGroupListViewItem : public QTreeWidgetItem
{
 public:
  /**
   * Constructor.
   * @param parent Reference to parent @ref QTreeWidgetItem
   * @param flight Reference to @ref FlightGroup object to display
   */
  FlightGroupListViewItem(QTreeWidgetItem* parent, FlightGroup* flightGroup);
  /**
   * Destructor
   */
  virtual ~FlightGroupListViewItem();
  /**
   * Contains reference to the @ref FlightGroup this @ref QTreeWidgetItem
   * is representing.
   */
  FlightGroup* flightGroup;
  /**
   * Called to make the item update itself, for example because the flight
   * group was optimized.
   */
  void update();

protected:
  /**
   * Creates the child nodes for this flight group node.
   */
  void createChildren();
};

#endif
