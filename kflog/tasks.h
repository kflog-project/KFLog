/***************************************************************************
                          tasks.h  -  description
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

#ifndef TASKS_H
#define TASKS_H

#include <qwidget.h>

#include "guicontrols/kfloglistview.h"

/**
  *@author Harald Maier
  */

class Tasks : public QWidget  {
   Q_OBJECT
public: 
	Tasks(QWidget *parent=0, const char *name=0);
	~Tasks();
private:
  KFLogListView *tasks;
};

#endif
