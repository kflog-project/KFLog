/***************************************************************************
                          coordedit.h  -  description
                             -------------------
    begin                : Mon Dec 3 2001
    copyright            : (C) 2001 by Harald Maier
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

#ifndef COORDEDIT_H
#define COORDEDIT_H

#include <qwidget.h>
#include <qlineedit.h>

/**
  *@author Harald Maier
  */

class CoordEdit : public QLineEdit  {
   Q_OBJECT
public:
	CoordEdit(QWidget *parent=0, const char *name=0);
	~CoordEdit() {};
	void keyPressEvent (QKeyEvent *e);
	void focusInEvent (QFocusEvent *e);
  /** No descriptions */
  void showEvent(QShowEvent *);
protected:
  /** No descriptions */
	QString mask;
	QString validDirection;
public slots: // Public slots
  /** No descriptions */
  void clear();
};

class LatEdit : public CoordEdit  {
   Q_OBJECT
public:
	LatEdit(QWidget *parent=0, const char *name=0);
	~LatEdit() {};
};

class LongEdit : public CoordEdit  {
   Q_OBJECT
public:
	LongEdit(QWidget *parent=0, const char *name=0);
	~LongEdit() {};
};
#endif
