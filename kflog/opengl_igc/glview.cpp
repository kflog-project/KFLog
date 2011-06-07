/***********************************************************************
**
**   glview.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Christof Bodner
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License: See the file COPYING for more information.
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

#include <cmath>
#include <cstdlib>

#include "../flight.h"
#include "../resource.h"
#include "../mapcontents.h"

#include <QtGui>
#include <QtOpenGL>

/*!
  This is the main class for the OpenGL implementation. The constructor
  initializes some variables.
*/

GLView::GLView( QWidget* parent ) : QGLWidget( parent )
{
  setToolTip(tr("Press left or right mouse button during move to change the view.\n"
                 "Use mouse wheel for zooming."));
  // These values should be stored KConfig
  xRot = -45.0;
  yRot = 0.0;
  zRot = -30.0;           // default object rotation
  scale = 1.25;           // default object scale
  heightExaggerate=2.0;   // exaggerates the heights

  // Initializations
  deltaX = 0.0;           // initial position of objects
  deltaY = 0.0;
  deltaZ = 0.0;
  boxObject = 0;          // bounding box object (0 means none)
}


/**
 * Release allocated resources
 */
GLView::~GLView()
{
  if( boxObject )
    {
      glDeleteLists( boxObject, 1 );
    }

  for( int i = 0; i < flightList.size(); i++ )
    {
      glDeleteLists( flightList.at( i ), 1 );
    }
}

void GLView::addShadow(Flight* flight)
{
  if (flight)
    {
      int length = flight->getRouteLength();

      GLuint list = glGenLists( 1 );

      glNewList( list, GL_COMPILE );

      qglColor( Qt::gray ); // Shorthand for glColor3f or glIndex
      glLineWidth( 2.0 );

      FlightPoint fPoint;
      int actx, acty, actz;
      fPoint = flight->getPoint( 0 );
      actx = fPoint.projP.x();
      acty = -fPoint.projP.y(); // Attention!
      actz = minz;
      glVertex3i( actx, acty, actz );
      minx = actx;
      maxx = actx;
      miny = acty;
      maxy = acty;
      minz = actz;
      maxz = actz;

      glBegin( GL_LINE_LOOP );

      for( int i = 1; i < length; i++ )
        {
          fPoint = flight->getPoint( i );
          actx = fPoint.projP.x();
          acty = -fPoint.projP.y();
          glVertex3i( actx, acty, actz );
        }

      glEnd();
      glEndList();

      flightList.append( list );
    }
}

void GLView::addFlight(Flight* flight)
{
  if( flight )
    {
      int length = flight->getRouteLength();

      // qDebug( "Adding %d points", length );

      extern MapConfig _globalMapConfig;

      GLuint list = glGenLists( 1 );

      glNewList( list, GL_COMPILE );

      qglColor( Qt::blue ); // Shorthand for glColor3f or glIndex

      glLineWidth( 2.0 );

      FlightPoint fPoint;
      int actx, acty, actz;
      fPoint = flight->getPoint( 0 );
      actx = fPoint.projP.x();
      acty = -fPoint.projP.y(); // Attention!
      actz = fPoint.height;
      glVertex3i( actx, acty, actz );
      minx = actx;
      maxx = actx;
      miny = acty;
      maxy = acty;
      minz = actz;
      maxz = actz;

      glBegin( GL_LINE_LOOP );

      for( int i = 1; i < length; i++ )
        {
          fPoint = flight->getPoint( i );
          actx = fPoint.projP.x();
          acty = -fPoint.projP.y();
          actz = fPoint.height;

          QPen drawP = _globalMapConfig.getDrawPen( &fPoint );
          qglColor( drawP.color() );
          glVertex3i( actx, acty, actz );
          maxx = qMax( long( actx ), maxx );
          minx = qMin( long( actx ), minx );
          maxy = qMax( long( acty ), maxy );
          miny = qMin( long( acty ), miny );
          maxz = qMax( long( actz ), maxz );
          minz = qMin( long( actz ), minz );
        }

      glEnd();
      glEndList();

      deltaX = -(maxx + minx) / 2.0;
      deltaY = -(maxy + miny) / 2.0;
      deltaZ = -(maxz + minz) / 2.0;
      flightList.append( list );

      // change Bounding Box
      scale = qMax( abs( maxx - minx ), abs( maxy - miny ) );
      scale = qMax( double( scale ), fabs( maxz - minz ) );
      scale = 1.0 / scale;

      if( boxObject )
        {
          glDeleteLists( boxObject, 1 );
        }

      boxObject = makeBoxObject();
      addShadow( flight );
    }
}

/*!
  The actual openGL commands for drawing are performed here.
  This function is called on every paint event.
*/

