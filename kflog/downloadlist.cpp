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
#include "map.h"
#include "mapcontents.h"

DownloadList::DownloadList(){
  srcList.setAutoDelete(true);
  destList.setAutoDelete(true);
  downloadRunning=false;
}

DownloadList::~DownloadList(){
}

// Temporarily disabled during transition to Qt4. Use QNetworkAccessManager in Qt4.
//void DownloadList::copyKURL(QUrl* src, QUrl* dest){
//  QStringList::Iterator it = banList.find(src->fileName());
//  qWarning(QString("it:%1").arg(*it));
//  if ((*it)!=""){ // URL found in banList
////    qWarning("found.");
//    return;
//  }
//  srcList.append(new QUrl(src->toString()));
//  destList.append(new QUrl(dest->toString()));
//  __schedule();
//}

// Temporarily disabled during transition to Qt4. Use QNetworkAccessManager in Qt4.
//void DownloadList::slotDownloadFinished(KIO::Job* job){
//  QStringList errorStrings;
//  downloadRunning=false;
//  int error;
//  error=job->error();
//  if (error){
//    if (errorList.findIndex(error)==-1){
//      job->showErrorDialog();
//      errorList.append(error);
//    }
//    banList.prepend(actualURL);
//  }
//  emit downloadFinished();
//  __schedule();
//}

void DownloadList::__schedule(){
  if (downloadRunning)
    return;
  if (!srcList.isEmpty()){
    downloadRunning=true;
    Q3Url* src = srcList.take(0);
//    Q3Url* dest = destList.take(0);
    actualURL=src->fileName();
    qWarning("actualURL:%s", (const char*)actualURL.toLatin1());
// Temporarily disabled during transition to Qt4. Use QNetworkAccessManager in Qt4.
//    KIO::Job* job = new KIO::FileCopyJob(*src, *dest, 0644, false, false, false, true);
////    delete src;
////    delete dest;
//    connect( job, SIGNAL(result(KIO::Job*)),
//             this, SLOT(slotDownloadFinished(KIO::Job*)) );
  }
  else {
    emit allDownloadsFinished();
  }
}
