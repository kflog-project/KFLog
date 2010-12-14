/***********************************************************************
**
**   kflogstartlogo.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "kflogstartlogo.h"

#include <QtGui>

KFLogStartLogo::KFLogStartLogo( QWidget *parent ) :
  QWidget( parent, Qt::FramelessWindowHint )
{
  qDebug() << "KFLogStartLogo()";

  setObjectName("KFLogStartLogo");

  // Destroy widget automatically, if it gets the close call.
  setAttribute(Qt::WA_DeleteOnClose);

  QPixmap pm(QDir::homePath() + "/.kflog/pics/splash.png");

  setBackgroundPixmap(pm);
  setMask(QBitmap(QDir::homePath() + "/.kflog/pics/splash_mask.png"));

  QDesktopWidget *desktop = QApplication::desktop();
  QRect rect = desktop->screenGeometry();

  move(rect.x() + (rect.width() - pm.width())/2,
       rect.y() + (rect.height() - pm.height())/2);

  setFixedSize(pm.size());
}

KFLogStartLogo::~KFLogStartLogo()
{
  qDebug() << "~KFLogStartLogo()";
}
