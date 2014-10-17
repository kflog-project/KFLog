# KFLog qmake project file

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
DEFINES += QT_5
}

TEMPLATE = lib

SOURCES =   soaringpilot.cpp \
            flightrecorder.cpp

HEADERS =   soaringpilot.h

logger.path  = /logger

# do not copy the desktop file because this plugin ist not ready for use.
# logger.files = xsp.desktop README
               
INSTALLS += logger

DESTDIR = ../../release/lib
