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
#include <qvaluelist.h>

class Flight;

class GLView : public QGLWidget
{
    Q_OBJECT

public:

    GLView( QWidget* parent, const char* name );
    ~GLView();
    virtual void addFlight(Flight* flight);

public slots:

    void		setXRotation( int degrees );
    void		setYRotation( int degrees );
    void		setZRotation( int degrees );

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeBoxObject();
    
private:

    GLuint boxObject;
    QValueList<GLuint> flightList;
    GLfloat xRot, yRot, zRot, deltaX, deltaY, scale;
    void slotRotateUp();
    void    mouseMoveEvent ( QMouseEvent * e );
    void    mousePressEvent ( QMouseEvent * e );
    void    wheelEvent ( QWheelEvent * e );
    QPoint  mouse_last;
};


#endif // GLVIEW_H
