/***********************************************************************
**
**   kflogtable.cpp
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

#include "kflogtable.h"

KFLogTable::KFLogTable(QWidget *parent, const char *name)
  : QTable(parent, name), oldCol(-1)
{
}

KFLogTable::KFLogTable(int numRows, int numCols, QWidget *parent, const char *name)
  : QTable(numRows, numCols, parent, name)
{
}

KFLogTable::~KFLogTable(){
}

/** insert new row into table
append row and shifting existing rows down*/
void KFLogTable::insertRow(int row)
{
  int rows = numRows();
  if (row >= 0 && row <= rows) {
    setNumRows(rows + 1);
    for (int i = rows - 1; i >= row; i--) {
      swapRows(i + 1, i);
    }
    setCurrentCell(row, currentColumn());
  }
}

/** remove row from table
shifting rows up and delete last row */
void KFLogTable::deleteRow(int row)
{
  int rows = numRows();
  if (row >= 0 && row < rows) {
    for (int i = row; i < rows - 1; i++) {
      swapRows(i, i + 1);
    }
    setNumRows(rows - 1);
    if (numRows() > 0) {
      setCurrentCell(row < numRows() ? row : row - 1, currentColumn());
    }
  }
}
/** No descriptions */
void KFLogTable::columnClicked(int col)
{
  if (col != oldCol) {
    oldCol = col;
    ascending = true;
  }
  else {
    ascending = !ascending;
  }

  sort();
}

void KFLogTable::sort()
{
  if (oldCol != -1) {
    sortColumn(oldCol, ascending, true);
    horizontalHeader()->setSortIndicator(oldCol, ascending);
  }
}
