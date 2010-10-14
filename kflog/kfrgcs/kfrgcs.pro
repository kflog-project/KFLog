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

HEADERS =   bbconv.h \
            grecord.h \
            utils.h \
            vla_support.h \
            vlapi2.h \
            vlapierr.h \
            vlapihlp.h \
            vlapityp.h \
            vlconv.h \
            volkslogger.h

logger.extra = mkdir -p  ~/.kflog/logger/; \
               cp ./gcs.desktop ~/.kflog/logger; \
               cp ./README ~/.kflog/logger
logger.path = ../pics
logger.files =
INSTALLS += logger

DESTDIR = ../../release
