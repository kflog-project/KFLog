/***************************************************************************
                          coordedit.cpp  -  description
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

#include <stdlib.h>
// include files for QT

// include files for KDE

// include files for project
#include "coordedit.h"

CoordEdit::CoordEdit(QWidget *parent, const char *name ) : QLineEdit(parent, name)
{
}

void CoordEdit::focusInEvent (QFocusEvent *e)
{
  // overwrite default behavior of QLineEdit
  // fake a mouse event to prevent text highlight
  e->setReason(QFocusEvent::Mouse);
  QLineEdit::focusInEvent(e);
  // set cursor to start of line
  home(false);
}

// this function handle all the keyboard input
void CoordEdit::keyPressEvent (QKeyEvent *e)
{
  QString s;
  int col;
  bool isNumber;

  if (e->text() != 0) {
    s = e->text().upper();
    col = cursorPosition();
    if (hasMarkedText()) {
      deselect();
    }

    switch (e->key()) {
    case Key_Backspace:
      col -= 1;
      // fall through
    case Key_Delete:
      setText(text().replace(col, 1, mask.mid(col, 1)));
      setCursorPosition(col);
      break;
    default:
      if (col == text().length() - 1) {
        if (validDirection.contains(s)) {
          setText(text().replace(text().length() - 1, 1, s));
          setCursorPosition(text().length());
        }
      }
      else {
        s.toInt(&isNumber);
        if (isNumber && mask[col] == '0') {
          setText(text().replace(col, 1, s));
          setCursorPosition(++col);
          // jump to next number field
          //col++;
          if (col < text().length() && mask[col] != '0') {
            setCursorPosition(col + 2);
          }
        }
      }
    } // switch
  }
  else {
    // route all other to default handler
    QLineEdit::keyPressEvent(e);
  }
}

/** No descriptions */
void CoordEdit::showEvent(QShowEvent *)
{
  if (text().isEmpty()) {
    setText(mask);
  }
}

LatEdit::LatEdit(QWidget *parent, const char *name ) : CoordEdit(parent, name)
{
  mask = "00° 00' 00\" N";
  validDirection = "NS";
}

LongEdit::LongEdit(QWidget *parent, const char *name ) : CoordEdit(parent, name)
{
  mask = "000° 00' 00\" E";
  validDirection = "WE";
}
