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

/***********************************************************************
**
**  Attention: y values have to be passed negative to OpenGL functions!
**
***********************************************************************/


#include "glview.h"

#include <math.h>
#include <stdlib.h>
#include "kaction.h"
#include "klocale.h"
#include "flight.h"
#include "resource.h"
#include "mapcontents.h"


/*!
  Create an OpenGL widget
*/

GLView::GLView( QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    // These values should be stored KConfig
    xRot = -45.0;
    yRot = 0.0;
    zRot = -30.0;		// default object rotation
    scale = 1.25;			// default object scale
    heightExaggerate=2.0;   //  exaggerates the heights

    // Initializations
    deltaX = 0.0;
    deltaY = 0.0;
    deltaZ = 0.0;
    boxObject = 0;
    flightList.clear();
}


/*!
  Release allocated resources
*/

GLView::~GLView()
{
    if (boxObject)
      glDeleteLists( boxObject, 1 );
    QValueList<GLuint>::iterator it;
    for ( it = flightList.begin(); it != flightList.end(); ++it )
      glDeleteLists((*it),1);
}

void GLView::addShadow(Flight* flight)
{
    GLuint list;
    unsigned int length;

    if (flight){
      length=flight->getRouteLength();
      extern MapMatrix _globalMapMatrix;
      QPoint point;
      list = glGenLists( 1 );

      glNewList( list, GL_COMPILE );

      qglColor( gray );		      // Shorthand for glColor3f or glIndex
      glLineWidth( 2.0 );

      flightPoint fPoint;
      int actx, acty, actz;
      fPoint=flight->getPoint(0);
      actx=fPoint.projP.x();
      acty=-fPoint.projP.y();  // Attention!
      actz=minz;
      glVertex3i(  actx,  acty, actz );
      minx=actx; maxx=actx; miny=acty; maxy=acty; minz=actz; maxz=actz;

      glBegin( GL_LINE_LOOP );
      for (int i=1;i<length;i++){
        fPoint=flight->getPoint(i);
        actx=fPoint.projP.x();
        acty=-fPoint.projP.y();
        glVertex3i(  actx,  acty, actz );
      }
      glEnd();
      glEndList();

      flightList.append(list);
    }
}

void GLView::addFlight(Flight* flight)
{
    GLuint list;
    unsigned int length;

    if (flight){
      length=flight->getRouteLength();
      qWarning(QString("Adding %1 points").arg(length));

      extern MapMatrix _globalMapMatrix;
      extern MapConfig _globalMapConfig;
      
      QPoint point;
        
      list = glGenLists( 1 );

      glNewList( list, GL_COMPILE );

      qglColor( blue );		      // Shorthand for glColor3f or glIndex

      glLineWidth( 2.0 );

      
      flightPoint fPoint;
      int actx, acty, actz;
      fPoint=flight->getPoint(0);
      actx=fPoint.projP.x();
      acty=-fPoint.projP.y();  // Attention!
      actz=fPoint.height;
      glVertex3i(  actx,  acty, actz );
      minx=actx; maxx=actx; miny=acty; maxy=acty; minz=actz; maxz=actz;

      glBegin( GL_LINE_LOOP );
      for (int i=1;i<length;i++){
        fPoint=flight->getPoint(i);
        actx=fPoint.projP.x();
        acty=-fPoint.projP.y();
        actz=fPoint.height;
        
        QPen drawP = _globalMapConfig.getDrawPen(&fPoint);
        qglColor(drawP.color());
        glVertex3i(  actx,  acty, actz );
        if (actx>maxx)
          maxx=actx;
        if (actx<minx)
          minx=actx;
        if (acty>maxy)
          maxy=acty;
        if (acty<miny)
          miny=acty;
        if (actz>maxz)
          maxz=actz;
        if (actz<minz)
          minz=actz;
      }
      glEnd();
      glEndList();

      deltaX=-(maxx+minx)/2.0;
      deltaY=-(maxy+miny)/2.0;
      deltaZ=-(maxz+minz)/2.0;
      flightList.append(list);
      // change Bounding Box
      scale=MAX(abs(maxx-minx),abs(maxy-miny));
      scale=MAX(scale,abs(maxz-minz));
      scale=1.0/scale;
      if (boxObject)
        glDeleteLists( boxObject, 1 );
      boxObject = makeBoxObject();
      addShadow(flight);
    }
}

/*!
  The actual openGL commands for drawing are performed here.
*/

void GLView::paintGL()
{
//    qWarning("GLBox::paintGL()");
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -10.0 );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glScalef( scale, scale, scale*heightExaggerate );
    glTranslatef( deltaX, deltaY, deltaZ );

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
    glVertex3f(  maxx,  maxy,  minz );
    glVertex3f(  maxx,  miny,  minz );
    glVertex3f(  minx,  miny,  minz );
    glVertex3f(  minx,  maxy,  minz );
    glEnd();

    glBegin( GL_LINE_LOOP );
    glVertex3f(  maxx,  maxy, maxz );
    glVertex3f(  maxx,  miny, maxz );
    glVertex3f(  minx,  miny, maxz );
    glVertex3f(  minx,  maxy, maxz );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f(  maxx,  maxy,  minz );   glVertex3f(  maxx,  maxy, maxz );
    glVertex3f(  maxx,  miny,  minz );   glVertex3f(  maxx,  miny, maxz );
    glVertex3f(  minx,  miny,  minz );   glVertex3f(  minx,  miny, maxz );
    glVertex3f(  minx,  maxy,  minz );   glVertex3f(  minx,  maxy, maxz );
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
    float phi=zRot/180.0*M_PI;
    float dx=(mouse_last.x()-e->x())/100.0/scale;
    float dy=(mouse_last.y()-e->y())/100.0/scale;
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
    qWarning(QString("zRot:%1 xRot:%2").arg(zRot).arg(xRot));
  }
}

void GLView::wheelEvent ( QWheelEvent * e )
{
  if (e->delta()>0)
    scale*=(e->delta()/100.0);
  else if (e->delta()<0)
    scale/=(-e->delta()/100.0);
  updateGL();
}