void GLView::paintGL()
{
  //    qWarning("GLBox::paintGL()");
  // initialise the state machine (here will be more e.g. the lighting)
  glClear( GL_COLOR_BUFFER_BIT );
  glLoadIdentity(); // unity matrix

  // the following lines are for the transformations
  // You have to read them "bottom up". E.g. the last translation
  // is actually the first that is performed (This is due to the
  // matrix operation)

  // move a little bit away from camera
  glTranslatef( 0.0, 0.0, -10.0 );

  // rotate around x-, y- and z-axis
  glRotatef( xRot, 1.0, 0.0, 0.0 );
  glRotatef( yRot, 0.0, 1.0, 0.0 );
  glRotatef( zRot, 0.0, 0.0, 1.0 );

  // scale objects
  glScalef( scale, scale, scale * heightExaggerate );
  // move the center of the object to 0,0,0
  glTranslatef( deltaX, deltaY, deltaZ );

  // now all objects that should be displayed
  // bounding box
  glCallList( boxObject );

  // flight list (including the shadows of the flights)
  for( int i = 0; i < flightList.size(); ++i )
    {
      glCallList( flightList.at( i ) );
    }
}

/*!
  Set up the OpenGL rendering state, and define display list. Called once on the
  initialization of the widget
*/

void GLView::initializeGL()
{
  // qDebug("GLBox::initializeGL()");

  qglClearColor(getBackgroundColor());
  glShadeModel( GL_FLAT );    // shading model
}


/*!
  Set up the OpenGL view port, matrix mode, etc. Here is the projection mode
*/

void GLView::resizeGL( int w, int h )
{
  // qDebug( "GLBox::resizeGL()" );

  glViewport( 0, 0, (GLint) w, (GLint) h );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -1.0, 1.0, -1.0 * h / w, 1.0 * h / w, 5.0, 16.0 );
  glMatrixMode( GL_MODELVIEW );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the bounding box
*/

GLuint GLView::makeBoxObject()
{
  GLuint list;                // OpenGL objects are referred to as unsigned integers

  list = glGenLists( 1 );     // generate new list (empty)

  glNewList( list, GL_COMPILE );  // set mode = compile (=faster in rendering, but
                                  // no changes allowed

  qglColor( Qt::white );                      // Shorthand for glColor3f or glIndex

  glLineWidth( 2.0 );

  glBegin( GL_LINE_LOOP );    //  Begin new line segment (closed)
  glVertex3f(  maxx,  maxy,  minz );   // define verteces as floating �oint values)
  glVertex3f(  maxx,  miny,  minz );
  glVertex3f(  minx,  miny,  minz );
  glVertex3f(  minx,  maxy,  minz );
  glEnd();                    //  End of segment

  glBegin( GL_LINE_LOOP );    // same as above
  glVertex3f(  maxx,  maxy, maxz );
  glVertex3f(  maxx,  miny, maxz );
  glVertex3f(  minx,  miny, maxz );
  glVertex3f(  minx,  maxy, maxz );
  glEnd();

  glBegin( GL_LINES );         // these are 4 lines (not closed, so 8 points)
  glVertex3f(  maxx,  maxy,  minz );   glVertex3f(  maxx,  maxy, maxz );
  glVertex3f(  maxx,  miny,  minz );   glVertex3f(  maxx,  miny, maxz );
  glVertex3f(  minx,  miny,  minz );   glVertex3f(  minx,  miny, maxz );
  glVertex3f(  minx,  maxy,  minz );   glVertex3f(  minx,  maxy, maxz );
  glEnd();

  glEndList();                 // end of object

  return list;                  // return object identifier
}

/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLView::setXRotation( int degrees )
{
  xRot = (GLfloat) (degrees % 360);
  updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLView::setYRotation( int degrees )
{
  yRot = (GLfloat) (degrees % 360);
  updateGL();
}

/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLView::setZRotation( int degrees )
{
  zRot = (GLfloat) (degrees % 360);
  updateGL();
}

/*!
  Zooms scene
*/

void GLView::zoom( float scalefactor )
{
  scale *= scalefactor;
  updateGL();
}

/*
  Slots
*/
void GLView::mousePressEvent ( QMouseEvent* event )
{
  mouse_last = event->pos(); // store mouse position
}

void GLView::mouseMoveEvent ( QMouseEvent* event )
{
  // qDebug() << "GLView::mouseMoveEvent: Button=" << event->buttons();

  if( event->buttons() == Qt::LeftButton)
    {
      // left button means translation
      float phi = zRot / 180.0 * M_PI;

      //    qWarning(QString("phi:%1").arg(phi));
      float dx = (mouse_last.x() - event->x()) / 100.0 / scale;
      float dy = (mouse_last.y() - event->y()) / 100.0 / scale;

      if( ((int) xRot % 360) > -90 )
        {
          dy = -dy;
        }

      deltaX -= dx * cos( phi ) + dy * sin( phi );
      deltaY += dx * sin( phi ) - dy * cos( phi );
      mouse_last = event->pos();
      updateGL();
    }
  else if( event->buttons() == Qt::RightButton )
    {
      // right button is rotation
      zRot -= mouse_last.x() - event->x();
      xRot -= mouse_last.y() - event->y();
      mouse_last = event->pos();
      updateGL();
      // qWarning(QString("zRot:%1 xRot:%2").arg(zRot).arg(xRot));
    }
}

/*!
  Zoom objects with mouse wheel
*/

void GLView::wheelEvent ( QWheelEvent * e )
{
  float factor = e->delta() / 100.0;

  if( factor < 0 )
    {
      factor = -1 / factor;
    }

  zoom( factor );
}

QColor GLView::getBackgroundColor()
{
  return QColor( 64, 102, 128 );
}
