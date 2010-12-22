# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   filser.cpp \
            flightrecorder.cpp

HEADERS =   filser.h               
               
logger.path  = /logger
logger.files = fil.desktop spg.desktop README
               
INSTALLS += logger

DESTDIR = ../../release/bin

#The following line was inserted by qt3to4
QT += xml  qt3support 
