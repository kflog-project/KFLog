/***********************************************************************
**
**   httpclient.cpp
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
**   $Id$
**
***********************************************************************/

/**
 * This class is a simple HTTP download client.
 */
#include <QtGui>
#include <QtNetwork>

#include "httpclient.h"
#include "authdialog.h"

HttpClient::HttpClient( QObject *parent, const bool showProgressDialog ) :
  QObject(parent),
  _parent(parent),
  _progressDialog(0),
  manager(0),
  reply(0),
  tmpFile(0),
  _url(""),
  _destination(""),
  downloadRunning(false),
  timer(0)
 {
   if( showProgressDialog )
     {
       _progressDialog = new QProgressDialog;
       connect( _progressDialog, SIGNAL(canceled()), this, SLOT(slotCancelDownload()) );
     }

   manager = new QNetworkAccessManager(this);
   manager->setCookieJar ( new QNetworkCookieJar(this) );

   connect( manager, SIGNAL(authenticationRequired( QNetworkReply *, QAuthenticator * )),
            this, SLOT(slotAuthenticationRequired( QNetworkReply *, QAuthenticator * )) );

#ifndef QT_NO_OPENSSL
   connect( manager, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> & )),
            this, SLOT(slotSslErrors( QNetworkReply *, const QList<QSslError> & )) );
#endif

   // timer to supervise connection.
   timer = new QTimer( this );
   timer->setInterval( 120000 ); // Timeout is 120s

   connect( timer, SIGNAL(timeout()), this, SLOT(slotCancelDownload()) );
}

HttpClient::~HttpClient()
{
  if( _progressDialog )
    {
      delete _progressDialog;
    }
}

bool HttpClient::downloadFile( QString &urlIn, QString &destinationIn )
{
  // qDebug() << "HttpClient::downloadFile: url=" << urlIn << ", dest=" << destinationIn;

  if( downloadRunning == true )
    {
      qWarning( "HttpClient(%d): download is running!", __LINE__ );
      return false;
    }

  _url = urlIn;
  _destination = destinationIn;

  QUrl url( urlIn );
  QFileInfo fileInfo( destinationIn );

  if( urlIn.isEmpty() || ! url.isValid() || fileInfo.fileName().isEmpty() )
    {
      qWarning( "HttpClient(%d): Url or destination file are invalid!", __LINE__ );
      return false;
    }

  tmpFile = new QFile( destinationIn + "." +
                       QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") );

  if( ! tmpFile->open( QIODevice::WriteOnly ) )
    {
      qWarning( "HttpClient(%d): Unable to open the file %s: %s",
                 __LINE__,
                 tmpFile->fileName ().toLatin1().data(),
                 tmpFile->errorString().toLatin1().data() );

      delete tmpFile;
      tmpFile = static_cast<QFile *> (0);
      return false;
    }

  // Check, if a proxy is defined in the configuration data. If true, we do use it.
  extern QSettings _settings;

  QString proxy = _settings.value( "/Internet/Proxy", "" ).toString();

  if( proxy.isEmpty() )
    {
      // Check the user's environment, if a proxy is defined there.
      char* proxyFromEnv = getenv("http_proxy");

      if( proxyFromEnv )
        {
          QString qProxy( proxyFromEnv );

          // remove an existing http prefix
          proxy = qProxy.remove("http://");
        }
    }

  if( ! proxy.isEmpty() )
    {
      QString hostName;
      quint16 port;

      if( parseProxy( proxy, hostName, port ) == true )
        {
          QNetworkProxy proxy;
          proxy.setType( QNetworkProxy::HttpProxy );
          proxy.setHostName( hostName );
          proxy.setPort( port );
          manager->setProxy( proxy );
        }
    }

  QNetworkRequest request;
  QString appl = QCoreApplication::applicationName() + "/" +
                 QCoreApplication::applicationVersion() +
                 " (Qt" + QT_VERSION_STR + "/X11)";

  request.setUrl( QUrl( _url, QUrl::TolerantMode ));
  request.setRawHeader( "User-Agent", appl.toAscii() );

  reply = manager->get(request);

  if( ! reply )
    {
      qWarning( "HttpClient(%d): Reply object is invalid!", __LINE__ );
      return false;
    }

  reply->setReadBufferSize(0);

  connect( reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );

  connect( reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(slotError(QNetworkReply::NetworkError)) );

  connect( reply, SIGNAL(finished()),
           this, SLOT(slotFinished()) );

  connect( reply, SIGNAL(downloadProgress(qint64, qint64)),
           this, SLOT(slotDownloadProgress( qint64, qint64 )) );

  downloadRunning = true;

  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->setWindowTitle( tr( "HTTP" ) );
      _progressDialog->setLabelText( tr( "Downloading %1" ).arg( fileInfo.fileName() ) );
      _progressDialog->show();
    }

  timer->start();
  return true;
}

/** User has canceled the download. */
void HttpClient::slotCancelDownload()
{
  qDebug( "HttpClient(%d): Download canceled!", __LINE__ );

  timer->stop();

  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->reset();
      //_progressDialog->hide();
    }

  if( reply )
   {
      // That aborts the running download and sets the error state in the
      // reply object to QNetworkReply::OperationCanceledError. As next the
      // error signal and then the finish signal is emitted.
     reply->abort();
   }
}

