#-------------------------------------------------
#
# Project created by QtCreator 2013-04-13T20:22:14
#
#-------------------------------------------------

QT       += core gui


CONFIG += plugin

CONFIG(debug, debug|release) {
    TARGET = qpsdd
}
CONFIG(release, debug|release) {
    TARGET = qpsd
}

TEMPLATE = lib

DESTDIR = $$[QT_INSTALL_PLUGINS]/imageformats

SOURCES += qpsdplugin.cpp \
    qpsdhandler.cpp

HEADERS += qpsdplugin.h \
    qpsdhandler.h
OTHER_FILES += \
    QPsdPlugin.json \
    LICENSE.LGPL \
    README.md

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
