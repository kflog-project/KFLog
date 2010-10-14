TEMPLATE = lib

SOURCES =   filser.cpp \
            flightrecorder.cpp

HEADERS =   filser.h

logger.extra = mkdir -p  ~/.kflog/logger/; \
               cp ./fil.desktop ~/.kflog/logger; \
               cp ./spg.desktop ~/.kflog/logger; \
               cp ./README ~/.kflog/logger
logger.path = ../pics
logger.files =
INSTALLS += logger

DESTDIR = ../../release
