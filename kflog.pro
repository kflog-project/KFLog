###############################################################################
# qmake project file for KFLog build
#
# $Id$
###############################################################################

# version check for Qt 4.6
contains(QT_VERSION, ^4\\.[6]\\..*) {
  MY_QT_REL=4_6
}

# version check for Qt 4.7
contains(QT_VERSION, ^4\\.[7]\\..*) {
  MY_QT_REL=4_7
}

# version check for both Qt releases. One must be installed
! contains(MY_QT_REL, 4_6|4_7) {
  message("Cannot build KFLog with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.6 or higher!")
}

TEMPLATE = subdirs

SUBDIRS =   kflog \
            kflog/kfrcai \
            kflog/kfrfil \
            kflog/kfrfla \
            kflog/kfrgcs \
            kflog/kfrgmn \
            kflog/kfrxsp \
            kflog/opengl_igc
