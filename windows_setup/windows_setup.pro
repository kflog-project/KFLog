###############################################################################
# qmake project file for KFLog build
#
# $Id$
###############################################################################

# version check for Qt 4.6
contains(QT_VERSION, ^4\\.[6]\\..*) {
  MY_QT_REL=4_6
}

# version check for Qt 4.7 and 4.8
contains(QT_VERSION, ^4\\.[78]\\..*) {
  MY_QT_REL=4_7
}

# version check for both Qt releases. One must be installed
! contains(MY_QT_REL, 4_6|4_7) {
  message("Cannot build KFLog with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.6 or higher!")
}

TARGET = dummy

message($$QMAKE_CC)

 buildkit.commands = powershell.exe $$_PRO_FILE_PWD_/buildKits.ps1 $$OUT_PWD
 QMAKE_EXTRA_TARGETS += buildkit

PRE_TARGETDEPS = buildkit

SOURCES += \
    dummy.cpp
