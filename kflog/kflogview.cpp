/***********************************************************************
**
**   kflogview.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

// include files for Qt
#include <qlabel.h>
#include <qlayout.h>
#include <qprinter.h>
#include <qpainter.h>

// application specific includes
#include "kflogview.h"
#include "kflogdoc.h"
#include "kflog.h"

KFLogView::KFLogView(QWidget *parent, const char *name) : QWidget(parent, name)
{
  QLabel* helloLabel = new QLabel("hallo, ich bin ein label", this);

  QBoxLayout* layout = new QBoxLayout(this, QBoxLayout::LeftToRight);
  layout->addWidget(helloLabel);
}

KFLogView::~KFLogView()
{
}
