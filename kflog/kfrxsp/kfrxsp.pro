# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   soaringpilot.cpp \
            flightrecorder.cpp

HEADERS =   soaringpilot.h

logger.path  = /logger

# do not copy the desktop file because this plugin ist not ready for use.
# logger.files = xsp.desktop README
               
INSTALLS += logger

DESTDIR = ../../release/bin
