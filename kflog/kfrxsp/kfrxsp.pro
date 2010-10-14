TEMPLATE = lib

SOURCES =   soaringpilot.cpp \
            flightrecorder.cpp

HEADERS =   soaringpilot.h

logger.extra = mkdir -p  ~/.kflog/logger/; \
               cp ./xsp.desktop ~/.kflog/logger; \
               cp ./README ~/.kflog/logger
logger.path = ../pics
logger.files =
INSTALLS += logger

DESTDIR = ../../release
