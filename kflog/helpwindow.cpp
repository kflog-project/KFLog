/***********************************************************************
**
**   helpwindow.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**
***********************************************************************/

#include "helpwindow.h"

#include <qvbox.h>
#include "mapconfig.h"
#include <qlayout.h>
#include <qtimer.h>
#include <klocale.h>
#include "resource.h"

HelpWindow::HelpWindow(QWidget* parent)
: QFrame(parent, "helpText")
{

  helpText = new KTextBrowser(this, "helpBrowser", true);

  QHBoxLayout* layout = new QHBoxLayout(this, 5);
  layout->addWidget(helpText);

  
}

HelpWindow::~HelpWindow(){
}


void HelpWindow::slotShowHelpText(QString text)
{
  helpText->setText(text);
}

void HelpWindow::slotClearView()
{
  QString text = "";
  helpText->setText(text);
}

