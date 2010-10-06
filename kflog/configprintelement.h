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

#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qpen.h>
#include <qpushbutton.h>
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
    void __defaultPen(QPtrList<QPen> *penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2);
    void __defaultPenBrush(QPtrList<QPen> *penList, bool *b, QPtrList<QBrush> *brushList,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, QColor defaultBrushColor1, QColor defaultBrushColor2,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2);
    void __fillStyle(QComboBox *pen, QComboBox *brush);
    void __readBorder(QString group, bool *b);
    void __readPen(QString group, QPtrList<QPen> *penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2);
    void __readPenBrush(QString group, QPtrList<QPen> *penList, bool *b, QPtrList<QBrush> *brushList,
        QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, QColor defaultBrushColor1, QColor defaultBrushColor2,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2);
    void __saveBrush(QPtrList<QBrush> *brushList);
    void __savePen(QPtrList<QPen> *penList, bool *b);
    void __showBrush(QPtrList<QBrush> *brushList);
    void __showPen(QPtrList<QPen> *a, bool *b);
    void __writeBrush(QString group, QPtrList<QBrush> *brushList, QPtrList<QPen> *penList, bool *b);
    void __writePen(QString group, QPtrList<QPen> *penList, bool *b);

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

    /** this is a temporary function and it is not needed in Qt 4 */
    QString __color2String(QColor);
    /** this is a temporary function and it is not needed in Qt 4 */
    QColor __string2Color(QString);
};

#endif
