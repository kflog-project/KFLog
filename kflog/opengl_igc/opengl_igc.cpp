/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
**               2011 by Axel Pauli
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <unistd.h>

#include <QtGui>

#include "openglwidget.h"

static const char* caption = "OpenGL IGC flight viewer";

OpenGLWidget* openGLWidget;

extern "C"
{
  /** returns a reference to the caption string */
  const char* getCaption();
}
extern "C"
{
  /** calls the former main function */
  QWidget* getMainWidget( QWidget* parent=0 );
}
extern "C"
{
  /** adds a Flight to the Display List */
  void addFlight(Flight*);
}

const char* getCaption()
{
  return caption;
}

QWidget* getMainWidget( QWidget* parent )
{
  openGLWidget = new OpenGLWidget( parent );
  openGLWidget->setWindowTitle( caption );
  openGLWidget->resize( 640,480 );
  openGLWidget->show();
  return openGLWidget;
}

void addFlight(Flight* flight)
{
  openGLWidget->addFlight(flight);
}
