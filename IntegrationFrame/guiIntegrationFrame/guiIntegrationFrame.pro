#-------------------------------------------------
#
# Project created by QtCreator 2016-05-13T08:39:37
#
#-------------------------------------------------

QT       += core gui widgets xml

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/guiIntegrationFrame
MOC_DIR = $$MOC_DIR/guiIntegrationFrame
UI_DIR = $$UI_DIR/guiIntegrationFrame

linux: {
    portable: {
        DESTDIR = $$DESTDIR/lib
    }
}

portable: {
    DEFINES += PORTABLE_CONFIG
}

TARGET = guiIntegrationFrame
TEMPLATE = lib

CONFIG += c++11
QMAKE_CXXFLAGS += -pedantic

DEFINES += GUIINTEGRATIONFRAME_LIBRARY

SOURCES += \
    mainwindow.cpp

HEADERS +=\
        guiintegrationframe_global.h \
    mainwindow.h

RESOURCES += \
    icons.qrc \
    guiIntegrationFrame.qrc

OTHER_FILES +=

unix: {
    target.path = $$LIBDIR
    INSTALLS += target
}

LIBS += -lcoreIntegrationFrame

DISTFILES += \
    general.css

FORMS += \
    mainwindow.ui
