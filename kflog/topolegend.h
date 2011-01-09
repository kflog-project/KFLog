/***********************************************************************
**
**   topolegend.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TOPO_LEGEND_H
#define TOPO_LEGEND_H

#include <qwidget.h>
#include <qlabel.h>
#include <q3ptrlist.h>
#include <q3scrollview.h>

/**
 * Generates a legend for the topography levels used on the map.
 * @author André Somers
 */

class TopoLegend : public Q3ScrollView
{
   Q_OBJECT

public:
 /**
   * Constructor. Normal QWidget parameters are used.
   */
  TopoLegend( QWidget *parent = 0 );

  /**
   * Destructor
   */
  ~TopoLegend();

protected:
 /**
  * Contains a list of the labels that have been created
  */
  Q3PtrList<QLabel> labelList;
 /**
  * Index of the currently highlighted elevation label
  */
  int currentHighlight;
  
public slots: // Public slots
  /**
   * Makes sure the indicated level is visible.
   */
  void highlightLevel(int level);

};

#endif
