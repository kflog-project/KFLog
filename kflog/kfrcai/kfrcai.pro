# KFLog qmake project file

TEMPLATE = lib

SOURCES =   cambridge.cpp \
            flightrecorder.cpp

HEADERS =   cambridge.h
               
logger.path  = /logger
logger.files = cai.desktop README.cai

INSTALLS += logger

DESTDIR = ../../release/lib
