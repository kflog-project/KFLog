/***********************************************************************
**
**   glview.cpp
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

#include "glview.h"

#include <math.h>
#include "kaction.h"
#include "klocale.h"
#include "flight.h"

/*!
  Create an OpenGL widget
*/

GLView::GLView( QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.25;			// default object scale
    deltaX = 0.0;
    deltaY = 0.0;
    boxObject = 0;
    flightList.clear();
}


/*!
  Release allocated resources
*/

GLView::~GLView()
{
    glDeleteLists( boxObject, 1 );
    QValueList<GLuint>::iterator it;
    for ( it = flightList.begin(); it != flightList.end(); ++it )
      glDeleteLists((*it),1);
}

void GLView::addFlight(Flight* flight)
{
    GLuint list;
    unsigned int length;

    if (flight){
      length=flight->getRouteLength();
      qWarning(QString("Adding %1 points").arg(length));

      extern MapMatrix _globalMapMatrix;

  //    QPoint topLeft = _globalMapMatrix.mapToWgs(QPoint(flight->getFlightRect().left(),flight->getFlightRect().top()));
  //    QPoint bottomRight = _globalMapMatrix.mapToWgs(QPoint(flight->getFlightRect().right(),flight->getFlightRect().bottom()));
  //


      QPoint point;
      point=flight->getPoint(0).projP;
      QRect taskRect = flight->getFlightRect();

      qWarning(QString("1. point: x:%1 y:%2 z:%3").arg(point.x()).arg(point.y()).arg(0));
      qWarning(QString("bBoxFlight: l:%1 r:%2 t:%3 b:%4")
        .arg(taskRect.left()).arg(taskRect.right()).arg(taskRect.top()).arg(taskRect.bottom()));

      list = glGenLists( 1 );

      glNewList( list, GL_COMPILE );

      qglColor( blue );		      // Shorthand for glColor3f or glIndex

      glLineWidth( 2.0 );

      glBegin( GL_LINE_LOOP );
      glVertex3f(  0.5,  0.5, -0.4 );   glVertex3f(  1.0,  0.5, 0.4 );
      glVertex3f(  0.5, -0.5, -0.4 );   glVertex3f(  1.0, -0.5, 0.4 );
      glEnd();

      glEndList();

      flightList.append(list);
    }
}

/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLView::paintGL()
{
//    qWarning("GLBox::paintGL()");
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -10.0 );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glTranslatef( deltaX, deltaY, 0.0 );

    glCallList( boxObject );
    QValueList<GLuint>::iterator it;
    for ( it = flightList.begin(); it != flightList.end(); ++it )
      glCallList((*it));
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLView::initializeGL()
{
    qWarning("GLBox::initializeGL()");
    qglClearColor( black ); 		// Let OpenGL clear to black
    boxObject = makeBoxObject();		// Generate an OpenGL display list
    glShadeModel( GL_FLAT );
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLView::resizeGL( int w, int h )
{
    qWarning("GLBox::resizeGL()");
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0*h/w, 1.0*h/w, 5.0, 16.0 );
    glMatrixMode( GL_MODELVIEW );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLView::makeBoxObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    qglColor( white );		      // Shorthand for glColor3f or glIndex

    glLineWidth( 2.0 );

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0,  1.0, -0.4 );
    glVertex3f(  1.0, -1.0, -0.4 );
    glVertex3f( -1.0, -1.0, -0.4 );
    glVertex3f( -1.0,  1.0, -0.4 );
    glEnd();

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0,  1.0, 0.4 );
    glVertex3f(  1.0, -1.0, 0.4 );
    glVertex3f( -1.0, -1.0, 0.4 );
    glVertex3f( -1.0,  1.0, 0.4 );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f(  1.0,  1.0, -0.4 );   glVertex3f(  1.0,  1.0, 0.4 );
    glVertex3f(  1.0, -1.0, -0.4 );   glVertex3f(  1.0, -1.0, 0.4 );
    glVertex3f( -1.0, -1.0, -0.4 );   glVertex3f( -1.0, -1.0, 0.4 );
    glVertex3f( -1.0,  1.0, -0.4 );   glVertex3f( -1.0,  1.0, 0.4 );
    glEnd();

    glEndList();

    return list;
}

/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLView::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLView::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLView::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}

/*
  Slots
*/
void GLView::mousePressEvent ( QMouseEvent * e )
{
  mouse_last=e->pos();
}

void GLView::mouseMoveEvent ( QMouseEvent * e )
{
  if (e->state() & LeftButton){
    float phi=zRot/1800.0*M_PI;
    float dx=(mouse_last.x()-e->x())/100.0;
    float dy=(mouse_last.y()-e->y())/100.0;
    deltaX-=dx*cos(phi)-dy*sin(phi);
    deltaY+=dx*sin(phi)+dy*cos(phi);
    mouse_last=e->pos();
    updateGL();
  }
  else if(e->state() & RightButton){
    zRot-=mouse_last.x()-e->x();
    xRot-=mouse_last.y()-e->y();
    mouse_last=e->pos();
    updateGL();
  }
}

void GLView::wheelEvent ( QWheelEvent * e )
{
  scale+=(e->delta()/600.0);
  updateGL();
}
