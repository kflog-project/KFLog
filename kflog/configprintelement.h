/***********************************************************************
**
**   configprintelement.h
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

#ifndef CONFIGPRINTELEMENT_H
#define CONFIGPRINTELEMENT_H

#include <kconfig.h>
#include <kcolorbutton.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qpen.h>
#include <qslider.h>
#include <qspinbox.h>

/**
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class ConfigPrintElement : public QFrame
{
  Q_OBJECT

  public:
    ConfigPrintElement(QWidget* parent, KConfig* cnf);
    ~ConfigPrintElement();
    /** */
//    enum ElementType {Road = 0, Highway, Railway, River, Canal, City,
//        AirA, AirB, AirC, AirD, AirElow, AirEhigh, AirF, ControlC, ControlD, Danger,
//        LowFlight, Restricted, TMZ, Forest, Trail, Railway_D, Aerial_Cable, River_T,
//        Glacier, PackIce, FAIAreaLow500, FAIAreaHigh500};

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
    void slotSetSecond();
    /** */
    void slotOk();

  private:
    KConfig* config;
    QCheckBox* border1;
    QCheckBox* border2;

    KColorButton* border1Color;
    KColorButton* border2Color;

    QSpinBox* border1Pen;
    QSpinBox* border2Pen;

    QPushButton* border1Button;

    QComboBox* border1PenStyle;
    QComboBox* border2PenStyle;

    KColorButton* border1BrushColor;
    KColorButton* border2BrushColor;

    QComboBox* border1BrushStyle;
    QComboBox* border2BrushStyle;

    QPtrList<QPen> airAPenList;
    QPtrList<QBrush> airABrushList;
    QPtrList<QPen> airBPenList;
    QPtrList<QBrush> airBBrushList;
    QPtrList<QPen> airCPenList;
    QPtrList<QBrush> airCBrushList;
    QPtrList<QPen> airDPenList;
    QPtrList<QBrush> airDBrushList;
    QPtrList<QPen> airElPenList;
    QPtrList<QBrush> airElBrushList;
    QPtrList<QPen> airEhPenList;
    QPtrList<QBrush> airEhBrushList;
    QPtrList<QPen> airFPenList;
    QPtrList<QBrush> airFBrushList;
    QPtrList<QPen> ctrCPenList;
    QPtrList<QBrush> ctrCBrushList;
    QPtrList<QPen> ctrDPenList;
    QPtrList<QBrush> ctrDBrushList;
    QPtrList<QPen> lowFPenList;
    QPtrList<QBrush> lowFBrushList;
    QPtrList<QPen> dangerPenList;
    QPtrList<QBrush> dangerBrushList;
    QPtrList<QPen> restrPenList;
    QPtrList<QBrush> restrBrushList;
    QPtrList<QPen> tmzPenList;
    QPtrList<QBrush> tmzBrushList;

    QPtrList<QPen> roadPenList;
    QPtrList<QPen> trailPenList;
    QPtrList<QPen> railPenList;
    QPtrList<QPen> rail_dPenList;
    QPtrList<QPen> riverPenList;
    QPtrList<QPen> river_tPenList;
    QPtrList<QBrush> river_tBrushList;
    QPtrList<QPen> highwayPenList;
    QPtrList<QPen> canalPenList;
    QPtrList<QPen> cityPenList;
    QPtrList<QPen> aerialcablePenList;
    QPtrList<QBrush> cityBrushList;
    QPtrList<QPen> forestPenList;
    QPtrList<QBrush> forestBrushList;
    QPtrList<QPen> glacierPenList;
    QPtrList<QBrush> glacierBrushList;
    QPtrList<QPen> packicePenList;
    QPtrList<QBrush> packiceBrushList;

    QPtrList<QPen> faiAreaLow500PenList;
    QPtrList<QBrush> faiAreaLow500BrushList;
    QPtrList<QPen> faiAreaHigh500PenList;
    QPtrList<QBrush> faiAreaHigh500BrushList;

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

    bool* roadBorder;
    bool* trailBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* rail_dBorder;
    bool* riverBorder;
    bool* river_tBorder;
    bool* cityBorder;
    bool* forestBorder;
    bool* glacierBorder;
    bool* packiceBorder;
    bool* canalBorder;
    bool* aerialcableBorder;

    bool* faiAreaLow500Border;
    bool* faiAreaHigh500Border;

    int oldElement;
    int currentElement;
};

#endif
