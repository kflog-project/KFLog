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
        AirC, AirD, AirElow, AirEhigh, AirF, ControlC, ControlD, Danger,
        LowFlight, Restricted, TMZ};

  signals:
    /** */
    void scaleChanged(int min, int max);

  public slots:
    /** */
    void slotOk();
    /** */
    void slotDefaultElements();
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
    void slotSelectElement(int);
    /** */
    void slotToggleFirst(bool);
    /** */
    void slotToggleSecond(bool);
    /** */
    void slotToggleThird(bool);
    /** */
    void slotToggleForth(bool);
    /** */
    void slotSetSecond();
    /** */
    void slotSetThird();
    /** */
    void slotSetForth();
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
    void __addPrintTab();
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
    QFrame* printPage;
    /** */
    QFrame* scalePage;
    /** */
    KConfig* config;
    /** */
    QLineEdit* igcPathE;
    QLineEdit* taskPathE;
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

    QCheckBox* border1;
    QCheckBox* border2;
    QCheckBox* border3;
    QCheckBox* border4;

    KColorButton* border1Color;
    KColorButton* border2Color;
    KColorButton* border3Color;
    KColorButton* border4Color;

    QSpinBox* border1Pen;
    QSpinBox* border2Pen;
    QSpinBox* border3Pen;
    QSpinBox* border4Pen;

    QPushButton* border1Button;
    QPushButton* border2Button;
    QPushButton* border3Button;

    KComboBox* border1PenStyle;
    KComboBox* border2PenStyle;
    KComboBox* border3PenStyle;
    KComboBox* border4PenStyle;

    KColorButton* border1BrushColor;
    KColorButton* border2BrushColor;
    KColorButton* border3BrushColor;
    KColorButton* border4BrushColor;

    KComboBox* border1BrushStyle;
    KComboBox* border2BrushStyle;
    KComboBox* border3BrushStyle;
    KComboBox* border4BrushStyle;

    KComboBox* elementSelect;

    QList<QPen> airCPenList;
    QList<QBrush> airCBrushList;
    QList<QPen> airDPenList;
    QList<QBrush> airDBrushList;
    QList<QPen> airElPenList;
    QList<QBrush> airElBrushList;
    QList<QPen> airEhPenList;
    QList<QBrush> airEhBrushList;
    QList<QPen> airFPenList;
    QList<QBrush> airFBrushList;
    QList<QPen> ctrCPenList;
    QList<QBrush> ctrCBrushList;
    QList<QPen> ctrDPenList;
    QList<QBrush> ctrDBrushList;
    QList<QPen> lowFPenList;
    QList<QBrush> lowFBrushList;
    QList<QPen> dangerPenList;
    QList<QBrush> dangerBrushList;
    QList<QPen> restrPenList;
    QList<QBrush> restrBrushList;
    QList<QPen> tmzPenList;
    QList<QBrush> tmzBrushList;

    QList<QPen> roadPenList;
    QList<QPen> railPenList;
    QList<QPen> riverPenList;
    QList<QPen> highwayPenList;
    QList<QPen> cityPenList;
    QList<QBrush> cityBrushList;

    bool* airCBorder;
    bool* airDBorder;
    bool* airElBorder;
    bool* airEhBorder;
    bool* airFBorder;
    bool* ctrCBorder;
    bool* ctrDBorder;
    bool* dangerBorder;
    bool* lowFBorder;
    bool* restrBorder;
    bool* tmzBorder;

    bool* roadBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* riverBorder;
    bool* cityBorder;

    int oldElement;
};

#endif
