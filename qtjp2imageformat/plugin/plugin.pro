# To build the plugin statically, uncomment the following line
#CONFIG += static

TEMPLATE = lib
CONFIG += qt plugin
win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release build_all
TARGET = qtjp2
VERSION = 2.3.0

TARGET = $$qtLibraryTarget($$TARGET)
DESTDIR = $$[QT_INSTALL_PLUGINS]/imageformats

include(../src/qtjp2imageformat.pri)
SOURCES += qtjp2imageformatplugin.cpp

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
