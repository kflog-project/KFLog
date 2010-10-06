/***********************************************************************
**
**   mapcontrolview.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPCONTROLVIEW_H
#define MAPCONTROLVIEW_H

#include <qlabel.h>

#include <qlcdnumber.h>
#include <qslider.h>
#include <qframe.h>

class KFLogApp;

/**
 * Creates widgets for controling the mapview. The class will create
 * the compass-rose, consisting of nine KButtons to move the map,
 * two labels showing the dimension of the current mapview and a slider
 * to set the scale.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapControlView : public QFrame
{
  Q_OBJECT

  public:
    /**
     * Creates all needed widgets.
     */
    MapControlView(QWidget *parent);
    /**
     * Destructor, does nothing special.
     */
    ~MapControlView();

  public slots:
    /**
     * Displays the map-data (height and width of the map, scale).
     */
    void slotShowMapData(QSize);
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
    /** */
    int __setScaleValue(int value);
    /** */
    int __getScaleValue(double value);

    QSlider* currentScaleSlider;
    QLCDNumber* currentScaleValue;
    QLabel* dimText;
};

#endif
