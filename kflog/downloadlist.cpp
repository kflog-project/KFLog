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
#include <qmessagebox.h>
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
  srcList.append(new KURL(src->url()));
  destList.append(new KURL (dest->url()));
  __schedule();
}

void DownloadList::slotDownloadFinished(KIO::Job* job){
  QStringList errorStrings;
  downloadRunning=false;
  int error;
  error=job->error();
  if (error){
//    job->showErrorDialog(0);
    int ret=QMessageBox::warning(0,i18n("Error downloading files"),job->errorString()+"\n"+
      i18n("Do you want to continue to download the map files?\n")+
      i18n("Warning: If you press \"Continue\" you may end in an endless loop"),
      "&Continue", "&Stop",
        0,      // Enter == button 0
        1 );    // Escape == button 1
    if (ret==1){
      KConfig* config = KGlobal::config();
      config->setGroup("General Options");
      config->writeEntry("Automatic Map Download",Inhibited,false);
      srcList.clear();
      destList.clear();
    }
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
