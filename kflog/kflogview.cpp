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
#include <qprinter.h>
#include <qpainter.h>

// application specific includes
#include "kflogview.h"
#include "kflogdoc.h"
#include "kflog.h"

KFLogView::KFLogView(QWidget *parent, const char *name) : QWidget(parent, name)
{
  setBackgroundMode(PaletteBase);
}

KFLogView::~KFLogView()
{
}

KFLogDoc *KFLogView::getDocument() const
{
  KFLogApp *theApp=(KFLogApp *) parentWidget();

  return theApp->getDocument();
}

void KFLogView::print(QPrinter *pPrinter)
{
  QPainter printpainter;
  printpainter.begin(pPrinter);
	
  // TODO: add your printing code here

  printpainter.end();
}
