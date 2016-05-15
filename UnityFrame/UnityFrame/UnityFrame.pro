#-------------------------------------------------
#
# Project created by QtCreator 2016-05-13T08:27:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/UnityFrame
MOC_DIR = $$MOC_DIR/UnityFrame
UI_DIR = $$UI_DIR/UnityFrame

linux: {
    TARGET = UnityFrame
}
!linux: {
    TARGET = UnityFrame
}
TEMPLATE = app

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic
linux|portable {
    QMAKE_LFLAGS += -Wl,-rpath,./lib
}

LIBS += -lcoreUnityFrame -lguiUnityFrame


SOURCES += main.cpp


win32: {
    RC_FILE = windows.rc
}

macx: {
    ICON = ../guiUnityFrame/img/UnityFrame.icns
}

OTHER_FILES += \
    windows.rc \
    UnityFrame.exe.manifest

unix: {
    target.path = $$BINDIR
    INSTALLS += target
}

RESOURCES += \
   # UnityFrame.qrc

