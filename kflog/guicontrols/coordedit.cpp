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
      if (col == 0) {
        if (validDirection.contains(s)) {
          setText(text().replace(0, 1, s));
          setCursorPosition(1);
        }
      }
      else {
        s.toInt(&isNumber);
        if (isNumber && mask[col] == '0') {
          setText(text().replace(col, 1, s));
          setCursorPosition(col + 1);
          // jump to next number field
          if (mask[col + 1] != '0') {
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

void CoordEdit::setSeconds(int s)
{
  int degree, min, sec;
  div_t divRes;
  QString tmp;

  divRes = div(s, 3600);
  degree = divRes.quot;
  divRes = div(divRes.rem, 60);
  min = divRes.quot;
  sec = divRes.rem;
  __formatCoord(degree, min, sec, s >= 0 ? 1 : -1);
}

LatEdit::LatEdit(QWidget *parent, const char *name ) : CoordEdit(parent, name)
{
  mask = "N00°00'00\"";
  validDirection = "NS";
}

LongEdit::LongEdit(QWidget *parent, const char *name ) : CoordEdit(parent, name)
{
  mask = "E000°00'00\"";
  validDirection = "WE";
}

/** No descriptions */
int LatEdit::seconds()
{
  int degree, min, sec, result;
  QString t = text();

  degree = t.mid(1, 2).toInt();
  min = t.mid(4, 2).toInt();
  sec = t.mid(7, 2).toInt();

  result = (degree * 3600) + (min * 60) + sec;
  if (result > 324000) { // 90° * 3600 sec
    result = 648000 - result;
  }

  if (t.left(1) == "S") {
    result = -result;
  }
  return result;
}

/** No descriptions */
int LongEdit::seconds()
{
  int degree, min, sec, result;
  QString t = text();

  degree = t.mid(1, 3).toInt();
  min = t.mid(5, 2).toInt();
  sec = t.mid(8, 2).toInt();

  result = (degree * 3600) + (min * 60) + sec;
  if (result > 648000) { // 180° * 3600 sec
    result = 1296000 - result;
  }

  if (t.left(1) == "W") {
    result = -result;
  }
  return result;
}

void LatEdit::__formatCoord(int degree, int min, int sec, int sign)
{
  QString tmp;
  tmp.sprintf("%c%02d°%02d'%02d\"", sign >= 0 ? 'N' : 'S', degree, min, sec);
  setText(tmp);
}

void LongEdit::__formatCoord(int degree, int min, int sec, int sign)
{
  QString tmp;
  tmp.sprintf("%c%03d°%02d'%02d\"", sign >= 0 ? 'E' : 'W', degree, min, sec);
  setText(tmp);
}
