# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   flarm.cpp \
            flightrecorder.cpp

HEADERS =   flarm.h
                           
logger.path  = /logger
logger.files = fla.desktop README.flm
               
INSTALLS += logger

DESTDIR = ../../release/lib
