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
logger.files = gmn.desktop README
               
INSTALLS += logger

DESTDIR = ../../release

#The following line was inserted by qt3to4
QT += xml  qt3support 
