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
  * @short Implements a FIFO for the download of files
  *
  * If it unavailable maps are detected on the local system, they
  * are passed to this class, wich serializes the downloading of these
  * missing maps.
  *
  * @author Christof Bodner
  * @version $Id$
  */

#include <qobject.h>
#include <qptrlist.h>
#include <kurl.h>
#include <kio/jobclasses.h>

class DownloadList : public QObject{
  Q_OBJECT
  
  public:
   /**
    * Constructor
    */
    DownloadList();
   /**
    * Destructor
    */
    ~DownloadList();
   /**
    * Adds a copy-task to the list of downloads
    */
    void copyKURL(KURL* src, KURL* dest);
  public slots: //should probably be protected?
   /**
    * Called if a download has finished. Resets @ref downloadRunning to false
    * and calls @ref __schedule to see if a new download should be started.
    */
    void slotDownloadFinished(KIO::Job* job);
  signals:
   /**
    * Emitted if a download is finished
    */
    void downloadFinished();
   /**
    * Emitted if all downloads are finished
    */
    void allDownloadsFinished();
  private:
   /**
    * Tries to start a new download, but only if there currently is no download
    * in progress.
    */
    void __schedule();
   /**
    * List of source URL's
    * @see destList
    */
    QPtrList<KURL> srcList;
   /**
    * List of target URL's
    * @see srcList
    */
    QPtrList<KURL> destList;
   /**
    * List of URL's which cannot be retrieved
    */
    QStringList banList;
   /**
    * actual processed URL
    */
    QString actualURL;
   /**
    * Is a download running or not?
    */
    bool downloadRunning;
   /**
    * List used to prevent multiple display of same error,
    * which annoys the user (what we obviously do not want)
    */
    QValueList<int> errorList;
};

#endif
