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

#include <ktextbrowser.h>
#include <qframe.h>

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
    /**
     * Writes the task and some headerinfo about the flight into the widget.
     */
    void setFlightData(Flight*);

  signals:
    /**
     * Emitted when the user selects one waypoint in the list.
     */
    void wpSelected(const unsigned int id);

  public slots:
    /**
     * Called when the user clickes on one of the waypoint-links.
     */
    void slotWPSelected(const QString &url);

  private:
    /**
     * The textwidget
     */
    KTextBrowser* flightDataText;
};

#endif
