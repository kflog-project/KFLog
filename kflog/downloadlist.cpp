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
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include "map.h"
#include "mapcontents.h"

DownloadList::DownloadList(){
  srcList.setAutoDelete(true);
  destList.setAutoDelete(true);
  downloadRunning=false;
}

DownloadList::~DownloadList(){
}

void DownloadList::copyKURL(KURL* src, KURL* dest){
  QStringList::Iterator it = banList.find(src->fileName());
  qWarning(QString("it:%1").arg(*it));
  if ((*it)!=""){ // URL found in banList
//    qWarning("found.");
    return;
  }
  srcList.append(new KURL(src->url()));
  destList.append(new KURL (dest->url()));
  __schedule();
}

void DownloadList::slotDownloadFinished(KIO::Job* job){
  QStringList errorStrings;
  downloadRunning=false;
  int error;
  error=job->error();
  KConfig* config = KGlobal::config();
  config->setGroup("General Options");
  if (error){
    if (errorList.findIndex(error)==-1){
      job->showErrorDialog();
      errorList.append(error);
    }
    banList.prepend(actualURL);
  }
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
    actualURL=src->fileName();
    qWarning(QString("actualURL:%1").arg(actualURL));
    KIO::Job* job = new KIO::FileCopyJob(*src, *dest, 0644, false, false, false, true);
//    delete src;
//    delete dest;
    connect( job, SIGNAL(result(KIO::Job*)),
             this, SLOT(slotDownloadFinished(KIO::Job*)) );
  }
  else {
    emit allDownloadsFinished();
  }
}
