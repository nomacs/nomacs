TEMPLATE = app
TARGET = bin/nomacs
target.path = /usr/bin
INSTALLS += target

icons.files = icons/*
icons.path  = /usr/share/pixmaps/
INSTALLS += icons

desktop.files = nomacs.desktop
desktop.path = /usr/share/applications/
INSTALLS += desktop

icons2.files = icons/nomacs.png
icons2.path = /usr/share/app-install/icons
INSTALLS += icons2

desktop2.files = nomacs.desktop
desktop2.path = /usr/share/app-install/desktop
INSTALLS += desktop2

DEFINES += WITH_OPENCV
DEFINES += QT_NO_DEBUG_OUTPUT
DEPENDPATH += . \
              nomacs 
INCLUDEPATH += . \
               nomacs 

CONFIG += link_pkgconfig
PKGCONFIG += exiv2 libraw opencv
QT += network gui core

# Input
HEADERS += nomacs/DkError.h \
           nomacs/DkImage.h \
           nomacs/DkMath.h \
	   nomacs/DkConnection.h \
           nomacs/DkNetwork.h \
           nomacs/DkNoMacs.h \
           nomacs/DkTimer.h \
           nomacs/DkUtils.h \
           nomacs/DkViewPort.h \
           nomacs/DkWidgets.h \
	   nomacs/DkMenu.h \
	   nomacs/DkSettings.h \
	   nomacs/BorderLayout.h \
           nomacs/resource.h 
FORMS += nomacs/nomacs.ui
SOURCES += nomacs/DkError.cpp \
           nomacs/DkImage.cpp \
           nomacs/DkMath.cpp \
	   nomacs/DkConnection.cpp \
           nomacs/DkNetwork.cpp \
           nomacs/DkNoMacs.cpp \
           nomacs/DkTimer.cpp \
           nomacs/DkUtils.cpp \
           nomacs/DkViewPort.cpp \
           nomacs/DkWidgets.cpp \
	   nomacs/DkSettings.cpp \
	   nomacs/BorderLayout.cpp \
	   nomacs/DkMenu.cpp \
           nomacs/main.cpp
RESOURCES += nomacs/nomacs.qrc
