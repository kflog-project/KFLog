/***********************************************************************
**
**   kflogtreewidget.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class KFLogTreeWidget
 *
 * \brief Small extension of the QTreeWidget class.
 *
 * This class is an extension of the QTreeWidget class. It saves and restores
 * a modified headline by the user.
 *
 * \date 2011
 *
 * \version $Id$
 *
 */

#ifndef KFLOG_TREE_WIDGET_H
#define KFLOG_TREE_WIDGET_H

#include <QTreeWidget>

class QString;
class RowDelegate;

class KFLogTreeWidget : public QTreeWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( KFLogTreeWidget )

public:

  /**
   * \param name Name is used for configuration storing and retrieving.
   *             Must be unique in the application scope otherwise configuration
   *             data is overwritten and not more usable.
   *
   * \param parent Pointer to the parent widget.
   */
  KFLogTreeWidget( const char *name, QWidget *parent=0 );

	virtual ~KFLogTreeWidget();

  /** load the configuration from the app's configuration */
  void loadConfig();

  /**
   * Adds additional space per row.
   *
   * \param pixels Number of space pixels to be added per row.
   */
  void addRowSpacing( const int pixels );

  /**
   * Resizes all columns in the tree to their content.
   */
  void resizeColumns2Content();

protected:

  /**
   * Handles right mouse clicks.
   */
  void mousePressEvent( QMouseEvent* event );

signals:

  /**
   * Emitted, when the right mouse button was pressed.
   *
   * \param item The item laying under the mouse pointer or NULL, if
   *             no item is touched.
   *
   * \param position The current mouse position.
   */
  void rightButtonPressed( QTreeWidgetItem *item, const QPoint &position );

private:

  /** store the configuration in the app's configuration */
  void saveConfig();

private:

  /** Name to be used for configuration storing and retrieving. */
  QString confName;

  RowDelegate* rowDelegate;
};

#endif
