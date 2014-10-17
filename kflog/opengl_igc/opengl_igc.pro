# KFLog qmake project file

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
DEFINES += QT_5
}

QT += network opengl

TEMPLATE = lib

INCLUDEPATH += ../

SOURCES =   glview.cpp \
            opengl_igc.cpp \
            openglwidget.cpp

HEADERS =   openglwidget.h \
            glview.h

DESTDIR = ../../release/lib
