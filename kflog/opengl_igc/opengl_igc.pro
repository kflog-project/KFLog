# KFLog4 qmake project file
# $Id$

QT += network opengl

TEMPLATE = lib

INCLUDEPATH += ../

SOURCES =   glview.cpp \
            opengl_igc.cpp \
            openglwidget.cpp

HEADERS =   openglwidget.h \
            glview.h

DESTDIR = ../../release/lib
