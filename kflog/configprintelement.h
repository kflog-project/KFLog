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
class ConfigPrintElement : public QFrame
{
  Q_OBJECT

  public:
    ConfigPrintElement(QWidget* parent, KConfig* cnf);
    ~ConfigPrintElement();
    /** */
    enum ElementType {Road = 0, Highway, Railway, River, Canal, City,
        AirA, AirB, AirC, AirD, AirElow, AirEhigh, AirF, ControlC, ControlD, Danger,
        LowFlight, Restricted, TMZ};

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

    KComboBox* border1PenStyle;
    KComboBox* border2PenStyle;

    KColorButton* border1BrushColor;
    KColorButton* border2BrushColor;

    KComboBox* border1BrushStyle;
    KComboBox* border2BrushStyle;

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

    QList<QPen> roadPenList;
    QList<QPen> railPenList;
    QList<QPen> riverPenList;
    QList<QPen> highwayPenList;
    QList<QPen> cityPenList;
    QList<QBrush> cityBrushList;

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
    bool* highwayBorder;
    bool* railBorder;
    bool* riverBorder;
    bool* cityBorder;

    int oldElement;
    int currentElement;
};

#endif
