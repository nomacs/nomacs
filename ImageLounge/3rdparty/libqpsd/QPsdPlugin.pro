#-------------------------------------------------
#
# Project created by QtCreator 2013-04-13T20:22:14
#
#-------------------------------------------------

QT += core gui


CONFIG += plugin

CONFIG(debug, debug|release) {
    TARGET = qpsdd
}
CONFIG(release, debug|release) {
    TARGET = qpsd
}

TEMPLATE = lib

SOURCES += qpsdplugin.cpp \
    qpsdhandler.cpp \
    qpsdhandler_p.cpp

HEADERS += qpsdplugin.h \
    qpsdhandler.h
OTHER_FILES += \
    QPsdPlugin.json \
    LICENSE.LGPL \
    README.md \
    CHANGELOG.md

target.path = $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
