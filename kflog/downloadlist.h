/***********************************************************************
**
**   downloadlist.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef DOWNLOADLIST_H
#define DOWNLOADLIST_H


/**
  *@author Christof Bodner
  * Implements a FIFO for the download of files
  */

#include <qobject.h>
#include <qlist.h>
#include <kurl.h>
#include <kio/jobclasses.h>

class DownloadList : public QObject{
  Q_OBJECT
  
  public:
    DownloadList();
    ~DownloadList();
    void copyKURL(KURL* src, KURL* dest);
  public slots:
    void slotDownloadFinished(KIO::Job* job);
  signals:
    void downloadFinished();
    void allDownloadsFinished();
  private:
    void __schedule();
    QPtrList<KURL> srcList;
    QPtrList<KURL> destList;
    bool downloadRunning;
};

#endif
