/***********************************************************************
**
**   rowdelegate.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2008 by Josua Dietze
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: rowdelegate.h 4502 2010-12-09 22:32:02Z axel $
**
***********************************************************************/

/**
 * \class RowDelegate
 *
 * \author Josua Dietze
 *
 * \brief Changes look of list items.
 *
 * \date 2008
 *
 */

#ifndef ROWDELEGATE_H
#define ROWDELEGATE_H

#include <QItemDelegate>

class RowDelegate : public QItemDelegate
{
  Q_OBJECT

public:

  /**
   *  Overwritten to add verticalRowMargin above and below the row.
   */
  RowDelegate(QWidget *parent = 0, int verticalRowMargin = 0 ) : QItemDelegate(parent)
  {
    vMargin = verticalRowMargin;
  };

  /**
   *  Set a new vertical margin value
   */
  void setVerticalMargin(int newValue) { vMargin = newValue; };

  /**
   *  Overwrite QItemDelegate::sizeHint to make row height variable.
   */
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:

  /** This stores the given height margin */
  int vMargin;
};

#endif
