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

#include <qtextview.h>
#include <qtabwidget.h>

class Flight;

/**
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class DataView : public QTabWidget
{
  public:
    /**
     * Creates a new DataView object
     */
    DataView(QWidget* parent);
    /** */
    ~DataView();
    /** */
    void setFlightData(Flight*);

  private:
    /** */
    QTextView* flightDataText;
};

#endif
