/***********************************************************************
**
**   topolegend.h
**
**   This file is part of KFLog2.
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

#ifndef TOPOLEGEND_H
#define TOPOLEGEND_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlist.h>
#include <qscrollview.h>

/**Generates a legend for the topography levels used on the map.
  *@author André Somers
  */

class TopoLegend : public QScrollView  {
   Q_OBJECT
public: 
	TopoLegend(QWidget *parent=0, const char *name=0);
	~TopoLegend();

protected:
  QList<QLabel> labelList;
  int currentHighlight;
  
public slots: // Public slots
  /** Makes sure the indicated level is visible. */
  void highlightLevel(int level);

};

#endif
