# KFLog4 qmake project file
# $Id$

QT += xml opengl qt3support network

TEMPLATE = lib

INCLUDEPATH += ../

SOURCES =   glview.cpp \
            opengl_igc.cpp \
            openglwidget.cpp

HEADERS =   openglwidget.h \
            opengl_igc.h \
            glview.h

DESTDIR = ../../release/bin
