TEMPLATE = subdirs

qtHaveModule(widgets) {
    no-png {
        message("Some graphics-related tools are unavailable without PNG support")
    } else {
        SUBDIRS = assistant \
                  pixeltool \
                  designer

        linguist.depends = designer
    }
}

SUBDIRS += linguist \
    qtattributionsscanner

qtConfig(library) {
    !android|android_app: SUBDIRS += qtplugininfo
}

config_clang: SUBDIRS += qdoc

if(!android|android_app):!uikit: SUBDIRS += qtpaths

mac {
    SUBDIRS += macdeployqt
}

qtHaveModule(dbus): SUBDIRS += qdbus

win32|winrt:SUBDIRS += windeployqt
winrt:SUBDIRS += winrtrunner
qtHaveModule(gui):!android:!uikit:!qnx:!winrt: SUBDIRS += qtdiag

qtNomakeTools( \
    pixeltool \
    macdeployqt \
)

# This is necessary to avoid a race condition between toolchain.prf
# invocations in a module-by-module cross-build.
cross_compile:isEmpty(QMAKE_HOST_CXX.INCDIRS) {
    qdoc.depends += qtattributionsscanner
    windeployqt.depends += qtattributionsscanner
    winrtrunner.depends += qtattributionsscanner
    linguist.depends += qtattributionsscanner
}
