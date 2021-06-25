QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QCUSTOMPLOT_USE_LIBRARY

indev {
    DEFINES += INDEV
}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogoptions.cpp \
    displayhandler.cpp \
    main.cpp \
    mainwindow.cpp \
    nvmlreader.cpp \
    ohmwrapper.cpp \
    perfreader.cpp \
    widgetstartup.cpp

HEADERS += \
    dialogoptions.h \
    displayhandler.h \
    mainwindow.h \
    nvml.h \
    nvmldefs.h \
    nvmlreader.h \
    ohmwrapper.h \
    perfreader.h \
    widgetstartup.h

FORMS += \
    dialogoptions.ui \
    mainwindow.ui \
    widgetstartup.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CONFIG(debug, release|debug) {
  win32:QCPLIB = qcustomplotd2
  else: QCPLIB = qcustomplotd
} else {
  win32:QCPLIB = qcustomplot2
  else: QCPLIB = qcustomplot
}
LIBS += -L./ -l$$QCPLIB

win32: LIBS += -lpdh
win32 {
    # QMAKE_POST_LINK += mt -nologo -manifest "$$PWD/manifest.xml" -outputresource:"$$PWD/$${TARGET}.exe" #FIXME: doesn't work, we need a manifest though
}

RC_ICONS = "icon.ico"
