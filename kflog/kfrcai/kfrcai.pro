# KFLog4 qmake project file
# $Id$

TEMPLATE = lib

SOURCES =   cambridge.cpp \
            flightrecorder.cpp

HEADERS =   cambridge.h
               
logger.path  = /logger
logger.files = cai.desktop README.cai

INSTALLS += logger

DESTDIR = ../../release/bin

#The following line was inserted by qt3to4
QT += xml  qt3support 
