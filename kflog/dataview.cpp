/***********************************************************************
**
**   dataview.cpp
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

#include "dataview.h"

#include <klocale.h>
#include <qframe.h>

DataView::DataView(QWidget* parent)
: QTabWidget(parent)
{
  QFrame* flightDataFrame = new QFrame(this);
  QFrame* taskDataFrame = new QFrame(this);
  QFrame* mapElementFrame = new QFrame(this);
  QFrame* searchDataFrame = new QFrame(this);

  addTab(flightDataFrame, i18n("&Flight"));
  addTab(taskDataFrame, i18n("&Task"));
  addTab(mapElementFrame, i18n("&Map"));
  addTab(searchDataFrame, i18n("&Search"));
}

DataView::~DataView()
{
}
