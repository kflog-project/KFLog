/***********************************************************************
**
**   flightlistviewitem.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * \class FlightListViewItem
  *
  * \author André Somers, Axel Pauli
  *
  * \brief List view item that contains a flight
  *
  * This class represents a flight in the object tree. It manages it's own
  * children, all you need to do is invoke it with the
  * @ref FlightListViewItem(QTreeWidgetItem* parent, Flight* flight) constructor.
  *
  * \date 2003-2011
  *
  * \version $Id$
  */

#ifndef FLIGHT_LIST_VIEW_ITEM_H
#define FLIGHT_LIST_VIEW_ITEM_H

#include <QTreeWidgetItem>

#define FLIGHT_LIST_VIEW_ITEM_TYPEID 10001

class Flight;

class FlightListViewItem : public QTreeWidgetItem
{
 public:
  /**
   * Constructor.
   * @param parent Reference to parent @ref QTreeWidgetItem
   * @param flight Reference to @ref Flight object to display
   */
  FlightListViewItem( QTreeWidgetItem* parent, Flight* flight);
  /**
   * Destructor
   */
  virtual ~FlightListViewItem();
  /**
   * Contains reference to the @ref Flight this @ref QTreeWidgetItem
   * is representing
   */
  Flight* flight;
  /**
   * Called to make the item update itself, for example because the flight
   * was optimized.
   */
  void update();

 protected:
  /**
   * Creates the child nodes for this flight node.
   */
  void createChildren();
};

#endif
