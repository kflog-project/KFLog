/***********************************************************************
**
**   glview.h
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

#ifndef GL_VIEW_H
#define GL_VIEW_H

#include <QGLWidget>

#include <QColor>
#include <QList>
#include <QPoint>
#include <QWidget>

#include <QMouseEvent>
#include <QWheelEvent>

class Flight;

class GLView : public QGLWidget
{
  Q_OBJECT

public:

  GLView( QWidget* parent );
  virtual ~GLView();

  virtual void addFlight(Flight* flight);
  virtual void addShadow(Flight* flight);

public slots:

  void setXRotation( int degrees );
  void setYRotation( int degrees );
  void setZRotation( int degrees );
  void zoom( float scalefactor );

protected:

  void mouseMoveEvent ( QMouseEvent * e );
  void mousePressEvent ( QMouseEvent * e );
  void wheelEvent ( QWheelEvent * e );

  void initializeGL();
  void paintGL();
  void resizeGL( int w, int h );

  virtual GLuint makeBoxObject();

private:

  QColor getBackgroundColor();
  GLuint boxObject;
  QList<GLuint> flightList;
  GLfloat xRot, yRot, zRot, deltaX, deltaY, deltaZ, scale, heightExaggerate;
  QPoint  mouse_last;
  long minx, maxx, miny, maxy, minz, maxz;
};

#endif // GL_VIEW_H
