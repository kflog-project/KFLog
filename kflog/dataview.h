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
 *
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
    /** */
    ~DataView();
    /** */
    void setFlightData(Flight*);

  signals:
    /** Emitted whe the user selects one waypoint in the list. */
    void wpSelected(const int id);

  public slots:
    /** */
    void slotWPSelected(const QString &url);

  private:
    /** */
    KTextBrowser* flightDataText;
};

#endif
