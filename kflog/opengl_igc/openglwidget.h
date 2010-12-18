/***********************************************************************
**
**   openglwidget.h
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

#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <qgl.h>

class GLView;
class Flight;

/**
  *@author Christof Bodner
  */

class OpenGLWidget : public QWidget  {
public: 
	OpenGLWidget(QWidget* parent=0);
	~OpenGLWidget();
  void addFlight(Flight* flight);

protected:
  GLView* glview;
};

#endif
