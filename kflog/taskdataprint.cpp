/***********************************************************************
**
**   taskdataprint.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>
#include <QPrinter>
#include <QPrintDialog>

#include "mapcalc.h"
#include "mainwindow.h"
#include "target.h"
#include "taskdataprint.h"
#include "wgspoint.h"

TaskDataPrint::TaskDataPrint(FlightTask* task)
{
  QPrinter printer( QPrinter::ScreenResolution );

  printer.setDocName( "kflog-task" );
  printer.setCreator( QString( "KFLog " ) + KFLOG_VERSION );
  printer.setOutputFileName( MainWindow::instance()->getApplicationDataDirectory() +
                             "/kflog-task.pdf" );

  QPrintDialog dialog( &printer, MainWindow::instance() );

  dialog.setWindowTitle( QObject::tr("Print Task") );
  dialog.setSizeGripEnabled ( true );
  dialog.setOptions( QAbstractPrintDialog::PrintToFile |
                     QAbstractPrintDialog::PrintSelection |
                     QAbstractPrintDialog::PrintPageRange |
                     QAbstractPrintDialog::PrintShowPageSize );

  if( dialog.exec() != QDialog::Accepted )
    {
      return;
    }

  QPainter painter;
  painter.begin( &printer );

  QFont font;
  font.setPixelSize( 8 );
  font.setStyle( QFont::StyleItalic );
  font.setStyleHint( QFont::SansSerif );

  painter.setFont( font );

  QString msg = QString("%1created by KFLog %2 (www.kflog.org)")
                        .arg( QChar(Qt::Key_copyright) )
                        .arg( KFLOG_VERSION );

  painter.drawText( 25, 12, msg );

  painter.setFont(QFont("helvetica", 18, QFont::Bold));
  painter.drawText(50, 50, QObject::tr("Flight planning") + ":");
  painter.setPen(QPen(Qt::black, 2));
  painter.drawLine(50, 56, 545, 56);

  painter.setFont(QFont("helvetica", 10));
  painter.drawText( 50, 100, QObject::tr("Task Type") + ": " +
                             task->getTaskTypeString() );

  painter.drawText( 50, 115, QObject::tr("Total Distance") + ": " +
                             task->getTotalDistanceString() );

  painter.drawText( 50, 130, QObject::tr("Task Distance") + ": " +
                             task->getTaskDistanceString() );

  painter.setPen( QPen( Qt::black, 2 ) );
  painter.drawLine( 50, 175, 545, 175 );

  painter.setFont( QFont( "helvetica", 12, QFont::Bold ) );
  painter.drawText( 50, 170, QObject::tr( "Task" ) + ":" );

  painter.setFont( QFont( "helvetica", 10 ) );

  int yPos = 210;

  for( int loop = 0; loop < task->getWPList().count(); loop++ )
    {
      Waypoint *cPoint = task->getWPList().at( loop );
      painter.drawText( 50, yPos, cPoint->name );

      painter.drawText( 125, yPos, WGSPoint::printPos(cPoint->origP.lat(), true) +
                                   " / " +
                                   WGSPoint::printPos(cPoint->origP.lon(), false) );
      yPos += 15;
    }

  painter.end();
}

TaskDataPrint::~TaskDataPrint()
{
}
