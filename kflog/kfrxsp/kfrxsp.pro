# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   soaringpilot.cpp \
            flightrecorder.cpp

HEADERS =   soaringpilot.h

logger.path  = /logger
logger.files = xsp.desktop README
               
INSTALLS += logger

DESTDIR = ../../release/bin

#The following line was inserted by qt3to4
QT += xml  qt3support 
