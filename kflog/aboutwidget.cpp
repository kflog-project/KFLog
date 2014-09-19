/***********************************************************************
**
**   aboutwidget.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c): 2010-2011 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/
#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "aboutwidget.h"

AboutWidget::AboutWidget( QWidget *parent ) :
  QWidget( parent, Qt::Window )
{
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout *vbox = new QVBoxLayout( this );

  m_headerIcon = new QLabel( this );
  m_headerText = new QLabel( this );

  m_about  = new QTextBrowser( this );
  m_about->setOpenLinks( true );
  m_about->setOpenExternalLinks( true );

  m_team   = new QTextBrowser( this );
  m_team->setOpenLinks( true );
  m_team->setOpenExternalLinks( true );

  m_eula   = new QTextBrowser( this );
  m_eula->setOpenLinks( true );
  m_eula->setOpenExternalLinks( true );


  QTabWidget *tabWidget = new QTabWidget( this );
  tabWidget->addTab( m_about, tr("About") );
  tabWidget->addTab( m_team, tr("Team") );
  tabWidget->addTab( m_eula, tr("Eula") );

  QPushButton *close = new QPushButton( tr("Ok"), this );

  QHBoxLayout *hbox = new QHBoxLayout;

  hbox->addWidget( m_headerIcon );
  hbox->addSpacing( 10 );
  hbox->addWidget( m_headerText );
  hbox->addStretch( 10 );

  vbox->addLayout( hbox );
  vbox->addWidget( tabWidget );
  vbox->addWidget( close );

  connect( close, SIGNAL(clicked()),  this, SLOT( close()) );
}
