/***********************************************************************
**
**   kflogstartlogo.h
**
**   This file is part of KFLog4.
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

#ifndef KFLOG_START_LOGO_H
#define KFLOG_START_LOGO_H

#include <QWidget>

/**
 * \class KFLogStartLogo
 *
 * \brief Creates the startup window displaying the logo.
 *
 * Creates the startup window displaying the logo.
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \date 2000-2010
 *
 * \version $Id$
 */
class KFLogStartLogo : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( KFLogStartLogo )

  public:
  /**
   * Creates a startup window.
   */
  KFLogStartLogo( QWidget *parent = 0 );
  /**
   * Destructor.
   */
  virtual ~KFLogStartLogo();
};

#endif
