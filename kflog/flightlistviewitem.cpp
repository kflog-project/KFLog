/***********************************************************************
**
**   flightlistviewitem.cpp
**
**   This file is part of KFLog4.
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
#include "evaluationframe.h"
#include "evaluationview.h"
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
  setText( 1, m_flight->getDate().toString(Qt::LocalDate) + ": " + m_flight->getPilot() );

  QStringList sl = (QStringList() << QObject::tr("Pilot") << m_flight->getPilot() );
  QTreeWidgetItem* subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  if( ! m_flight->getCopilotName().isEmpty() )
    {
      sl = (QStringList() << QObject::tr("Copilot")
                          << m_flight->getCopilotName() );
      subItem = new QTreeWidgetItem( sl );
      subItem->setFlags( Qt::ItemIsEnabled );
      addChild( subItem );
    }

  sl = (QStringList() << QObject::tr("Date")
                      << m_flight->getDate().toString(Qt::LocalDate) );
  subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

  sl = (QStringList() << QObject::tr("Glider")
                      << m_flight->getType() + ", " + m_flight->getID() );
  subItem = new QTreeWidgetItem( sl );
  subItem->setFlags( Qt::ItemIsEnabled );
  addChild( subItem );

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

  EvaluationFrame * EvalFrame = 0;
  EvaluationView * EvalView = 0;

  if( 0 != _mainWindow )
    {
      if( 0 != _mainWindow->getEvaluationWindow() )
        {
          EvalFrame = _mainWindow->getEvaluationWindow()->getEvalFrame();

          if( 0 != EvalFrame )
            {
              EvalView = _mainWindow->getEvaluationWindow()->getEvalFrame()->getEvalView();
            }
        }
    }

  // Reset cursors in evaluation dialog
  if( EvalFrame != 0 && EvalView != 0 )
    {
      EvalFrame->slotShowFlight( m_flight );
      EvalView->slotSetCursors( m_flight, cursor1, cursor2 );
    }

  // Reset flight flags at the map
  QPoint& p1 = route.first()->projP;
  QPoint& p2 = route.last()->projP;

  extern Map *_globalMap;
  _globalMap->slotDrawCursor( p1, p2 );
}
