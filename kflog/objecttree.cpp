/***********************************************************************
**
**   objecttree.cpp
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

#include "objecttree.h"
#include "flightlistviewitem.h"
#include "tasklistviewitem.h"

#include "flight.h"
#include "flighttask.h"
#include "mapcontents.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>



ObjectTree::ObjectTree(QWidget *parent, const char *name ) : QListView(parent,name) {
  setShowSortIndicator(false);
  setAllColumnsShowFocus(true);
  setRootIsDecorated(true);
  setSorting(-1);
  //setSelectionMode(Extended);
  
  colName = addColumn(i18n("Name"));
  colDesc = addColumn(i18n("Description"));
  
  FlightRoot = new QListViewItem(this,i18n("Flights"));
  FlightRoot->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));
  FlightRoot->setSelectable(false);

  /* removed for now, as the concept is likely to disapear
  FlightGroupRoot = new QListViewItem(this,FlightRoot,i18n("Flight groups"));
  FlightGroupRoot->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("flightgroup", KIcon::NoGroup, KIcon::SizeSmall));
  FlightGroupRoot->setSelectable(false);
  */
  TaskRoot = new QListViewItem(this,FlightRoot,i18n("Tasks"));
  TaskRoot->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("task", KIcon::NoGroup, KIcon::SizeSmall));
  TaskRoot->setSelectable(false);

  connect(this, SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotSelected(QListViewItem*)));
}

ObjectTree::~ObjectTree(){
}

/**
 * Called if a new flight has been added.
 */
void ObjectTree::slotNewFlightAdded(Flight * flight){
  QListViewItem * node=new FlightListViewItem(FlightRoot,flight);
}

/**
 * Called if a new task has been created or loaded.
 */
void ObjectTree::slotNewTaskAdded(FlightTask * task){
  QListViewItem * node=new TaskListViewItem(TaskRoot,task);
}

/** Called if the selection has changed. */
void ObjectTree::slotSelected(QListViewItem * itm){
  extern MapContents _globalMapContents;
  
  if (!itm) return;
  BaseFlightElement * flt=0;

  switch (itm->rtti()) { //the rtti (Run Time Type Identification is used to see what kind of listview item we are dealing with
    case FLIGHTLISTVIEWITEM_TYPEID:
      flt=((FlightListViewItem*)itm)->flight;
      break;
    case TASKLISTVIEWITEM_TYPEID:
      flt=((TaskListViewItem*)itm)->task;
      break;
    default:
      flt=0;
  }
  
  if (flt && flt!=_globalMapContents.getFlight())
    emit selectedFlight(flt);
  
}

/** This slot is called if the currently selected flight has changed. */
void ObjectTree::slotSelectedFlightChanged(BaseFlightElement * bfe){
  QListViewItem * itm=0;
  
  if (FlightRoot->childCount()!=0) {
    itm = FlightRoot->firstChild();
    while (itm!=0) {
      if (((FlightListViewItem*)itm)->flight==bfe) {
        if (!itm->isSelected()) setSelected(itm,true);
        ensureItemVisible(itm);
        return;
      }
      itm=itm->nextSibling();
    }
  }

  if (TaskRoot->childCount()!=0) {
    itm = TaskRoot->firstChild();
    while (itm!=0) {
      if (((TaskListViewItem*)itm)->task==bfe) {
        if (!itm->isSelected()) setSelected(itm,true);
        ensureItemVisible(itm);
        return;
      }
      itm=itm->nextSibling();
    }
  }

  
           

}
