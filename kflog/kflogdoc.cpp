/***********************************************************************
**
**   kflogdoc.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

// include files for Qt
#include <qdir.h>
#include <qwidget.h>

// include files for KDE
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/netaccess.h>

// application specific includes
#include "kflogdoc.h"
#include "kflog.h"
#include "kflogview.h"

QList<KFLogView> *KFLogDoc::pViewList = 0L;

KFLogDoc::KFLogDoc(QWidget *parent, const char *name) : QObject(parent, name)
{
  if(!pViewList)
  {
    pViewList = new QList<KFLogView>();
  }

  pViewList->setAutoDelete(true);
}

KFLogDoc::~KFLogDoc()
{
}

void KFLogDoc::addView(KFLogView *view)
{
  pViewList->append(view);
}

void KFLogDoc::removeView(KFLogView *view)
{
  pViewList->remove(view);
}
void KFLogDoc::setURL(const KURL &url)
{
  doc_url=url;
}

const KURL& KFLogDoc::URL() const
{
  return doc_url;
}

void KFLogDoc::slotUpdateAllViews(KFLogView *sender)
{
  KFLogView *w;
  if(pViewList)
  {
    for(w=pViewList->first(); w!=0; w=pViewList->next())
    {
      if(w!=sender)
        w->repaint();
    }
  }

}

bool KFLogDoc::saveModified()
{
  bool completed=true;

  if(modified)
  {
    KFLogApp *win=(KFLogApp *) parent();
    int want_save = KMessageBox::warningYesNoCancel(win, i18n("Warning"),
                                         i18n("The current file has been modified.\n"
                                              "Do you want to save it?"));
    switch(want_save)
    {
      case 1:
           if (doc_url.fileName() == i18n("Untitled"))
           {
             win->slotFileSaveAs();
           }
           else
           {
             saveDocument(URL());
       	   };

       	   deleteContents();
           completed=true;
           break;

      case 2:
           setModified(false);
           deleteContents();
           completed=true;
           break;	

      case 3:
           completed=false;
           break;

      default:
           completed=false;
           break;
    }
  }

  return completed;
}

void KFLogDoc::closeDocument()
{
  deleteContents();
}

bool KFLogDoc::newDocument()
{
  /////////////////////////////////////////////////
  // TODO: Add your document initialization code here
  /////////////////////////////////////////////////
  modified=false;
  doc_url.setFileName(i18n("Untitled"));

  return true;
}

bool KFLogDoc::openDocument(const KURL& url, const char *format /*=0*/)
{
  QString tmpfile;
  KIO::NetAccess::download( url, tmpfile );
  /////////////////////////////////////////////////
  // TODO: Add your document opening code here
  /////////////////////////////////////////////////

  KIO::NetAccess::removeTempFile( tmpfile );

  modified=false;
  return true;
}

bool KFLogDoc::saveDocument(const KURL& url, const char *format /*=0*/)
{
  /////////////////////////////////////////////////
  // TODO: Add your document saving code here
  /////////////////////////////////////////////////

  modified=false;
  return true;
}

void KFLogDoc::deleteContents()
{
  /////////////////////////////////////////////////
  // TODO: Add implementation to delete the document contents
  /////////////////////////////////////////////////

}
