/***********************************************************************
**
**   kflogtable.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef KFLOGTABLE_H
#define KFLOGTABLE_H

#include <qwidget.h>
#include <qtable.h>

/**A QTable with some usefull new functions
  *@author Harald Maier
  */

class KFLogTable : public QTable  {
   Q_OBJECT
public: 
	KFLogTable(QWidget *parent=0, const char *name=0);
  KFLogTable(int numRows, int numCols, QWidget *parent = 0, const char *name = 0);
	~KFLogTable();
  void deleteRow(int row);
  void insertRow(int row);
  void sort();
protected slots: // Protected slots
  /** No descriptions */
  void columnClicked(int col);
private:
  int oldCol;
  bool ascending;
};

#endif
