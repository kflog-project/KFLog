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

#include <QApplication>
#include <QBitmap>
#include <QDesktopWidget>
#include <QDir>
#include <QPixmap>

KFLogStartLogo::KFLogStartLogo()
    : QWidget( 0, "KFlogStartWindow",  Qt::WStyle_Customize | Qt::WStyle_NoBorder )
{
  QPixmap pm(QDir::homeDirPath() + "/.kflog/pics/splash.png");

  setBackgroundPixmap(pm);
  setMask(QBitmap(QDir::homeDirPath() + "/.kflog/pics/splash_mask.png"));

  QDesktopWidget *desktop = QApplication::desktop();
  QRect rect = desktop->screenGeometry();
  move(rect.x() + (rect.width() - pm.width())/2,
       rect.y() + (rect.height() - pm.height())/2);

  setFixedSize(pm.size());
}

KFLogStartLogo::~KFLogStartLogo()
{

}
