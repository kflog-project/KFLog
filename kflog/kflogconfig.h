/***********************************************************************
**
**   kflogconfig.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef KFLOGCONFIG_H
#define KFLOGCONFIG_H

#include <kconfig.h>
#include <kdialogbase.h>

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <qcheckbox.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qpen.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qwidget.h>

class KFLogApp;

/**
 * This class provides the config-dialog for KFLog.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class KFLogConfig : public KDialogBase
{
  Q_OBJECT

  public:
    /** */
    KFLogConfig(QWidget* parent, KConfig* config, const char* name);
    /** */
    ~KFLogConfig();
    /** */
    enum ElementType {Road = 0, Highway, Railway, River, Canal, City,
        AirA, AirB, AirC, AirD, AirElow, AirEhigh, AirF, ControlC, ControlD, Danger,
        LowFlight, Restricted, TMZ};

  signals:
    /** */
    void scaleChanged(int min, int max);
    /** */
    void configOk();

  public slots:
    /** */
    void slotOk();
    /** */
    void slotDefaultPath();
    /** */
    void slotDefaultScale();
    /** */
    void slotSearchFlightPath();
    /** */
    void slotSearchMapPath();
    /** */
    void slotSearchTaskPath();
    /** */
    void slotSearchWaypointPath();
    /** */
    void slotShowLowerLimit(int value);
    /** */
    void slotShowUpperLimit(int value);
    /** */
    void slotShowSwitchScale(int value);
    /** */
    void slotShowReduceScaleA(int value);
    /** */
    void slotShowReduceScaleB(int value);
    /** */
    void slotShowReduceScaleC(int value);

  private:
    /** */
    void __addIDTab();
    /** */
    void __addMapTab();
    /** */
    void __addPathTab();
    /** */
    void __addTopographyTab();
    /** */
    void __addScaleTab();
    /** */
    int __setScaleValue(int value);
    /** */
    int __getScaleValue(double value);
    /** */
    QFrame* idPage;
    /** */
    QFrame* mapPage;
    /** */
    QFrame* pathPage;
    /** */
    QFrame* topoPage;
    /** */
    QFrame* scalePage;
    /** */
    KConfig* config;
    /** */
    QLineEdit* igcPathE;
    QLineEdit* taskPathE;
    QLineEdit* waypointPathE;
    QLineEdit* mapPathE;
    QLineEdit* homeLatE;
    QLineEdit* homeLonE;
    QLineEdit* homeNameE;

    QSlider* lLimit;
    QSlider* uLimit;
    QSlider* switchScale;
    QSlider* reduce1;
    QSlider* reduce2;
    QSlider* reduce3;
    QLCDNumber* lLimitN;
    QLCDNumber* uLimitN;
    QLCDNumber* switchScaleN;
    QLCDNumber* reduce1N;
    QLCDNumber* reduce2N;
    QLCDNumber* reduce3N;

    KComboBox* elementSelect;
};

#endif
