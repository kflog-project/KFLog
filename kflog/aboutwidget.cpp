/***********************************************************************
**
**   aboutwidget.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c): 2010-2011 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "aboutwidget.h"

AboutWidget::AboutWidget( QWidget *parent ) :
  QWidget( parent, Qt::Window )
{
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout *vbox = new QVBoxLayout( this );

  headerIcon = new QLabel( this );
  headerText = new QLabel( this );

  about  = new QTextBrowser( this );
  about->setOpenLinks( true );
  about->setOpenExternalLinks( true );

  team   = new QTextBrowser( this );
  team->setOpenLinks( true );
  team->setOpenExternalLinks( true );

  QTabWidget *tabWidget = new QTabWidget( this );
  tabWidget->addTab( about, tr("About") );
  tabWidget->addTab( team, tr("Team") );

  QPushButton *close = new QPushButton( tr("Ok"), this );

  QHBoxLayout *hbox = new QHBoxLayout;

  hbox->addWidget( headerIcon );
  hbox->addSpacing( 10 );
  hbox->addWidget( headerText );
  hbox->addStretch( 10 );

  vbox->addLayout( hbox );
  vbox->addWidget( tabWidget );
  vbox->addWidget( close );

  connect( close, SIGNAL(clicked()),  this, SLOT( close()) );
}
