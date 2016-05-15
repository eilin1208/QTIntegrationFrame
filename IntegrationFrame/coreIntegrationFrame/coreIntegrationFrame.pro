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
    IntegrationFrame.cpp \
    plugins/plugintype.cpp \
    impl/pluginmanagerimpl.cpp \
    config_builder/cfgcategory.cpp \
    config_builder/cfgentry.cpp \
    config_builder/cfglazyinitializer.cpp \
    config_builder/cfgmain.cpp \
    services/config.cpp \
    services/notifymanager.cpp \
    db/db.cpp \
    db/attachguard.cpp \
    common/readwritelocker.cpp \
    returncode.cpp \
    services/functionmanager.cpp \
    translations.cpp \
    common/utils.cpp \
    db/sqlquery.cpp \
    db/sqlresultsrow.cpp \
    db/dbsqlite3.cpp

HEADERS +=\
    common/global.h \
    common/unused.h \
    coreIntegrationFrame_global.h \
    IntegrationFrame.h \
    services/pluginmanager.h \
    plugins/plugin.h \
    plugins/plugintype.h \
    impl/pluginmanagerimpl.h \
    config_builder.h \
    config_builder/cfgcategory.h \
    config_builder/cfgentry.h \
    config_builder/cfglazyinitializer.h \
    config_builder/cfgmain.h \
    services/config.h \
    services/notifymanager.h \
    db/db.h \
    dbobjecttype.h \
    interruptable.h \
    db/attachguard.h \
    common/readwritelocker.h \
    dialect.h \
    returncode.h \
    services/functionmanager.h \
    translations.h \
    common/utils.h \
    collationmanager.h \
    db/sqlquery.h \
    db/sqlresultsrow.h \
    db/dbsqlite3.h \
    db/abstractdb3.h \
    db/stdsqlite3driver.h
