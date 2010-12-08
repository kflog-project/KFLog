TEMPLATE = lib

SOURCES =   glview.cpp \
            opengl_igc.cpp \
            openglwidget.cpp

HEADERS =   openglwidget.h \
            opengl_igc.h \
            glview.h

DESTDIR = ../../release
#The following line was inserted by qt3to4
QT += xml  opengl qt3support 
