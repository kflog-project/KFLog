/***********************************************************************
**
**   configdrawelement.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CONFIGDRAWELEMENT_H
#define CONFIGDRAWELEMENT_H

#include <kconfig.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qpen.h>
#include <qslider.h>
#include <qspinbox.h>

/**
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class ConfigDrawElement : public QFrame
{
  Q_OBJECT

  public:
    ConfigDrawElement(QWidget* parent, KConfig* cnf);
    ~ConfigDrawElement();
    /** */
    enum ElementType {Road = 0, Highway, Railway, River, Canal, City,
        AirA, AirB, AirC, AirD, AirElow, AirEhigh, AirF, ControlC, ControlD, Danger,
        LowFlight, Restricted, TMZ, Forest, Trail, Railway_D, Aerial_Cable, River_T,
        Glacier, PackIce, FAIAreaLow500, FAIAreaHigh500};

  public slots:
    /** */
    void slotSelectElement(int);
    /** */
    void slotDefaultElements();
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
    void slotOk();

  private:
    KConfig* config;
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

    QList<QPen> airAPenList;
    QList<QBrush> airABrushList;
    QList<QPen> airBPenList;
    QList<QBrush> airBBrushList;
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
/*
    QList<QPen> roadPenList;
    QList<QPen> railPenList;
    QList<QPen> riverPenList;
    QList<QPen> highwayPenList;
    QList<QPen> cityPenList;
    QList<QBrush> cityBrushList;
    QList<QPen> forestPenList;
    QList<QBrush> forestBrushList;
*/
    QList<QPen> highwayPenList;
    QList<QPen> roadPenList;
    QList<QPen> trailPenList;
    QList<QPen> railPenList;
    QList<QPen> rail_dPenList;
    QList<QPen> aerialcablePenList;
    QList<QPen> riverPenList;
    QList<QPen> river_tPenList;
    QList<QBrush> river_tBrushList;
    QList<QPen> canalPenList;
    QList<QPen> cityPenList;
    QList<QBrush> cityBrushList;
    QList<QPen> forestPenList;
    QList<QPen> glacierPenList;
    QList<QPen> packicePenList;
    QList<QBrush> forestBrushList;
    QList<QBrush> glacierBrushList;
    QList<QBrush> packiceBrushList;

    QList<QPen> faiAreaLow500PenList;
    QList<QBrush> faiAreaLow500BrushList;
    QList<QPen> faiAreaHigh500PenList;
    QList<QBrush> faiAreaHigh500BrushList;

    bool* airABorder;
    bool* airBBorder;
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
/*
    bool* roadBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* riverBorder;
    bool* cityBorder;
    bool* forestBorder;
*/
    bool* trailBorder;
    bool* roadBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* rail_dBorder;
    bool* aerialcableBorder;
    bool* riverBorder;
    bool* river_tBorder;
    bool* canalBorder;
    bool* cityBorder;

    bool* forestBorder;
    bool* glacierBorder;
    bool* packiceBorder;

    bool* faiAreaLow500Border;
    bool* faiAreaHigh500Border;

    int oldElement;
    int currentElement;
};

#endif
