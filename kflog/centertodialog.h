/***********************************************************************
**
**   centertodialog.h
**
**   This file is part of KFLog2.
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

#include <kdialogbase.h>

#include <guicontrols/coordedit.h>
/**
 * @author Andree Somers
 */
class CenterToDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /**
     */
    CenterToDialog(QWidget* parent, const char* name);
    /**
     */
    ~CenterToDialog();

  signals:
    /**
     * Signal send when coordinate is selected
     */
    void centerTo(int lattitude, int longitude);

  private:
    /** */
    LatEdit* latE;
    /** */
    LongEdit* longE;

  private slots:
    /** */
    void slotOk();
};

#endif
