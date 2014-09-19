/***********************************************************************
**
**   kflogtreewidget.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "kflogtreewidget.h"
#include "rowdelegate.h"

// Application settings object
extern QSettings _settings;

KFLogTreeWidget::KFLogTreeWidget( const char *name, QWidget *parent ) :
  QTreeWidget( parent ),
  confName( name ),
  rowDelegate( 0 ),
  showColMenu( 0 ),
  menuActionGroup( 0 )
{
  setObjectName( "KFLogTreeWidget" );
#ifdef QT_5
#pragma warning - need functional review here
  header()->setSectionResizeMode( QHeaderView::Interactive );
#else
  header()->setResizeMode( QHeaderView::Interactive );
#endif
}

KFLogTreeWidget::~KFLogTreeWidget()
{
  if( ! confName.isEmpty() )
    {
      saveConfig();
    }
}

void KFLogTreeWidget::saveConfig()
{
  if( confName.isEmpty() )
    { // Configuration name is empty, do nothing.
      return;
    }

  QHeaderView* headerView = header();

  if( headerView->count() == 0 )
    {
      return;
    }

  QByteArray array = headerView->saveState();

  QString path = "/KFLogTreeWidget/" + confName + "-Header";

  _settings.setValue( path, array );
}

void KFLogTreeWidget::loadConfig()
{
  if( confName.isEmpty() )
    { // Configuration name is empty, do nothing.
      return;
    }

  QHeaderView* headerView = header();

  if( headerView->count() == 0 )
    {
      return;
    }

  QString path = "/KFLogTreeWidget/" + confName + "-Header";

  bool ok = headerView->restoreState( _settings.value( path ).toByteArray() );

  if( ! ok )
    {
      qWarning() << "KFLogTreeWidget::loadConfig(): Could not restore header of"
                 << confName;
    }
}

void KFLogTreeWidget::addRowSpacing( const int pixels )
{
  if( ! rowDelegate )
    {
      rowDelegate = new RowDelegate( this );
      setItemDelegate( rowDelegate );
    }

  rowDelegate->setVerticalMargin( pixels );
}

void KFLogTreeWidget::mousePressEvent( QMouseEvent* event )
{
  if( event->button() == Qt::RightButton )
    {
      // Emit a signal, when the right button was pressed.
      QTreeWidgetItem *item = itemAt( event->pos() );
      emit rightButtonPressed( item, event->pos() );
      return;
    }

  if( event->button() == Qt::MidButton )
    {
      // Create a menu for switch on/off of tree view columns.
      createShowColMenu();
      showColMenu->exec( QCursor::pos() );
      return;
    }

  // Call base class mouse event handler.
  QTreeWidget::mousePressEvent( event );
}

void KFLogTreeWidget::slotResizeColumns2Content()
{
  for( int i = 0 ; i < columnCount(); i++ )
    {
      resizeColumnToContents( i );
    }
}

void KFLogTreeWidget::createShowColMenu()
{
  if( header()->count() == 0 || showColMenu != 0 )
    {
      return;
    }

  showColMenu = new QMenu(this);
  showColMenu->setTitle( tr("Toggle columns") );
  showColMenu->setToolTip(tr("Toggle visibility of table columns"));

  menuActionGroup = new QActionGroup(this);
  menuActionGroup->setExclusive( false );

  QAction *action = new QAction( tr("Show all columns"), this );
  action->setData( -1 );

  menuActionGroup->addAction( action );
  showColMenu->addAction( action );
  showColMenu->addSeparator();

  QTreeWidgetItem* hi = headerItem();

  for( int i = 0; i < header()->count(); i++ )
    {
      QAction *action = new QAction( hi->text(i), this );
      action->setCheckable( true );
      action->setChecked( isColumnHidden(i) == false );
      action->setData( i );

      menuActionGroup->addAction( action );
      showColMenu->addAction( action );
    }

  connect( menuActionGroup, SIGNAL(triggered(QAction *)),
           SLOT(slotMenuActionTriggered( QAction *)) );
}

/**
 * Called, if a menu action is toggled.
 */
void KFLogTreeWidget::slotMenuActionTriggered( QAction* action )
{
  int columnIdx = action->data().toInt();

  if( columnIdx == -1 )
    {
      // All columns should be switched on.
      QList<QAction *> actions = menuActionGroup->actions ();

      for( int i = 0; i < actions.size(); i++ )
        {
          columnIdx = actions.at(i)->data().toInt();

          if( columnIdx == -1 )
            {
              continue;
            }

          setColumnHidden( columnIdx, false );
          actions.at(i)->setChecked( true );
        }

      slotResizeColumns2Content();
      return;
    }

  // Only one column has to handle.
  setColumnHidden( columnIdx, ( action->isChecked() == false ) );
  slotResizeColumns2Content();
}

void KFLogTreeWidget::slotShowColMenu()
{
  createShowColMenu();
  showColMenu->exec(  QCursor::pos() );
}
