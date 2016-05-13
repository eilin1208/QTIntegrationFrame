#-------------------------------------------------
#
# Project created by QtCreator 2016-05-13T08:44:44
#
#-------------------------------------------------
include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/coreIntegrationFrame
MOC_DIR = $$MOC_DIR/coreIntegrationFrame
UI_DIR = $$UI_DIR/coreIntegrationFrame

QT       += network xml
QT       -= gui

TARGET = coreIntegrationFrame
TEMPLATE = lib

DEFINES += COREINTEGRATIONFRAME_LIBRARY

win32 {
    #LIBS += -lpsapi $$PWD/../../../lib/libquazip.a
    LIBS += -limagehlp

    THE_FILE = $$PWD/qt.conf
    THE_DEST = $${DESTDIR}
    THE_FILE ~= s,/,\\,g
    THE_DEST ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY $$THE_FILE $$THE_DEST $$escape_expand(\\n\\t)
}

linux: {
    DEFINES += SYS_PLUGINS_DIR=$$LIBDIR/IntegrationFrame
    portable: {
        DESTDIR = $$DESTDIR/lib
    }
}

macx: {
    out_file = $$DESTDIR/lib $$TARGET .dylib
    QMAKE_POST_LINK += install_name_tool -change libsqlite3.dylib @loader_path/../Frameworks/libsqlite3.dylib $$join(out_file)
}

#LIBS += -lsqlite3

portable {
    DEFINES += PORTABLE_CONFIG
}

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic


SOURCES += \
    IntegrationFrame.cpp

HEADERS +=\
    common/global.h \
    common/unused.h \
    coreIntegrationFrame_global.h \
    IntegrationFrame.h
