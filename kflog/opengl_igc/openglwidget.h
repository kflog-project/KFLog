/***********************************************************************
**
**   openglwidget.h
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

#ifndef OPEN_GL_WIDGET_H
#define OPEN_GL_WIDGET_H

#include <QWidget>

class GLView;
class Flight;

/**
 * \class OpenGLWidget
 *
 * \author Christof Bodner, Axel Pauli
 *
 * \date 2003-2011
 *
 * \version $Id$
 */

class OpenGLWidget : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( OpenGLWidget )

 public:

	OpenGLWidget(QWidget* parent=0);

	virtual ~OpenGLWidget();

  void addFlight(Flight* flight);

 protected:

  GLView* glview;
};

#endif
