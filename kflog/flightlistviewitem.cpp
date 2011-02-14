/***********************************************************************
**
**   flightlistviewitem.cpp
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

#include <QtGui>

#include "flightlistviewitem.h"
#include "tasklistviewitem.h"
#include "flight.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

FlightListViewItem::FlightListViewItem( QTreeWidgetItem* parent, Flight* flight) :
  QTreeWidgetItem(parent),
  flight(flight)
{
  createChildren();
}

FlightListViewItem::~FlightListViewItem()
{
}

/**
 * Called to make the item update itself, for example because the flight was optimized.
 */
void FlightListViewItem::update()
{
  /* This function updates the flight node after something has changed. It would be better
     to check what was changed, and react accordingly. This is pretty complex though, and
     even just resetting the text for each child node is more work than just deleting them
     and then re-creating them. f*/


  //first, delete all child nodes
  Q3ListViewItem * itm = firstChild();
  while (itm!=0) {
    delete itm;
    itm=firstChild();
  }

  //now, recreate them
  createChildren();

}

/** Creates the child nodes for this flight node. */
void FlightListViewItem::createChildren()
{
  setText( 0, QFileInfo(flight->getFileName()).fileName() );
  setText( 1, flight->getDate().toString(Qt::LocalDate) + ": " + flight->getPilot() );

  QTreeWidgetItem* subItem = new QTreeWidgetItem( this, QObject::tr("Pilot"),
                                                  flight->getPilot() );
  subItem->setFlags( Qt::ItemIsEnabled );

  subItem = new QTreeWidgetItem( this, QObject::tr("Date"),
                                 flight->getDate().toString(Qt::LocalDate) );
  subItem->setFlags( Qt::ItemIsEnabled );

  subItem = new QTreeWidgetItem( this, QObject::tr("Glider"),
                                 flight->getType() + ", " + flight->getID());
  subItem->setFlags( Qt::ItemIsEnabled );

  subItem =new QTreeWidgetItem( this, QObject::tr("Points"),
                                flight->getPoints(true));
  subItem->setFlags( Qt::ItemIsEnabled );

  if( flight->isOptimized() )
    {
      subItem = new QTreeWidgetItem( this, QObject::tr("Points (optimized)"),
                                     flight->getPoints(false) );
      subItem->setFlags( Qt::ItemIsEnabled );
    }

  subItem = new QTreeWidgetItem( this, QObject::tr("Total distance"),
                                 flight->getDistance(true) );
  subItem->setFlags( Qt::ItemIsEnabled );

  subItem = new TaskListViewItem( this, &flight->getTask(true) );
  subItem->setIcon(0, _mainWindow->getPixmap("task_16.png") );
  subItem->setFlags( Qt::ItemIsEnabled );

  if( flight->isOptimized() )
    {
      subItem = new TaskListViewItem( this, &flight->getTask(false) );
      subItem->setIcon(0, _mainWindow->getPixmap("task_16.png") );
      subItem->setFlags( Qt::ItemIsEnabled );
   }
}
