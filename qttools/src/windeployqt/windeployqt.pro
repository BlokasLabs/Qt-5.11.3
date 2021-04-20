option(host_build)
QT = core-private
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII QT_NO_FOREACH

SOURCES += main.cpp utils.cpp qmlutils.cpp elfreader.cpp
HEADERS += utils.h qmlutils.h elfreader.h

CONFIG += force_bootstrap

win32: LIBS += -lshlwapi

QMAKE_TARGET_DESCRIPTION = "Qt Windows Deployment Tool"
load(qt_tool)
