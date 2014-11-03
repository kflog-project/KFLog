/***********************************************************************
**
**   rowdelegate.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2008 by Josua Dietze
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "rowdelegate.h"

/** Adds the vertical margin to the standard sizeHint */
QSize RowDelegate::sizeHint( const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
  QSize newSize = QItemDelegate::sizeHint( option, index );
  newSize.setHeight( newSize.height() + vMargin );
  return newSize;
}
