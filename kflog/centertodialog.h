/***********************************************************************
**
**   centertodialog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Andree Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CENTERTODIALOG_H
#define CENTERTODIALOG_H

#include <qdialog.h>

#include "guicontrols/coordedit.h"
/**
 * @short Dialog to enter the coordinates to center the map to
 *
 * This dialog is shown if the user wants to center the map on a specific
 * coordinate. The user can enter a latitude and a longitude, and after
 * pressing OK the map is asked to center on that coordinate.
 *
 * @author André Somers
 * @version $Id$
 */
class CenterToDialog : public QDialog
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    CenterToDialog(QWidget* parent, const char* name);
    /**
     * Destructor
     */
    ~CenterToDialog();

  signals:
    /**
     * Signal send when coordinate is selected
     */
    void centerTo(int lattitude, int longitude);

  private:
    /**
     * Entrybox for the latitude
     */
    LatEdit* latE;
    /**
     * Entrybox for the longitude
     */
    LongEdit* longE;

  private slots:
    /**
     * Received if the OK button is clicked
     */
    void slotOk();
};

#endif
