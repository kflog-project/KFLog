/***********************************************************************
**
**   whatsthat.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers / TrollTech
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "whatsthat.h"
#include <qtooltip.h>
#include <qstylesheet.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qcursor.h>

// shadowWidth not const, for XP drop-shadow-fu turns it to 0
int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
const int vMargin = 8;
const int hMargin = 12;

WhatsThat::WhatsThat( QWidget* w, const QString& txt, QWidget* parent, const char* name, int timeout, const QPoint * pos)
    : QWidget( parent, name, WType_Popup ), text( txt ), pressed( FALSE ), widget( w ), suggestedPos(pos)
{

    setBackgroundMode( NoBackground );
    setPalette( QToolTip::palette() );
    setMouseTracking( TRUE );

    if ( widget )
      connect( widget, SIGNAL( destroyed() ), this, SLOT( hide() ) );

    QRect r;

    doc = 0;
    if ( QStyleSheet::mightBeRichText( text ) ) {
      QFont f = QApplication::font( this );
      doc = new QSimpleRichText( text, f );
      doc->adjustSize();
      r.setRect( 0, 0, doc->width(), doc->height() );
    } else {
      int sw = QApplication::desktop()->width() / 3;
      if ( sw < 200 )
        sw = 200;
      else if ( sw > 300 )
        sw = 300;

      r = fontMetrics().boundingRect( 0, 0, sw, 1000,
          Qt::AlignTop + Qt::WordBreak + Qt::ExpandTabs,
          text );
    }

    resize( r.width() + 2*hMargin + shadowWidth, r.height() + 2*vMargin + shadowWidth );
    position();    
    autohideTimer=new QTimer(this);
    connect(autohideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    if (timeout>0)
      autohideTimer->start(timeout,true);
}

WhatsThat::~WhatsThat()
{
  delete doc;
}

void WhatsThat::hide()
{
    QWidget::hide();
    delete this;

}

void WhatsThat::mousePressEvent( QMouseEvent* e )
{
    pressed = TRUE;
    if ( e->button() == LeftButton && rect().contains( e->pos() ) ) {
      if ( doc )
        anchor = doc->anchorAt( e->pos() -  QPoint( hMargin, vMargin) );
      return;
    }
    hide();
}

void WhatsThat::mouseReleaseEvent( QMouseEvent*  )
{
    if ( !pressed )
      return;
  

    hide();
}

void WhatsThat::mouseMoveEvent( QMouseEvent* )
{
}


void WhatsThat::keyPressEvent( QKeyEvent* )
{
    hide();
}



void WhatsThat::paintEvent( QPaintEvent* )
{
    bool drawShadow = TRUE;

    QRect r = rect();
    if ( drawShadow ) {
      r.setWidth(r.width()-shadowWidth);
      r.setHeight(r.height()-shadowWidth);
    }
    QPainter p( this);
    p.setPen( colorGroup().foreground() );
    p.drawRect( r );
    p.setPen( colorGroup().mid() );
    p.setBrush( colorGroup().brush( QColorGroup::Background ) );
    int w = r.width();
    int h = r.height();
    p.drawRect( 1, 1, w-2, h-2 );
    
    if ( drawShadow ) {
      p.setPen( colorGroup().shadow() );
      p.drawPoint( w + 5, 6 );
      p.drawLine( w + 3, 6, w + 5, 8 );
      p.drawLine( w + 1, 6, w + 5, 10 );
      int i;
      for( i=7; i < h; i += 2 )
        p.drawLine( w, i, w + 5, i + 5 );
      for( i = w - i + h; i > 6; i -= 2 )
        p.drawLine( i, h, i + 5, h + 5 );
      for( ; i > 0 ; i -= 2 )
        p.drawLine( 6, h + 6 - i, i + 5, h + 5 );
    }
    p.setPen( colorGroup().foreground() );
    //r.addCoords( hMargin, vMargin, -hMargin, -vMargin );
    r.moveBy(hMargin,vMargin);
    r.setWidth(r.width()-2*hMargin);
    r.setHeight(r.height()-2*vMargin);

    if ( doc ) {
      doc->draw( &p, r.x(), r.y(), r, colorGroup(), 0 );
    } else {
      p.drawText( r, AlignTop + WordBreak + ExpandTabs, text );
    }
}

/** Tries to find itself a good position to display. */

void WhatsThat::position(){
    // okay, now to find a suitable location

    QRect screen = QApplication::desktop()->rect();

    int x;
    int w = this->width();
    int h = this->height();
    int sx = screen.x();
    int sy = screen.y();

    QPoint ppos = QCursor::pos();
    if (suggestedPos) ppos=*suggestedPos;
    
    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    QPoint pos;
    if ( widget )
      pos = widget->mapToGlobal( QPoint( 0,0 ) );

    if ( widget && w > widget->width() + 16 )
      x = pos.x() + widget->width()/2 - w/2;
    else
      x = ppos.x() - w/2;

    // squeeze it in if that would result in part of what's this
    // being only partially visible
    if ( x + w  + shadowWidth > sx+screen.width() )
      x = (widget? (QMIN(screen.width(),
            pos.x() + widget->width())
          ) : screen.width() )
          - w;

    if ( x < sx )
      x = sx;

    int y;
    if ( widget && h > widget->height() + 16 ) {
      y = pos.y() + widget->height() + 2; // below, two pixels spacing
      // what's this is above or below, wherever there's most space
      if ( y + h + 10 > sy+screen.height() )
        y = pos.y() + 2 - shadowWidth - h; // above, overlap
    }
    y = ppos.y() + 2;

  // squeeze it in if that would result in part of what's this
  // being only partially visible
  if ( y + h + shadowWidth > sy+screen.height() )
    y = ( widget ? (QMIN(screen.height(),
          pos.y() + widget->height())
        ) : screen.height() )
      - h;
  if ( y < sy )
    y = sy;

  this->move( x, y );
  this->show();

}

void WhatsThat::leaveEvent(QEvent *) {
  hide();
}
