# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   filser.cpp \
            flightrecorder.cpp

HEADERS =   filser.h               
               
logger.path  = /logger
logger.files = fil.desktop spg.desktop README.fil
               
INSTALLS += logger

DESTDIR = ../../release/bin
