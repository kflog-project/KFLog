# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   garminfr.cpp \
            garmin.cpp \
            flightrecorder.cpp

HEADERS =   flightrecorder.h \
            garmin.h \
            garmintypes.h \
            garminfr.h

logger.path  = /logger

# do not copy the desktop file because this plugin ist not ready for use.
# logger.files = gmn.desktop README.gmn
               
INSTALLS += logger

DESTDIR = ../../release/lib
