/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
//
// Qt OpenGL example: Box
//
// A small example showing how a GLWidget can be used just as any Qt widget
// 
// File: main.cpp
//
// The main() function 
// 

#include <unistd.h>
#include <qapplication.h>
#include <qgl.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qvariant.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qthread.h>
#include "opengl_igc.h"
#include "openglwidget.h"

static const char* caption = "KFLog OpenGL IGC file viewer";
OpenGLWidget* openGLWidget;

extern "C"
{
  /** returns a reference to the caption string */
  const char* getCaption();
}
extern "C"
{
  /** calls the former main function */
  QWidget* getMainWidget();
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

QWidget* getMainWidget()
{
  openGLWidget = new OpenGLWidget;
  openGLWidget->resize(640,480);
  openGLWidget->show();
  return openGLWidget;
}

void addFlight(Flight* flight)
{
  openGLWidget->addFlight(flight);
}
