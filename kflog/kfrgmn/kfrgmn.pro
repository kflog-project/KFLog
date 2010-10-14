TEMPLATE = lib

SOURCES =   garminfr.cpp \
            garmin.cpp \
            flightrecorder.cpp

HEADERS =   flightrecorder.h \
            garmin.h \
            garmintypes.h \
            garminfr.h

logger.extra = mkdir -p  ~/.kflog/logger/; \
               cp ./gmn.desktop ~/.kflog/logger; \
               cp ./README ~/.kflog/logger
logger.path = ../pics
logger.files =
INSTALLS += logger

DESTDIR = ../../release