/**
 * Network error occurred. Don't call abort() or close() in this method,
 * that leads to an endless loop!
 */
void HttpClient::slotError( QNetworkReply::NetworkError code )
{
  if( ! reply )
    {
      // Do ignore this call, because the reply object is already destroyed.
      // Do happen if the user don't close the message box.
      return;
    }

  qWarning( "HttpClient(%d): Network error %d, %s ",
           __LINE__, code, reply->errorString().toLatin1().data() );

  timer->start();

  if( code == QNetworkReply::NoError ||
      code == QNetworkReply::OperationCanceledError )
    {
      // Ignore these errors.
      return;
    }

  // If progress dialog is not activated, do not report anything more.
  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->reset();
      _progressDialog->hide();

      QMessageBox::information( 0, QObject::tr("HTTP-%1").arg(code),
                               QObject::tr("Download failed with: %1")
                               .arg(reply->errorString() ));
    }
}

/** Report download progress to the user */
void HttpClient::slotDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  // qDebug() << "HttpClient::slotDownloadProgress" << bytesReceived << bytesTotal;

  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      // Report results to the progress dialog.
      _progressDialog->setMaximum( bytesTotal );
      _progressDialog->setValue( bytesReceived );
    }
  else
    {
      // Emit this signal to the outside, when no progress dialog is set up.
      emit downloadProgress( bytesReceived, bytesTotal );
    }

  timer->start();
}

void HttpClient::slotAuthenticationRequired( QNetworkReply * /* reply */,
                                             QAuthenticator *authenticator )
{
  timer->stop();
  getUserPassword( authenticator );
  timer->start();
}

void HttpClient::slotProxyAuthenticationRequired( const QNetworkProxy & /* proxy */,
                                                  QAuthenticator *authenticator )
{
  timer->stop();
  getUserPassword( authenticator );
  timer->start();
}

void HttpClient::getUserPassword( QAuthenticator *authenticator )
{
  QString user, password;
  QString title = QObject::tr( "%1 at %2" ).arg( authenticator->realm() ).arg( QUrl(_url).host() );

  AuthDialog *dlg = new AuthDialog( user, password, title );
  dlg->adjustSize();

  if( dlg->exec() == QDialog::Accepted )
    {
      authenticator->setUser( user );
      authenticator->setPassword( password );
    }
}

#ifndef QT_NO_OPENSSL

void HttpClient::slotSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  QString errorString;

  for( int i = 0; i < errors.size(); i++ )
    {
      if( ! errorString.isEmpty() )
        {
          errorString += ", ";
        }

      errorString += errors.at(i).errorString();
    }

  timer->stop();

  if( QMessageBox::warning( 0, QObject::tr( "HTTP SSL Error" ),
      QObject::tr("One or more SSL errors has occurred: %1" ).arg( errorString ),
      QMessageBox::Ignore | QMessageBox::Abort ) == QMessageBox::Ignore )
    {
      reply->ignoreSslErrors();
    }

  timer->start();
}

#endif

/** Downloaded data for reading available. Put all of them into the opened
 *  temporary result file.
 */
void HttpClient::slotReadyRead()
{
  if( reply && tmpFile )
    {
      QByteArray byteArray = reply->readAll();

      if( byteArray.size() > 0 )
        {
          tmpFile->write( byteArray );
        }
    }

  timer->start();
}

/**
 * Download is finished. Close destination file and reply instance.
 * The reply instance has to be deleted.
 */
void HttpClient::slotFinished()
{
  timer->stop();

  // Hide progress dialog.
  if ( _progressDialog != static_cast<QProgressDialog *> (0) )
    {
      _progressDialog->reset();
      _progressDialog->hide();
    }

  if( reply && tmpFile )
    {
      qDebug( "Download %s finished with %d", _url.toLatin1().data(), reply->error() );

      // read reply error status
      enum QNetworkReply::NetworkError error = reply->error();

      // close opened IO device
      reply->close();

      // close temporary file
      tmpFile->close();

      if( error != QNetworkReply::NoError )
        {
          // Request was aborted, tmp file is removed.
          tmpFile->remove();
        }
      else
        {
          // Read last received bytes. Seems not to be necessary.
          // slotReadyRead();

          // Remove an old existing destination file before rename file.
          QFile::remove( _destination );

          // Rename temporary file to destination file.
          tmpFile->rename( _destination );
        }

      delete tmpFile;
      tmpFile = static_cast<QFile *> (0);

      // Destroy reply object
      reply->deleteLater();
      reply = static_cast<QNetworkReply *> (0);

      // Reset run flag before signal emit because signal finished can trigger
      // the next download.
      downloadRunning = false;

      // Inform about the result.
      emit finished( _url, error );
    }
}

/**
 * Returns true, if proxy parameters are valid.
 */
bool HttpClient::parseProxy( QString proxyIn, QString& hostName, quint16& port )
{
  QStringList proxyParams = proxyIn.split(":", QString::SkipEmptyParts);

  if( proxyParams.size() != 2 )
    {
      // too less or too much parameters
      return false;
    }

  hostName = proxyParams.at(0).trimmed();

  bool ok = false;

  port = proxyParams.at(1).trimmed().toUShort(&ok);

  if( hostName.isEmpty() || ok == false )
    {
      return false;
    }

  return true;
}
