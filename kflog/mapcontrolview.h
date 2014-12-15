/***********************************************************************
**
**   mapcontrolview.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class MapControlView
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \short Map control widget
 *
 * Creates widgets for controlling the map view. The class will create
 * the compass-rose, consisting of nine buttons to move the map,
 * two labels showing the dimension of the current map view and a slider
 * to set the scale.
 *
 * \date 2000-2014
 *
 * \version 1.1
 */

#ifndef MAP_CONTROL_VIEW_H
#define MAP_CONTROL_VIEW_H

#include <QWidget>
#include <QLabel>
#include <QLCDNumber>
#include <QSlider>
#include <QSize>

class MapControlView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MapControlView )

  public:
    /**
     * Creates all needed widgets.
     */
    MapControlView(QWidget *parent);

    /**
     * Destructor, does nothing special.
     */
    virtual ~MapControlView();

  public slots:
    /**
     * Displays the map-data (height and width of the map, scale).
     */
    void slotShowMapData(QSize size);

    /**
     * Sets the scale, called when the slider is moved.
     */
    void slotSetScale();

    /**
     * Shows the scale in the lcd-label and sets the slider.
     */
    void slotShowScaleChange(int value);

    /** */
    void slotSetMinMaxValue(int min, int max);

  signals:
    /** */
    void scaleChanged(double);

  private:

    /** Sets the what's that help text. */
    void setHelpText();

    /** */
    int __setScaleValue(int value);

    /** */
    int __getScaleValue(double value);

    QSize       m_mapSize;
    QSlider*    m_currentScaleSlider;
    QLCDNumber* m_currentScaleValue;
    QLabel*     m_dimLabel;
    QLabel*     m_dimText;
};

#endif
