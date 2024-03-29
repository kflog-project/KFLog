/***********************************************************************
**
**   downloadmanager.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c): 2010-2023 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class DownloadManager
 *
 * \author Axel Pauli
 *
 * \brief Manager for HTTP download handling
 *
 * This class handles the HTTP download requests in Cumulus. Downloads
 * of different map files can be requested.
 *
 * \date 2010-2023
 */

#pragma once

#include <QtCore>

#include "httpclient.h"

class DownloadManager : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( DownloadManager )

 public:

  /**
   * Types used by automatic download actions.
   */
  enum AutoDownloadType { ADT_NotSet = 0, Automatic, Inhibited };

  /**
   * Creates a download manager instance.
   */
  DownloadManager( QObject *parent = 0 );

  virtual ~DownloadManager();

  /**
   * Requests to download the passed url and to store the result under the
   * passed file destination. Destination must consist of a full path.
   */
  bool downloadRequest( QString &url, QString &destination );

  /**
   * Displays a message box and ask the user, weather files shall be downloaded
   * from the Internet.
   */
  int askUserForDownload();

  signals:

   /** Sends out a status message. */
   void status( const QString& msg );

   /** Sends a finish signal if all requested downloads are done. */
   void finished( int requests, int errors );

   /** Sends a network error signal, if such problem occurred. */
   void networkError();

   /**
    *  Sends a signal if an airspace file has been downloaded successfully.
    */
   void airspaceDownloaded();

 private:

  /**
   * Returns the free size of the file system in MB for non root users.
   */
  double getFreeUserSpace( QString& path );

 private slots:

  /** Catch a finish signal with the downloaded url and the related result. */
  void slotFinished( QString &url, QNetworkReply::NetworkError code );

 private:

  /** HTTP download client */
  HttpClient *client;

  /** Download working flag */
  bool downloadRunning;

  /** Set of urls to be downloaded, used for fast checks */
  QSet<QString> urlSet;

  /**
   * The download queue containing url and destination as string pair.
   */
  QQueue< QPair<QString, QString> > queue;

  /** Mutex to protect local data accesses. */
  QMutex mutex;

  /** Mutex to protect global data accesses. */
  static QMutex mutexGlobal;

  /** Counter for download request. */
  int requests;

  /** Counter for download errors. */
  int errors;

  /**
   * Required minimum space in bytes on file system destination to
   * execute the download request.
   */
  static const double MinFsSpace;

  /**
   * A black list, containing all not downloadable URLs to avoid a download
   * dead lock.
   */
  static QStringList blackList;

  /**
   * A log list, containing all downloaded URLs to avoid a download
   * dead lock. Per session only one successful download is possible.
   * There was observed the case, that a downloaded map file had not
   * the right format and were downloaded again and again.
   */
  static QSet<QString> logList;

  /**
   * Used to determine, if we shall display message box asking for download
   * of missing data from the Internet.
   *
   */
  static bool askUser;
};

