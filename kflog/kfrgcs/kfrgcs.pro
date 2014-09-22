# KFLog qmake project file

TEMPLATE = lib

SOURCES =   volkslogger.cpp \
            vlconv.cpp \
            vlapisys_linux.cpp \
            vlapihlp.cpp \
            vlapi2.cpp \
            utils.cpp \
            grecord.cpp \
            flightrecorder.cpp \
            dbbconv.cpp

HEADERS =   dbbconv.h \
            grecord.h \
            utils.h \
            vla_support.h \
            vlapi2.h \
            vlapierr.h \
            vlapihlp.h \
            vlapityp.h \
            vlconv.h \
            volkslogger.h

logger.path  = /logger
logger.files = gcs.desktop README.gcs

INSTALLS += logger

DESTDIR = ../../release/lib
