TEMPLATE = app
TARGET   = HUpnpAvSimpleTestApp
QT      += network testlib xml phonon
CONFIG  += warn_on

INCLUDEPATH += \
    ../../hupnp/include/ \
    ../../hupnp_av/include/

LIBS += -L"../../hupnp/bin" -lHUpnp \
        -L"../../hupnp_av/bin" -lHUpnpAv \
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
    QMAKE_POST_LINK += copy ..\\..\\hupnp\\bin\\* bin /Y $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += copy ..\\..\\hupnp_av\\bin\\* bin /Y
}
else {
    LIBS += -lQtSolutions_SOAP-2.7
    !macx:QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN

    QMAKE_POST_LINK += cp -Rf $$PWD/descriptions bin/descriptions &
    QMAKE_POST_LINK += cp -Rf ../../hupnp/bin/* bin &
    QMAKE_POST_LINK += cp -fR ../../hupnp_av/bin/* bin
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
    mediaserver_window.h \
    mediarenderer_window.h \
    mediarenderer_connectionwindow.h \
    controlpoint_navigator.h \
    controlpoint_navigatoritem.h \
    dataitem_display.h \
    rendererconnections.h \
    mediamanager_dialog.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    controlpoint_window.cpp \
    mediaserver_window.cpp \
    mediarenderer_window.cpp \
    mediarenderer_connectionwindow.cpp \
    controlpoint_navigator.cpp \
    controlpoint_navigatoritem.cpp \
    dataitem_display.cpp \
    rendererconnections.cpp \
    mediamanager_dialog.cpp

FORMS += \
    mainwindow.ui \
    controlpoint.ui \
    mediaserver_window.ui \
    mediarenderer_window.ui \
    mediarenderer_connectionwindow.ui \
    mediamanager_dialog.ui
