/***********************************************************************
**
**   AirSpaceListViewItem.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c): 2013 by Matthias Degenkolb
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "airspacelistviewitem.h"
#include "flight.h"
#include "waypoint.h"
#include "mainwindow.h"
#include "evaluationframe.h"
#include "evaluationdialog.h"
#include "evaluationview.h"

extern MainWindow *_mainWindow;

AirSpaceListViewItem::AirSpaceListViewItem( QTreeWidgetItem* parent,
                                            Flight* theFlight ) :
  QTreeWidgetItem( parent, AIRSPACE_LIST_VIEW_ITEM_TYPEID ),
  m_Flight(theFlight)
{
  createChildren();
  setExpanded( true );
}

AirSpaceListViewItem::~AirSpaceListViewItem()
{
}

/**
 * Called to make the item update itself, for example because the flight was
 * optimized.
 */
void AirSpaceListViewItem::update()
{
  /* This function updates the task node after something has changed. It would be better
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

/** This function populates the node with data from the task */
void AirSpaceListViewItem::createChildren()
{
//#define AIRSPACELISTVIEWITEM_EXTENDED_TRACE
  if( m_Flight != 0 )
    {
      setText( 0, QString( "AirSpace" ) );
      setText( 1, QString( "violations / intersections" ) );

      // Goal of awd: in vertical view glider is either
      // - below
      // - inside
      // - above
      // (bogus of 40000m should be sufficient)
      AirspaceWarningDistance awd;
      awd.horClose.setMeters( 0 );
      awd.horVeryClose.setMeters( 0 );
      awd.verAboveClose.setMeters( 40000 );
      awd.verAboveVeryClose.setMeters( 0 );
      awd.verBelowClose.setMeters( 40000 );
      awd.verBelowVeryClose.setMeters( 0 );

      QList<Flight::AirSpaceIntersection> Violations =
          m_Flight->getFlightAirSpaceIntersections( 2038, 0, &awd );
      if( Violations.count() > 0 )
        {
          for( int i = 0; i < Violations.count(); i++ )
            {
              QString AirSpaceName;
              AirSpaceName += Airspace::getTypeName( Violations[i].AirSpace().getObjectType() );
              AirSpaceName += " ";
              AirSpaceName += Violations[i].AirSpace().getName();

              QStringList sl = (QStringList() << QString( "Name" ) << AirSpaceName);
              QTreeWidgetItem* subItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(this, sl, Violations[i], m_Flight );
              subItem->setFlags( Qt::ItemIsEnabled );
              QBrush col1 = subItem->foreground( 0 );
              QBrush col2 = subItem->foreground( 1 );

              QString ViolationType;

              switch( Violations[i].Type() )
                {
                case Airspace::Inside:
                  ViolationType = "INSIDE";
                  col1 = QBrush( Qt::red );
                  col2 = QBrush( Qt::red );
                  break;
                case Airspace::NearBelow:
                case Airspace::VeryNearBelow:
                  ViolationType = "Below";
                  break;
                case Airspace::NearAbove:
                case Airspace::VeryNearAbove:
                  ViolationType = "Above";
                  break;
                case Airspace::None:
                default:
                  ViolationType = "None";
                  break;
                }
              subItem->setForeground( 0, col1 );
              subItem->setForeground( 1, col2 );

              QString InfoString;
              InfoString = Violations[i].AirSpace().getInfoString( false );
              sl = (QStringList() << QString( "Airspace Range" ) << InfoString);
              QTreeWidgetItem* subsubItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(subItem, sl, Violations[i], m_Flight );
              subsubItem->setFlags( Qt::ItemIsEnabled );
              subsubItem->setForeground( 0, col1 );
              subsubItem->setForeground( 1, col2 );

              sl = (QStringList() << QString( "Type of Contact" ) << ViolationType);
              subsubItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(subItem, sl, Violations[i], m_Flight );
              subsubItem->setFlags( Qt::ItemIsEnabled );
              subsubItem->setForeground( 0, col1 );
              subsubItem->setForeground( 1, col2 );

              QDateTime Start;
              Start.setTime_t(m_Flight->getPoint( Violations[i].FirstIndexPointinRoute() ).time );
              sl = (QStringList() << QString( "Start" )
                  << Start.toString( QString( "dd-MM-yyyy hh:mm:ss" ) ));
              subsubItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(subItem, sl, Violations[i], m_Flight );
              subsubItem->setFlags( Qt::ItemIsEnabled );
              subsubItem->setForeground( 0, col1 );
              subsubItem->setForeground( 1, col2 );

#ifdef AIRSPACELISTVIEWITEM_EXTENDED_TRACE
              QString FirstID;
              FirstID.sprintf("%d",Violations[i].FirstIndexPointinRoute());
              sl = (QStringList() << QString("First ID") << FirstID);
              subsubItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(subItem, sl, Violations[i], m_Flight );
              subsubItem->setFlags( Qt::ItemIsEnabled );
              subsubItem->setForeground(0,col1);
              subsubItem->setForeground(1,col2);
#endif

              QDateTime End;
              End.setTime_t(m_Flight->getPoint( Violations[i].LastIndexPointinRoute() ).time );
              sl = (QStringList() << QString( "End" )
                  << End.toString( QString( "dd-MM-yyyy hh:mm:ss" ) ));
              subsubItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(subItem, sl, Violations[i], m_Flight );
              subsubItem->setFlags( Qt::ItemIsEnabled );
              subsubItem->setForeground( 0, col1 );
              subsubItem->setForeground( 1, col2 );

#ifdef AIRSPACELISTVIEWITEM_EXTENDED_TRACE
              QString LastID;
              LastID.sprintf("%d",Violations[i].LastIndexPointinRoute());
              sl = (QStringList() << QString("Last ID") << LastID);
              subsubItem = new AirSpaceListViewItem::AirSpaceFlagListViewItem(subItem, sl, Violations[i], m_Flight );
              subsubItem->setFlags( Qt::ItemIsEnabled );
              subsubItem->setForeground(0,col1);
              subsubItem->setForeground(1,col2);
#endif
            }
        }
      else
        {
          QStringList sl = (QStringList() << QString( "<<none found>>" ));
          QTreeWidgetItem* subItem = new QTreeWidgetItem( this, sl );
          subItem->setFlags( Qt::ItemIsEnabled );
        }
    }
}

AirSpaceListViewItem::AirSpaceFlagListViewItem::AirSpaceFlagListViewItem( QTreeWidgetItem* parent, const QStringList & Strings, Flight::AirSpaceIntersection theFlightIntersection, Flight * theFlight)
    :QTreeWidgetItem( parent, Strings, AIRSPACE_FLAG_LIST_VIEW_ITEM_TYPEID ), m_ItemToActivate(theFlightIntersection),m_Flight(theFlight)
{}

void AirSpaceListViewItem::AirSpaceFlagListViewItem::Activate()
{
  // - richtiger FLug muss aktiviert werden
  // - beide Cursors an Anfang und Ende des Luftraums setzen.
  if( m_Flight != 0 )
    {
      QList<FlightPoint*> Route = m_Flight->getRoute();
      time_t cursor1 = Route[m_ItemToActivate.FirstIndexPointinRoute()]->time;
      time_t cursor2 = Route[m_ItemToActivate.LastIndexPointinRoute()]->time;

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

      if( 0 != EvalView )
        {
          EvalFrame->slotShowFlight( m_Flight );
          EvalView->slotSetCursors( m_Flight, cursor1, cursor2 );
        }
    }
}
