/***********************************************************************
**
**   configprintelement.h
**
**   This file is part of KFLog4.
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

#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <q3frame.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <q3ptrlist.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qspinbox.h>

/**
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class ConfigPrintElement : public Q3Frame
{
  Q_OBJECT

  public:
    ConfigPrintElement(QWidget* parent);
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

  private slots:
  void slotSelectBorder1Color();
  void slotSelectBorder2Color();
  void slotSelectBorder1BrushColor();
  void slotSelectBorder2BrushColor();

  private:
    void __defaultPen(Q3PtrList<QPen> *penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2);
    void __defaultPenBrush(Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, QColor defaultBrushColor1, QColor defaultBrushColor2,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2);
    void __fillStyle(QComboBox *pen, QComboBox *brush);
    void __readBorder(QString group, bool *b);
    void __readPen(QString group, Q3PtrList<QPen> *penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2);
    void __readPenBrush(QString group, Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, QColor defaultBrushColor1, QColor defaultBrushColor2,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2);
    void __saveBrush(Q3PtrList<QBrush> *brushList);
    void __savePen(Q3PtrList<QPen> *penList, bool *b);
    void __showBrush(Q3PtrList<QBrush> *brushList);
    void __showPen(Q3PtrList<QPen> *a, bool *b);
    void __writeBrush(QString group, Q3PtrList<QBrush> *brushList, Q3PtrList<QPen> *penList, bool *b);
    void __writePen(QString group, Q3PtrList<QPen> *penList, bool *b);

    QCheckBox* border1;
    QCheckBox* border2;

    QColor border1Color;
    QPushButton* border1ColorButton;
    QColor border2Color;
    QPushButton* border2ColorButton;

    QSpinBox* border1Pen;
    QSpinBox* border2Pen;

    QPushButton* border1Button;

    QComboBox* border1PenStyle;
    QComboBox* border2PenStyle;

    QColor border1BrushColor;
    QPushButton* border1BrushColorButton;
    QColor border2BrushColor;
    QPushButton* border2BrushColorButton;

    QComboBox* border1BrushStyle;
    QComboBox* border2BrushStyle;

    Q3PtrList<QPen> airAPenList;
    Q3PtrList<QBrush> airABrushList;
    Q3PtrList<QPen> airBPenList;
    Q3PtrList<QBrush> airBBrushList;
    Q3PtrList<QPen> airCPenList;
    Q3PtrList<QBrush> airCBrushList;
    Q3PtrList<QPen> airDPenList;
    Q3PtrList<QBrush> airDBrushList;
    Q3PtrList<QPen> airElPenList;
    Q3PtrList<QBrush> airElBrushList;
    Q3PtrList<QPen> airEhPenList;
    Q3PtrList<QBrush> airEhBrushList;
    Q3PtrList<QPen> airFPenList;
    Q3PtrList<QBrush> airFBrushList;
    Q3PtrList<QPen> ctrCPenList;
    Q3PtrList<QBrush> ctrCBrushList;
    Q3PtrList<QPen> ctrDPenList;
    Q3PtrList<QBrush> ctrDBrushList;
    Q3PtrList<QPen> lowFPenList;
    Q3PtrList<QBrush> lowFBrushList;
    Q3PtrList<QPen> dangerPenList;
    Q3PtrList<QBrush> dangerBrushList;
    Q3PtrList<QPen> restrPenList;
    Q3PtrList<QBrush> restrBrushList;
    Q3PtrList<QPen> tmzPenList;
    Q3PtrList<QBrush> tmzBrushList;

    Q3PtrList<QPen> roadPenList;
    Q3PtrList<QPen> trailPenList;
    Q3PtrList<QPen> railPenList;
    Q3PtrList<QPen> rail_dPenList;
    Q3PtrList<QPen> riverPenList;
    Q3PtrList<QPen> river_tPenList;
    Q3PtrList<QBrush> river_tBrushList;
    Q3PtrList<QPen> highwayPenList;
    Q3PtrList<QPen> canalPenList;
    Q3PtrList<QPen> cityPenList;
    Q3PtrList<QPen> aerialcablePenList;
    Q3PtrList<QBrush> cityBrushList;
    Q3PtrList<QPen> forestPenList;
    Q3PtrList<QBrush> forestBrushList;
    Q3PtrList<QPen> glacierPenList;
    Q3PtrList<QBrush> glacierBrushList;
    Q3PtrList<QPen> packicePenList;
    Q3PtrList<QBrush> packiceBrushList;

    Q3PtrList<QPen> faiAreaLow500PenList;
    Q3PtrList<QBrush> faiAreaLow500BrushList;
    Q3PtrList<QPen> faiAreaHigh500PenList;
    Q3PtrList<QBrush> faiAreaHigh500BrushList;

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

    /** this is a temporary function and it is not needed in Qt 4 */
    QString __color2String(QColor);
    /** this is a temporary function and it is not needed in Qt 4 */
    QColor __string2Color(QString);
};

#endif
