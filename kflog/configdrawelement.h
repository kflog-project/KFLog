/***********************************************************************
**
**   configdrawelement.h
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

#ifndef CONFIGDRAWELEMENT_H
#define CONFIGDRAWELEMENT_H

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
class ConfigDrawElement : public Q3Frame
{
  Q_OBJECT

  public:
    ConfigDrawElement(QWidget* parent);
    ~ConfigDrawElement();
    /** */
//    enum ElementType {Road = 0, Highway, Railway, River, Canal, City,
//        PPl_1, PPl_2, PPl_3, PPl_4, PPl_5,
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

  private slots:
  void slotSelectBorder1Color();
  void slotSelectBorder2Color();
  void slotSelectBorder3Color();
  void slotSelectBorder4Color();
  void slotSelectBorder1BrushColor();
  void slotSelectBorder2BrushColor();
  void slotSelectBorder3BrushColor();
  void slotSelectBorder4BrushColor();

  private:
    void __defaultPen(Q3PtrList<QPen> *penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4);
    void __defaultPenBrush(Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
        QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4);
    void __fillStyle(QComboBox *pen, QComboBox *brush);
    void __readBorder(QString group, bool *b);
    void __readPen(QString group, Q3PtrList<QPen> *penList,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, Qt::PenStyle defaultPenStyle3, Qt::PenStyle defaultPenStyle4);
    void __readBrush(QString group, Q3PtrList<QBrush> *brushList,
        QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4);
    void __saveBrush(Q3PtrList<QBrush> *brushList);
    void __savePen(Q3PtrList<QPen> *penList, bool *b);
    void __showBrush(Q3PtrList<QBrush> *brushList);
    void __showPen(Q3PtrList<QPen> *a, bool *b);
    void __writeBrush(QString group, Q3PtrList<QBrush> *brushList, Q3PtrList<QPen> *penList, bool *b);
    void __writePen(QString group, Q3PtrList<QPen> *penList, bool *b);

    QCheckBox* border1;
    QCheckBox* border2;
    QCheckBox* border3;
    QCheckBox* border4;

    QColor border1Color;
    QPushButton* border1ColorButton;
    QColor border2Color;
    QPushButton* border2ColorButton;
    QColor border3Color;
    QPushButton* border3ColorButton;
    QColor border4Color;
    QPushButton* border4ColorButton;

    QSpinBox* border1Pen;
    QSpinBox* border2Pen;
    QSpinBox* border3Pen;
    QSpinBox* border4Pen;

    QPushButton* border1Button;
    QPushButton* border2Button;
    QPushButton* border3Button;

    QComboBox* border1PenStyle;
    QComboBox* border2PenStyle;
    QComboBox* border3PenStyle;
    QComboBox* border4PenStyle;

    QColor border1BrushColor;
    QPushButton* border1BrushColorButton;
    QColor border2BrushColor;
    QPushButton* border2BrushColorButton;
    QColor border3BrushColor;
    QPushButton* border3BrushColorButton;
    QColor border4BrushColor;
    QPushButton* border4BrushColorButton;

    QComboBox* border1BrushStyle;
    QComboBox* border2BrushStyle;
    QComboBox* border3BrushStyle;
    QComboBox* border4BrushStyle;

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
/*
    QPtrList<QPen> roadPenList;
    QPtrList<QPen> railPenList;
    QPtrList<QPen> riverPenList;
    QPtrList<QPen> highwayPenList;
    QPtrList<QPen> cityPenList;
    QPtrList<QBrush> cityBrushList;
    QPtrList<QPen> forestPenList;
    QPtrList<QBrush> forestBrushList;
*/
    Q3PtrList<QPen> highwayPenList;
    Q3PtrList<QPen> roadPenList;
    Q3PtrList<QPen> trailPenList;
    Q3PtrList<QPen> railPenList;
    Q3PtrList<QPen> rail_dPenList;
    Q3PtrList<QPen> aerialcablePenList;
    Q3PtrList<QPen> riverPenList;
    Q3PtrList<QPen> river_tPenList;
    Q3PtrList<QBrush> river_tBrushList;
    Q3PtrList<QPen> canalPenList;
    Q3PtrList<QPen> cityPenList;
    Q3PtrList<QBrush> cityBrushList;
    Q3PtrList<QPen> ppl_1PenList;
    Q3PtrList<QPen> ppl_2PenList;
    Q3PtrList<QPen> ppl_3PenList;
    Q3PtrList<QPen> ppl_4PenList;
    Q3PtrList<QPen> ppl_5PenList;


    Q3PtrList<QPen> forestPenList;
    Q3PtrList<QPen> glacierPenList;
    Q3PtrList<QPen> packicePenList;
    Q3PtrList<QBrush> forestBrushList;
    Q3PtrList<QBrush> glacierBrushList;
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
    bool* ppl_1Border;
    bool* ppl_2Border;
    bool* ppl_3Border;
    bool* ppl_4Border;
    bool* ppl_5Border;

    bool* forestBorder;
    bool* glacierBorder;
    bool* packiceBorder;

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
