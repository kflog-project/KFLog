/***************************************************************************
                          tasks.cpp  -  description
                             -------------------
    begin                : Fri Mar 1 2002
    copyright            : (C) 2002 by Harald Maier
    email                : harry@kflog.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tasks.h"

#include <qlayout.h>

#include <klocale.h>

Tasks::Tasks(QWidget *parent, const char *name )
  : QWidget(parent, name)
{
  tasks =  new KFLogListView("Tasks", this, "tasks");
//  tasks->setShowSortIndicator(true);
  tasks->setSorting(-1);
  tasks->setAllColumnsShowFocus(true);
  tasks->setRootIsDecorated(true);

  tasks->addColumn(i18n("Task"));
  tasks->addColumn(i18n("total Dist."));

  QVBoxLayout *layout = new QVBoxLayout(this, 5, 5);
  layout->addWidget(tasks);

  QListViewItem *i = new QListViewItem(tasks, "TASK001");
  i->setText(1, "252.3 km");

  QListViewItem *ii = new QListViewItem(i, "Takeoff");
  ii = new QListViewItem(i, ii, "Start");
  ii = new QListViewItem(i, ii, "Turnpoint");
  ii = new QListViewItem(i, ii, "End");
  ii = new QListViewItem(i, ii, "Landing");
}

Tasks::~Tasks()
{
}
