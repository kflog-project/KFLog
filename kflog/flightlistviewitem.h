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

#ifndef FLIGHT_LIST_VIEW_ITEM_H
#define FLIGHT_LIST_VIEW_ITEM_H

#include <QTreeWidgetItem>

#define FLIGHT_LIST_VIEW_ITEM_TYPEID 10001

class Flight;

/**
  * @short Listview item that contains a flight
  * @author André Somers
  *
  * This class represents a flight in the object tree. It manages it's own
  * children, all you need to do is invoke it with the
  * @ref QFlightListViewItem(QListViewItem * parent, Flight * flight) constructor.
  */

class FlightListViewItem : public QTreeWidgetItem
{
 public:
  /**
   * Constructor.
   * @param parent Reference to parent @ref QListViewItem
   * @param flight Reference to @ref Flight object to display
   */
  FlightListViewItem( QTreeWidgetItem* parent, Flight* flight);
  /**
   * Destructor
   */
  ~FlightListViewItem();
  /**
   * Contains reference to the @ref Flight this @ref QListViewItem
   * is representing
   */
  Flight* flight;
  /**
   * Called to make the item update itself, for example because the flight
   * was optimized.
   */
  void update();
  /**
   * @returns an identifier with the value FLIGHT_LIST_VIEW_ITEM_TYPEID for
   * runtime type checking
   */
  inline virtual int rtti() const
    {
      return FLIGHT_LIST_VIEW_ITEM_TYPEID;
    };

 protected: // Protected methods
  /**
   * Creates the child nodes for this flight node.
   */
  void createChildren();
};

#endif
