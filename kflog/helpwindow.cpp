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

HelpWindow::HelpWindow( QWidget* parent ) : QWidget( parent )
{
  setObjectName( "HelpWindow" );
  setWindowTitle( tr("KFLog Help") );

  browser = new QTextBrowser(this);
  browser->setOpenLinks( true );
  browser->setOpenExternalLinks( true );

  QHBoxLayout* hbox = new QHBoxLayout(this);
  hbox->addWidget( browser );
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
