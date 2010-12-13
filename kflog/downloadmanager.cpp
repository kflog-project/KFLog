/***********************************************************************
**
**   downloadmanager.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: downloadmanager.cpp 3931 2010-04-08 13:38:28Z axel $
**
***********************************************************************/

/**
 * This class is a HTTP download manager. It processes download requests
 * in their incoming order one after another, not in parallel.
 */

#include <sys/statvfs.h>

#include <QtCore>
#include <QtNetwork>

#include "downloadmanager.h"

const ulong DownloadManager::MinFsSpace = 25*1024*1024; // 25MB

/**
 * Creates a download manager instance.
 */
DownloadManager::DownloadManager( QObject *parent ) :
  QObject(parent),
  client(0),
  downloadRunning(false),
  requests(0),
  errors(0)
{
  client = new HttpClient(this, false);

  connect( client, SIGNAL( finished(QString &, QNetworkReply::NetworkError) ),
           this, SLOT( slotFinished(QString &, QNetworkReply::NetworkError) ));
}

/**
 * Requests to download the passed url and to store the result under the
 * passed file destination. Destination must consist of a full path.
 */
bool DownloadManager::downloadRequest( QString &url, QString &destination )
{
  mutex.lock();

  // Check input parameters. If url was already requested the download
  // is rejected.
  if( url.isEmpty() || urlSet.contains(url) || destination.isEmpty() )
    {
      mutex.unlock();
      return false;
    }

  QString destDir = QFileInfo(destination).absolutePath();

  // Check free size of destination file system. If size is less than 25MB the
  // download is rejected.
  if( getFreeUserSpace( destDir ) < MinFsSpace )
    {
      qWarning( "DownloadManager(%d): Free space on %s less than %ldMB!",
                __LINE__, destDir.toLatin1().data(), MinFsSpace/(1024*1024) );

      mutex.unlock();
      return false;
    }

  if( downloadRunning == false )
    {
      // No download is running, do start the next one
      if( client->downloadFile( url, destination ) == false )
        {
          qWarning( "DownloadManager(%d): Download of '%s' failed!",
                     __LINE__, url.toLatin1().data() );

          // Start of download failed.
          mutex.unlock();
          return false;
        }

      QString destFile = QFileInfo(destination).fileName();
      emit status( tr("downloading ") + destFile );
      downloadRunning = true;
    }

  // Insert request in queue.
  urlSet.insert( url );
  QPair<QString, QString> pair( url, destination );
  queue.enqueue( pair );
  requests++;

  mutex.unlock();
  return true;
}

/**
 * Catches a finish signal with the downloaded url and the related result
 * from the HTTP client.
 */
void DownloadManager::slotFinished( QString &urlIn, QNetworkReply::NetworkError codeIn )
{
  mutex.lock();

  if( codeIn != QNetworkReply::NoError )
    {
      // Error already reported by the HTTP client
      errors++;
    }

  if( codeIn != QNetworkReply::NoError && codeIn != QNetworkReply::ContentNotFoundError )
    {
      // There was a fatal problem on the network. We do abort all further downloads
      // to avoid an error avalanche.
      qWarning( "DownloadManager(%d): Network problem occurred, canceling of all downloads!",
                __LINE__ );

      queue.clear();
      urlSet.clear();
      downloadRunning = false;
      emit networkError();
      mutex.unlock();
      return;
    }

  // Remove the last done request from the queue and from the url set.
  if( ! queue.isEmpty() )
    {
      QPair<QString, QString> pair = queue.dequeue();

      if( codeIn == QNetworkReply::NoError )
        {
          if( pair.second.contains( "welt2000.txt") )
            {
              // Special check for Welt2000 download to signal that. The signal
              // is the trigger for the reload of the Welt2000 data file.
              emit welt2000Downloaded();
            }
          else if( pair.second.contains( "/airspaces/") )
            {
              // Special check for an airspace download. The signal is the
              // trigger for the reload of the airspace data file.
              emit airspaceDownloaded();
            }
        }
    }

  // Remove last done request from the url set.
  urlSet.remove( urlIn );

  if( queue.isEmpty() )
    {
      // No more entries in queue. All downloads are finished.
      downloadRunning = false;
      emit status( tr("Downloads finished") );
      emit finished( requests, errors );
      mutex.unlock();
      return;
    }

  // Get next request from the queue
  QPair<QString, QString> pair = queue.head();

  QString url = pair.first;
  QString destination = pair.second;

  QString destDir = QFileInfo(destination).absolutePath();

  // Check free size of destination file system. If size is less than 25MB the
  // download is not executed.
  if( getFreeUserSpace( destDir ) < MinFsSpace )
    {
      qWarning( "DownloadManager(%d): Free space on %s less than %ldMB!",
                __LINE__, destDir.toLatin1().data(), MinFsSpace/(1024*1024) );

      mutex.unlock();

      // We simulate an operation cancel error, if the file system has
      // not enough space available.
      slotFinished( url, QNetworkReply::OperationCanceledError );
      return;
    }

  sleep(1); // make a short break

  // Start the next download.
  if( client->downloadFile( url, destination ) == false )
    {
    // Start of download failed.
      qWarning( "DownloadManager(%d): Download of '%s' failed!",
                 __LINE__, url.toLatin1().data() );

      mutex.unlock();

      // We simulate an operation cancel error, if download
      // could not be started.
      slotFinished( url, QNetworkReply::OperationCanceledError );
      return;
    }

  QString destFile = QFileInfo(destination).fileName();
  emit status( tr("downloading ") + destFile );

  mutex.unlock();
  return;
}

/**
 * Returns the free size of the file system in bytes for non root users.
 * The passed path must be exist otherwise the call will fail!
 */
ulong DownloadManager::getFreeUserSpace( QString& path )
{
  struct statvfs buf;
  int res;

  res = statvfs( path.toLatin1().data(), &buf );

  if( res )
    {
      qWarning( "DownloadManager(%d): Free space check failed for %s!",
                __LINE__, path.toLatin1().data() );

      perror("GetFreeUserSpace");
      return 0;
    }

#if 0
  qDebug() << "DM: FSBlockSize=" << buf.f_bsize
           << "FSSizeInBlocks=" << buf.f_blocks
           << "FreeAvail=" << buf.f_bfree
           << "FreeAvailNonRoot=" << buf.f_bavail*buf.f_bsize/(1024*1024) << "MB";
#endif

  // free size available to non-superuser in bytes
  return buf.f_bavail * buf.f_bsize;
}
