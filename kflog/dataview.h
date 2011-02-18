/***********************************************************************
**
**   dataview.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef DATA_VIEW_H
#define DATA_VIEW_H

#include <QWidget>
#include <QTextBrowser>
#include <QUrl>

class BaseFlightElement;
class Flight;
class FlightTask;

/**
 * \class DataView
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Provides a widget for displaying the flight data.
 *
 * Provides a widget for displaying the flight data.
 *
 * \date 2001-2011
 *
 * \version $Id$
 */
class DataView : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( DataView )

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
     * Writes the task and some header info about the flight into the widget.
     */
    void setFlightData();
    /**
     * Called when the user clicks on one of the waypoint links.
     */
    void slotWPSelected(const QUrl &link);
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
