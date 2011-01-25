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
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class ConfigDrawElement
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \short Configuration widget for map items.
 *
 * \date 2001-2011
 *
 * \version $Id$
 */

#ifndef CONFIG_DRAW_ELEMENT_H
#define CONFIG_DRAW_ELEMENT_H

#include <QCheckBox>
#include <QColor>
#include <qcombobox.h>
#include <qlcdnumber.h>
#include <QLineEdit>
#include <QList>
#include <QPen>
#include <qpushbutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <QWidget>

class ConfigDrawElement : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( ConfigDrawElement )

  public:

    ConfigDrawElement( QWidget* parent=0 );

    virtual ~ConfigDrawElement();

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

    void __defaultPen(QList<QPen> &penList, bool *b,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4);

    void __defaultPenBrush(QList<QPen> &penList, bool *b, QList<QBrush> &brushList,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
        QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4);

    void __fillStyle(QComboBox *pen, QComboBox *brush);

    void __readBorder(QString group, bool *array);

    void __readPen(QString group, QList<QPen> &penList,
        QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
        int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
        Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, Qt::PenStyle defaultPenStyle3, Qt::PenStyle defaultPenStyle4);

    void __readBrush(QString group, QList<QBrush> &brushList,
        QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
        Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4);

    void __saveBrush(QList<QBrush> &brushList);

    void __savePen(QList<QPen> &penList, bool *b);

    void __showBrush(QList<QBrush> &brushList);

    void __showPen(QList<QPen> &a, bool *b);

    void __writeBrush(QString group, QList<QBrush> &brushList, QList<QPen> &penList, bool *b);

    void __writePen(QString group, QList<QPen> &penList, bool *b);

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
    QList<QPen> ppl_1PenList;
    QList<QPen> ppl_2PenList;
    QList<QPen> ppl_3PenList;
    QList<QPen> ppl_4PenList;
    QList<QPen> ppl_5PenList;


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

    bool airABorder[4];
    bool airBBorder[4];
    bool airCBorder[4];
    bool airDBorder[4];
    bool airElBorder[4];
    bool airEhBorder[4];
    bool airFBorder[4];
    bool ctrCBorder[4];
    bool ctrDBorder[4];
    bool dangerBorder[4];
    bool lowFBorder[4];
    bool restrBorder[4];
    bool tmzBorder[4];
    bool trailBorder[4];
    bool roadBorder[4];
    bool highwayBorder[4];
    bool railBorder[4];
    bool rail_dBorder[4];
    bool aerialcableBorder[4];
    bool riverBorder[4];
    bool river_tBorder[4];
    bool canalBorder[4];
    bool cityBorder[4];
    bool ppl_1Border[4];
    bool ppl_2Border[4];
    bool ppl_3Border[4];
    bool ppl_4Border[4];
    bool ppl_5Border[4];

    bool forestBorder[4];
    bool glacierBorder[4];
    bool packiceBorder[4];

    bool faiAreaLow500Border[4];
    bool faiAreaHigh500Border[4];

    int oldElement;
    int currentElement;
};

#endif
