/***********************************************************************
**
**   helpwindow.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Florian Ehinger
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "helpwindow.h"
#include "mainwindow.h"

extern QSettings _settings;

HelpWindow::HelpWindow( QWidget* parent ) : QWidget( parent )
{
  setObjectName( "HelpWindow" );
  setWindowTitle( tr("KFLog Help") );

  browser = new QTextBrowser(this);
  browser->setOpenLinks( true );
  browser->setOpenExternalLinks( true );

  QHBoxLayout* hbox = new QHBoxLayout(this);
  hbox->addWidget( browser );

  if( _settings.value("/HelpWindow/FirstOpen", true).toBool() )
    {
      // On first call we need a reasonable size of the widget. All other
      // is later on handled by the dock widget.
      _settings.setValue( "/HelpWindow/FirstOpen", false );
      resize(600, 300);
    }
}

HelpWindow::~HelpWindow()
{
}

void HelpWindow::slotShowHelpText(QString& text)
{
  browser->setHtml( text );
  //setVisible( true );
}

void HelpWindow::slotClearView()
{
  browser->clear();
}
