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

#include <QtGui>

#include "kflogtreewidget.h"
#include "rowdelegate.h"

extern QSettings _settings;

KFLogTreeWidget::KFLogTreeWidget( const char *name, QWidget *parent ) :
  QTreeWidget( parent ),
  confName( name ),
  rowDelegate( 0 )
{
  setObjectName( "KFLogTreeWidget" );
}

KFLogTreeWidget::~KFLogTreeWidget()
{
  if( ! confName.isEmpty() )
    {
      saveConfig();
    }
}

/** store the configuration in the app's configuration */
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

  QString path = "/KFLogTreeWidget/" + confName;

  _settings.setValue( path, array );
}

/** load the configuration from the app's configuration */
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

  QString path = "/KFLogTreeWidget/" + confName;

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
  if( event->button() != Qt::RightButton )
    {
      QTreeWidget::mousePressEvent( event );
      return;
    }

  qDebug() << "KFLogTreeWidget::mousePressEvent() Pos=" << event->pos();

  QTreeWidgetItem *item = itemAt( event->pos() );

  emit rightButtonPressed( item, event->pos() );
}
