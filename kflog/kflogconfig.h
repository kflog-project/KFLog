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
#include <qbuttongroup.h>
#include <qpushbutton.h>

class KFLogApp;
class LatEdit;
class LongEdit;

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
        LowFlight, Restricted, Tmz, Forest, Trail, Railway_D, Aerial_Cable, River_T,
        Glacier, Pack_Ice, FAIAreaLow500, FAIAreaHigh500};

    enum DefaultWaypointCatalog {Empty = 0, LastUsed, Specific};
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
    void slotDefaultProjection();
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
    void slotSelectProjection(int);
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
    /** */
    void slotSelectDefaultCatalog(int item);
    /** */
    void slotDefaultWaypoint();
    /** */
    void slotSearchDefaultWaypoint();
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
    void __addProjectionTab();
    /** */
    void __addScaleTab();
    /** */
    int __setScaleValue(int value);
    /** */
    int __getScaleValue(double value);
  /** Add a tab for waypoint catalog configuration at sartup
Setting will be overwritten by commandline switch */
  void __addWaypointTab();
    /** */
    QFrame* idPage;
    /** */
    QFrame* mapPage;
    /** */
    QFrame* pathPage;
    /** */
    QFrame* topoPage;
    /** */
    QFrame* projPage;
    /** */
    QFrame* scalePage;
    /** */
    QFrame* waypointPage;
    /** */
    KConfig* config;
    /** */
    QLineEdit* igcPathE;
    QLineEdit* taskPathE;
    QLineEdit* waypointPathE;
    QLineEdit* mapPathE;
    QLineEdit* catalogPathE;
    LatEdit* homeLatE;
    LongEdit* homeLonE;
    QLineEdit* homeNameE;
    QLineEdit* preNameE;
    QLineEdit* surNameE;
    QLineEdit* dateOfBirthE;

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
    KComboBox* projectionSelect;
    LatEdit* firstParallel;
    LatEdit* secondParallel;
    LongEdit* originLongitude;
    QButtonGroup *waypointButtonGroup;
    QPushButton* catalogPathSearch;

    int cylinPar;
    int lambertV1;
    int lambertV2;
    int lambertOrigin;
    int currentProjType;
};

#endif
