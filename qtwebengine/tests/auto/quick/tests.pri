include($$QTWEBENGINE_OUT_ROOT/qtwebengine-config.pri)
QT_FOR_CONFIG += webengine-private

TEMPLATE = app

CONFIG += testcase
CONFIG += c++11

VPATH += $$_PRO_FILE_PWD_
TARGET = tst_$$TARGET

SOURCES += $${TARGET}.cpp
INCLUDEPATH += \
    $$PWD \
    ../shared

QT += testlib network quick webengine

# This define is used by some tests to look up resources in the source tree
DEFINES += TESTS_SOURCE_DIR=\\\"$$PWD/\\\"

qtConfig(webengine-testsupport) {
    DEFINES += ENABLE_QML_TESTSUPPORT_API
}

include(../embed_info_plist.pri)
