TEMPLATE = app
TARGET   = HUpnpSimpleTestApp
QT      += network testlib xml
CONFIG  += warn_on

INCLUDEPATH += ../../hupnp/include

LIBS += -L"../../hupnp/bin" -lHUpnp \
        -L"../../hupnp/lib/qtsoap-2.7-opensource/lib"

win32 {
    debug {
        LIBS += -lQtSolutions_SOAP-2.7d
    }
    else {
        LIBS += -lQtSolutions_SOAP-2.7
    }

    LIBS += -lws2_32

    DESCRIPTIONS = $$PWD\\descriptions
    DESCRIPTIONS = $${replace(DESCRIPTIONS, /, \\)}
    QMAKE_POST_LINK += xcopy $$DESCRIPTIONS bin\\descriptions /E /Y /C /I $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += copy ..\\..\\hupnp\\bin\\* bin /Y
}
else {
    LIBS += -lQtSolutions_SOAP-2.7
    !macx:QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN

    QMAKE_POST_LINK += cp -Rf $$PWD/descriptions bin &
    QMAKE_POST_LINK += cp -Rf ../../hupnp/bin/* bin
}

macx {
  CONFIG -= app_bundle
  #CONFIG += x86 x86_64
}

OBJECTS_DIR = obj
MOC_DIR = obj

DESTDIR = ./bin

HEADERS += \
    mainwindow.h \
    controlpoint_window.h \
    device_window.h \
    controlpoint_navigator.h \
    controlpoint_navigatoritem.h \
    dataitem_display.h \
    invokeactiondialog.h \
    allowedvaluelist_input.h \
    genericinput.h \
    i_dataholder.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    controlpoint_window.cpp \
    device_window.cpp \
    controlpoint_navigator.cpp \
    controlpoint_navigatoritem.cpp \
    dataitem_display.cpp \
    invokeactiondialog.cpp \
    allowedvaluelist_input.cpp \
    genericinput.cpp \
    i_dataholder.cpp

FORMS += \
    mainwindow.ui \
    controlpoint.ui \
    device_window.ui \
    invokeactiondialog.ui \
    genericinput.ui \
    allowedvaluelist_input.ui
