QT += gui-private waylandclient-private
CONFIG += wayland-scanner

QMAKE_USE += wayland-client
qtConfig(xkbcommon-evdev): \
    QMAKE_USE_PRIVATE += xkbcommon_evdev

WAYLANDCLIENTSOURCES += \
    ../../../3rdparty/protocol/xdg-toplevel-icon-v1.xml \
    ../../../3rdparty/protocol/xdg-decoration-unstable-v1.xml \
    ../../../3rdparty/protocol/xdg-shell-stable.xml

HEADERS += \
    qwaylandxdgtopleveliconv1_p.h \
    qwaylandxdgdecorationv1_p.h \
    qwaylandxdgshell_p.h \
    qwaylandxdgshellintegration_p.h \

SOURCES += \
    qwaylandxdgtopleveliconv1.cpp \
    main.cpp \
    qwaylandxdgdecorationv1.cpp \
    qwaylandxdgshell.cpp \
    qwaylandxdgshellintegration.cpp \

OTHER_FILES += \
    BACKPORT.txt \
    xdg-shell.json

PLUGIN_TYPE = wayland-shell-integration
PLUGIN_CLASS_NAME = QWaylandXdgShellStableIntegrationPlugin
load(qt_plugin)
