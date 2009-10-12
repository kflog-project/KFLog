/***********************************************************************
**
**   kfloglistview.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/


#ifndef KFLOGLISTVIEW_H
#define KFLOGLISTVIEW_H

#include <qlistview.h>
#include <qstring.h>
#include <qwidget.h>

/**a KListView that store and load its configuration
  *@author Harald Maier
  */

class KFLogListView : public QListView  {
   Q_OBJECT
public:
	KFLogListView(const char *persistendName, QWidget *parent=0, const char *name=0);
	KFLogListView(QWidget *parent=0, const char *name=0);
	~KFLogListView();

  //source: http://api.kde.org/3.5-api/kdelibs-apidocs/kdeui/html/klistview_8cpp_source.html#l01858
  int itemIndex( const QListViewItem * ) const;
  //source: http://api.kde.org/3.5-api/kdelibs-apidocs/kdeui/html/klistview_8cpp_source.html#l01877
  QListViewItem* itemAtIndex(int );
private: // Private attributes
  /**  */
  QString confName;
public slots: // Public slots
  /** load the configuration from the app's configuration */
  void loadConfig();
private slots: // Private slots
  /** store the configuration in the app's configuration */
  void storeConfig();
};

#endif
