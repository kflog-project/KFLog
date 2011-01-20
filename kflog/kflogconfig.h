/***********************************************************************
**
**   kflogconfig.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2010 by Constantijn Neeteson, Heiner Lamprecht, Florian Ehinger
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef KFLOG_CONFIG_H
#define KFLOG_CONFIG_H

#include <q3buttongroup.h>
#include <QComboBox>
#include <QCheckBox>
#include <QDialog>
#include <QFrame>
#include <QLayout>
#include <QLCDNumber>
#include <QLineEdit>
#include <q3listview.h>
#include <QPen>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTreeWidget>
#include <QWidget>

#include "guicontrols/coordedit.h"

/**
 * \class KFLogConfig
 *
 * \short Configuration dialog widget
 *
 * This class provides the configuration dialog for KFLog.
 *
 * \author Heiner Lamprecht, Florian Ehinger, , Constantijn Neeteson, Axel Pauli
 *
 * \date 2000-2011
 *
 * @version $Id$
 */
class KFLogConfig : public QDialog
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( KFLogConfig )

  public:

    KFLogConfig( QWidget* parent=0 );

    virtual ~KFLogConfig();

    /** */
    enum ElementType { Road = 0, Highway, Railway, River, Canal, City,
        AirA, AirB, AirC, AirD, AirElow, AirEhigh, AirF, ControlC, ControlD, Danger,
        LowFlight, Restricted, Tmz, Forest, Trail, Railway_D, Aerial_Cable, River_T,
        Glacier, PackIce, FAIAreaLow500, FAIAreaHigh500 };

    enum DefaultWaypointCatalog { Empty = 0, LastUsed, Specific };

  signals:
    /** */
    void scaleChanged(int min, int max);
    /** */
    void configOk();
    /** */
    void newDrawType(int type);

  public slots:
    /** */
    void slotOk();

    void slotPageClicked( QTreeWidgetItem * item, int column );
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
    void slotShowWpLabel(int value);
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
    /**
    * slot needed to trigger an update of the menu Flight=>Show Flightdata
    */
    void slotDrawTypeSelect();
    /** */
    void slotSelectFlightTypeLeftTurnColor();
    /** */
    void slotSelectFlightTypeRightTurnColor();
    /** */
    void slotSelectFlightTypeMixedTurnColor();
    /** */
    void slotSelectFlightTypeStraightColor();
    /** */
    void slotSelectFlightTypeSolidColor();
    /** */
    void slotSelectFlightTypeEngineNoiseColor();

  private:
    /** */
    void __addIDTab();
    /** */
    void __addMapTab();
    /** */
    void __addFlightTab();
    /** */
    void __addPathTab();
    /** */
    void __addProjectionTab();
    /** */
    void __addScaleTab();
    /** */
    int __setScaleValue(int value);
    /** */
    int __getScaleValue(double value);
    /** */
    void __addAirfieldTab();
    /** */
    void __addWaypointTab();

    /** this is a temporary function and it is not needed in Qt 4 */
    QString __color2String(QColor);

    /** this is a temporary function and it is not needed in Qt 4 */
    QColor __string2Color(QString);

    QGridLayout *configLayout;

    QTreeWidget *setupTree;

    QFrame *activePage;
    /** */
    QFrame* idPage;
    /** */
    QFrame* mapPage;
    /** */
    QFrame* flightPage;
    /** */
    QFrame* pathPage;
    /** */
    QFrame* topoPage;
    /** */
    QFrame* projPage;
    /** */
    QFrame* scalePage;
    /** */
    QFrame* airfieldPage;
    /** */
    QFrame* waypointPage;
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
    QSpinBox* flightPathWidthE;

    QLineEdit* filterWelt2000;
    QSpinBox* homeRadiusWelt2000;

    /**
     * Initial value of home radius.
     */
    int homeRadiusWelt2000Value;

    /**
     * Initial value of country filter
     */
    QString filterWelt2000Text;

    bool needUpdateDrawType;

    QSlider* lLimit;
    QSlider* uLimit;
    QSlider* switchScale;
    QSlider* wpLabel;
    QSlider* reduce1;
    QSlider* reduce2;
    QSlider* reduce3;
    QLCDNumber* lLimitN;
    QLCDNumber* uLimitN;
    QLCDNumber* switchScaleN;
    QLCDNumber* wpLabelN;
    QLCDNumber* reduce1N;
    QLCDNumber* reduce2N;
    QLCDNumber* reduce3N;
    QComboBox* elementSelect;
    QComboBox* drawTypeSelect;
    QComboBox* projectionSelect;
    LatEdit* firstParallel;
    LatEdit* secondParallel;
    LongEdit* originLongitude;

    Q3ButtonGroup *waypointButtonGroup;
    QPushButton* catalogPathSearch;
    QPushButton* flightTypeLeftTurnColorButton;
    QColor flightTypeLeftTurnColor;
    QPushButton* flightTypeRightTurnColorButton;
    QColor flightTypeRightTurnColor;
    QPushButton* flightTypeMixedTurnColorButton;
    QColor flightTypeMixedTurnColor;
    QPushButton* flightTypeStraightColorButton;
    QColor flightTypeStraightColor;
    QPushButton* flightTypeSolidColorButton;
    QColor flightTypeSolidColor;
    QPushButton* flightTypeEngineNoiseColorButton;
    QColor flightTypeEngineNoiseColor;

    int cylinPar;
    int lambertV1;
    int lambertV2;
    int lambertOrigin;
    int currentProjType;
};

#endif
