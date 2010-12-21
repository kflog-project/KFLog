/***********************************************************************
**
**   whatsthat.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WHATS_THAT_H
#define WHATS_THAT_H

#include <QString>
#include <QTimer>
#include <QWidget>
#include <QTextDocument>


/**
 * \author André Somers, Axel Pauli
 *
 * \brief Popup window to display user information.
 *
 * This class is used to display user information e.g. about airspaces.
 * It is realized as a frame less popup window and is closed by tipping on
 * it or automatically after a certain time. It can display plain or html text.
 * HTML text must be enclosed in HTML start and end tags.
 *
 * \date 2002-2010
 *
 * \version $Id$
 */
class WhatsThat : public QWidget
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( WhatsThat )

public:

  WhatsThat( QWidget* parent,
             QString& txt,
             int timeout=10000,
             const QPoint pos = QPoint(-9999,-9999) );

  virtual ~WhatsThat();

public slots:

  void hide();

protected:

  void mousePressEvent( QMouseEvent* );
  void keyPressEvent( QKeyEvent* );
  void paintEvent( QPaintEvent* );
  void leaveEvent( QEvent *);

private:

  /** Tries to find itself a good position to display. */
  void position();

private:

  QWidget *parent;

  // text to be displayed
  QTextDocument *doc;
  // width of document
  int docW;
  // height of document
  int docH;

  QTimer * autohideTimer;
  QPoint suggestedPos;
};

#endif
