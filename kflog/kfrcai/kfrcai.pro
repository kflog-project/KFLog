TEMPLATE = lib

SOURCES =   cambridge.cpp \
            flightrecorder.cpp

HEADERS =   cambridge.h

logger.extra = mkdir -p  ~/.kflog/logger/; \
               cp ./cai.desktop ~/.kflog/logger; \
               cp ./README ~/.kflog/logger
logger.path = ../pics
logger.files =
INSTALLS += logger

DESTDIR = ../../release
