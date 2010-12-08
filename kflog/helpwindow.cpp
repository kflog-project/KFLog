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

#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3Frame>

HelpWindow::HelpWindow(QWidget* parent)
: Q3Frame(parent, "helpText")
{

  helpText = new Q3TextBrowser(this, "helpBrowser");

  Q3HBoxLayout* layout = new Q3HBoxLayout(this, 5);
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

