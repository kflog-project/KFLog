/***********************************************************************
**
**   kflogstartlogo.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "kflogstartlogo.h"
#include <kapp.h>

KFLogStartLogo::KFLogStartLogo()
    : QWidget(0,0,WStyle_NoBorder | WStyle_Customize | WStyle_StaysOnTop)
{
  setBackgroundMode(NoBackground);
  QPixmap pm;
  pm.load("/home/heiner/Entwicklung/kflog2/kflog/pics/splash.png");
  setBackgroundPixmap(pm);

  setGeometry( ( ( QApplication::desktop()->width() - pm.width() ) / 2 ),
               ( ( QApplication::desktop()->height() - pm.height() ) / 2 ),
               pm.width(), pm.height() );
}

KFLogStartLogo::~KFLogStartLogo()
{

}

void KFLogStartLogo::mousePressEvent(QMouseEvent*)  {  close();  }

bool KFLogStartLogo::event(QEvent* ev)
{
//  raise();
  return QWidget::event(ev);
}
