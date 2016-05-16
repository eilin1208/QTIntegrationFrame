#-------------------------------------------------
#
# Project created by QtCreator 2016-05-13T08:44:44
#
#-------------------------------------------------
include($$PWD/../dirs.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/coreUnityFrame
MOC_DIR = $$MOC_DIR/coreUnityFrame
UI_DIR = $$UI_DIR/coreUnityFrame

QT       += network script
QT       -= gui

TARGET = coreUnityFrame
TEMPLATE = lib

DEFINES += COREUNITYFRAME_LIBRARY

win32 {
    LIBS += -lpsapi $$PWD/../../../lib/libquazip.a
    LIBS += -limagehlp

    THE_FILE = $$PWD/qt.conf
    THE_DEST = $${DESTDIR}
    THE_FILE ~= s,/,\\,g
    THE_DEST ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY $$THE_FILE $$THE_DEST $$escape_expand(\\n\\t)
}

linux: {
    DEFINES += SYS_PLUGINS_DIR=$$LIBDIR/UnityFrame
    portable: {
        DESTDIR = $$DESTDIR/lib
    }
}

macx: {
    out_file = $$DESTDIR/lib $$TARGET .dylib
    QMAKE_POST_LINK += install_name_tool -change libsqlite3.dylib @loader_path/../Frameworks/libsqlite3.dylib $$join(out_file)
}

LIBS += -lsqlite3

portable {
    DEFINES += PORTABLE_CONFIG
}

CONFIG   += c++11
QMAKE_CXXFLAGS += -pedantic


SOURCES += \
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
    translations.cpp \
    common/utils.cpp \
    db/sqlquery.cpp \
    db/sqlresultsrow.cpp \
    db/dbsqlite3.cpp \
    db/abstractdb.cpp \
    log.cpp \
    db/sqlerrorcodes.cpp \
    services/dbmanager.cpp \
    db/sqlerrorresults.cpp \
    db/asyncqueryrunner.cpp \
    rsa/BigInt.cpp \
    rsa/RSA.cpp \
    rsa/PrimeGenerator.cpp \
    common/utils_sql.cpp \
    parser/lexer.cpp \
    parser/token.cpp \
    parser/keywords.cpp \
    parser/lexer_low_lev.cpp \
    UnityFrame.cpp \
    committable.cpp

HEADERS +=\
    common/global.h \
    common/unused.h \
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
    translations.h \
    common/utils.h \
    db/sqlquery.h \
    db/sqlresultsrow.h \
    db/dbsqlite3.h \
    db/abstractdb3.h \
    db/stdsqlite3driver.h \
    db/abstractdb.h \
    common/bihash.h \
    log.h \
    db/sqlerrorcodes.h \
    services/dbmanager.h \
    db/sqlerrorresults.h \
    db/asyncqueryrunner.h \
    rsa/BigInt.h \
    rsa/RSA.h \
    rsa/PrimeGenerator.h \
    common/utils_sql.h \
    parser/lexer.h \
    parser/token.h \
    parser/keywords.h \
    parser/lexer_low_lev.h \
    coreUnityFrame_global.h \
    UnityFrame.h \
    committable.h
