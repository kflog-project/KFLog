# KFLog4 qmake project file
# $Id: kfrfla.pro 1214 2011-06-19 06:56:21Z axel $

TEMPLATE = lib

SOURCES =   flarm.cpp \
            flightrecorder.cpp

HEADERS =   flarm.h
                           
logger.path  = /logger
logger.files = fla.desktop README.flm
               
INSTALLS += logger

DESTDIR = ../../release/lib
