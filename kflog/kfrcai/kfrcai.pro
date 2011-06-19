# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   cambridge.cpp \
            flightrecorder.cpp

HEADERS =   cambridge.h
               
logger.path  = /logger
logger.files = cai.desktop README.cai

INSTALLS += logger

DESTDIR = ../../release/lib
