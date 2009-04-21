/***********************************************************************
**
**   flightgrouplistviewitem.h
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

#ifndef FLIGHTGROUPLISTVIEWITEM_H
#define FLIGHTGROUPLISTVIEWITEM_H

#include <qlistview.h>
#define FLIGHTGROUPLISTVIEWITEM_TYPEID 10002
class FlightGroup;
class Flight;

/**
  * @short Listview item that contains a flightGroup
  * @author Constantijn Neeteson
  *
  * This class represents a flight in the objecttree. It manages it's own
  * childeren, all you need to do is invoke it with the
  * @ref FlightGroupListViewItem(QListViewItem * parent, FlightGroup * flightGroup) constructor.
  */

class FlightGroupListViewItem : public QListViewItem  {
public:
  /**
   * Constructor.
   * @param parent Reference to parent @ref QListViewItem
   * @param flight Reference to @ref FlightGroup object to display
   */
  FlightGroupListViewItem(QListViewItem * parent, FlightGroup * flightGroup);
  /**
   * Destructor
   */
  ~FlightGroupListViewItem();
  /**
   * Contains reference to the @ref FlightGroup this @ref QListViewItem is representing
   */
  FlightGroup * flightGroup;
  /**
   * Called to make the item update itself, for example because the flight was optimized.
   */
  void update();
  /**
   * @returns an identifier with the value FLIGHTGROUPLISTVIEWITEM_TYPEID for runtime typechecking
   */
  inline virtual int rtti() const{return FLIGHTGROUPLISTVIEWITEM_TYPEID;};

protected: // Protected methods
  /**
   * Creates the childnodes for this flightnode.
   */
  void createChildren();
};

#endif
