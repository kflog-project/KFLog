/***********************************************************************
**
**   downloadlist.cpp
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

#include "downloadlist.h"
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include "map.h"

DownloadList::DownloadList(){
  srcList.setAutoDelete(true);
  destList.setAutoDelete(true);
  downloadRunning=false;
}

DownloadList::~DownloadList(){
}

void DownloadList::copyKURL(KURL* src, KURL* dest){
  srcList.append(new KURL(src->url()));
  destList.append(new KURL (dest->url()));
  __schedule();
}

void DownloadList::slotDownloadFinished(KIO::Job* job){
  downloadRunning=false;
  if (job->error())
    job->showErrorDialog(0);
  emit downloadFinished();
  __schedule();
}

void DownloadList::__schedule(){
  if (downloadRunning)
    return;
  if (!srcList.isEmpty()){
    downloadRunning=true;
    KURL* src = srcList.take(0);
    KURL* dest = destList.take(0);
    KIO::CopyJob *job = KIO::copy(*src,*dest);
    delete src;
    delete dest;
    connect( job, SIGNAL(result(KIO::Job*)),
             this, SLOT(slotDownloadFinished(KIO::Job*)) );
  }
  else
    emit allDownloadsFinished();
}
