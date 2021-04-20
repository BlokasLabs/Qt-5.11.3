SOURCES += \
    $$PWD/qqmlbind.cpp \
    $$PWD/qqmlconnections.cpp \
    $$PWD/qqmllistmodel.cpp \
    $$PWD/qqmllistmodelworkeragent.cpp \
    $$PWD/qqmlmodelsmodule.cpp \
    $$PWD/qqmlmodelindexvaluetype.cpp \
    $$PWD/qqmlobjectmodel.cpp \
    $$PWD/qquickpackage.cpp \
    $$PWD/qquickworkerscript.cpp \
    $$PWD/qqmlinstantiator.cpp

HEADERS += \
    $$PWD/qqmlbind_p.h \
    $$PWD/qqmlconnections_p.h \
    $$PWD/qqmllistmodel_p.h \
    $$PWD/qqmllistmodel_p_p.h \
    $$PWD/qqmllistmodelworkeragent_p.h \
    $$PWD/qqmlmodelsmodule_p.h \
    $$PWD/qqmlmodelindexvaluetype_p.h \
    $$PWD/qqmlobjectmodel_p.h \
    $$PWD/qquickpackage_p.h \
    $$PWD/qquickworkerscript_p.h \
    $$PWD/qqmlinstantiator_p.h \
    $$PWD/qqmlinstantiator_p_p.h

qtConfig(qml-delegate-model) {
    SOURCES += \
        $$PWD/qqmldelegatemodel.cpp

    HEADERS += \
        $$PWD/qqmldelegatemodel_p.h \
        $$PWD/qqmldelegatemodel_p_p.h
}

qtConfig(animation) {
    SOURCES += \
        $$PWD/qqmltimer.cpp

    HEADERS += \
    $$PWD/qqmltimer_p.h
}
