/***********************************************************************
**
**   whatsthat.h
**
**   This file is part of Cumulus.
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

#ifndef WHATSTHAT_H
#define WHATSTHAT_H

#include <qwidget.h>
#include <qtimer.h>
#include <qstring.h>
#include <qsimplerichtext.h>


/**Rip-off from Qt's QWhatsThis system in order to be able to control the lifespan.
  *@author André Somers
  */

class WhatsThat : public QWidget
{
    Q_OBJECT
public:
    WhatsThat( QWidget* w, const QString& txt, QWidget* parent, const char* name, int timeout=5000, const QPoint * pos=0 );
    ~WhatsThat() ;

public slots:
    void hide();

protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void paintEvent( QPaintEvent* );
    void leaveEvent( QEvent *);
    
private:
    QString text;
    QSimpleRichText* doc;
    QString anchor;
    bool pressed;
    QWidget* widget;
    QTimer * autohideTimer;
    QPoint * suggestedPos;
    
private: // Private methods
  /** Tries to find itself a good position to display. */
  void position();
};


#endif
