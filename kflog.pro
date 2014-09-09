###############################################################################
#
# qmake project file for whole KFLog build
#
###############################################################################

# version check for Qt 4.6
contains(QT_VERSION, ^4\\.[6]\\..*) {
  MY_QT_REL=4_6
}

# version check for Qt 4.7 and 4.8
contains(QT_VERSION, ^4\\.[78]\\..*) {
  MY_QT_REL=4_7
}

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
DEFINES += QT_5
}

# Version check for both Qt 4.x releases. One must be installed, if Qt 5.x is not
# installed.
lessThan(QT_MAJOR_VERSION, 5) {
  ! contains(MY_QT_REL, 4_6|4_7) {
  message("Cannot build KFLog with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.6 or higher!")
  }
}

TEMPLATE = subdirs

!win32 {
    SUBDIRS =   kflog \
                kflog/kfrcai \
                kflog/kfrfil \
                kflog/kfrfla \
                kflog/kfrgcs \
                kflog/kfrgmn \
                kflog/kfrxsp \
                kflog/opengl_igc
}
win32 {
    SUBDIRS =   kflog
}
