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


#include <qheader.h>

#include <kconfig.h>
#include <kglobal.h>

KFLogListView::KFLogListView(const char *persistendName, QWidget *parent, const char *name)
  : KListView(parent, name)
{
  confName = persistendName;
}

KFLogListView::KFLogListView(QWidget *parent, const char *name)
  : KListView(parent, name)
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
    KConfig* config = KGlobal::config();
    config->setGroup(confName);

    QValueList<int> l;
    QHeader *h = header();
    for (int i = 0; i < h->count(); i++) {
      l.append(i);
      l.append(h->mapToSection(i));
    }

    config->writeEntry("ColumnToSection", l);
  }
}

/** load the configuration from the app's configuration */
void KFLogListView::loadConfig()
{
  if (!confName.isEmpty()) {
    KConfig* config = KGlobal::config();
    config->setGroup(confName);

    QValueList<int> l = config->readIntListEntry("ColumnToSection");
    QValueList<int>::Iterator it;
    QHeader *h = header();
	int col, section;
	
    for (it = l.begin(); it != l.end(); ++it) {
      col = *it;
      ++it;
      section = *it;
      h->moveSection(section, col);
    }
  }
}
