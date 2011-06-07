/***********************************************************************
**
**   openglwidget.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Christof Bodner
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "openglwidget.h"
#include "glview.h"

OpenGLWidget::OpenGLWidget( QWidget* parent ) : QWidget(parent)
{
  setWindowFlags( Qt::Window );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  glview = new GLView( this );

  QHBoxLayout* flayout = new QHBoxLayout( this );

  if( !glview->isValid() )
    {
       QString text(tr("<html>No OpenGL extension for display found!<br><br>"
                       "Please check your configuration!</html>"));

       QMessageBox::critical( this,
                              tr("Error"),
                              text,
                              QMessageBox::Ok );

       QLabel* label= new QLabel(text);
       label->setAlignment( Qt::AlignHCenter|Qt::AlignVCenter );
       flayout->addWidget( label );
    }
  else
    {
      flayout->addWidget( glview );
    }
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::addFlight(Flight* flight)
{
  if( glview->isValid() && flight )
    {
      glview->addFlight( flight );
    }
}
