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

#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include "openglwidget.h"
#include "glview.h"

OpenGLWidget::OpenGLWidget(QWidget* parent, char *name)
  :QWidget(parent,name){
   glview=new GLView(this,"OpenGL Viewer");
   QHBoxLayout* flayout = new QHBoxLayout( this, 2, 2, "flayout");

   if (!glview->isValid())
   {
     QString text=tr("No OpenGL extension for display found! Check your configuration!");
     QMessageBox::critical(this, tr("Error"), "<qt>" + text +"</qt>", QMessageBox::Ok, 0, 0 );
     QLabel* label= new QLabel(text,this);
     label->setAlignment( AlignHCenter|AlignVCenter );
     flayout->addWidget(label,1);
   }
   else
   {
     flayout->addWidget( glview, 1 );
   }
}

OpenGLWidget::~OpenGLWidget(){
}

void OpenGLWidget::addFlight(Flight* flight){
  if (glview->isValid())
    glview->addFlight(flight);
}
