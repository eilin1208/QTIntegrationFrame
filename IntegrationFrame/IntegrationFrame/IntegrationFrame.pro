#-------------------------------------------------
#
# Project created by QtCreator 2016-05-13T08:27:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/IntegrationFrame
MOC_DIR = $$MOC_DIR/IntegrationFrame
UI_DIR = $$UI_DIR/IntegrationFrame

linux: {
    TARGET = integrationframe
}
!linux: {
    TARGET = IntegrationFrame
}
TEMPLATE = app

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic
linux|portable {
    QMAKE_LFLAGS += -Wl,-rpath,./lib
}

#LIBS += -lcoreSQLiteStudio -lguiSQLiteStudio


SOURCES += main.cpp


win32: {
    RC_FILE = windows.rc
}

macx: {
    #ICON = ../guiSQLiteStudio/img/sqlitestudio.icns
}

OTHER_FILES += \
    windows.rc \
    IntegrationFrame.exe.manifest

unix: {
    target.path = $$BINDIR
    INSTALLS += target
}

RESOURCES += \
    IntegrationFrame.qrc

