# qmake project file for KFLog build
# $Id$

# version check for Qt 4.7
! contains(QT_VERSION, ^4\\.[7]\\..*) {
  message("Cannot build KFLog with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.7. or higher!")
}

TEMPLATE = subdirs

SUBDIRS =   kflog \
            kflog/kfrcai \
            kflog/kfrfil \
            kflog/kfrgcs \
            kflog/kfrgmn \
            kflog/kfrxsp \
            kflog/opengl_igc
