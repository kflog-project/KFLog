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
//Added by qt3to4:
#include <QFocusEvent>
#include <QKeyEvent>
#include <QShowEvent>

/**
  *@short baseclass for @ref LatEdit and @ref LongEdit
  *@author Harald Maier
  *
  * Implements a widget for editing a coordinate
  */

class CoordEdit : public QLineEdit  {
   Q_OBJECT
public:
 /**
  * Constructor. Takes the default @ref QWidget arguments
  */
	CoordEdit(QWidget *parent=0, const char *name=0);
 /**
  * Destructor
  */ 
	~CoordEdit() {};
 /**
  * A key is pressed. Processes the key to see if it's valid input.
  */ 
	void keyPressEvent (QKeyEvent *e);
 /**
  * The widget gets the focus
  */ 
	void focusInEvent (QFocusEvent *e);
  /**
   * The widget is shown
   */
  void showEvent(QShowEvent *);
protected:
  /**
   * Contains the mask used for the entrywidget
   */
	QString mask;
  /**
   * Sets the valid directions (EW vs NS)
   */
	QString validDirection;
public slots: // Public slots
  /**
   * Clears the widget
   */
  void clear();
};

/**
 * @short Implements a Latitude editor widget
 * @author Harald Maier
 */
class LatEdit : public CoordEdit  {
   Q_OBJECT
public:
 /**
  * Constructor. Takes the default @ref QWidget arguments
  */
	LatEdit(QWidget *parent=0, const char *name=0, int base=0);
 /**
  * Destructor
  */
	~LatEdit() {};
};

/**
 * @short Implements a Longitude editor widget
 * @author Harald Maier
 */
class LongEdit : public CoordEdit  {
   Q_OBJECT
public:
 /**
  * Constructor. Takes the default @ref QWidget arguments
  */
	LongEdit(QWidget *parent=0, const char *name=0, int base=0);
 /**
  * Destructor
  */
	~LongEdit() {};
};
#endif
