# KFLog qmake project file

TEMPLATE = lib

SOURCES =   filser.cpp \
            flightrecorder.cpp

HEADERS =   filser.h
                           
logger.path  = /logger
logger.files = fil.desktop spg.desktop README.fil
               
INSTALLS += logger

DESTDIR = ../../release/lib
