/***********************************************************************
**
**   flightlistviewitem.h
**
**   This file is part of KFLog4.
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

#ifndef FLIGHTLISTVIEWITEM_H
#define FLIGHTLISTVIEWITEM_H

#include <q3listview.h>
#define FLIGHTLISTVIEWITEM_TYPEID 10001
class Flight;

/**
  * @short Listview item that contains a flight
  * @author André Somers
  *
  * This class represents a flight in the objecttree. It manages it's own
  * childeren, all you need to do is invoke it with the
  * @ref QFlightListViewItem(QListViewItem * parent, Flight * flight) constructor.
  */

class FlightListViewItem : public Q3ListViewItem  {
public:
  /**
   * Constructor.
   * @param parent Reference to parent @ref QListViewItem
   * @param flight Reference to @ref Flight object to display
   */
  FlightListViewItem(Q3ListViewItem * parent, Flight * flight);
  /**
   * Destructor
   */
  ~FlightListViewItem();
  /**
   * Contains reference to the @ref Flight this @ref QListViewItem is representing
   */
  Flight * flight;
  /**
   * Called to make the item update itself, for example because the flight was optimized.
   */
  void update();
  /**
   * @returns an identifier with the value FLIGHTLISTVIEWITEM_TYPEID for runtime typechecking
   */
  inline virtual int rtti() const{return FLIGHTLISTVIEWITEM_TYPEID;};

protected: // Protected methods
  /**
   * Creates the childnodes for this flightnode.
   */
  void createChildren();
};

#endif
