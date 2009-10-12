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
#include <qsettings.h>

// include files for KDE
#include <kapp.h>

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
  QString inputCols="0" + validDirection;

  if (e->text().length()!=0 || e->key()==Key_Left || e->key()==Key_Right) {     //somehow, the original code let modifier keys through. This replacement does not.
    s = e->text().upper();
    col = cursorPosition();
    if (hasMarkedText()) {
      deselect();
    }

    switch (e->key()) {
    case Key_Return:
    case Key_Escape:
      QLineEdit::keyPressEvent(e);
      break;
    case Key_Backspace:
      while (!inputCols.contains(mask.mid(col-1,1))) setCursorPosition(--col); //move cursor to previous editable character
      col -= 1;
      setText(text().replace(col, 1, mask.mid(col, 1)));
      setCursorPosition(col);
      break;
    case Key_Delete:
      setText(text().replace(col, 1, mask.mid(col, 1)));
      setCursorPosition(col);
      break;
    case Key_Left:
      col--;
      setCursorPosition(col);
      while (!inputCols.contains(mask.mid(col,1))) setCursorPosition(--col); //move cursor in front of previous editable character
      break;
    case Key_Right:
      col++;
      setCursorPosition(col);
      while (!inputCols.contains(mask.mid(col,1))) setCursorPosition(++col); //move cursor in front of next editable character
      break;
    default:

      if (col == (int)text().length() - 1) {
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
          if ((col < (int)text().length()) && (mask[col] != '0')) {
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

LatEdit::LatEdit(QWidget *parent, const char *name, int base ) : CoordEdit(parent, name)
{
  if (base==0) {
    extern QSettings _settings;

    base = _settings.readNumEntry("/MapData/HomesiteLatitude", 1);
  }
  mask = "00° 00' 00\" S";
  if (base>0) mask = "00° 00' 00\" N";
  validDirection = "NS";
}

LongEdit::LongEdit(QWidget *parent, const char *name, int base ) : CoordEdit(parent, name)
{
  if (base==0) {
    extern QSettings _settings;

    base = _settings.readNumEntry("/MapData/HomesiteLongitude", 1);
  }
  mask = "000° 00' 00\" W";
  if (base>0) mask = "000° 00' 00\" E";
  validDirection = "WE";
}
/** No descriptions */
void CoordEdit::clear()
{
  setText(mask);
}
