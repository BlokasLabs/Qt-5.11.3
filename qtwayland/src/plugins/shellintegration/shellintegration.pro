TEMPLATE = subdirs
QT_FOR_CONFIG += waylandclient-private

SUBDIRS += xdg-shell
qtConfig(wayland-ivi-shell): SUBDIRS += ivi-shell
