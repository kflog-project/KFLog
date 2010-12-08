/***********************************************************************
**
**   kfloglistview.cpp
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

#include "kfloglistview.h"
#include <q3header.h>
#include <qsettings.h>
#include <qstringlist.h>

KFLogListView::KFLogListView(const char *persistendName, QWidget *parent, const char *name)
  : Q3ListView(parent, name)
{
  confName = persistendName;
}

KFLogListView::KFLogListView(QWidget *parent, const char *name)
  : Q3ListView(parent, name)
{
  confName = QString::null;
}

KFLogListView::~KFLogListView()
{
  if (!confName.isEmpty()) {
    storeConfig();
  }
}

/** store the configuration in the app's configuration */
void KFLogListView::storeConfig()
{
  if (!confName.isEmpty()) {
    extern QSettings _settings;

    QStringList l;
    QString convertInt;
    Q3Header *h = header();
    for (int i = 0; i < h->count(); i++) {
      l.append(convertInt.setNum(i));
      l.append(convertInt.setNum(h->mapToSection(i)));
    }

    _settings.writeEntry("/KFLog/"+confName+"/ColumnToSection", l);
  }
}

/** load the configuration from the app's configuration */
void KFLogListView::loadConfig()
{
  if (!confName.isEmpty()) {
    extern QSettings _settings;

    QStringList l = _settings.readListEntry("/KFLog/"+confName+"/ColumnToSection");
    QStringList::Iterator it;
    Q3Header *h = header();
    int col, section;

    for (it = l.begin(); it != l.end(); ++it) {
      col = (*it).toInt();
      ++it;
      section = (*it).toInt();
      h->moveSection(section, col);
    }
  }
}

//source: http://api.kde.org/3.5-api/kdelibs-apidocs/kdeui/html/klistview_8cpp_source.html#l01858
int KFLogListView::itemIndex( const Q3ListViewItem *item ) const
{
    if ( !item )
        return -1;

    if ( item == firstChild() )
        return 0;
    else {
        Q3ListViewItemIterator it(firstChild());
        uint j = 0;
        for ( ; it.current() && it.current() != item; ++it, ++j ) ;

        if( !it.current() )
          return -1;

        return j;
    }
}

//source: http://api.kde.org/3.5-api/kdelibs-apidocs/kdeui/html/klistview_8cpp_source.html#l01877
Q3ListViewItem* KFLogListView::itemAtIndex(int index)
{
   if (index<0)
      return 0;

   int j(0);
   for (Q3ListViewItemIterator it=firstChild(); it.current(); ++it)
   {
      if (j==index)
         return it.current();
      ++j;
   };
   return 0;
}
