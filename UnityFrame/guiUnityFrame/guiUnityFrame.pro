#-------------------------------------------------
#
# Project created by QtCreator 2016-05-13T08:39:37
#
#-------------------------------------------------

QT       += core gui widgets xml

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/guiUnityFrame
MOC_DIR = $$MOC_DIR/guiUnityFrame
UI_DIR = $$UI_DIR/guiUnityFrame

linux: {
    portable: {
        DESTDIR = $$DESTDIR/lib
    }
}

portable: {
    DEFINES += PORTABLE_CONFIG
}

TARGET = guiUnityFrame
TEMPLATE = lib

CONFIG += c++11
QMAKE_CXXFLAGS += -pedantic

DEFINES += GUIUNITYFRAME_LIBRARY

SOURCES += \
    mainwindow.cpp \
    icon.cpp \
    iconmanager.cpp \
    common/extactioncontainer.cpp \
    common/extactionprototype.cpp \
    common/extaction.cpp

HEADERS +=\
    mainwindow.h \
    icon.h \
    iconmanager.h \
    common/extactioncontainer.h \
    common/extactionprototype.h \
    common/extaction.h \
    guiUnityFrame_global.h

RESOURCES += \
    icons.qrc \
    guiUnityFrame.qrc

OTHER_FILES +=

unix: {
    target.path = $$LIBDIR
    INSTALLS += target
}

LIBS += -lcoreUnityFrame

DISTFILES += \
    general.css

FORMS += \
    mainwindow.ui
