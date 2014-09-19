/***********************************************************************
**
**   whatsthat.cpp
**
**   This file is part of Cumulus
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

#ifdef QT_5
    #include <QtWidgets>
    #include <QApplication>
#else
    #include <QtGui>
#endif

#include "whatsthat.h"

const int vMargin = 5;
const int hMargin = 5;

WhatsThat::WhatsThat( QWidget* parent,
                      QString& txt,
                      int timeout,
                      const QPoint pos ) :
  QWidget( parent, Qt::Popup ),
  parent(parent),
  suggestedPos(pos)
{
  setObjectName("WhatsThat");
  setAttribute( Qt::WA_DeleteOnClose );

  autohideTimer = new QTimer(this);
  autohideTimer->setSingleShot( true );

  doc = new QTextDocument( this );

  doc->setDefaultFont( QFont ("Helvetica", 16 ) );

  // check, what kind of text has been passed
  if( txt.contains("<html>", Qt::CaseInsensitive ) ||
      txt.contains("<qt>", Qt::CaseInsensitive ) )
    {
      // qDebug("HTML=%s", txt.latin1());
      doc->setHtml( txt );
    }
  else
    {
      //qDebug("PLAIN=%s", txt.latin1());
      doc->setPlainText( txt );
    }

  // get current document size
  QSize docSize = doc->size().toSize();

  docW = docSize.width();
  docH = docSize.height();

  resize( docW + 2*hMargin, docH + 2*vMargin );

  position();

  // qDebug("DocSize: w=%d, h=%d", docW + 2*hMargin, docH + 2*vMargin);

  // @AP: Widget will be destroyed, if timer expired. If timeout is
  // zero, manual quit is expected by the user.
  if( timeout > 0 )
    {
      connect(autohideTimer, SIGNAL(timeout()), this, SLOT(hide()));
      autohideTimer->start(timeout);
    }

  repaint();
}

WhatsThat::~WhatsThat()
{
  delete doc;
}

void WhatsThat::hide()
{
  autohideTimer->stop();
  QWidget::close();
}

void WhatsThat::mousePressEvent( QMouseEvent* )
{
  hide();
}

void WhatsThat::keyPressEvent( QKeyEvent* )
{
  hide();
}

void WhatsThat::leaveEvent( QEvent* )
{
  hide();
}

void WhatsThat::paintEvent( QPaintEvent* )
{
  QPixmap pm = QPixmap( docW+1, docH+1 );
  pm.fill(QColor(255, 255, 224)); // LightYellow www.wackerart.de/rgbfarben.html

  QPainter docP;

  docP.begin(&pm);
  doc->drawContents( &docP );
  docP.end();

  QPainter p( this );
  p.fillRect( rect(), Qt::red );

  p.drawPixmap( hMargin, vMargin, pm );
}

/** Tries to find itself a good position to display. */

void WhatsThat::position()
{
  int shadowWidth = 0;

  // okay, now to find a suitable location
  QRect screen = QApplication::desktop()->rect();

  int x;
  int w = this->width();
  int h = this->height();
  int sx = screen.x();
  int sy = screen.y();

  QPoint ppos = QCursor::pos();

  if( suggestedPos.x() != -9999 && suggestedPos.y() != -9999 )
    {
      ppos = suggestedPos;
    }

  // first try locating the widget immediately above/below,
  // with nice alignment if possible.
  QPoint pos;

  if( parent )
    {
      pos = parent->mapToGlobal( QPoint( 0, 0 ) );
    }

  if( parent && w > parent->width() + 16 )
    x = pos.x() + parent->width() / 2 - w / 2;
  else
    x = ppos.x() - w / 2;

  // squeeze it in if that would result in part of what's this
  // being only partially visible
  if( x + w + shadowWidth > sx + screen.width() )
    x = (parent ? (qMin(screen.width(),
        pos.x() + parent->width())) : screen.width()) - w;

  if( x < sx )
    x = sx;

  int y;

  if( parent && h > parent->height() + 16 )
    {
      y = pos.y() + parent->height() + 2; // below, two pixels spacing
      // what's this is above or below, wherever there's most space
      if( y + h + 10 > sy + screen.height() )
        y = pos.y() + 2 - shadowWidth - h; // above, overlap
    }
  y = ppos.y() + 2;

  // squeeze it in if that would result in part of what's this
  // being only partially visible
  if( y + h + shadowWidth > sy + screen.height() )
    y = (parent ? (qMin(screen.height(),
        pos.y() + parent->height())) : screen.height()) - h;
  if( y < sy )
    y = sy;

  this->move( x, y );
}
