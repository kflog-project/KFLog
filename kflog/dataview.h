/***********************************************************************
**
**   dataview.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef DATAVIEW_H
#define DATAVIEW_H

#include <QFrame>
#include <QTextBrowser>

#include "flighttask.h"
class Flight;

/**
 * Provides widgets for displaying the flight-data.
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class DataView : public QFrame
{
  Q_OBJECT

  public:
    /**
     * Creates a new DataView object
     */
    DataView(QWidget* parent);
    /**
     * Destructor
     */
    ~DataView();

  signals:
    /**
     * Emitted when the user selects one waypoint in the list.
     */
    void wpSelected(const int id);
    /** No descriptions */
    void flightSelected(BaseFlightElement *);
    /** No descriptions */
    void editFlightGroup();

  public slots:
    /**
     * Writes the task and some headerinfo about the flight into the widget.
     */
    void setFlightData();
    /**
     * Called when the user clickes on one of the waypoint-links.
     */
    void slotWPSelected(const QString &url);
    /**
      * Sets the contents to the currently active task
    	*/
    void slotShowTaskText(FlightTask* task);
    /**
      * Clears the contents of the dataview
      */
    void slotClearView();

  private:
    /** */
    QString __writeTaskInfo(FlightTask*);
    /**
     * The textwidget
     */
    QTextBrowser* flightDataText;
};

#endif
