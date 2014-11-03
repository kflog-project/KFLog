/***********************************************************************
**
**   flightlistviewitem.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "airspacelistviewitem.h"
#include "evaluationdialog.h"
#include "flightlistviewitem.h"
#include "tasklistviewitem.h"
#include "flight.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

FlightListViewItem::FlightListViewItem( QTreeWidgetItem* parent, Flight* flight) :
  QTreeWidgetItem( parent, FLIGHT_LIST_VIEW_ITEM_TYPEID ),
  m_flight(flight)
{
  createChildren();
}

FlightListViewItem::~FlightListViewItem()
{
}

/**
 * Called to make the item update itself, for example because the flight was
 * optimized.
 */
void FlightListViewItem::update()
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

/** Creates the child nodes for this flight node. */
void FlightListViewItem::createChildren()
{
  setText( 0, QFileInfo(m_flight->getFileName()).fileName() );
  setText( 1, m_flight->getDate() + ": " + m_flight->getPilot() );

  QStringList sl = (QStringList() << QObject::tr("Pilot") << m_flight->getPilot() );
  QTreeWidgetItem* subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  if( ! m_flight->getCopilot().isEmpty() )
    {
      sl = (QStringList() << QObject::tr("Copilot")
                          << m_flight->getCopilot() );
      subItem = new QTreeWidgetItem( sl );
      subItem->setFlags( Qt::ItemIsEnabled );
      addChild( subItem );
    }

  sl = (QStringList() << QObject::tr("Date")
                      << m_flight->getDate());
  subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("Glider")
                      << m_flight->getGliderType() + ", " + m_flight->getGliderRegistration() );
  subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  if( m_flight->getFlightStaticData().competitionId.isEmpty() == false )
    {
      sl = (QStringList() << QObject::tr("Competition Id")
                          << m_flight->getFlightStaticData().competitionId );
      subItem = new QTreeWidgetItem( sl );
      subItem->setFlags( Qt::ItemIsEnabled );
      addChild( subItem );
    }

  if( m_flight->getFlightStaticData().competitionClass.isEmpty() == false )
    {
      sl = (QStringList() << QObject::tr("Competition class")
                          << m_flight->getFlightStaticData().competitionClass );
      subItem = new QTreeWidgetItem( sl );
      subItem->setFlags( Qt::ItemIsEnabled );
      addChild( subItem );
    }

  sl = (QStringList() << QObject::tr("Points")
                      << m_flight->getPoints(true) );
  subItem =new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  if( m_flight->isOptimized() )
    {
      sl = (QStringList() << QObject::tr("Points (optimized)")
                          << m_flight->getPoints(false) );

      subItem = new QTreeWidgetItem( sl );
      subItem->setFlags( Qt::ItemIsEnabled );
      addChild( subItem );
    }

  sl = (QStringList() << QObject::tr("Total distance")
                      << m_flight->getDistance(true) );
  subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  // Flight recorder data
  subItem = new QTreeWidgetItem( sl );
  subItem->setText(0, QObject::tr("FR-Device") );
  subItem->setText(1, QObject::tr("Data") );
  subItem->setFlags( Qt::ItemIsEnabled );
  subItem->setIcon(0, _mainWindow->getPixmap("recorder_icon_32.png") );
  addChild( subItem );

  QTreeWidgetItem* parent = subItem;

  sl = (QStringList() << QObject::tr("Manufacturer")
                      << m_flight->getFlightStaticData().frManufacturer );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("FR-Type")
                      << m_flight->getFlightStaticData().frType );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("Firmware version")
                      << m_flight->getFlightStaticData().firmewareVersion );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("Hardware version")
                      << m_flight->getFlightStaticData().hardwareVersion );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("Pressure altitude sensor")
                      << m_flight->getFlightStaticData().altitudePressureSensor );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("GPS manufacturer")
                      << m_flight->getFlightStaticData().gpsManufacturer );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("GPS Datum")
                      << m_flight->getFlightStaticData().gpsDatum );

  subItem = new QTreeWidgetItem( parent, sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  subItem = new TaskListViewItem( this, m_flight->getTask(true) );
  subItem->setIcon(0, _mainWindow->getPixmap("task_16.png") );
  subItem->setFlags( Qt::ItemIsEnabled );

  subItem = new AirSpaceListViewItem( this, m_flight );
  subItem->setIcon(0, _mainWindow->getPixmap("kde_vectorgfx_16.png") );
  subItem->setFlags( Qt::ItemIsEnabled );

  if( m_flight->isOptimized() )
    {
      subItem = new TaskListViewItem( this, m_flight->getTask(false) );
      subItem->setIcon(0, _mainWindow->getPixmap("task_16.png") );
      subItem->setFlags( Qt::ItemIsEnabled );
   }
}

void FlightListViewItem::activate()
{
  if( m_flight == 0 )
    {
      return;
    }

  if( m_flight->getRoute().size() > 0 )
    {
      // Reset flight cursors at the map
      QPoint& p1 = m_flight->getRoute().first()->projP;
      QPoint& p2 = m_flight->getRoute().last()->projP;

      extern Map *_globalMap;
      _globalMap->slotDrawCursor( p1, p2 );
    }

  QList<FlightPoint*>& route = m_flight->getRoute();
  time_t cursor1 = route.first()->time;
  time_t cursor2 = route.last()->time;

  EvaluationDialog* evalDialog = 0;

  if( _mainWindow )
    {
      evalDialog = _mainWindow->getEvaluationWindow();

      if( evalDialog )
        {
          evalDialog->slotShowFlightData();
          evalDialog->slotSetCursors( cursor1, cursor2 );
        }
    }

  // Reset flight flags at the map
  QPoint& p1 = route.first()->projP;
  QPoint& p2 = route.last()->projP;

  extern Map *_globalMap;
  _globalMap->slotDrawCursor( p1, p2 );
}
