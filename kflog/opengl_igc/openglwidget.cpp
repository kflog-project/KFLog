/***********************************************************************
**
**   openglwidget.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Christof Bodner
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <qlayout.h>
#include "openglwidget.h"
#include "glview.h"

OpenGLWidget::OpenGLWidget(QWidget* parent, char *name)
  :QWidget(parent,name){
   glview=new GLView(this,"OpenGL Viewer");
   QHBoxLayout* flayout = new QHBoxLayout( this, 2, 2, "flayout");
   flayout->addWidget( glview, 1 );
}

OpenGLWidget::~OpenGLWidget(){
}

void OpenGLWidget::addFlight(Flight* flight){
  glview->addFlight(flight);
}
