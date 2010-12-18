/***********************************************************************
**
**   glview.h
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

#ifndef GLVIEW_H
#define GLVIEW_H

#include <qgl.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>

class Flight;

class GLView : public QGLWidget
{
    Q_OBJECT

public:

    GLView( QWidget* parent );
    ~GLView();
    virtual void addFlight(Flight* flight);
    virtual void addShadow(Flight* flight);

public slots:

    void		setXRotation( int degrees );
    void		setYRotation( int degrees );
    void		setZRotation( int degrees );
    void    zoom( float scalefactor );

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeBoxObject();
    
private:

    QColor getBackgroundColor();		//returns the background color 
    						//should read from Settings dialog
    GLuint boxObject;
    Q3ValueList<GLuint> flightList;
    GLfloat xRot, yRot, zRot, deltaX, deltaY, deltaZ, scale, heightExaggerate;
    void slotRotateUp();
    void    mouseMoveEvent ( QMouseEvent * e );
    void    mousePressEvent ( QMouseEvent * e );
    void    wheelEvent ( QWheelEvent * e );
    QPoint  mouse_last;
    long minx, maxx, miny, maxy, minz, maxz;
};


#endif // GLVIEW_H
