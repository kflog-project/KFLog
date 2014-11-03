/***********************************************************************
**
**   centertodialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Andreé Somers, 2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class CenterToDialog
 *
 * \short Dialog to enter the coordinates to center the map to
 *
 * This dialog is shown if the user wants to center the map on a specific
 * coordinate. The user can enter a latitude and a longitude, and after
 * pressing OK the map is asked to center on that coordinate.
 *
 * \author André Somers, Axel Pauli
 *
 * \date 2002-2011
 *
 * \version $Id$
 */

#ifndef CENTER_TO_DIALOG_H
#define CENTER_TO_DIALOG_H

#include <QDialog>

#include "coordedit.h"

class CenterToDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( CenterToDialog )

public:
  /**
   * Constructor
   */
  CenterToDialog( QWidget* parent );
  /**
   * Destructor
   */
  virtual ~CenterToDialog();

signals:
  /**
   * Signal send when coordinate is selected
   */
  void centerTo(int lattitude, int longitude);

private:
  /**
   * Entry box for the latitude
   */
  LatEdit* latE;
  /**
   * Entry box for the longitude
   */
  LongEdit* longE;

private slots:
  /**
   * Received if the OK button is clicked
   */
  void slotOk();
};

#endif
