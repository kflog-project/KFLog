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

#include <flight.h>

/**
 * This class provides a dialog for the OLC-declaration.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class OLCDialog : public QDialog
{
  Q_OBJECT

  public:
    /** */
    OLCDialog(QWidget* parent, const char* name, Flight* currentFlight);
    /** */
    ~OLCDialog();

  public slots:
    /**
     */
    void slotSend();

  private:
    /**
     * Reads the data from the flight and the task and fills the fields.
     */
    void __fillDataFields();
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
    /** */
    Flight* currentFlight;
    /** */
    int taskColID;
    /** */
    int taskColWP;
    /** */
    int taskColLat;
    /** */
    int taskColLon;
    /** */
    int taskColDist;
    /** */
    int taskColTime;
};

#endif
