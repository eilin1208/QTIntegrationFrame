TEMPLATE = subdirs

core.subdir = coreUnityFrame

#tests.subdir = Tests
#tests.depends = core

gui.subdir = guiUnityFrame
gui.depends = core

#cli.subdir = UnityFramecli
#cli.depends = core

gui_app.subdir = UnityFrame
gui_app.depends = gui

#update_app.subdir = UpdateUnityFrame
#update_app.depends = core

SUBDIRS += \
    core \
    gui \
#    cli \
    gui_app

if(contains(DEFINES,tests)) {
    SUBDIRS += tests
}

OUTPUT_DIR_NAME = output

win32: {
#    SUBDIRS += update_app
}

linux: {
    portable.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE
    tgz.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE tgz
    pkg.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist
    pkg_plugins.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist_plugins
    pkg_full.commands = sh $$PWD/create_linux_portable.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist_full
    QMAKE_EXTRA_TARGETS += portable tgz pkg pkg_plugins pkg_full
}

macx: {
    bundle.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE
    dmg.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dmg
    pkg.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist
    pkg_plugins.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist_plugins
    pkg_full.commands = sh $$PWD/create_macosx_bundle.sh $$PWD/../$$OUTPUT_DIR_NAME $$QMAKE_QMAKE dist_full
    QMAKE_EXTRA_TARGETS += bundle dmg pkg pkg_plugins pkg_full
}
