/***********************************************************************
**
**   olcdialog.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef OLCDIALOG_H
#define OLCDIALOG_H

#include <kcombobox.h>
#include <klistview.h>
#include <krestrictedline.h>

#include <qcheckbox.h>
#include <qdialog.h>
#include <qlabel.h>

/**
 * This class provides a dialog for the OLC-declaration.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class OLCDialog : public QDialog
{
  public:
    /** */
    OLCDialog(QWidget* parent, const char* name);
    /** */
    ~OLCDialog();

  private:
    /** */
    KLineEdit* preName;
    /** */
    KLineEdit* surName;
    /** */
    KRestrictedLine* birthday;
    /** */
    KLineEdit* gliderType;
    /** */
    KLineEdit* gliderID;
    /** */
    KListView* taskList;
    /** */
    KRestrictedLine* daec;
    /** */
    QCheckBox* pureGlider;
    /** */
    KComboBox* classSelect;
    /** */
    KLineEdit* startPoint;
    /** */
    QLabel* startPos;
    /** */
    QLabel* routeLength;
    /** */
    QLabel* routePoints;
    /** */
    KComboBox* olcName;
};

#endif
