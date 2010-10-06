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
class ConfigDrawElement : public QFrame
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
    void __defaultPen(QPtrList<QPen> *penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4);
    void __defaultPenBrush(QPtrList<QPen> *penList, bool *b, QPtrList<QBrush> *brushList,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
        QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4);
    void __fillStyle(QComboBox *pen, QComboBox *brush);
    void __readBorder(QString group, bool *b);
    void __readPen(QString group, QPtrList<QPen> *penList,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, Qt::PenStyle defaultPenStyle3, Qt::PenStyle defaultPenStyle4);
    void __readBrush(QString group, QPtrList<QBrush> *brushList,
        QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4);
    void __saveBrush(QPtrList<QBrush> *brushList);
    void __savePen(QPtrList<QPen> *penList, bool *b);
    void __showBrush(QPtrList<QBrush> *brushList);
    void __showPen(QPtrList<QPen> *a, bool *b);
    void __writeBrush(QString group, QPtrList<QBrush> *brushList, QPtrList<QPen> *penList, bool *b);
    void __writePen(QString group, QPtrList<QPen> *penList, bool *b);

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
    QPtrList<QPen> highwayPenList;
    QPtrList<QPen> roadPenList;
    QPtrList<QPen> trailPenList;
    QPtrList<QPen> railPenList;
    QPtrList<QPen> rail_dPenList;
    QPtrList<QPen> aerialcablePenList;
    QPtrList<QPen> riverPenList;
    QPtrList<QPen> river_tPenList;
    QPtrList<QBrush> river_tBrushList;
    QPtrList<QPen> canalPenList;
    QPtrList<QPen> cityPenList;
    QPtrList<QBrush> cityBrushList;
    QPtrList<QPen> ppl_1PenList;
    QPtrList<QPen> ppl_2PenList;
    QPtrList<QPen> ppl_3PenList;
    QPtrList<QPen> ppl_4PenList;
    QPtrList<QPen> ppl_5PenList;


    QPtrList<QPen> forestPenList;
    QPtrList<QPen> glacierPenList;
    QPtrList<QPen> packicePenList;
    QPtrList<QBrush> forestBrushList;
    QPtrList<QBrush> glacierBrushList;
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
