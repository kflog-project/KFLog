/***********************************************************************
**
**   main.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001      by Heiner Lamprecht
**                   2010-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Main of KFLog
 *
 * This file contains the start procedure of the KFLog GUI. KFLog is a C++
 * Application built with the QT/X11 SDK from Nokia. Qt is a cross-platform
 * application and UI framework. See here for more information:
 *
 * http://qt.nokia.com
 *
 * KFLog is built with the release 4.7.x.
 *
 * \date 2001-2011
 *
 * \version $Id$
 */

#include <unistd.h>
#include <libgen.h>

#include <QtGui>

#include "kflogconfig.h"
#include "mainwindow.h"
#include "target.h"

/**
 * Pointer to the main window.
 */
MainWindow *_mainWindow = static_cast<MainWindow *> (0);

/**
 * Contains all settings of KFLog. The setting file is stored under
 * $HOME/.config/KFLog as kflog.conf.
 */
QSettings _settings( QSettings::UserScope, "KFLog", "kflog" );

/**
 * List of command line-options
 */

//static KCmdLineOptions options[] =
//{
//  { "e", 0, 0 },
//  { "export-png ", I18N_NOOP("export to png graphics file"), "file:out.png" },
//  { "w", 0, 0 },
//  { "width ", I18N_NOOP("width of pixmap"), "640" },
//  { "h", 0, 0 },
//  { "height ", I18N_NOOP("height of pixmap"), "480" },
//  { "c", 0, 0 },
//  { "nocomment", I18N_NOOP("suppress comment"), 0 },
//  { "b", 0, 0 },
//  { "batch", I18N_NOOP("quit after export (batch mode)"), 0 },
//  { "waypoints ", I18N_NOOP("waypoint-catalog to open"), 0 },
//  { "+[File]", I18N_NOOP("igc-file to open"), 0 },
//  { 0, 0, 0 }
//};

/*************************************************************************
 *
 * Okay, now let's start :-)
 *
 */
int main(int argc, char **argv)
{
  QApplication app( argc, argv );

  QCoreApplication::setOrganizationName("KFLog");
  QCoreApplication::setOrganizationDomain("www.kflog.org");
  QCoreApplication::setApplicationName("kflog");
  QCoreApplication::setApplicationVersion( KFLOG_VERSION );

  // Set the compile date of the application.
  _settings.setValue( "/Main/CompileDate", __DATE__ );

  // Reset the locale that is used for number formatting to "C" locale.
  setlocale(LC_NUMERIC, "C");

  // Make sure the application uses utf8 encoding for translated widgets
  QTextCodec::setCodecForTr( QTextCodec::codecForName ("UTF-8") );

  // Make install root of KFLog available for other modules via
  // QSettings. The assumption is that KFLog is installed at
  // <root>/bin/kflog. The <root> path will be passed to QSettings.
  char *callPath = dirname(argv[0]);
  char *startDir = getcwd(0,0);
  chdir( callPath );
  char *callDir = getcwd(0,0);
  QString root = QString(dirname(callDir));
  _settings.setValue( "/Path/InstallRoot", root );

  // change back to start directory
  chdir( startDir );
  free( callDir );
  free( startDir );

  qDebug() << "KFLog Version:" << KFLOG_VERSION;
  qDebug() << "KFLog Built Date:" << __DATE__;
  qDebug() << "KFLog Install Root:" << root;

  bool showStartLogo = false;

  if( _settings.value( "/GeneralOptions/Logo", true ).toBool() )
  {
    showStartLogo = true;

    QSplashScreen splash( root + "/pics/splash.png"  );
    splash.setMask( QBitmap( root + "/pics/splash_mask.png" ));
    splash.show();
    QCoreApplication::processEvents();

    _mainWindow = new MainWindow;
    _mainWindow->setVisible( true );

    splash.finish( _mainWindow );
  }
  else
    {
      _mainWindow = new MainWindow;
      _mainWindow->setVisible( true );
    }

  QString argument, fileOpenIGC, fileExportPNG, width = "640", height = "480";
  QString waypointsOptionArg;

  bool batch = false, comment = true, exportPNG = false, fileOpen = false;

  for( int i = 0; i < app.argc(); i++ )
    {
      argument = QString( app.argv()[i] );

      if( argument == "--batch" || argument == "-b" )
        {
          batch = true;
        }
      else if( (argument == "--export-png" || argument == "-e") && i + 2 < app.argc() )
        {
          exportPNG = true;
          fileExportPNG = QString( app.argv()[i++] );
        }
      else if( argument == "--export-png" || argument == "-e" )
        {
          exportPNG = true;
          fileExportPNG = QString( "file:out.png" );
        }
      else if( argument == "--height" || argument == "-h" )
        {
          if( i + 2 < app.argc() )
            height = QString( app.argv()[i++] );
        }
      else if( argument == "--width" || argument == "-w" )
        {
          if( i + 2 < app.argc() )
            width = QString( app.argv()[i++] );
        }
      else if( argument == "--nocomment" || argument == "-c" )
        {
          comment = false;
        }
      else if( argument == "--waypoints" && i + 1 < app.argc() )
        {
          waypointsOptionArg = app.argv()[i++];
        }
      else if( i != 0 )
        {
          fileOpen = true;
          fileOpenIGC = QString( app.argv()[i] );
        }
    }

  if( !waypointsOptionArg.isEmpty() )
    {
      qWarning() << "WaypointCatalog specified at startup"
                 << waypointsOptionArg;

      _mainWindow->slotSetWaypointCatalog( waypointsOptionArg );
    }
  else
    {
      // read the user configuration
      int useCatalog = _settings.value("/Waypoints/DefaultWaypointCatalog", KFLogConfig::LastUsed ).toInt();

      switch( useCatalog )
        {
        case KFLogConfig::LastUsed:
          // no break;
        case KFLogConfig::Specific:
          waypointsOptionArg = _settings.value( "/Waypoints/DefaultCatalogName", "" ).toString();
          _mainWindow->slotSetWaypointCatalog( waypointsOptionArg );
        }
    }

  if( fileOpen )
    {
      if( exportPNG )
        {
          _settings.setValue( "/GeneralOptions/ShowWaypointWarnings", false );
        }

      _mainWindow->slotOpenFile( QUrl(fileOpenIGC) );

      if( exportPNG )
        {
          _settings.setValue( "/CommentSettings/ShowComment", comment );
          qWarning() << "Writing PNG...";
          QUrl url( fileExportPNG );
          _mainWindow->slotSavePixmap( url, width.toInt(), height.toInt() );
        }

      if( batch )
        {
          qWarning() << "Exiting.";
          return 0;
        }

    }

  // start window manager event processing loop
  int result = QApplication::exec();

  // delete MainWindow
  delete _mainWindow;

  return result;
}
