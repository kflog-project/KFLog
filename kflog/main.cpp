/***********************************************************************
**
**   main.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001      by Heiner Lamprecht
**                   2010-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Main of KFLog
 *
 * This file contains the start procedure of the KFLog GUI. KFLog is a C++
 * Application built with the QT/X11 SDK from Digia. Qt is a cross-platform
 * application and UI framework. See here for more information:
 *
 * http://qt-project.org/
 *
 * KFLog is built with the release 4.8.x.
 *
 * \date 2001-2014
 *
 * \version $Id$
 */

#include <unistd.h>
#include <libgen.h>

#ifdef QT_5
    #include <QtWidgets>
    #include <QApplication>
#else
    #include <QtGui>
#endif

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
  QLocale::setDefault(QLocale::C);
  QTranslator translator;
  translator.load(QLocale::system(),"kflog-");
  app.installTranslator(&translator);

  qDebug() << QLocale::system();
// if I understand the documentation right - QT5 is only working with UTF8
#ifndef QT_5
  // Make sure the application uses utf8 encoding for translated widgets
  QTextCodec::setCodecForTr( QTextCodec::codecForName ("UTF-8") );
#endif
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

  if( _settings.value( "/GeneralOptions/Logo", true ).toBool() )
    {
      QSplashScreen splash( QPixmap( ":/pics/splash.png" ) );
      splash.setMask( QBitmap( ":/pics/splash_mask.png" ) );
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

  for( int i = 0; i < app.arguments().size(); i++ )
    {
      argument = QString( app.arguments().at(i) );

      if( argument == "--batch" || argument == "-b" )
        {
          batch = true;
        }
      else if( (argument == "--export-png" || argument == "-e") && i + 2 < app.arguments().size() )
        {
          exportPNG = true;
          fileExportPNG = QString( app.arguments().at(i++) );
        }
      else if( argument == "--export-png" || argument == "-e" )
        {
          exportPNG = true;
          fileExportPNG = QString( "file:kflog_map.png" );
        }
      else if( argument == "--height" || argument == "-h" )
        {
          if( i + 2 < app.arguments().size() )
            height = QString( app.arguments().at(i++) );
        }
      else if( argument == "--width" || argument == "-w" )
        {
          if( i + 2 < app.arguments().size() )
            width = QString( app.arguments().at(i++) );
        }
      else if( argument == "--nocomment" || argument == "-c" )
        {
          comment = false;
        }
      else if( argument == "--waypoints" && i + 1 < app.arguments().size() )
        {
          waypointsOptionArg = app.arguments().at(i++);
        }
      else if( i != 0 )
        {
          fileOpen = true;
          fileOpenIGC = QString( app.arguments().at(i) );
        }
    }

  if( ! waypointsOptionArg.isEmpty() )
    {
      qDebug() << "WaypointCatalog"
               << waypointsOptionArg
               << "specified at startup, trying to open it.";

      _mainWindow->slotSetWaypointCatalog( waypointsOptionArg );
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
          qDebug() << "Writing PNG...";
          QUrl url( fileExportPNG );
          _mainWindow->slotSavePixmap( url, width.toInt(), height.toInt() );
        }

      if( batch )
        {
          qDebug() << "Exiting.";
          return 0;
        }

    }

  // start window manager event processing loop
  int result = QApplication::exec();

  // delete MainWindow
  delete _mainWindow;

  return result;
}
