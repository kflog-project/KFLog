/***********************************************************************
**
**   kflogstartlogo.h
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

#ifndef KFLOGSTARTLOGO_H
#define KFLOGSTARTLOGO_H

#include <qframe.h>

/**
  * Creates the startup-window displaying the logo.
  *
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  */
class KFLogStartLogo : public QFrame
{
  Q_OBJECT

  public:
    /**
     * Creates a new startup-window.
     */
    KFLogStartLogo();
    /**
     * Destructor, does nothing special.
     */
    ~KFLogStartLogo();

  protected:
    /** */
    virtual bool event(QEvent*);
    /** */
    virtual void mousePressEvent(QMouseEvent*);
};

#endif
