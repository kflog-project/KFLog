# KFLog qmake project file

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
DEFINES += QT_5
}

TEMPLATE = lib

SOURCES =   flarm.cpp \
            flightrecorder.cpp

HEADERS =   flarm.h
                           
logger.path  = /logger
logger.files = fla.desktop README.flm
               
INSTALLS += logger

DESTDIR = ../../release/lib
